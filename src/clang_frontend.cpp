#include "misra/clang_frontend.hpp"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Config/llvm-config.h"

namespace misra {
namespace {

// State shared by every translation unit of one analysis run.
struct Shared final {
  explicit Shared(AnalysisContext& analysis_context)
      : context(analysis_context) {}

  AnalysisContext& context;
  // Facts reached through headers are seen once per including translation
  // unit; every fact is de-duplicated by (kind, expansion location).
  std::set<std::string> seen;
  std::size_t translation_unit = 0U;

  [[nodiscard]] bool fresh(std::string key) {
    return seen.insert(std::move(key)).second;
  }
};

std::string location_text(const SourceLocation& location) {
  return location.file + ':' + std::to_string(location.line) + ':' +
         std::to_string(location.column);
}

// Macro policy (docs/macro-policy.md):
//  * the reported location is always the expansion site;
//  * from_macro marks constructs produced by a macro expansion;
//  * locations inside system headers are never reported;
//  * for `structural` facts, code whose spelling is inside a system header
//    (for example a library macro body) belongs to the library and is ignored.
std::optional<SourceLocation> locate(const clang::SourceManager& sm,
                                     const clang::SourceLocation raw,
                                     const bool structural = true) {
  if (raw.isInvalid()) {
    return std::nullopt;
  }
  const clang::SourceLocation expansion = sm.getExpansionLoc(raw);
  if (expansion.isInvalid() || sm.isInSystemHeader(expansion)) {
    return std::nullopt;
  }
  if (structural && sm.isInSystemHeader(sm.getSpellingLoc(raw))) {
    return std::nullopt;
  }
  const clang::PresumedLoc presumed = sm.getPresumedLoc(expansion);
  if (presumed.isInvalid()) {
    return std::nullopt;
  }
  const std::string file = presumed.getFilename();
  if (file.empty() || file.front() == '<') {
    return std::nullopt;  // built-in or command-line definitions
  }
  return SourceLocation{file, presumed.getLine(), presumed.getColumn(),
                        raw.isMacroID()};
}

bool is_essentially_boolean(const clang::Expr* expression,
                            const clang::SourceManager& sm,
                            const clang::LangOptions& lang) {
  const clang::Expr* const stripped = expression->IgnoreParenImpCasts();
  if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(stripped)) {
    return binary->isComparisonOp() || binary->isLogicalOp();
  }
  if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(stripped)) {
    return unary->getOpcode() == clang::UO_LNot;
  }
  if (stripped->getType()->isBooleanType()) {
    return true;
  }
  // <stdbool.h> true/false expand to the integer constants 1 and 0.
  if (stripped->getBeginLoc().isMacroID() &&
      llvm::isa<clang::IntegerLiteral>(stripped)) {
    const llvm::StringRef name = clang::Lexer::getImmediateMacroName(
        stripped->getBeginLoc(), sm, lang);
    return name == "true" || name == "false";
  }
  return false;
}

bool has_variable_length_array(clang::QualType type) {
  for (;;) {
    if (type.isNull()) {
      return false;
    }
    if (type->isVariableArrayType()) {
      return true;
    }
    if (const auto* array = type->getAsArrayTypeUnsafe()) {
      type = array->getElementType();
    } else if (type->isPointerType()) {
      type = type->getPointeeType();
    } else {
      return false;
    }
  }
}

struct LoopScan final {
  std::size_t breaks = 0U;
  std::set<const clang::LabelDecl*> inner_labels;
  std::vector<const clang::LabelDecl*> goto_targets;
};

