#include "misra/clang_frontend.hpp"

#include "essential_type.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "clang/Analysis/Analyses/ReachableCode.h"
#include "clang/Analysis/CFG.h"
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
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/BitVector.h"
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
  std::size_t next_scope = 1U;

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
        context_(shared.context) {
    push_scope(ast.getTranslationUnitDecl());  // file scope
  }

  // ---------------------------------------------------------------- traversal
  // Scopes (needed by Rules 5.2 and 5.3): the function name belongs to the
  // enclosing scope; parameters and the outermost block share one scope for a
  // definition, and a declaration has its own prototype scope.
  bool TraverseFunctionDecl(clang::FunctionDecl* declaration) {
    const clang::FunctionDecl* const saved = current_function_;
    const bool definition = declaration->doesThisDeclarationHaveABody();
    if (definition) {
      current_function_ = declaration;
    }
    if (!declaration->getDeclContext()->isFunctionOrMethod()) {
      record_identifier(declaration, IdentifierSpace::Ordinary);
    }
    push_scope(definition ? static_cast<const void*>(declaration->getBody())
                          : static_cast<const void*>(declaration));
    const bool result = Base::TraverseFunctionDecl(declaration);
    pop_scope();
    current_function_ = saved;
    return result;
  }

  bool TraverseCompoundStmt(clang::CompoundStmt* statement) {
    const bool own_scope = scopes_.back().key != statement;
    if (own_scope) {
      push_scope(statement);
    }
    const bool result = Base::TraverseCompoundStmt(statement);
    if (own_scope) {
      pop_scope();
    }
    return result;
  }

  bool TraverseForStmt(clang::ForStmt* statement) {
    push_scope(statement);
    const bool result = Base::TraverseForStmt(statement);
    pop_scope();
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

  // 16.2: a case/default label belongs directly to the compound statement that
  // is the body of its switch. Chained labels (case 1: case 2:) are one unit.
  bool VisitSwitchCase(clang::SwitchCase* label) {
    const clang::Stmt* current = label;
    for (;;) {
      const auto parents = ast_.getParents(*current);
      if (parents.empty()) {
        return true;
      }
      const clang::Stmt* const parent = parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return true;
      }
      if (llvm::isa<clang::SwitchCase>(parent)) {
        current = parent;
        continue;
      }
      if (llvm::isa<clang::SwitchStmt>(parent)) {
        return true;  // unbraced body: a 15.6 matter, not this rule
      }
      bool in_switch_body = false;
      if (llvm::isa<clang::CompoundStmt>(parent)) {
        const auto outer = ast_.getParents(*parent);
        if (!outer.empty()) {
          if (const auto* owner = outer[0].get<clang::SwitchStmt>()) {
            in_switch_body = owner->getBody() == parent;
          }
        }
      }
      if (!in_switch_body) {
        syntax(label->getKeywordLoc(), "misra-c2012-16.2-label-not-in-switch-body");
      }
      return true;
    }
  }

  bool VisitSwitchStmt(clang::SwitchStmt* statement) {
    check_switch_shape(statement);
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
    // 17.7: the value returned by a non-void function must be used. This
    // covers calls through function pointers; an explicit cast to void is a
    // deliberate discard and is accepted.
    if (!call->getType()->isVoidType() && !value_is_used(call, true)) {
      syntax(call->getBeginLoc(), "misra-c2012-17.7-return-value-ignored");
    }
    const clang::FunctionDecl* const callee = call->getDirectCallee();
    if (callee == nullptr && current_function_ != nullptr) {
      note_indirect_call(call);
    }
    if (callee == nullptr || !callee->getIdentifier()) {
      return true;
    }
    if (sm_.isInSystemHeader(callee->getLocation())) {
      // A library function that receives a function address may call it back.
      for (const clang::Expr* argument : call->arguments()) {
        if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(
                argument->IgnoreParenImpCasts())) {
          if (const auto* target =
                  llvm::dyn_cast<clang::FunctionDecl>(reference->getDecl())) {
            note_call_edge(call, target, true);
          }
        }
      }
    }
    // 17.3: a function used without a visible declaration. Compiler builtins
    // (including those behind <stdarg.h>) are declared implicitly by design.
    if (callee->isImplicit() && !callee->getName().starts_with("__builtin_")) {
      syntax(call->getBeginLoc(), "misra-c2012-17.3-implicit-declaration");
    }
    note_call_edge(call, callee);
    if (callee->hasPrototype()) {
      const unsigned count = std::min(call->getNumArgs(), callee->getNumParams());
      for (unsigned index = 0U; index < count; ++index) {
        check_assignment(callee->getParamDecl(index)->getType(),
                         call->getArg(index), call->getArg(index)->getExprLoc());
        // 17.5: an array argument must be at least as large as the declared
        // bound of the array parameter. Only arguments that are themselves
        // arrays of known size can be judged.
        const auto* declared = llvm::dyn_cast_or_null<clang::ConstantArrayType>(
            callee->getParamDecl(index)->getOriginalType()->getAsArrayTypeUnsafe());
        const auto* supplied = llvm::dyn_cast_or_null<clang::ConstantArrayType>(
            call->getArg(index)->IgnoreParenImpCasts()->getType()->getAsArrayTypeUnsafe());
        if (declared != nullptr && supplied != nullptr &&
            supplied->getSize().ult(declared->getSize())) {
          syntax(call->getArg(index)->getExprLoc(),
                 "misra-c2012-17.5-array-argument-too-small");
        }
      }
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

  bool VisitVAArgExpr(clang::VAArgExpr* expression) {
    // A use of a banned library facility: keyed on the invocation even though
    // the va_arg macro itself is spelled in a system header.
    syntax(expression->getBuiltinLoc(), "misra-c2012-17.1-stdarg", false);
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator* expression) {
    if (expression->isAssignmentOp()) {
      note_parameter_modification(expression->getLHS(),
                                  expression->getOperatorLoc());
    }
    if (expression->getOpcode() == clang::BO_Comma) {
      syntax(expression->getOperatorLoc(), "misra-c2012-12.3-comma");
    }
    if (expression->isAssignmentOp() && value_is_used(expression)) {
      syntax(expression->getOperatorLoc(),
             "misra-c2012-13.4-assignment-value-used");
    }
    check_binary_types(expression);
    return true;
  }

  // ------------------------------------------------- Section 10 (type rules)
  bool VisitUnaryOperator(clang::UnaryOperator* expression) {
    using essential::Operand;
    if (expression->isIncrementDecrementOp()) {
      note_parameter_modification(expression->getSubExpr(),
                                  expression->getOperatorLoc());
    }
    std::optional<Operand> position;
    switch (expression->getOpcode()) {
      case clang::UO_Plus: position = Operand::UnaryPlus; break;
      case clang::UO_Minus: position = Operand::UnaryMinus; break;
      case clang::UO_Not: position = Operand::LeftShift; break;
      case clang::UO_LNot: position = Operand::Logical; break;
      case clang::UO_PreInc:
      case clang::UO_PostInc:
      case clang::UO_PreDec:
      case clang::UO_PostDec: position = Operand::AddSub; break;
      default: break;
    }
    if (position &&
        !essential::appropriate(
            *position, essential::of_expression(expression->getSubExpr(), ast_))) {
      syntax(expression->getOperatorLoc(),
             "misra-c2012-10.1-inappropriate-operand");
    }
    return true;
  }

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expression) {
    if (!essential::appropriate(
            essential::Operand::Subscript,
            essential::of_expression(expression->getIdx(), ast_))) {
      syntax(expression->getExprLoc(), "misra-c2012-10.1-inappropriate-operand");
    }
    return true;
  }

  bool VisitConditionalOperator(clang::ConditionalOperator* expression) {
    if (!essential::appropriate(
            essential::Operand::Logical,
            essential::of_expression(expression->getCond(), ast_))) {
      syntax(expression->getQuestionLoc(),
             "misra-c2012-10.1-inappropriate-operand");
    }
    if (essential::categories_differ(
            essential::of_expression(expression->getTrueExpr(), ast_),
            essential::of_expression(expression->getFalseExpr(), ast_))) {
      syntax(expression->getQuestionLoc(),
             "misra-c2012-10.4-operand-category-mismatch");
    }
    return true;
  }

  // ---------------------------------------- Section 11 (pointer conversions)
  bool VisitCastExpr(clang::CastExpr* cast) {
    check_pointer_conversion(cast);
    return true;
  }

  bool VisitCStyleCastExpr(clang::CStyleCastExpr* cast) {
    const clang::Expr* const operand = cast->getSubExpr();
    if (essential::is_composite(operand, ast_)) {
      const essential::Type from = essential::of_expression(operand, ast_);
      const essential::Type to = essential::of_type(cast->getType(), ast_);
      if (from.category != essential::Category::Other &&
          to.category != essential::Category::Other &&
          (from.category != to.category || to.width > from.width)) {
        syntax(cast->getLParenLoc(), "misra-c2012-10.8-composite-cast");
      }
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt* statement) {
    if (statement->getRetValue() == nullptr && current_function_ != nullptr &&
        !current_function_->getReturnType()->isVoidType()) {
      syntax(statement->getReturnLoc(), "misra-c2012-17.4-missing-return");
    }
    if (statement->getRetValue() != nullptr && current_function_ != nullptr) {
      check_assignment(current_function_->getReturnType(),
                       statement->getRetValue(), statement->getReturnLoc());
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
      if (!is_direct_callee(reference)) {
        note_address_taken(function, reference->getBeginLoc());
      }
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
      if (!function->getReturnType()->isVoidType() && falls_off_end(function)) {
        syntax(function->getLocation(), "misra-c2012-17.4-missing-return");
      }
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
    if (variable->isFileVarDecl() || !variable->hasExternalStorage()) {
      record_identifier(variable, IdentifierSpace::Ordinary);
    }
    const clang::QualType declared =
        llvm::isa<clang::ParmVarDecl>(variable)
            ? llvm::cast<clang::ParmVarDecl>(variable)->getOriginalType()
            : variable->getType();
    if (has_variable_length_array(declared)) {
      syntax(variable->getLocation(), "misra-c2012-18.8-variable-length-array");
    }
    if (llvm::isa<clang::ParmVarDecl>(variable)) {
      if (const clang::ArrayType* array = declared->getAsArrayTypeUnsafe();
          array != nullptr &&
          array->getSizeModifier() == clang::ArraySizeModifier::Static) {
        syntax(variable->getLocation(), "misra-c2012-17.6-static-array-parameter");
      }
    }
    if (variable->getType().isRestrictQualified()) {
      syntax(variable->getLocation(), "misra-c2012-8.14-restrict");
    }
    if (const clang::Expr* const init = variable->getInit();
        init != nullptr && !llvm::isa<clang::InitListExpr>(init) &&
        !llvm::isa<clang::ParmVarDecl>(variable)) {
      check_assignment(variable->getType(), init, variable->getLocation());
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

  bool VisitEnumConstantDecl(clang::EnumConstantDecl* enumerator) {
    record_identifier(enumerator, IdentifierSpace::Ordinary);
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* field) {
    record_identifier(field, IdentifierSpace::Member);
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
    record_identifier(declaration, IdentifierSpace::Ordinary);
    return true;
  }

  bool VisitTagDecl(clang::TagDecl* tag) {
    if (!tag->isImplicit()) {
      record_identifier(tag, IdentifierSpace::Tag);
    }
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

  static bool uses_usual_arithmetic_conversions(
      const clang::BinaryOperatorKind op) {
    switch (op) {
      case clang::BO_Mul: case clang::BO_Div: case clang::BO_Rem:
      case clang::BO_Add: case clang::BO_Sub:
      case clang::BO_LT: case clang::BO_GT: case clang::BO_LE: case clang::BO_GE:
      case clang::BO_EQ: case clang::BO_NE:
      case clang::BO_And: case clang::BO_Xor: case clang::BO_Or:
      case clang::BO_MulAssign: case clang::BO_DivAssign:
      case clang::BO_RemAssign: case clang::BO_AddAssign:
      case clang::BO_SubAssign: case clang::BO_AndAssign:
      case clang::BO_XorAssign: case clang::BO_OrAssign:
        return true;
      default:
        return false;
    }
  }

  // True when control can reach the closing brace of a non-void function. The
  // test mirrors the compiler's own missing-return analysis: some reachable
  // predecessor of the exit block must end in something other than a return
  // or a call that does not return.
  [[nodiscard]] bool falls_off_end(const clang::FunctionDecl* function) {
    if (function->getBody() == nullptr) {
      return false;
    }
    std::unique_ptr<clang::CFG> cfg = clang::CFG::buildCFG(
        function, function->getBody(), &ast_, clang::CFG::BuildOptions());
    if (!cfg) {
      return false;
    }
    llvm::BitVector reachable(cfg->getNumBlockIDs(), false);
    clang::reachable_code::ScanReachableFromBlock(&cfg->getEntry(), reachable);
    for (const clang::CFGBlock* block : cfg->getExit().preds()) {
      if (block == nullptr || !reachable[block->getBlockID()] ||
          block->hasNoReturnElement()) {
        continue;
      }
      if (!block->empty()) {
        if (const auto last = block->back().getAs<clang::CFGStmt>();
            last && llvm::isa<clang::ReturnStmt>(last->getStmt())) {
          continue;
        }
      }
      return true;
    }
    return false;
  }

  // 17.8: assignment, increment or decrement of a parameter itself (not of
  // the object it points to).
  void note_parameter_modification(const clang::Expr* target,
                                   const clang::SourceLocation loc) {
    if (const auto* reference =
            llvm::dyn_cast<clang::DeclRefExpr>(target->IgnoreParenImpCasts())) {
      if (llvm::isa<clang::ParmVarDecl>(reference->getDecl())) {
        syntax(loc, "misra-c2012-17.8-parameter-modified");
      }
    }
  }

  [[nodiscard]] std::string function_key(const clang::FunctionDecl* function) {
    const std::string name = function->getNameAsString();
    if (function->isExternallyVisible()) {
      return name;
    }
    const auto location = locate(sm_, function->getFirstDecl()->getLocation(), false);
    return location ? location->file + "::" + name : name;
  }

  void note_call_edge(const clang::CallExpr* call,
                      const clang::FunctionDecl* callee,
                      const bool via_pointer = false) {
    if (current_function_ == nullptr ||
        sm_.isInSystemHeader(callee->getLocation())) {
      return;
    }
    if (const auto location = locate(sm_, call->getBeginLoc(), false)) {
      const std::string caller = function_key(current_function_);
      const std::string target = function_key(callee);
      if (shared_.fresh("edge|" + caller + "|" + target + "|" +
                        location_text(*location))) {
        context_.program.call_edges.push_back(
            {*location, caller, target, via_pointer});
      }
    }
  }

  // True when the reference is, possibly through parentheses and implicit
  // function-to-pointer conversion, the callee of a call expression.
  [[nodiscard]] bool is_direct_callee(const clang::DeclRefExpr* reference) {
    const clang::Stmt* current = reference;
    for (;;) {
      const auto parents = ast_.getParents(*current);
      if (parents.empty()) {
        return false;
      }
      const clang::Stmt* const parent = parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return false;
      }
      if (llvm::isa<clang::ParenExpr>(parent) ||
          llvm::isa<clang::ImplicitCastExpr>(parent)) {
        current = parent;
        continue;
      }
      const auto* call = llvm::dyn_cast<clang::CallExpr>(parent);
      return call != nullptr && call->getCallee() == current;
    }
  }

  void note_address_taken(const clang::FunctionDecl* function,
                          const clang::SourceLocation raw) {
    if (sm_.isInSystemHeader(function->getLocation())) {
      return;
    }
    if (const auto location = locate(sm_, raw, false)) {
      const std::string key = function_key(function);
      if (shared_.fresh("taken|" + key + "|" + location_text(*location))) {
        context_.program.address_taken.push_back(
            {*location, key,
             function->getType().getCanonicalType().getAsString()});
      }
    }
  }

  void note_indirect_call(const clang::CallExpr* call) {
    const clang::QualType callee_type =
        call->getCallee()->IgnoreParenImpCasts()->getType();
    if (!callee_type->isFunctionPointerType()) {
      return;
    }
    if (const auto location = locate(sm_, call->getBeginLoc(), false)) {
      const std::string caller = function_key(current_function_);
      if (shared_.fresh("indirect|" + caller + "|" + location_text(*location))) {
        context_.program.indirect_calls.push_back(
            {*location, caller,
             callee_type->getPointeeType().getCanonicalType().getAsString()});
      }
    }
  }

  // True when the last statement of a switch clause is an unconditional break,
  // possibly as the last statement of a nested block.
  static bool ends_with_break(const clang::Stmt* statement) {
    if (statement == nullptr) {
      return false;
    }
    if (llvm::isa<clang::BreakStmt>(statement)) {
      return true;
    }
    if (const auto* block = llvm::dyn_cast<clang::CompoundStmt>(statement)) {
      return !block->body_empty() && ends_with_break(block->body_back());
    }
    return false;
  }

  // Rules 16.3, 16.5, 16.6 and 16.7.
  void check_switch_shape(const clang::SwitchStmt* statement) {
    if (essential::of_expression(statement->getCond(), ast_).category ==
        essential::Category::Boolean) {
      syntax(statement->getSwitchLoc(),
             "misra-c2012-16.7-boolean-switch-expression");
    }
    const auto* body = llvm::dyn_cast_or_null<clang::CompoundStmt>(statement->getBody());
    if (body == nullptr) {
      return;
    }
    struct Clause final {
      const clang::SwitchCase* first;
      const clang::Stmt* last;
    };
    std::vector<Clause> clauses;
    std::vector<const clang::SwitchCase*> labels;  // in source order
    for (const clang::Stmt* child : body->body()) {
      if (const auto* label = llvm::dyn_cast<clang::SwitchCase>(child)) {
        const clang::Stmt* inner = label;
        while (const auto* chained = llvm::dyn_cast<clang::SwitchCase>(inner)) {
          labels.push_back(chained);
          inner = chained->getSubStmt();
        }
        clauses.push_back({label, inner});
      } else if (!clauses.empty()) {
        clauses.back().last = child;
      }
    }
    if (clauses.size() < 2U) {
      syntax(statement->getSwitchLoc(), "misra-c2012-16.6-too-few-clauses");
    }
    for (const Clause& clause : clauses) {
      if (!ends_with_break(clause.last)) {
        syntax(clause.first->getKeywordLoc(),
               "misra-c2012-16.3-clause-not-terminated-by-break");
      }
    }
    for (std::size_t index = 0U; index < labels.size(); ++index) {
      if (llvm::isa<clang::DefaultStmt>(labels[index]) && index != 0U &&
          index + 1U != labels.size()) {
        syntax(labels[index]->getKeywordLoc(),
               "misra-c2012-16.5-default-position");
      }
    }
  }

  void check_pointer_conversion(const clang::CastExpr* cast) {
    switch (cast->getCastKind()) {
      case clang::CK_BitCast:
      case clang::CK_NoOp:
      case clang::CK_PointerToIntegral:
      case clang::CK_IntegralToPointer:
      case clang::CK_NullToPointer:
        break;
      default:
        return;
    }
    const bool explicit_cast = llvm::isa<clang::ExplicitCastExpr>(cast);
    const clang::Expr* const operand = cast->getSubExpr();
    const clang::QualType to = cast->getType().getCanonicalType();
    const clang::QualType from = operand->getType().getCanonicalType();
    const clang::SourceLocation loc = cast->getBeginLoc();
    const bool null_operand =
        operand->isNullPointerConstant(
            ast_, clang::Expr::NPC_ValueDependentIsNull) !=
        clang::Expr::NPCK_NotNull;

    const auto pointee = [](const clang::QualType t) {
      return t->isPointerType() ? t->getPointeeType().getCanonicalType()
                                : clang::QualType();
    };
    const auto object_pointer = [&](const clang::QualType t) {
      const clang::QualType p = pointee(t);
      return !p.isNull() && !p->isVoidType() && !p->isFunctionType();
    };
    const auto void_pointer = [&](const clang::QualType t) {
      const clang::QualType p = pointee(t);
      return !p.isNull() && p->isVoidType();
    };
    const auto incomplete_pointer = [&](const clang::QualType t) {
      return object_pointer(t) && pointee(t)->isIncompleteType();
    };
    const auto non_boolean_arithmetic = [](const clang::QualType t) {
      return t->isArithmeticType() && !t->isBooleanType();
    };

    if (cast->getCastKind() == clang::CK_NullToPointer) {
      // 11.9: NULL is the only permitted null pointer constant.
      const clang::Expr* const written = operand->IgnoreParenImpCasts();
      const bool is_null_macro =
          written->getBeginLoc().isMacroID() &&
          clang::Lexer::getImmediateMacroName(written->getBeginLoc(), sm_,
                                              ast_.getLangOpts()) == "NULL";
      if (!is_null_macro) {
        syntax(loc, "misra-c2012-11.9-null-pointer-constant");
      }
      return;
    }

    // 11.8: a conversion between pointers must not drop const or volatile
    // from the pointed-to type (explicit or implicit).
    if (to->isPointerType() && from->isPointerType()) {
      const clang::QualType to_pointee = to->getPointeeType();
      const clang::QualType from_pointee = from->getPointeeType();
      if ((from_pointee.isConstQualified() && !to_pointee.isConstQualified()) ||
          (from_pointee.isVolatileQualified() &&
           !to_pointee.isVolatileQualified())) {
        syntax(loc, "misra-c2012-11.8-qualifier-removed");
      }
    }

    if (null_operand) {
      return;  // converting a null pointer constant is not reported below
    }

    // 11.5: void pointer to object pointer, explicit or implicit.
    if (void_pointer(from) && object_pointer(to) &&
        cast->getCastKind() == clang::CK_BitCast) {
      syntax(loc, "misra-c2012-11.5-void-pointer-to-object");
    }

    if (!explicit_cast) {
      return;
    }
    // 11.1: function pointers convert only to function pointers.
    if (to->isFunctionPointerType() != from->isFunctionPointerType()) {
      syntax(loc, "misra-c2012-11.1-function-pointer-conversion");
    }
    // 11.2: pointer to incomplete type (conversions to void* are tolerated).
    if ((incomplete_pointer(to) || incomplete_pointer(from)) &&
        !void_pointer(to) && !void_pointer(from) &&
        to.getUnqualifiedType() != from.getUnqualifiedType()) {
      syntax(loc, "misra-c2012-11.2-incomplete-pointer-conversion");
    }
    // 11.3: pointer to one object type to pointer to a different object type.
    if (object_pointer(to) && object_pointer(from) &&
        !incomplete_pointer(to) && !incomplete_pointer(from) &&
        pointee(to).getUnqualifiedType() != pointee(from).getUnqualifiedType()) {
      syntax(loc, "misra-c2012-11.3-object-pointer-type-change");
    }
    // 11.4: object pointer to or from an integer type.
    if ((object_pointer(to) && from->isIntegerType() &&
         !from->isBooleanType()) ||
        (object_pointer(from) && to->isIntegerType() && !to->isBooleanType())) {
      syntax(loc, "misra-c2012-11.4-pointer-integer-conversion");
    }
    // 11.6: void pointer to or from an arithmetic type.
    if ((void_pointer(to) && non_boolean_arithmetic(from)) ||
        (void_pointer(from) && non_boolean_arithmetic(to))) {
      syntax(loc, "misra-c2012-11.6-void-pointer-arithmetic-conversion");
    }
  }

  void check_binary_types(const clang::BinaryOperator* expression) {
    using essential::Operand;
    const clang::BinaryOperatorKind op = expression->getOpcode();
    const clang::SourceLocation loc = expression->getOperatorLoc();
    if (op == clang::BO_Assign) {
      check_assignment(essential::of_expression(expression->getLHS(), ast_),
                       expression->getRHS(), loc);
      return;
    }
    if (op == clang::BO_Comma) {
      return;
    }
    const essential::Type left = essential::of_expression(expression->getLHS(), ast_);
    const essential::Type right = essential::of_expression(expression->getRHS(), ast_);

    Operand left_position = Operand::Equality;
    Operand right_position = Operand::Equality;
    bool additive = false;
    bool subtractive = false;
    switch (op) {
      case clang::BO_Mul: case clang::BO_Div:
      case clang::BO_MulAssign: case clang::BO_DivAssign:
        left_position = right_position = Operand::MulDiv; break;
      case clang::BO_Rem: case clang::BO_RemAssign:
        left_position = right_position = Operand::Modulo; break;
      case clang::BO_Add: case clang::BO_AddAssign:
        left_position = right_position = Operand::AddSub; additive = true; break;
      case clang::BO_Sub: case clang::BO_SubAssign:
        left_position = right_position = Operand::AddSub; subtractive = true; break;
      case clang::BO_Shl: case clang::BO_Shr:
      case clang::BO_ShlAssign: case clang::BO_ShrAssign:
        left_position = Operand::LeftShift;
        right_position = Operand::RightShift; break;
      case clang::BO_LT: case clang::BO_GT: case clang::BO_LE: case clang::BO_GE:
        left_position = right_position = Operand::Relational; break;
      case clang::BO_EQ: case clang::BO_NE:
        left_position = right_position = Operand::Equality; break;
      case clang::BO_And: case clang::BO_Xor: case clang::BO_Or:
      case clang::BO_AndAssign: case clang::BO_XorAssign: case clang::BO_OrAssign:
        left_position = right_position = Operand::LeftShift; break;
      case clang::BO_LAnd: case clang::BO_LOr:
        left_position = right_position = Operand::Logical; break;
      default:
        return;
    }
    if (!essential::appropriate(left_position, left) ||
        !essential::appropriate(right_position, right)) {
      syntax(loc, "misra-c2012-10.1-inappropriate-operand");
    }
    if ((additive || subtractive) &&
        essential::character_arithmetic_misused(subtractive, left, right)) {
      syntax(loc, "misra-c2012-10.2-character-arithmetic");
    }
    if (uses_usual_arithmetic_conversions(op)) {
      if (essential::categories_differ(left, right)) {
        syntax(loc, "misra-c2012-10.4-operand-category-mismatch");
      }
      const auto wider_than_composite = [&](const clang::Expr* composite,
                                            const essential::Type& composite_type,
                                            const essential::Type& other) {
        return essential::is_composite(composite, ast_) &&
               !other.is_constant() && composite_type.is_arithmetic() &&
               composite_type.category == other.category &&
               other.width > composite_type.width;
      };
      if (wider_than_composite(expression->getLHS(), left, right) ||
          wider_than_composite(expression->getRHS(), right, left)) {
        syntax(loc, "misra-c2012-10.7-composite-operand-width");
      }
    }
  }

  // Assignment, initialization, argument passing and return are all treated
  // as assignments (Rules 10.3 and 10.6).
  void check_assignment(const clang::QualType destination_type,
                        const clang::Expr* source_expression,
                        const clang::SourceLocation loc) {
    check_assignment(essential::of_type(destination_type, ast_),
                     source_expression, loc);
  }

  void check_assignment(const essential::Type& destination,
                        const clang::Expr* source_expression,
                        const clang::SourceLocation loc) {
    const essential::Type source = essential::of_expression(source_expression, ast_);
    if (destination.category == essential::Category::Other ||
        source.category == essential::Category::Other) {
      return;
    }
    if (essential::assignment_violates_category_or_width(destination, source)) {
      syntax(loc, "misra-c2012-10.3-assignment-category-or-width");
    } else if (essential::is_composite(source_expression, ast_) &&
               source.is_arithmetic() && source.category == destination.category &&
               destination.width > source.width) {
      syntax(loc, "misra-c2012-10.6-composite-assigned-wider");
    }
  }

  void push_scope(const void* key) {
    const std::size_t id = shared_.next_scope++;
    const std::size_t parent = scopes_.empty() ? 0U : scopes_.back().id;
    context_.scope_parents[id] = parent;
    scopes_.push_back({key, id});
  }

  void pop_scope() { scopes_.pop_back(); }

  std::size_t member_scope(const clang::RecordDecl* record) {
    const void* const key = record->getCanonicalDecl();
    const auto found = member_scopes_.find(key);
    if (found != member_scopes_.end()) {
      return found->second;
    }
    const std::size_t id = shared_.next_scope++;
    context_.scope_parents[id] = 0U;
    member_scopes_[key] = id;
    return id;
  }

  void record_identifier(const clang::NamedDecl* declaration,
                         const IdentifierSpace space) {
    if (declaration->getName().empty() || declaration->isImplicit()) {
      return;
    }
    // Parameters of a function *type* (a pointer-to-function declarator or
    // typedef) live in their own prototype scope, not in the enclosing one.
    if (llvm::isa<clang::ParmVarDecl>(declaration) &&
        !llvm::isa<clang::FunctionDecl>(declaration->getDeclContext())) {
      return;
    }
    const auto location = locate(sm_, declaration->getLocation(), false);
    if (!location) {
      return;
    }
    std::size_t scope = scopes_.back().id;
    if (space == IdentifierSpace::Member) {
      const auto* record = llvm::dyn_cast<clang::RecordDecl>(
          declaration->getDeclContext());
      if (record == nullptr) {
        return;
      }
      scope = member_scope(record);
    }
    const std::string name = declaration->getNameAsString();
    if (shared_.fresh("ident|" + std::to_string(shared_.translation_unit) + '|' +
                      std::to_string(scope) + '|' +
                      declaration_key(*location, name))) {
      context_.identifiers.push_back(
          {*location, name, scope, space, shared_.translation_unit});
    }
  }

  void syntax(const clang::SourceLocation raw, const std::string_view kind,
              const bool structural = true,
              const FindingCertainty certainty = FindingCertainty::Definite) {
    if (const auto location = locate(sm_, raw, structural)) {
      add(context_.syntax, std::string(kind),
          SyntaxFact{*location, kind, certainty});
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
    std::vector<long long> dimensions;  // -1: no constant bound
    if (const auto* value = llvm::dyn_cast<clang::ValueDecl>(declaration)) {
      clang::QualType canonical = value->getType().getCanonicalType();
      while (const clang::ArrayType* array = ast_.getAsArrayType(canonical)) {
        const auto* constant = llvm::dyn_cast<clang::ConstantArrayType>(array);
        dimensions.push_back(
            constant != nullptr ? constant->getSize().getSExtValue() : -1);
        canonical = array->getElementType().getCanonicalType();
      }
      type = canonical.getAsString();
    }
    const std::string name = declaration->getNameAsString();
    if (shared_.fresh("decl|" + std::to_string(shared_.translation_unit) + '|' +
                      declaration_key(*location, name))) {
      context_.program.declarations.push_back(
          {*location, name, kind, linkage, definition, std::move(type),
           std::move(parameter_names), std::move(dimensions),
           shared_.translation_unit});
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
              essential::of_expression(condition, ast_).category ==
                  essential::Category::Boolean});
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
  [[nodiscard]] bool value_is_used(const clang::Stmt* node,
                                   const bool void_cast_is_use = false) {
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
        if (binary->getOpcode() != clang::BO_Comma) {
          return true;
        }
        if (binary->getLHS() == current) {
          return false;  // the left operand's value is discarded
        }
        current = parent;  // the comma's value is its right operand's
        continue;
      }
      if (const auto* conditional =
              llvm::dyn_cast<clang::ConditionalOperator>(parent)) {
        if (conditional->getCond() == current) {
          return true;
        }
        current = parent;  // a result operand: used iff the ?: value is used
        continue;
      }
      if (const auto* compound = llvm::dyn_cast<clang::CompoundStmt>(parent)) {
        if (!compound->body_empty() && compound->body_back() == current) {
          const auto outer = ast_.getParents(*compound);
          return !outer.empty() && outer[0].get<clang::StmtExpr>() != nullptr;
        }
        return false;
      }
      if (const auto* cast = llvm::dyn_cast<clang::CastExpr>(parent)) {
        return void_cast_is_use || !cast->getType()->isVoidType();
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

  struct Scope final {
    const void* key;
    std::size_t id;
  };
  std::vector<Scope> scopes_;
  std::map<const void*, std::size_t> member_scopes_;
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
  // Recent Clang turns several constructs that MISRA rules exist to find into
  // hard errors in C99 and later (implicit function declarations, implicit
  // int, a return without a value in a non-void function, pointer/integer
  // mix-ups). Downgrade exactly those diagnostics to warnings so the code
  // is still analysed and the corresponding rule can report it, instead of
  // the whole translation unit failing. Unrelated errors still fail.
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      {"-Wno-error=implicit-function-declaration", "-Wno-error=implicit-int",
       "-Wno-error=return-type", "-Wno-error=int-conversion",
       "-Wno-error=incompatible-pointer-types",
       "-Wno-error=incompatible-function-pointer-types"},
      clang::tooling::ArgumentInsertPosition::END));
  FactActionFactory factory(shared);
  if (tool.run(&factory) != 0) {
    result.success = false;
    result.error_message = "Clang could not analyze one or more translation units";
  }
  return result;
}

}  // namespace misra