void scan_loop_body(const clang::Stmt* statement, const bool nested,
                    LoopScan& scan) {
  if (statement == nullptr) {
    return;
  }
  if (llvm::isa<clang::BreakStmt>(statement)) {
    if (!nested) {
      ++scan.breaks;
    }
    return;
  }
  if (const auto* jump = llvm::dyn_cast<clang::GotoStmt>(statement)) {
    scan.goto_targets.push_back(jump->getLabel());
    return;
  }
  if (const auto* label = llvm::dyn_cast<clang::LabelStmt>(statement)) {
    scan.inner_labels.insert(label->getDecl());
  }
  const bool breakable = llvm::isa<clang::ForStmt>(statement) ||
                         llvm::isa<clang::WhileStmt>(statement) ||
                         llvm::isa<clang::DoStmt>(statement) ||
                         llvm::isa<clang::SwitchStmt>(statement);
  for (const clang::Stmt* child : statement->children()) {
    scan_loop_body(child, nested || breakable, scan);
  }
}

std::size_t count_returns(const clang::Stmt* statement) {
  if (statement == nullptr) {
    return 0U;
  }
  std::size_t count = llvm::isa<clang::ReturnStmt>(statement) ? 1U : 0U;
  for (const clang::Stmt* child : statement->children()) {
    count += count_returns(child);
  }
  return count;
}

class FactVisitor final : public clang::RecursiveASTVisitor<FactVisitor> {
 public:
  FactVisitor(clang::ASTContext& ast, Shared& shared)
      : ast_(ast),
        sm_(ast.getSourceManager()),
        shared_(shared),
        context_(shared.context) {}

  // ---------------------------------------------------------------- traversal
  bool TraverseFunctionDecl(clang::FunctionDecl* declaration) {
    const clang::FunctionDecl* const saved = current_function_;
    if (declaration->doesThisDeclarationHaveABody()) {
      current_function_ = declaration;
    }
    const bool result = Base::TraverseFunctionDecl(declaration);
    current_function_ = saved;
    return result;
  }

  // -------------------------------------------------------------- control flow
  bool VisitGotoStmt(clang::GotoStmt* statement) {
    used_labels_.insert(statement->getLabel());
    const clang::SourceLocation goto_spelling = statement->getGotoLoc();
    const clang::SourceLocation label_spelling =
        statement->getLabel()->getLocation();
    const auto location = locate(sm_, goto_spelling, false);
    const auto target = locate(sm_, label_spelling, false);
    if (!location || !target) {
      return true;
    }
    if (shared_.fresh("goto|" + location_text(*location))) {
      context_.control_flow.goto_statements.push_back(
          {*location, *target,
           sm_.isBeforeInTranslationUnit(goto_spelling, label_spelling),
           goto_spelling.isMacroID(), label_spelling.isMacroID()});
    }
    return true;
  }

  bool VisitLabelStmt(clang::LabelStmt* statement) {
    labels_.push_back(statement);
    return true;
  }

  bool VisitAddrLabelExpr(clang::AddrLabelExpr* expression) {
    used_labels_.insert(expression->getLabel());
    return true;
  }

  bool VisitIfStmt(clang::IfStmt* statement) {
    record_body(statement->getThen(), "if", statement->getIfLoc());
    record_controlling(statement->getCond(), "if");
    const clang::Stmt* const else_branch = statement->getElse();
    if (else_branch != nullptr && !llvm::isa<clang::IfStmt>(else_branch)) {
      record_body(else_branch, "else", statement->getElseLoc());
    }
    if (else_branch != nullptr) {
      else_if_members_.insert(else_branch);
    }
    // Chain head: an if that is not itself the else branch of another if.
    if (else_if_members_.count(statement) == 0U &&
        llvm::isa_and_nonnull<clang::IfStmt>(else_branch)) {
      const clang::IfStmt* last = statement;
      while (const auto* next =
                 llvm::dyn_cast_or_null<clang::IfStmt>(last->getElse())) {
        last = next;
      }
      if (const auto loc = locate(sm_, last->getIfLoc())) {
        add(context_.control_flow.if_else_chains, "if-chain",
            IfElseChainFact{*loc, last->getElse() != nullptr});
      }
    }
    return true;
  }

  bool VisitForStmt(clang::ForStmt* statement) {
    record_body(statement->getBody(), "for", statement->getForLoc());
    record_controlling(statement->getCond(), "for");
    record_loop(statement->getBody(), statement->getForLoc());
    return true;
  }

  bool VisitWhileStmt(clang::WhileStmt* statement) {
    record_body(statement->getBody(), "while", statement->getWhileLoc());
    record_controlling(statement->getCond(), "while");
    record_loop(statement->getBody(), statement->getWhileLoc());
    return true;
  }

  bool VisitDoStmt(clang::DoStmt* statement) {
    record_body(statement->getBody(), "do", statement->getDoLoc());
    record_controlling(statement->getCond(), "do");
    record_loop(statement->getBody(), statement->getDoLoc());
    return true;
  }

  bool VisitSwitchStmt(clang::SwitchStmt* statement) {
    record_body(statement->getBody(), "switch", statement->getSwitchLoc());
    bool has_default = false;
    for (const clang::SwitchCase* c = statement->getSwitchCaseList();
         c != nullptr; c = c->getNextSwitchCase()) {
      if (llvm::isa<clang::DefaultStmt>(c)) {
        has_default = true;
      }
    }
    if (const auto loc = locate(sm_, statement->getSwitchLoc())) {
      add(context_.control_flow.switch_statements, "switch",
          SwitchStatementFact{*loc, has_default});
    }
    return true;
  }

  // -------------------------------------------------------------- expressions
  bool VisitCallExpr(clang::CallExpr* call) {
    const clang::FunctionDecl* const callee = call->getDirectCallee();
    if (callee == nullptr || !callee->getIdentifier()) {
      return true;
    }
    const clang::SourceLocation spelling = call->getBeginLoc();
    // Calls are keyed on the call site: a user macro wrapping a banned
    // library function is reported, library-internal macros are not.
    if (const auto loc = locate(sm_, spelling, false)) {
      add(context_.calls, "call:" + callee->getName().str(),
          CallFact{*loc, callee->getName().str(),
                   sm_.isInSystemHeader(callee->getLocation()),
                   spelling.isMacroID()});
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator* expression) {
    if (expression->getOpcode() == clang::BO_Comma) {
      syntax(expression->getOperatorLoc(), "misra-c2012-12.3-comma");
    }
    if (expression->isAssignmentOp() && value_is_used(expression)) {
      syntax(expression->getOperatorLoc(),
             "misra-c2012-13.4-assignment-value-used");
    }
    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral* literal) {
    const std::string text = literal_text(literal->getBeginLoc());
    if (text.size() > 1U && text[0] == '0' && text[1] >= '0' &&
        text[1] <= '9') {
      syntax(literal->getBeginLoc(), "misra-c2012-7.1-octal-constant");
    }
    if (text.find('l') != std::string::npos) {
      syntax(literal->getBeginLoc(), "misra-c2012-7.3-lowercase-l-suffix");
    }
    return true;
  }

  bool VisitFloatingLiteral(clang::FloatingLiteral* literal) {
    if (literal_text(literal->getBeginLoc()).find('l') != std::string::npos) {
      syntax(literal->getBeginLoc(), "misra-c2012-7.3-lowercase-l-suffix");
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* reference) {
    const clang::ValueDecl* const declaration = reference->getDecl();
    referenced_.insert(declaration);
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(declaration)) {
      if (variable->isFileVarDecl()) {
        static_users_[variable->getCanonicalDecl()].insert(current_function_);
      }
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration)) {
      note_external_reference(function);
    } else if (const auto* variable =
                   llvm::dyn_cast<clang::VarDecl>(declaration)) {
      if (variable->isFileVarDecl()) {
        note_external_reference(variable);
      }
    }
    return true;
  }

  bool VisitTypedefTypeLoc(clang::TypedefTypeLoc type_loc) {
    note_declaration_use(type_loc.getTypedefNameDecl());
    return true;
  }

  bool VisitTagTypeLoc(clang::TagTypeLoc type_loc) {
    const clang::TagDecl* const tag = type_loc.getDecl();
    if (type_loc.getNameLoc() == tag->getLocation() &&
        declaring_typedef_tags_.count(tag->getLocation()) != 0U) {
      return true;  // the tag is only the body of a typedef declaration
    }
    const clang::TagDecl* const definition = tag->getDefinition();
    note_declaration_use(definition != nullptr ? definition : tag);
    return true;
  }

  // ------------------------------------------------------------- declarations
  bool VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function->isImplicit()) {
      return true;
    }
    const auto location = locate(sm_, function->getLocation(), false);
    if (!location) {
      return true;
    }
    const bool definition = function->doesThisDeclarationHaveABody();
    const bool external = function->isExternallyVisible();

    std::vector<std::string> parameter_names;
    for (const clang::ParmVarDecl* parameter : function->parameters()) {
      parameter_names.push_back(parameter->getNameAsString());
    }
    declare(function, DeclKind::Function,
            external ? LinkageKind::External : LinkageKind::Internal,
            definition, std::move(parameter_names));

    // 8.2: function types in prototype form with named parameters.
    bool prototype_problem = !function->hasPrototype();
    for (const clang::ParmVarDecl* parameter : function->parameters()) {
      if (parameter->getName().empty()) {
        prototype_problem = true;
      }
    }
    if (prototype_problem) {
      syntax(function->getLocation(), "misra-c2012-8.2-prototype-form");
    }
    // 8.4: compatible declaration visible before an external definition.
    if (definition && external && function->getPreviousDecl() == nullptr &&
        function->getName() != "main") {
      syntax(function->getLocation(), "misra-c2012-8.4-no-visible-declaration");
    }
    // 8.8: static on every declaration of an internal-linkage entity.
    if (function->getStorageClass() != clang::SC_Static &&
        function->getFirstDecl() != function &&
        function->getFirstDecl()->getStorageClass() == clang::SC_Static) {
      syntax(function->getLocation(), "misra-c2012-8.8-missing-static");
    }
    // 8.10: inline functions are declared static.
    if (function->isInlineSpecified() &&
        function->getStorageClass() != clang::SC_Static) {
      syntax(function->getLocation(), "misra-c2012-8.10-inline-not-static");
    }
    // 8.14: restrict.
    if (function->getReturnType().isRestrictQualified()) {
      syntax(function->getLocation(), "misra-c2012-8.14-restrict");
    }
    if (definition) {
      defined_functions_.push_back(function);
      const auto* body = llvm::dyn_cast_or_null<clang::CompoundStmt>(function->getBody());
      if (body != nullptr) {
        const std::size_t returns = count_returns(body);
        const bool ends = !body->body_empty() &&
                          llvm::isa<clang::ReturnStmt>(body->body_back());
        add(context_.function_exits, "exit",
            FunctionExitFact{*location, returns, ends});
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* variable) {
    if (variable->isImplicit()) {
      return true;
    }
    const clang::QualType declared =
        llvm::isa<clang::ParmVarDecl>(variable)
            ? llvm::cast<clang::ParmVarDecl>(variable)->getOriginalType()
            : variable->getType();
    if (has_variable_length_array(declared)) {
      syntax(variable->getLocation(), "misra-c2012-18.8-variable-length-array");
    }
    if (variable->getType().isRestrictQualified()) {
      syntax(variable->getLocation(), "misra-c2012-8.14-restrict");
    }
    if (!variable->isFileVarDecl()) {
      return true;
    }
    const bool definition =
        variable->isThisDeclarationADefinition(ast_) !=
        clang::VarDecl::DeclarationOnly;
    const bool external = variable->isExternallyVisible();
    declare(variable, DeclKind::Object,
            external ? LinkageKind::External : LinkageKind::Internal,
            definition, {});

    if (definition && external && variable->getPreviousDecl() == nullptr) {
      syntax(variable->getLocation(), "misra-c2012-8.4-no-visible-declaration");
    }
    if (variable->getStorageClass() != clang::SC_Static &&
        variable->getFirstDecl() != variable &&
        variable->getFirstDecl()->getStorageClass() == clang::SC_Static) {
      syntax(variable->getLocation(), "misra-c2012-8.8-missing-static");
    }
    if (!definition && external && variable->getType()->isIncompleteArrayType()) {
      syntax(variable->getLocation(), "misra-c2012-8.11-extern-array-no-size");
    }
    if (definition && !external &&
        variable->getStorageClass() == clang::SC_Static &&
        locate(sm_, variable->getLocation())) {
      static_objects_.insert(variable->getCanonicalDecl());
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* field) {
    if (has_variable_length_array(field->getType())) {
      syntax(field->getLocation(), "misra-c2012-18.8-variable-length-array");
    }
    if (field->getType().isRestrictQualified()) {
      syntax(field->getLocation(), "misra-c2012-8.14-restrict");
    }
    return true;
  }

  bool VisitTypedefDecl(clang::TypedefDecl* declaration) {
    if (declaration->isImplicit()) {
      return true;
    }
    if (has_variable_length_array(declaration->getUnderlyingType())) {
      syntax(declaration->getLocation(), "misra-c2012-18.8-variable-length-array");
    }
    if (declaration->getUnderlyingType().isRestrictQualified()) {
      syntax(declaration->getLocation(), "misra-c2012-8.14-restrict");
    }
    if (const clang::TypeSourceInfo* const info =
            declaration->getTypeSourceInfo()) {
      clang::TypeLoc type_loc = info->getTypeLoc();
      if (const auto elaborated = type_loc.getAs<clang::ElaboratedTypeLoc>()) {
        type_loc = elaborated.getNamedTypeLoc();
      }
      if (const auto tag_loc = type_loc.getAs<clang::TagTypeLoc>()) {
        declaring_typedef_tags_.insert(tag_loc.getDecl()->getLocation());
      }
    }
    declare(declaration, DeclKind::Typedef, LinkageKind::None, true, {});
    return true;
  }

  bool VisitTagDecl(clang::TagDecl* tag) {
    if (tag->isImplicit() || !tag->isCompleteDefinition() ||
        tag->getIdentifier() == nullptr) {
      return true;
    }
    declare(tag, DeclKind::Tag, LinkageKind::None, true, {});
    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* enumeration) {
    if (!enumeration->isCompleteDefinition()) {
      return true;
    }
    std::map<std::string, int> counts;
    const auto enumerator_value = [](const clang::EnumConstantDecl* e) {
      llvm::SmallString<32> text;
      e->getInitVal().toString(text, 10);
      return text.str().str();
    };
    for (const clang::EnumConstantDecl* e : enumeration->enumerators()) {
      ++counts[enumerator_value(e)];
    }
    for (const clang::EnumConstantDecl* e : enumeration->enumerators()) {
      if (e->getInitExpr() == nullptr && counts[enumerator_value(e)] > 1) {
        syntax(e->getLocation(), "misra-c2012-8.12-implicit-enumerator-duplicate");
      }
    }
    return true;
  }

  // Called once the whole translation unit has been traversed.
  void finish() {
    for (const clang::FunctionDecl* function : defined_functions_) {
      for (const clang::ParmVarDecl* parameter : function->parameters()) {
        if (!parameter->getName().empty() && referenced_.count(parameter) == 0U) {
          syntax(parameter->getLocation(), "misra-c2012-2.7-unused-parameter");
        }
      }
    }
    for (const clang::LabelStmt* label : labels_) {
      if (used_labels_.count(label->getDecl()) == 0U) {
        syntax(label->getIdentLoc(), "misra-c2012-2.6-unused-label");
      }
    }
    for (const clang::VarDecl* variable : static_objects_) {
      const auto users = static_users_.find(variable);
      if (users != static_users_.end() && users->second.size() == 1U &&
          *users->second.begin() != nullptr) {
        syntax(variable->getLocation(), "misra-c2012-8.9-single-function-use");
      }
    }
  }

 private:
  using Base = clang::RecursiveASTVisitor<FactVisitor>;

  // Adds a fact once per (tag, expansion location) across all translation
  // units.
  template <typename Vector, typename Fact>
  void add(Vector& vector, const std::string& tag, Fact fact) {
    if (shared_.fresh(tag + '|' + location_text(fact.location))) {
      vector.push_back(std::move(fact));
    }
  }

  void syntax(const clang::SourceLocation raw, const std::string_view kind) {
    if (const auto location = locate(sm_, raw)) {
      add(context_.syntax, std::string(kind),
          SyntaxFact{*location, kind});
    }
  }

  void declare(const clang::NamedDecl* declaration, const DeclKind kind,
               const LinkageKind linkage, const bool definition,
               std::vector<std::string> parameter_names) {
    const auto location = locate(sm_, declaration->getLocation(), false);
    if (!location || declaration->getName().empty()) {
      return;
    }
    std::string type;
    if (const auto* value = llvm::dyn_cast<clang::ValueDecl>(declaration)) {
      type = value->getType().getCanonicalType().getAsString();
    }
    const std::string name = declaration->getNameAsString();
    if (shared_.fresh("decl|" + std::to_string(shared_.translation_unit) + '|' +
                      declaration_key(*location, name))) {
      context_.program.declarations.push_back(
          {*location, name, kind, linkage, definition, std::move(type),
           std::move(parameter_names), shared_.translation_unit});
    }
  }

  void note_external_reference(const clang::NamedDecl* declaration) {
    if (!declaration->isExternallyVisible() ||
        sm_.isInSystemHeader(declaration->getLocation())) {
      return;
    }
    const std::string name = declaration->getNameAsString();
    if (shared_.fresh("ref|" + std::to_string(shared_.translation_unit) + '|' +
                      name)) {
      context_.program.references.push_back({name, shared_.translation_unit});
    }
  }

  void note_declaration_use(const clang::NamedDecl* declaration) {
    if (const auto location = locate(sm_, declaration->getLocation(), false)) {
      context_.program.referenced_declarations.insert(
          declaration_key(*location, declaration->getNameAsString()));
    }
  }

  void record_body(const clang::Stmt* body, const std::string_view construct,
                   const clang::SourceLocation construct_loc) {
    if (body == nullptr) {
      return;
    }
    if (const auto loc = locate(sm_, construct_loc)) {
      add(context_.control_flow.statement_bodies,
          std::string("body:") + std::string(construct),
          StatementBodyFact{*loc, construct,
                            llvm::isa<clang::CompoundStmt>(body)});
    }
  }

  void record_controlling(const clang::Expr* condition,
                          const std::string_view construct) {
    if (condition == nullptr) {
      return;
    }
    if (const auto loc = locate(sm_, condition->getExprLoc())) {
      add(context_.controlling_expressions, "cond",
          ControllingExpressionFact{
              *loc, construct,
              is_essentially_boolean(condition, sm_, ast_.getLangOpts())});
    }
  }

  void record_loop(const clang::Stmt* body, const clang::SourceLocation loop_loc) {
    LoopScan scan;
    scan_loop_body(body, false, scan);
    std::size_t jumps = scan.breaks;
    for (const clang::LabelDecl* target : scan.goto_targets) {
      if (scan.inner_labels.count(target) == 0U) {
        ++jumps;
      }
    }
    if (const auto loc = locate(sm_, loop_loc)) {
      add(context_.loop_exits, "loop", LoopExitFact{*loc, jumps});
    }
  }

  [[nodiscard]] std::string literal_text(const clang::SourceLocation raw) const {
    const clang::SourceLocation spelling = sm_.getSpellingLoc(raw);
    return clang::Lexer::getSourceText(
               clang::CharSourceRange::getTokenRange(spelling, spelling), sm_,
               ast_.getLangOpts())
        .str();
  }

  // True when the value produced by an assignment is consumed by its context.
  [[nodiscard]] bool value_is_used(const clang::Stmt* node) {
    const clang::Stmt* current = node;
    for (;;) {
      const auto parents = ast_.getParents(*current);
      if (parents.empty()) {
        return false;
      }
      const clang::Stmt* const parent = parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return true;  // for example an initializer of a declaration
      }
      if (llvm::isa<clang::ParenExpr>(parent)) {
        current = parent;
        continue;
      }
      if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(parent)) {
        return !(binary->getOpcode() == clang::BO_Comma &&
                 binary->getLHS() == current);
      }
      if (const auto* cast = llvm::dyn_cast<clang::CastExpr>(parent)) {
        return !cast->getType()->isVoidType();
      }
      if (llvm::isa<clang::Expr>(parent)) {
        return true;
      }
      if (const auto* s = llvm::dyn_cast<clang::IfStmt>(parent)) {
        return s->getCond() == current;
      }
      if (const auto* s = llvm::dyn_cast<clang::WhileStmt>(parent)) {
        return s->getCond() == current;
      }
      if (const auto* s = llvm::dyn_cast<clang::DoStmt>(parent)) {
        return s->getCond() == current;
      }
      if (const auto* s = llvm::dyn_cast<clang::ForStmt>(parent)) {
        return s->getCond() == current;
      }
      if (const auto* s = llvm::dyn_cast<clang::SwitchStmt>(parent)) {
        return s->getCond() == current;
      }
      return llvm::isa<clang::ReturnStmt>(parent);
    }
  }

  clang::ASTContext& ast_;
  clang::SourceManager& sm_;
  Shared& shared_;
  AnalysisContext& context_;

  const clang::FunctionDecl* current_function_ = nullptr;
  std::set<const clang::Stmt*> else_if_members_;
  std::set<const clang::Decl*> referenced_;
  std::set<const clang::LabelDecl*> used_labels_;
  std::vector<const clang::LabelStmt*> labels_;
  std::vector<const clang::FunctionDecl*> defined_functions_;
  std::set<const clang::VarDecl*> static_objects_;
  std::map<const clang::VarDecl*, std::set<const clang::FunctionDecl*>>
      static_users_;
  std::set<clang::SourceLocation> declaring_typedef_tags_;
};

class PreprocessorRecorder final : public clang::PPCallbacks {
 public:
  PreprocessorRecorder(clang::SourceManager& sm, Shared& shared)
      : sm_(sm), shared_(shared) {}

#if LLVM_VERSION_MAJOR >= 19
  void InclusionDirective(clang::SourceLocation hash_loc, const clang::Token&,
                          llvm::StringRef file_name, bool is_angled,
                          clang::CharSourceRange, clang::OptionalFileEntryRef,
                          llvm::StringRef, llvm::StringRef,
                          const clang::Module*, bool,
                          clang::SrcMgr::CharacteristicKind) override {
#else
  void InclusionDirective(clang::SourceLocation hash_loc, const clang::Token&,
                          llvm::StringRef file_name, bool is_angled,
                          clang::CharSourceRange, clang::OptionalFileEntryRef,
                          llvm::StringRef, llvm::StringRef,
                          const clang::Module*,
                          clang::SrcMgr::CharacteristicKind) override {
#endif
    if (const auto location = locate(sm_, hash_loc)) {
      if (shared_.fresh("include|" + file_name.str() + '|' +
                        location_text(*location))) {
        shared_.context.includes.push_back(
            {*location, file_name.str(), is_angled});
      }
    }
  }

  void MacroDefined(const clang::Token& name_token,
                    const clang::MacroDirective* directive) override {
    const clang::MacroInfo* const info = directive->getMacroInfo();
    if (info->isBuiltinMacro()) {
      return;
    }
    const auto location = locate(sm_, info->getDefinitionLoc(), false);
    if (!location) {
      return;
    }
    const std::string name = name_token.getIdentifierInfo()->getName().str();
    if (shared_.fresh("macro|" + std::to_string(shared_.translation_unit) + '|' +
                      declaration_key(*location, name))) {
      shared_.context.program.macros.push_back(
          {*location, name, info->isFunctionLike(), shared_.translation_unit});
    }
  }

  void MacroExpands(const clang::Token& name_token, const clang::MacroDefinition&,
                    clang::SourceRange, const clang::MacroArgs*) override {
    note_use(name_token);
  }

  void Defined(const clang::Token& name_token, const clang::MacroDefinition&,
               clang::SourceRange) override {
    note_use(name_token);
  }

  void Ifdef(clang::SourceLocation, const clang::Token& name_token,
             const clang::MacroDefinition&) override {
    note_use(name_token);
  }

  void Ifndef(clang::SourceLocation, const clang::Token& name_token,
              const clang::MacroDefinition&) override {
    note_use(name_token);
  }

 private:
  void note_use(const clang::Token& name_token) {
    if (const clang::IdentifierInfo* const info = name_token.getIdentifierInfo()) {
      shared_.context.program.used_macros.insert(info->getName().str());
    }
  }

  clang::SourceManager& sm_;
  Shared& shared_;
};

class FactConsumer final : public clang::ASTConsumer {
 public:
  explicit FactConsumer(Shared& shared) : shared_(shared) {}

  void HandleTranslationUnit(clang::ASTContext& context) override {
    FactVisitor visitor(context, shared_);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    visitor.finish();
  }

 private:
  Shared& shared_;
};

class FactAction final : public clang::ASTFrontendAction {
 public:
  explicit FactAction(Shared& shared) : shared_(shared) {}

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler, llvm::StringRef) override {
    compiler.getPreprocessor().addPPCallbacks(
        std::make_unique<PreprocessorRecorder>(compiler.getSourceManager(),
                                               shared_));
    return std::make_unique<FactConsumer>(shared_);
  }

 private:
  Shared& shared_;
};

class FactActionFactory final : public clang::tooling::FrontendActionFactory {
 public:
  explicit FactActionFactory(Shared& shared) : shared_(shared) {}

  std::unique_ptr<clang::FrontendAction> create() override {
    ++shared_.translation_unit;
    ++shared_.context.translation_units;
    return std::make_unique<FactAction>(shared_);
  }

 private:
  Shared& shared_;
};

}  // namespace

FrontendResult ClangFrontend::analyze(
    const std::string& compilation_database,
    const std::vector<std::string>& requested_files) const {
  std::string load_error;
  std::unique_ptr<clang::tooling::JSONCompilationDatabase> database =
      clang::tooling::JSONCompilationDatabase::loadFromFile(
          compilation_database, load_error,
          clang::tooling::JSONCommandLineSyntax::AutoDetect);
  if (!database) {
    return {false, {}, {}, "cannot load compilation database: " + load_error};
  }

  std::vector<std::string> files = requested_files;
  if (files.empty()) {
    files = database->getAllFiles();
  }
  if (files.empty()) {
    return {false, {}, {}, "compilation database contains no source files"};
  }

  FrontendResult result{true, {}, files, {}};
  Shared shared(result.context);
  clang::tooling::ClangTool tool(*database, files);
  FactActionFactory factory(shared);
  if (tool.run(&factory) != 0) {
    result.success = false;
    result.error_message = "Clang could not analyze one or more translation units";
  }
  return result;
}

}  // namespace misra
