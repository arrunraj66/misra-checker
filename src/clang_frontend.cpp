#include "misra/clang_frontend.hpp"

#include "essential_type.hpp"

#include <algorithm>
#include <cstdint>
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
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticSema.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
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

// Side effects visible without knowing what a called function does. A call is
// only a *potential* side effect.
struct SideEffects final {
  bool assignment = false;
  bool incdec = false;
  bool call = false;
  bool volatile_read = false;

  [[nodiscard]] bool definite() const {
    return assignment || incdec || volatile_read;
  }
};

void collect_side_effects(const clang::Stmt* statement, SideEffects& effects) {
  if (statement == nullptr || llvm::isa<clang::UnaryExprOrTypeTraitExpr>(statement)) {
    return;  // sizeof and alignof do not evaluate their operand
  }
  if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(statement);
      binary != nullptr && binary->isAssignmentOp()) {
    effects.assignment = true;
  }
  if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(statement);
      unary != nullptr && unary->isIncrementDecrementOp()) {
    effects.incdec = true;
  }
  if (llvm::isa<clang::CallExpr>(statement)) {
    effects.call = true;
  }
  if (const auto* cast = llvm::dyn_cast<clang::ImplicitCastExpr>(statement);
      cast != nullptr && cast->getCastKind() == clang::CK_LValueToRValue &&
      cast->getSubExpr()->getType().isVolatileQualified()) {
    effects.volatile_read = true;
  }
  for (const clang::Stmt* child : statement->children()) {
    collect_side_effects(child, effects);
  }
}

// Variables an expression or statement modifies (assigns, increments, or
// takes the address of).
void collect_modified_variables(const clang::Stmt* statement,
                                std::set<const clang::VarDecl*>& modified) {
  if (statement == nullptr) {
    return;
  }
  const auto note = [&modified](const clang::Expr* target) {
    if (const auto* reference =
            llvm::dyn_cast<clang::DeclRefExpr>(target->IgnoreParenImpCasts())) {
      if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl())) {
        modified.insert(variable->getCanonicalDecl());
      }
    }
  };
  if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(statement);
      binary != nullptr && binary->isAssignmentOp()) {
    note(binary->getLHS());
  }
  if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(statement);
      unary != nullptr &&
      (unary->isIncrementDecrementOp() || unary->getOpcode() == clang::UO_AddrOf)) {
    note(unary->getSubExpr());
  }
  for (const clang::Stmt* child : statement->children()) {
    collect_modified_variables(child, modified);
  }
}

// Variables read and written by an expression, for the sequencing rule.
struct Effects final {
  std::set<const clang::VarDecl*> reads;
  std::set<const clang::VarDecl*> writes;
};

void merge(Effects& into, const Effects& from) {
  into.reads.insert(from.reads.begin(), from.reads.end());
  into.writes.insert(from.writes.begin(), from.writes.end());
}

bool overlap(const std::set<const clang::VarDecl*>& a,
             const std::set<const clang::VarDecl*>& b) {
  for (const clang::VarDecl* variable : a) {
    if (b.count(variable) != 0U) {
      return true;
    }
  }
  return false;
}

// Two operands whose evaluation order is unspecified conflict when one writes
// what the other reads or writes.
bool unsequenced_conflict(const Effects& a, const Effects& b) {
  return overlap(a.writes, b.reads) || overlap(a.writes, b.writes) ||
         overlap(a.reads, b.writes);
}

bool is_string_literal_expression(const clang::Expr* expression) {
  for (;;) {
    expression = expression->IgnoreParenImpCasts();
    if (const auto* cast = llvm::dyn_cast<clang::CStyleCastExpr>(expression)) {
      expression = cast->getSubExpr();
      continue;
    }
    return llvm::isa<clang::StringLiteral>(expression);
  }
}

// The one initializer form every aggregate may use: {0}, possibly nested.
bool is_zero_brace(const clang::InitListExpr* syntactic) {
  if (syntactic == nullptr || syntactic->getNumInits() != 1U) {
    return false;
  }
  const clang::Expr* const only = syntactic->getInit(0)->IgnoreParenImpCasts();
  if (const auto* literal = llvm::dyn_cast<clang::IntegerLiteral>(only)) {
    return literal->getValue() == 0U;
  }
  if (const auto* nested = llvm::dyn_cast<clang::InitListExpr>(only)) {
    return is_zero_brace(nested);
  }
  return false;
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
    check_goto_label_block(statement);
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
    check_for_loop(statement);
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
    if (expression->isLogicalOp()) {
      SideEffects effects;
      collect_side_effects(expression->getRHS(), effects);
      if (effects.definite()) {
        syntax(expression->getOperatorLoc(), "misra-c2012-13.5-logical-rhs-side-effects");
      } else if (effects.call) {
        syntax(expression->getOperatorLoc(), "misra-c2012-13.5-logical-rhs-side-effects",
               true, FindingCertainty::Possible);
      }
    }
    check_binary_types(expression);
    check_pointer_arithmetic(expression);
    return true;
  }

  // ------------------------------------------------- Section 10 (type rules)
  bool VisitUnaryOperator(clang::UnaryOperator* expression) {
    using essential::Operand;
    if (expression->isIncrementDecrementOp()) {
      note_parameter_modification(expression->getSubExpr(),
                                  expression->getOperatorLoc());
    }
    if (expression->getOpcode() == clang::UO_Minus &&
        expression->getType()->isUnsignedIntegerType()) {
      if (const auto value = expression->getSubExpr()->getIntegerConstantExpr(ast_);
          value && !value->isZero()) {
        syntax(expression->getOperatorLoc(),
               "misra-c2012-12.4-constant-unsigned-wrap");
      }
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
    check_constant_subscript(expression);
    if (!essential::appropriate(
            essential::Operand::Subscript,
            essential::of_expression(expression->getIdx(), ast_))) {
      syntax(expression->getExprLoc(), "misra-c2012-10.1-inappropriate-operand");
    }
    return true;
  }

  bool VisitConditionalOperator(clang::ConditionalOperator* expression) {
    note_invariant_condition(expression->getCond(), "?:");
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

  // ----------------------------------------------- Section 9 (initialization)
  bool VisitInitListExpr(clang::InitListExpr* list) {
    if (list->isSemanticForm()) {
      return true;  // the syntactic form carries the designators
    }
    // 13.1: initializer lists must not contain persistent side effects.
    for (const clang::Expr* init : list->inits()) {
      if (init == nullptr) {
        continue;
      }
      SideEffects effects;
      collect_side_effects(init, effects);
      if (effects.definite()) {
        syntax(init->getBeginLoc(), "misra-c2012-13.1-initializer-side-effects");
      } else if (effects.call) {
        syntax(init->getBeginLoc(), "misra-c2012-13.1-initializer-side-effects", true,
               FindingCertainty::Possible);
      }
    }
    // 9.4: an element must not be initialized twice by designators.
    std::set<std::string> seen;
    for (const clang::Expr* init : list->inits()) {
      const auto* designated = llvm::dyn_cast_or_null<clang::DesignatedInitExpr>(init);
      if (designated == nullptr) {
        continue;
      }
      std::string key;
      for (const auto& designator : designated->designators()) {
        if (designator.isFieldDesignator()) {
          key += "." + designator.getFieldName()->getName().str();
        } else if (designator.isArrayDesignator()) {
          const auto index =
              designated->getArrayIndex(designator)->getIntegerConstantExpr(ast_);
          if (!index) {
            key.clear();
            break;
          }
          key += "[" + std::to_string(index->getSExtValue()) + "]";
        } else {
          key.clear();
          break;
        }
      }
      if (!key.empty() && !seen.insert(key).second) {
        syntax(designated->getBeginLoc(),
               "misra-c2012-9.4-duplicate-initialization");
      }
    }
    return true;
  }

  bool VisitCompoundLiteralExpr(clang::CompoundLiteralExpr* literal) {
    if (const auto* list =
            llvm::dyn_cast_or_null<clang::InitListExpr>(literal->getInitializer())) {
      check_initializer_tree(list);
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt* statement) {
    if (statement->getRetValue() != nullptr &&
        automatic_object_address(statement->getRetValue()) != nullptr) {
      syntax(statement->getReturnLoc(), "misra-c2012-18.6-automatic-address-escapes");
    }
    if (statement->getRetValue() == nullptr && current_function_ != nullptr &&
        !current_function_->getReturnType()->isVoidType()) {
      syntax(statement->getReturnLoc(), "misra-c2012-17.4-missing-return");
    }
    if (statement->getRetValue() != nullptr && current_function_ != nullptr) {
      check_assignment(current_function_->getReturnType(),
                       statement->getRetValue(), statement->getReturnLoc());
      check_string_literal_target(current_function_->getReturnType(),
                                  statement->getRetValue(), statement->getReturnLoc());
    }
    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral* literal) {
    const std::string text = literal_text(literal->getBeginLoc());
    if (literal->getType()->isUnsignedIntegerType() &&
        text.find_first_of("uU") == std::string::npos) {
      syntax(literal->getBeginLoc(), "misra-c2012-7.2-missing-unsigned-suffix");
    }
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
    note_automatic_use(reference);
    if (const auto* parameter = llvm::dyn_cast<clang::ParmVarDecl>(reference->getDecl());
        parameter != nullptr && parameter->getType()->isPointerType()) {
      parameter_references_[parameter].push_back(reference);
    }
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
        address_taken_in_unit_.insert(function->getCanonicalDecl());
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
    check_pointer_nesting(function->getReturnType(), function->getLocation());
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
    check_pointer_nesting(declared, variable->getLocation());
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
      check_string_literal_target(variable->getType(), init, variable->getLocation());
    }
    if (const auto* list =
            llvm::dyn_cast_or_null<clang::InitListExpr>(variable->getInit())) {
      check_initializer_tree(list);
      check_designated_array_size(variable, list);
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

  bool VisitStringLiteral(clang::StringLiteral* literal) {
    for (unsigned index = 0U; index < literal->getNumConcatenated(); ++index) {
      check_escape_termination(literal->getStrTokenLoc(index));
    }
    return true;
  }

  bool VisitCharacterLiteral(clang::CharacterLiteral* literal) {
    check_escape_termination(literal->getBeginLoc());
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* record) {
    if (record->isUnion() && record->isCompleteDefinition() && !record->isImplicit()) {
      syntax(record->getLocation(), "misra-c2012-19.2-union");
    }
    return true;
  }

  bool VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr* expression) {
    if (expression->getKind() == clang::UETT_SizeOf && !expression->isArgumentType()) {
      SideEffects effects;
      collect_side_effects(expression->getArgumentExpr(), effects);
      if (effects.definite()) {
        syntax(expression->getOperatorLoc(), "misra-c2012-13.6-sizeof-side-effects");
      } else if (effects.call) {
        syntax(expression->getOperatorLoc(), "misra-c2012-13.6-sizeof-side-effects",
               true, FindingCertainty::Possible);
      }
    }
    return true;
  }

  // Every full expression: unsequenced accesses (13.2) and increments mixed
  // with other side effects (13.3).
  bool VisitStmt(clang::Stmt* statement) {
    const auto* expression = llvm::dyn_cast<clang::Expr>(statement);
    if (expression == nullptr) {
      return true;
    }
    const auto parents = ast_.getParents(*statement);
    if (parents.empty()) {
      return true;
    }
    const clang::Stmt* const parent = parents[0].get<clang::Stmt>();
    if (parent != nullptr && llvm::isa<clang::Expr>(parent)) {
      return true;  // a sub-expression
    }
    const bool initializer = parents[0].get<clang::VarDecl>() != nullptr;
    (void)sequence_effects(expression);
    SideEffects effects;
    collect_side_effects(expression, effects);
    if (initializer) {
      effects.assignment = true;
    }
    if (effects.incdec && (effects.assignment || effects.call || effects.volatile_read)) {
      syntax(expression->getExprLoc(), "misra-c2012-13.3-increment-with-side-effects");
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* field) {
    record_identifier(field, IdentifierSpace::Member);
    if (field->isBitField()) {
      const clang::QualType type = field->getType().getCanonicalType().getUnqualifiedType();
      const bool appropriate = type->isBooleanType() ||
                               type->isSpecificBuiltinType(clang::BuiltinType::Int) ||
                               type->isSpecificBuiltinType(clang::BuiltinType::UInt);
      if (!appropriate) {
        syntax(field->getLocation(), "misra-c2012-6.1-bit-field-type");
      }
      if (!field->getName().empty() && field->getBitWidthValue(ast_) == 1U &&
          type->isSignedIntegerType() && !type->isBooleanType()) {
        syntax(field->getLocation(), "misra-c2012-6.2-single-bit-signed");
      }
    }
    if (field->getType()->isIncompleteArrayType()) {
      syntax(field->getLocation(), "misra-c2012-18.7-flexible-array-member");
    }
    check_pointer_nesting(field->getType(), field->getLocation());
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
    check_pointer_nesting(declaration->getUnderlyingType(), declaration->getLocation());
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
    report_uninitialized_reads();
    report_pointers_that_could_be_const();
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

  // ------------------------------------------------- Stage D helper functions
  // 13.2: operands whose order of evaluation is unspecified must not conflict.
  Effects sequence_effects(const clang::Stmt* node) {
    Effects result;
    if (node == nullptr || llvm::isa<clang::UnaryExprOrTypeTraitExpr>(node)) {
      return result;
    }
    const auto scalar_variable = [](const clang::Expr* expression) -> const clang::VarDecl* {
      const auto* reference =
          llvm::dyn_cast<clang::DeclRefExpr>(expression->IgnoreParenImpCasts());
      if (reference == nullptr) {
        return nullptr;
      }
      const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
      return (variable != nullptr && variable->getType()->isScalarType())
                 ? variable->getCanonicalDecl()
                 : nullptr;
    };
    if (const auto* expression = llvm::dyn_cast<clang::Expr>(node)) {
      if (const clang::VarDecl* variable = scalar_variable(expression);
          variable != nullptr && llvm::isa<clang::DeclRefExpr>(expression->IgnoreParens())) {
        result.reads.insert(variable);
        return result;
      }
    }
    if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(node)) {
      const clang::BinaryOperatorKind op = binary->getOpcode();
      if (op == clang::BO_Comma || op == clang::BO_LAnd || op == clang::BO_LOr) {
        merge(result, sequence_effects(binary->getLHS()));  // sequenced
        merge(result, sequence_effects(binary->getRHS()));
        return result;
      }
      const Effects right = sequence_effects(binary->getRHS());
      if (binary->isAssignmentOp()) {
        if (const clang::VarDecl* target = scalar_variable(binary->getLHS())) {
          if (right.writes.count(target) != 0U) {
            syntax(binary->getOperatorLoc(), "misra-c2012-13.2-unsequenced-access");
          }
          result = right;
          result.writes.insert(target);
          if (op != clang::BO_Assign) {
            result.reads.insert(target);
          }
          return result;
        }
      }
      const Effects left = sequence_effects(binary->getLHS());
      if (unsequenced_conflict(left, right)) {
        syntax(binary->getOperatorLoc(), "misra-c2012-13.2-unsequenced-access");
      }
      merge(result, left);
      merge(result, right);
      return result;
    }
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(node);
        unary != nullptr && unary->isIncrementDecrementOp()) {
      if (const clang::VarDecl* target = scalar_variable(unary->getSubExpr())) {
        result.reads.insert(target);
        result.writes.insert(target);
        return result;
      }
    }
    if (const auto* call = llvm::dyn_cast<clang::CallExpr>(node)) {
      std::vector<Effects> operands;
      operands.push_back(sequence_effects(call->getCallee()));
      for (const clang::Expr* argument : call->arguments()) {
        operands.push_back(sequence_effects(argument));
      }
      for (std::size_t i = 0U; i < operands.size(); ++i) {
        for (std::size_t j = i + 1U; j < operands.size(); ++j) {
          if (unsequenced_conflict(operands[i], operands[j])) {
            syntax(call->getBeginLoc(), "misra-c2012-13.2-unsequenced-access");
          }
        }
        merge(result, operands[i]);
      }
      return result;
    }
    if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(node)) {
      const Effects base = sequence_effects(subscript->getBase());
      const Effects index = sequence_effects(subscript->getIdx());
      if (unsequenced_conflict(base, index)) {
        syntax(subscript->getExprLoc(), "misra-c2012-13.2-unsequenced-access");
      }
      merge(result, base);
      merge(result, index);
      return result;
    }
    for (const clang::Stmt* child : node->children()) {
      merge(result, sequence_effects(child));
    }
    return result;
  }

  // 14.1 and 14.2: structure of a for loop (the checkable part).
  void check_for_loop(const clang::ForStmt* loop) {
    std::set<const clang::VarDecl*> counters;
    SideEffects increment_effects;
    if (loop->getInc() != nullptr) {
      collect_modified_variables(loop->getInc(), counters);
      collect_side_effects(loop->getInc(), increment_effects);
    }
    bool ill_formed = counters.size() > 1U || increment_effects.call;
    if (loop->getCond() != nullptr) {
      SideEffects condition_effects;
      collect_side_effects(loop->getCond(), condition_effects);
      ill_formed = ill_formed || condition_effects.assignment || condition_effects.incdec;
    }
    std::set<const clang::VarDecl*> modified_in_body;
    collect_modified_variables(loop->getBody(), modified_in_body);
    for (const clang::VarDecl* counter : counters) {
      ill_formed = ill_formed || modified_in_body.count(counter) != 0U;
      if (counter->getType()->isFloatingType()) {
        syntax(loop->getForLoc(), "misra-c2012-14.1-floating-loop-counter");
      }
    }
    if (ill_formed) {
      syntax(loop->getForLoc(), "misra-c2012-14.2-ill-formed-for");
    }
  }

  // 14.3: a controlling expression with a value known at compile time. The
  // infinite-loop idiom while (1) / for (;;) is accepted.
  void note_invariant_condition(const clang::Expr* condition,
                                const std::string_view construct) {
    bool value = false;
    if (!condition->EvaluateAsBooleanCondition(value, ast_)) {
      return;
    }
    if (value && (construct == "while" || construct == "for")) {
      return;
    }
    syntax(condition->getExprLoc(), "misra-c2012-14.3-invariant-condition");
  }

  // 15.3: the label must be in the goto's own block or one enclosing it.
  void check_goto_label_block(const clang::GotoStmt* statement) {
    const clang::LabelStmt* const label = statement->getLabel()->getStmt();
    if (label == nullptr) {
      return;
    }
    const clang::Stmt* block = label;
    while (block != nullptr && !llvm::isa<clang::CompoundStmt>(block)) {
      block = parent_statement(block);
    }
    if (block == nullptr) {
      return;
    }
    const clang::Stmt* current = statement;
    while (current != nullptr && current != block) {
      current = parent_statement(current);
    }
    if (current == nullptr) {
      syntax(statement->getGotoLoc(), "misra-c2012-15.3-goto-into-block");
    }
  }

  // 7.4: a string literal reaching a pointer to non-const character type.
  void check_string_literal_target(const clang::QualType destination,
                                   const clang::Expr* source,
                                   const clang::SourceLocation loc) {
    if (destination->isPointerType() &&
        !destination->getPointeeType().isConstQualified() &&
        destination->getPointeeType()->isCharType() &&
        is_string_literal_expression(source)) {
      syntax(loc, "misra-c2012-7.4-string-literal-to-non-const");
    }
  }

  // 4.1: an octal or hexadecimal escape must be followed by the end of the
  // literal or by another escape sequence.
  void check_escape_termination(const clang::SourceLocation raw) {
    const clang::SourceLocation spelling = sm_.getSpellingLoc(raw);
    const std::string text =
        clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(spelling, spelling),
                                    sm_, ast_.getLangOpts())
            .str();
    const std::size_t open = text.find_first_of("\"'");
    if (open == std::string::npos || text.size() < open + 2U) {
      return;
    }
    const std::size_t close = text.size() - 1U;  // the closing quote
    const auto is_hex = [](const char c) {
      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    };
    for (std::size_t i = open + 1U; i < close; ++i) {
      if (text[i] != '\\') {
        continue;
      }
      const std::size_t first = i + 1U;
      if (first >= close) {
        break;
      }
      std::size_t end = first + 1U;  // one past the escape sequence
      bool numeric = false;
      if (text[first] == 'x') {
        end = first + 1U;
        while (end < close && is_hex(text[end])) {
          ++end;
        }
        numeric = end > first + 1U;
      } else if (text[first] >= '0' && text[first] <= '7') {
        end = first;
        while (end < close && end < first + 3U && text[end] >= '0' && text[end] <= '7') {
          ++end;
        }
        numeric = true;
      }
      if (numeric && end < close && text[end] != '\\') {
        syntax(raw, "misra-c2012-4.1-unterminated-escape");
      }
      i = end - 1U;
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

  // ------------------------------------------------------ Section 18 (pointers)
  // 18.5: more than two levels of pointer in a declaration. Levels are counted
  // on the type as written, so a typedef-name stops the count.
  void check_pointer_nesting(clang::QualType type, const clang::SourceLocation loc) {
    unsigned levels = 0U;
    for (;;) {
      const clang::Type* const raw = type.getTypePtrOrNull();
      if (raw == nullptr) {
        break;
      }
      if (const auto* pointer = llvm::dyn_cast<clang::PointerType>(raw)) {
        ++levels;
        type = pointer->getPointeeType();
      } else if (const auto* array = llvm::dyn_cast<clang::ArrayType>(raw)) {
        type = array->getElementType();
      } else if (const auto* paren = llvm::dyn_cast<clang::ParenType>(raw)) {
        type = paren->getInnerType();
      } else {
        break;
      }
    }
    if (levels > 2U) {
      syntax(loc, "misra-c2012-18.5-pointer-nesting");
    }
  }

  // The array variable a pointer expression certainly points into, if the
  // expression is a plain array, &array[i], or array +/- n.
  [[nodiscard]] const clang::VarDecl* array_object(const clang::Expr* expression) {
    expression = expression->IgnoreParenImpCasts();
    if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(expression)) {
      const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
      return (variable != nullptr && variable->getType()->isArrayType())
                 ? variable->getCanonicalDecl()
                 : nullptr;
    }
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(expression);
        unary != nullptr && unary->getOpcode() == clang::UO_AddrOf) {
      if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(
              unary->getSubExpr()->IgnoreParenImpCasts())) {
        return array_object(subscript->getBase());
      }
      return nullptr;
    }
    if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(expression);
        binary != nullptr &&
        (binary->getOpcode() == clang::BO_Add || binary->getOpcode() == clang::BO_Sub)) {
      if (binary->getLHS()->getType()->isPointerType()) {
        return array_object(binary->getLHS());
      }
      if (binary->getRHS()->getType()->isPointerType()) {
        return array_object(binary->getRHS());
      }
    }
    return nullptr;
  }

  [[nodiscard]] static const clang::ConstantArrayType* constant_array(
      const clang::Expr* expression) {
    return llvm::dyn_cast_or_null<clang::ConstantArrayType>(
        expression->IgnoreParenImpCasts()->getType()->getAsArrayTypeUnsafe());
  }

  void check_pointer_arithmetic(const clang::BinaryOperator* expression) {
    const clang::BinaryOperatorKind op = expression->getOpcode();
    const clang::Expr* const left = expression->getLHS();
    const clang::Expr* const right = expression->getRHS();
    const clang::SourceLocation loc = expression->getOperatorLoc();
    const bool both_pointers = left->getType()->isPointerType() &&
                               right->getType()->isPointerType();

    // 18.4: + - += -= applied to a pointer.
    if ((op == clang::BO_Add || op == clang::BO_Sub || op == clang::BO_AddAssign ||
         op == clang::BO_SubAssign) &&
        (left->getType()->isPointerType() || right->getType()->isPointerType())) {
      syntax(loc, "misra-c2012-18.4-pointer-arithmetic");
    }
    // 18.2 / 18.3: two pointers into two different, named arrays.
    if (both_pointers) {
      const clang::VarDecl* const a = array_object(left);
      const clang::VarDecl* const b = array_object(right);
      if (a != nullptr && b != nullptr && a != b) {
        if (op == clang::BO_Sub) {
          syntax(loc, "misra-c2012-18.2-subtraction-different-arrays");
        } else if (op == clang::BO_LT || op == clang::BO_GT || op == clang::BO_LE ||
                   op == clang::BO_GE) {
          syntax(loc, "misra-c2012-18.3-relational-different-arrays");
        }
      }
    }
    // 18.1: array +/- constant leaving the array (one past the end is allowed).
    if (op == clang::BO_Add || op == clang::BO_Sub) {
      const clang::Expr* const pointer = left->getType()->isPointerType() ? left : right;
      const clang::Expr* const offset = (pointer == left) ? right : left;
      if (const auto* array = constant_array(pointer); array != nullptr &&
          !offset->getType()->isPointerType() &&
          !(op == clang::BO_Sub && pointer != left)) {
        if (const auto value = offset->getIntegerConstantExpr(ast_)) {
          const llvm::APSInt signed_value = value->extOrTrunc(64);
          const std::int64_t amount = signed_value.getSExtValue();
          const std::int64_t index = (op == clang::BO_Sub) ? -amount : amount;
          if (index < 0 || static_cast<std::uint64_t>(index) > array->getSize().getZExtValue()) {
            syntax(loc, "misra-c2012-18.1-out-of-bounds");
          }
        }
      }
    }
    // 18.6: the address of an automatic object stored where it outlives it.
    if (op == clang::BO_Assign && persists_beyond_function(left) &&
        automatic_object_address(right) != nullptr) {
      syntax(loc, "misra-c2012-18.6-automatic-address-escapes");
    }
  }

  // 18.1: a constant subscript outside the bounds of a known array.
  void check_constant_subscript(const clang::ArraySubscriptExpr* expression) {
    const auto* array = constant_array(expression->getBase());
    if (array == nullptr) {
      return;
    }
    const auto index = expression->getIdx()->getIntegerConstantExpr(ast_);
    if (!index) {
      return;
    }
    const std::int64_t value = index->extOrTrunc(64).getSExtValue();
    const std::uint64_t size = array->getSize().getZExtValue();
    bool address_taken = false;  // &a[size] designates one past the end
    {
      const clang::Stmt* current = expression;
      for (;;) {
        const clang::Stmt* const parent = parent_statement(current);
        if (parent != nullptr && llvm::isa<clang::ParenExpr>(parent)) {
          current = parent;
          continue;
        }
        const auto* unary = llvm::dyn_cast_or_null<clang::UnaryOperator>(parent);
        address_taken = unary != nullptr && unary->getOpcode() == clang::UO_AddrOf;
        break;
      }
    }
    const bool one_past = address_taken && value >= 0 &&
                          static_cast<std::uint64_t>(value) == size;
    if ((value < 0 || static_cast<std::uint64_t>(value) >= size) && !one_past) {
      syntax(expression->getExprLoc(), "misra-c2012-18.1-out-of-bounds");
    }
  }

  // The automatic object whose address `expression` certainly is, if any.
  [[nodiscard]] static const clang::VarDecl* automatic_base(
      const clang::Expr* lvalue) {
    lvalue = lvalue->IgnoreParenImpCasts();
    if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(lvalue)) {
      const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
      return (variable != nullptr && variable->hasLocalStorage()) ? variable : nullptr;
    }
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(lvalue);
        member != nullptr && !member->isArrow()) {
      return automatic_base(member->getBase());
    }
    if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(lvalue);
        subscript != nullptr && subscript->getBase()->IgnoreParenImpCasts()->getType()->isArrayType()) {
      return automatic_base(subscript->getBase());
    }
    return nullptr;
  }

  [[nodiscard]] static const clang::VarDecl* automatic_object_address(
      const clang::Expr* expression) {
    expression = expression->IgnoreParenImpCasts();
    if (const auto* conditional = llvm::dyn_cast<clang::ConditionalOperator>(expression)) {
      const clang::VarDecl* const first = automatic_object_address(conditional->getTrueExpr());
      return first != nullptr ? first : automatic_object_address(conditional->getFalseExpr());
    }
    if (const auto* cast = llvm::dyn_cast<clang::CStyleCastExpr>(expression)) {
      return automatic_object_address(cast->getSubExpr());
    }
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(expression);
        unary != nullptr && unary->getOpcode() == clang::UO_AddrOf) {
      return automatic_base(unary->getSubExpr());
    }
    if (expression->getType()->isArrayType()) {  // an array decays to its address
      return automatic_base(expression);
    }
    return nullptr;
  }

  // True when storing into `lvalue` outlives the current call: a static or
  // global object, or something reached through a pointer parameter.
  [[nodiscard]] static bool persists_beyond_function(const clang::Expr* lvalue) {
    lvalue = lvalue->IgnoreParenImpCasts();
    if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(lvalue)) {
      const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
      return variable != nullptr && variable->hasGlobalStorage();
    }
    const auto through_parameter = [](const clang::Expr* pointer) {
      const auto* reference =
          llvm::dyn_cast<clang::DeclRefExpr>(pointer->IgnoreParenImpCasts());
      return reference != nullptr && llvm::isa<clang::ParmVarDecl>(reference->getDecl());
    };
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(lvalue);
        unary != nullptr && unary->getOpcode() == clang::UO_Deref) {
      return through_parameter(unary->getSubExpr());
    }
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(lvalue)) {
      return member->isArrow() ? through_parameter(member->getBase())
                               : persists_beyond_function(member->getBase());
    }
    if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(lvalue)) {
      return through_parameter(subscript->getBase()) ||
             persists_beyond_function(subscript->getBase());
    }
    return false;
  }

  // 9.2 and 9.3 on a whole initializer: braces for every nested aggregate and
  // no partially initialized array. {0} is exempt, as are designated lists.
  void check_initializer_tree(const clang::InitListExpr* given) {
    const clang::InitListExpr* semantic =
        given->isSemanticForm() ? given : given->getSemanticForm();
    const clang::InitListExpr* syntactic =
        given->isSemanticForm() ? given->getSyntacticForm() : given;
    if (semantic == nullptr) {
      semantic = given;
    }
    if (is_zero_brace(syntactic)) {
      return;
    }
    walk_initializer(semantic, syntactic);
  }

  void walk_initializer(const clang::InitListExpr* list,
                        const clang::InitListExpr* syntactic) {
    if (const auto* array = llvm::dyn_cast_or_null<clang::ConstantArrayType>(
            list->getType()->getAsArrayTypeUnsafe());
        array != nullptr && !is_zero_brace(syntactic)) {
      bool designated = false;
      if (syntactic != nullptr) {
        for (const clang::Expr* init : syntactic->inits()) {
          designated = designated || llvm::isa_and_nonnull<clang::DesignatedInitExpr>(init);
        }
      }
      const bool string_init =
          list->getNumInits() == 1U &&
          llvm::isa_and_nonnull<clang::StringLiteral>(
              list->getInit(0)->IgnoreParenImpCasts());
      if (!designated && !string_init &&
          list->getNumInits() < array->getSize().getZExtValue()) {
        syntax(list->getBeginLoc(), "misra-c2012-9.3-partial-array-init");
      }
    }
    for (const clang::Expr* child : list->inits()) {
      const auto* nested = llvm::dyn_cast_or_null<clang::InitListExpr>(child);
      if (nested == nullptr) {
        continue;
      }
      if (nested->getType()->isAggregateType() && !starts_with_brace(nested)) {
        syntax(nested->getBeginLoc(), "misra-c2012-9.2-initializer-braces");
      }
      walk_initializer(nested, nested->getSyntacticForm());
    }
  }

  [[nodiscard]] bool starts_with_brace(const clang::InitListExpr* list) const {
    bool invalid = false;
    const char* const text = sm_.getCharacterData(
        sm_.getSpellingLoc(list->getBeginLoc()), &invalid);
    return !invalid && text != nullptr && *text == '{';
  }

  // 9.5: designators that select array elements need an explicit array size.
  void check_designated_array_size(const clang::VarDecl* variable,
                                   const clang::InitListExpr* list) {
    const clang::InitListExpr* const syntactic =
        list->isSemanticForm() ? list->getSyntacticForm() : list;
    if (syntactic == nullptr || variable->getTypeSourceInfo() == nullptr) {
      return;
    }
    bool array_designator = false;
    for (const clang::Expr* init : syntactic->inits()) {
      if (const auto* designated =
              llvm::dyn_cast_or_null<clang::DesignatedInitExpr>(init)) {
        for (const auto& designator : designated->designators()) {
          array_designator = array_designator || designator.isArrayDesignator() ||
                             designator.isArrayRangeDesignator();
        }
      }
    }
    if (!array_designator) {
      return;
    }
    const clang::TypeLoc type_loc =
        variable->getTypeSourceInfo()->getTypeLoc().getUnqualifiedLoc();
    if (const auto array_loc = type_loc.getAs<clang::ArrayTypeLoc>();
        array_loc && array_loc.getSizeExpr() == nullptr) {
      syntax(variable->getLocation(), "misra-c2012-9.5-designated-array-size");
    }
  }

  // ------------------------------------------------------------------ 8.13
  [[nodiscard]] const clang::Stmt* parent_statement(const clang::Stmt* node) {
    const auto parents = ast_.getParents(*node);
    return parents.empty() ? nullptr : parents[0].get<clang::Stmt>();
  }

  static bool points_to_const(const clang::QualType type) {
    return type->isPointerType() && type->getPointeeType().isConstQualified();
  }

  // An lvalue that designates (part of) the pointee: true when every use only
  // reads it.
  [[nodiscard]] bool lvalue_is_only_read(const clang::Stmt* node) {
    for (;;) {
      const auto parents = ast_.getParents(*node);
      const clang::Stmt* const parent =
          parents.empty() ? nullptr : parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return false;
      }
      if (llvm::isa<clang::ParenExpr>(parent)) {
        node = parent;
        continue;
      }
      if (const auto* cast = llvm::dyn_cast<clang::ImplicitCastExpr>(parent)) {
        if (cast->getCastKind() == clang::CK_LValueToRValue) {
          return true;
        }
        if (cast->getCastKind() == clang::CK_ArrayToPointerDecay) {
          const clang::Stmt* const above = parent_statement(parent);
          if (const auto* subscript =
                  llvm::dyn_cast_or_null<clang::ArraySubscriptExpr>(above);
              subscript != nullptr && subscript->getBase() == parent) {
            node = subscript;  // p->array[i] is again an lvalue into the pointee
            continue;
          }
        }
        return false;
      }
      if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(parent);
          member != nullptr && !member->isArrow()) {
        node = parent;  // (*p).field
        continue;
      }
      return llvm::isa<clang::UnaryExprOrTypeTraitExpr>(parent);
    }
  }

  // The value of a pointer expression: true when every use leaves the pointee
  // unmodified and never hands the pointer to something that is not
  // pointer-to-const. Anything not recognised counts as a possible change.
  [[nodiscard]] bool pointer_value_is_only_read(const clang::Stmt* node,
                                                const clang::FunctionDecl* function) {
    for (;;) {
      const auto parents = ast_.getParents(*node);
      if (parents.empty()) {
        return false;
      }
      if (const auto* variable = parents[0].get<clang::VarDecl>()) {
        return points_to_const(variable->getType());
      }
      const clang::Stmt* const parent = parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return false;
      }
      if (llvm::isa<clang::ParenExpr>(parent)) {
        node = parent;
        continue;
      }
      if (const auto* cast = llvm::dyn_cast<clang::CastExpr>(parent)) {
        if (cast->getCastKind() == clang::CK_LValueToRValue) {
          node = parent;
          continue;
        }
        if (cast->getCastKind() == clang::CK_PointerToBoolean) {
          return true;
        }
        return points_to_const(cast->getType());
      }
      if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(parent)) {
        switch (unary->getOpcode()) {
          case clang::UO_Deref:
            return lvalue_is_only_read(parent);
          case clang::UO_PreInc: case clang::UO_PostInc:
          case clang::UO_PreDec: case clang::UO_PostDec:
            node = parent;  // changes the pointer, not the pointee
            continue;
          case clang::UO_LNot:
            return true;
          default:
            return false;
        }
      }
      if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(parent)) {
        if (binary->isComparisonOp() || binary->isLogicalOp()) {
          return true;
        }
        if (binary->getOpcode() == clang::BO_Comma) {
          if (binary->getLHS() == node) {
            return true;
          }
          node = parent;
          continue;
        }
        if (binary->isAssignmentOp()) {
          if (binary->getLHS() == node) {
            if (binary->getOpcode() == clang::BO_Assign) {
              return true;  // the pointer variable itself is overwritten
            }
            node = parent;  // p += n: the value is the new pointer
            continue;
          }
          return points_to_const(binary->getLHS()->getType());
        }
        if (binary->getOpcode() == clang::BO_Add ||
            binary->getOpcode() == clang::BO_Sub) {
          if (!binary->getType()->isPointerType()) {
            return true;  // p - q is an integer
          }
          node = parent;
          continue;
        }
        return false;
      }
      if (const auto* conditional = llvm::dyn_cast<clang::ConditionalOperator>(parent)) {
        if (conditional->getCond() == node) {
          return true;
        }
        node = parent;
        continue;
      }
      if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(parent)) {
        return subscript->getBase() == node && lvalue_is_only_read(parent);
      }
      if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(parent)) {
        return member->isArrow() && member->getBase() == node &&
               lvalue_is_only_read(parent);
      }
      if (const auto* call = llvm::dyn_cast<clang::CallExpr>(parent)) {
        const clang::FunctionDecl* const callee = call->getDirectCallee();
        if (callee == nullptr || !callee->hasPrototype()) {
          return false;
        }
        for (unsigned index = 0U; index < call->getNumArgs() && index < callee->getNumParams();
             ++index) {
          if (call->getArg(index) == node) {
            return points_to_const(callee->getParamDecl(index)->getType());
          }
        }
        return false;
      }
      if (llvm::isa<clang::ReturnStmt>(parent)) {
        return function != nullptr && points_to_const(function->getReturnType());
      }
      if (const auto* s = llvm::dyn_cast<clang::IfStmt>(parent)) {
        return s->getCond() == node;
      }
      if (const auto* s = llvm::dyn_cast<clang::WhileStmt>(parent)) {
        return s->getCond() == node;
      }
      if (const auto* s = llvm::dyn_cast<clang::ForStmt>(parent)) {
        return s->getCond() == node;
      }
      return false;
    }
  }

  // A pointer parameter whose every use only reads the pointee could point to
  // const. A function whose address is taken keeps its signature (a callback
  // type may dictate it) and is skipped; an externally visible function may be
  // bound to a function-pointer type elsewhere, so it is only "possible".
  void report_pointers_that_could_be_const() {
    for (const clang::FunctionDecl* function : defined_functions_) {
      if (address_taken_in_unit_.count(function->getCanonicalDecl()) != 0U) {
        continue;
      }
      for (const clang::ParmVarDecl* parameter : function->parameters()) {
        const clang::QualType type = parameter->getType();
        if (!type->isPointerType()) {
          continue;
        }
        const clang::QualType pointee = type->getPointeeType();
        if (pointee.isConstQualified() || pointee->isVoidType() ||
            pointee->isFunctionType() || pointee->isPointerType() ||
            pointee->isIncompleteType()) {
          continue;
        }
        const auto found = parameter_references_.find(parameter);
        if (found == parameter_references_.end() || found->second.empty()) {
          continue;
        }
        bool read_only = true;
        for (const clang::DeclRefExpr* use : found->second) {
          if (!pointer_value_is_only_read(use, function)) {
            read_only = false;
            break;
          }
        }
        if (read_only) {
          syntax(parameter->getLocation(), "misra-c2012-8.13-pointer-could-be-const",
                 true,
                 function->isExternallyVisible() ? FindingCertainty::Possible
                                                 : FindingCertainty::Definite);
        }
      }
    }
  }

  // 9.1 (approximation): an automatic scalar without an initializer that is
  // read before any assignment or address-of appears in source order. Branches
  // and loops are not modelled, so every finding is only "possible".
  void note_automatic_use(const clang::DeclRefExpr* reference) {
    const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
    if (variable == nullptr || !variable->hasLocalStorage() ||
        llvm::isa<clang::ParmVarDecl>(variable) || variable->getInit() != nullptr ||
        variable->getType().isVolatileQualified() ||
        !variable->getType()->isScalarType()) {
      return;
    }
    const clang::Stmt* current = reference;
    bool unevaluated = false;
    for (;;) {  // sizeof (x) does not read x
      const auto parents = ast_.getParents(*current);
      if (parents.empty() || parents[0].get<clang::Stmt>() == nullptr) {
        break;
      }
      current = parents[0].get<clang::Stmt>();
      if (llvm::isa<clang::UnaryExprOrTypeTraitExpr>(current)) {
        unevaluated = true;
        break;
      }
    }
    if (unevaluated) {
      return;
    }
    const clang::VarDecl* const key = variable->getCanonicalDecl();
    if (automatic_usage_.count(key) == 0U) {
      usage_order_.push_back(key);
    }
    UseEvents& events = automatic_usage_[key];
    current = reference;
    for (;;) {
      const auto parents = ast_.getParents(*current);
      const clang::Stmt* const parent =
          parents.empty() ? nullptr : parents[0].get<clang::Stmt>();
      if (parent != nullptr && llvm::isa<clang::ParenExpr>(parent)) {
        current = parent;
        continue;
      }
      if (const auto* binary = llvm::dyn_cast_or_null<clang::BinaryOperator>(parent);
          binary != nullptr && binary->isAssignmentOp() && binary->getLHS() == current) {
        if (binary->getOpcode() != clang::BO_Assign) {
          events.reads.push_back(reference->getBeginLoc());
        }
        events.writes.push_back(binary->getEndLoc());  // after the right side
        return;
      }
      if (const auto* unary = llvm::dyn_cast_or_null<clang::UnaryOperator>(parent)) {
        if (unary->getOpcode() == clang::UO_AddrOf) {
          events.writes.push_back(unary->getEndLoc());
          return;
        }
        if (unary->isIncrementDecrementOp()) {
          events.reads.push_back(reference->getBeginLoc());
          events.writes.push_back(unary->getEndLoc());
          return;
        }
      }
      events.reads.push_back(reference->getBeginLoc());
      return;
    }
  }

  void report_uninitialized_reads() {
    const auto earliest = [this](const std::vector<clang::SourceLocation>& locations) {
      clang::SourceLocation best = locations.front();
      for (const clang::SourceLocation candidate : locations) {
        if (sm_.isBeforeInTranslationUnit(candidate, best)) {
          best = candidate;
        }
      }
      return best;
    };
    for (const clang::VarDecl* variable : usage_order_) {
      const UseEvents& events = automatic_usage_[variable];
      if (events.reads.empty()) {
        continue;
      }
      const clang::SourceLocation first_read = earliest(events.reads);
      if (events.writes.empty() ||
          sm_.isBeforeInTranslationUnit(first_read, earliest(events.writes))) {
        syntax(first_read, "misra-c2012-9.1-uninitialized-read", true,
               FindingCertainty::Possible);
      }
    }
  }

  // 12.2: a constant shift count must lie in [0, width of the left operand).
  // Counts that are not constants cannot be judged here.
  void check_shift_count(const clang::BinaryOperator* expression,
                         const essential::Type& left,
                         const essential::Type& right) {
    const clang::BinaryOperatorKind op = expression->getOpcode();
    if (op != clang::BO_Shl && op != clang::BO_Shr && op != clang::BO_ShlAssign &&
        op != clang::BO_ShrAssign) {
      return;
    }
    if (!right.is_constant() || left.width == 0U ||
        left.category == essential::Category::Other ||
        left.category == essential::Category::Boolean ||
        left.category == essential::Category::Floating) {
      return;
    }
    const bool negative = right.constant->isSigned() && right.constant->isNegative();
    if (negative || right.constant->getActiveBits() > 32U ||
        right.constant->getZExtValue() >= left.width) {
      syntax(expression->getOperatorLoc(), "misra-c2012-12.2-shift-count-range");
    }
  }

  // 12.4: evaluating a constant unsigned expression must not wrap around.
  void check_constant_unsigned_wrap(const clang::BinaryOperator* expression) {
    const clang::BinaryOperatorKind op = expression->getOpcode();
    if ((op != clang::BO_Add && op != clang::BO_Sub && op != clang::BO_Mul) ||
        !expression->getType()->isUnsignedIntegerType()) {
      return;
    }
    const auto left = expression->getLHS()->getIntegerConstantExpr(ast_);
    const auto right = expression->getRHS()->getIntegerConstantExpr(ast_);
    if (!left || !right) {
      return;
    }
    const unsigned width = static_cast<unsigned>(ast_.getTypeSize(expression->getType()));
    const llvm::APInt a = left->extOrTrunc(width);
    const llvm::APInt b = right->extOrTrunc(width);
    bool overflow = false;
    switch (op) {
      case clang::BO_Add: (void)a.uadd_ov(b, overflow); break;
      case clang::BO_Sub: (void)a.usub_ov(b, overflow); break;
      default: (void)a.umul_ov(b, overflow); break;
    }
    if (overflow) {
      syntax(expression->getOperatorLoc(), "misra-c2012-12.4-constant-unsigned-wrap");
    }
  }

  void check_binary_types(const clang::BinaryOperator* expression) {
    using essential::Operand;
    const clang::BinaryOperatorKind op = expression->getOpcode();
    const clang::SourceLocation loc = expression->getOperatorLoc();
    if (op == clang::BO_Assign) {
      check_string_literal_target(expression->getLHS()->getType(), expression->getRHS(), loc);
      check_assignment(essential::of_expression(expression->getLHS(), ast_),
                       expression->getRHS(), loc);
      return;
    }
    if (op == clang::BO_Comma) {
      return;
    }
    const essential::Type left = essential::of_expression(expression->getLHS(), ast_);
    const essential::Type right = essential::of_expression(expression->getRHS(), ast_);
    check_shift_count(expression, left, right);
    check_constant_unsigned_wrap(expression);

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
    note_invariant_condition(condition, construct);
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

  struct UseEvents final {
    std::vector<clang::SourceLocation> reads;
    std::vector<clang::SourceLocation> writes;  // end of the writing expression
  };
  std::map<const clang::ParmVarDecl*, std::vector<const clang::DeclRefExpr*>>
      parameter_references_;
  std::set<const clang::FunctionDecl*> address_taken_in_unit_;
  std::map<const clang::VarDecl*, UseEvents> automatic_usage_;
  std::vector<const clang::VarDecl*> usage_order_;
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
  PreprocessorRecorder(clang::SourceManager& sm, clang::Preprocessor& pp,
                       Shared& shared)
      : sm_(sm), pp_(pp), lang_(pp.getLangOpts()), shared_(shared) {}

  // Rules 20.1, 20.3, 20.13: every project file is scanned once as raw tokens,
  // because these concern text the preprocessor itself consumes.
  void FileChanged(clang::SourceLocation loc, FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind type,
                   clang::FileID) override {
    if (reason != EnterFile || type != clang::SrcMgr::C_User) {
      return;
    }
    const clang::FileID file = sm_.getFileID(loc);
    if (scanned_.insert(file).second) {
      scan_file(file);
    }
  }

  void MacroUndefined(const clang::Token& name_token, const clang::MacroDefinition&,
                      const clang::MacroDirective*) override {
    report(name_token.getLocation(), "misra-c2012-20.5-undef");
  }

  void If(clang::SourceLocation, clang::SourceRange condition,
          ConditionValueKind) override {
    check_condition(condition);
  }

  void Elif(clang::SourceLocation, clang::SourceRange condition,
            ConditionValueKind value, clang::SourceLocation) override {
    if (value != CVK_NotEvaluated) {
      check_condition(condition);
    }
  }

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
    const std::string written = file_name.str();
    if (written.find_first_of("'\"\\") != std::string::npos ||
        written.find("//") != std::string::npos ||
        written.find("/*") != std::string::npos) {
      report(hash_loc, "misra-c2012-20.2-header-name-characters");
    }
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
    check_macro_definition(name_token, info);
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
  void report(const clang::SourceLocation raw, const std::string_view kind) {
    if (const auto location = locate(sm_, raw, false)) {
      if (shared_.fresh(std::string(kind) + '|' + location_text(*location))) {
        shared_.context.syntax.push_back({*location, kind});
      }
    }
  }

  static const std::set<std::string>& directive_names() {
    static const std::set<std::string> names{
        "define", "undef",  "include", "include_next", "import", "if",
        "ifdef",  "ifndef", "elif",    "elifdef",      "elifndef", "else",
        "endif",  "line",   "error",   "warning",      "pragma", "ident",
        "sccs",   "assert", "unassert", "include_macros"};
    return names;
  }

  void scan_file(const clang::FileID file) {
    bool invalid = false;
    const llvm::StringRef buffer = sm_.getBufferData(file, &invalid);
    if (invalid) {
      return;
    }
    // 4.2: trigraph sequences, wherever they occur in the text.
    for (std::size_t at = buffer.find("??"); at != llvm::StringRef::npos;
         at = buffer.find("??", at + 1U)) {
      if (at + 2U < buffer.size() &&
          std::string_view("=/'()!<>-").find(buffer[at + 2U]) != std::string_view::npos) {
        report(sm_.getLocForStartOfFile(file).getLocWithOffset(static_cast<int>(at)),
               "misra-c2012-4.2-trigraph");
      }
    }
    clang::Lexer lexer(sm_.getLocForStartOfFile(file), lang_, buffer.begin(),
                       buffer.begin(), buffer.end());
    lexer.SetCommentRetentionState(true);
    clang::Token pending;
    bool has_pending = false;
    const auto next = [&](clang::Token& token) {
      if (has_pending) {
        token = pending;
        has_pending = false;
        return true;
      }
      for (;;) {
        lexer.LexFromRawLexer(token);
        if (token.is(clang::tok::eof)) {
          return false;
        }
        if (!token.is(clang::tok::comment)) {
          return true;
        }
        check_comment(token);
      }
    };
    const auto push_back = [&](const clang::Token& token) {
      pending = token;
      has_pending = true;
    };

    clang::Token token;
    bool code_seen = false;  // anything other than a directive so far
    while (next(token)) {
      if (!(token.isAtStartOfLine() && token.is(clang::tok::hash))) {
        code_seen = true;
        continue;
      }
      const clang::SourceLocation hash_loc = token.getLocation();
      clang::Token name;
      if (!next(name)) {
        break;
      }
      if (name.isAtStartOfLine()) {  // a null directive is valid
        push_back(name);
        continue;
      }
      std::string directive;
      bool valid = name.is(clang::tok::numeric_constant);  // line marker
      if (name.is(clang::tok::raw_identifier)) {
        directive = name.getRawIdentifier().str();
        valid = directive_names().count(directive) != 0U;
      }
      if (!valid) {
        report(hash_loc, "misra-c2012-20.13-invalid-directive");
      }
      if (directive == "include" || directive == "include_next" ||
          directive == "import") {
        if (code_seen) {
          report(hash_loc, "misra-c2012-20.1-include-after-code");
        }
        clang::Token operand;
        if (next(operand)) {
          if (operand.isAtStartOfLine()) {
            push_back(operand);
            report(hash_loc, "misra-c2012-20.3-include-operand");
          } else if (!operand.isOneOf(clang::tok::less, clang::tok::string_literal)) {
            report(hash_loc, "misra-c2012-20.3-include-operand");
          }
        }
      }
      clang::Token rest;  // skip the remainder of the directive line
      while (next(rest)) {
        if (rest.isAtStartOfLine()) {
          push_back(rest);
          break;
        }
      }
    }
  }

  // 3.1: /* or // inside a comment. 3.2: a // comment continued by a backslash.
  void check_comment(const clang::Token& comment) {
    bool invalid = false;
    const char* const data = sm_.getCharacterData(comment.getLocation(), &invalid);
    if (invalid || data == nullptr) {
      return;
    }
    const std::string_view text(data, comment.getLength());
    const bool line_comment = text.substr(0U, 2U) == "//";
    std::string_view inner = text.substr(2U);
    if (!line_comment && inner.size() >= 2U && inner.substr(inner.size() - 2U) == "*/") {
      inner.remove_suffix(2U);
    }
    for (std::string_view marker : {std::string_view("/*"), std::string_view("//")}) {
      const std::size_t at = inner.find(marker);
      if (at != std::string_view::npos) {
        report(comment.getLocation().getLocWithOffset(static_cast<int>(at + 2U)),
               "misra-c2012-3.1-comment-marker-in-comment");
        break;
      }
    }
    if (line_comment && (text.find("\\\n") != std::string_view::npos ||
                         text.find("\\\r\n") != std::string_view::npos)) {
      report(comment.getLocation(), "misra-c2012-3.2-line-splice-in-comment");
    }
  }

  void check_macro_definition(const clang::Token& name_token,
                              const clang::MacroInfo* info) {
    const clang::SourceLocation definition = info->getDefinitionLoc();
    if (name_token.getIdentifierInfo()->isKeyword(lang_)) {
      report(definition, "misra-c2012-20.4-keyword-macro");
    }
    const auto tokens = info->tokens();
    const std::size_t count = tokens.size();
    const auto is_parameter = [&](const clang::Token& token) {
      const clang::IdentifierInfo* const identifier = token.getIdentifierInfo();
      if (identifier == nullptr) {
        return false;
      }
      for (const clang::IdentifierInfo* parameter : info->params()) {
        if (parameter == identifier) {
          return true;
        }
      }
      return info->isVariadic() && identifier->isStr("__VA_ARGS__");
    };
    const auto is_operator = [&](const std::size_t index) {
      return index < count && (tokens[index].is(clang::tok::hashhash) ||
                               (info->isFunctionLike() && tokens[index].is(clang::tok::hash)));
    };

    for (std::size_t index = 0U; index < count; ++index) {  // 20.10
      if (is_operator(index)) {
        report(tokens[index].getLocation(), "misra-c2012-20.10-hash-operator");
        break;
      }
    }
    if (!info->isFunctionLike()) {
      return;
    }
    std::map<const clang::IdentifierInfo*, std::pair<bool, bool>> usage;  // operand, other
    for (std::size_t index = 0U; index < count; ++index) {
      if (index + 2U < count && tokens[index].is(clang::tok::hash) &&
          is_parameter(tokens[index + 1U]) &&
          tokens[index + 2U].is(clang::tok::hashhash)) {
        report(tokens[index].getLocation(), "misra-c2012-20.11-stringize-then-paste");
      }
      if (!is_parameter(tokens[index])) {
        continue;
      }
      const bool operand = (index > 0U && is_operator(index - 1U)) ||
                           (index + 1U < count &&
                            tokens[index + 1U].is(clang::tok::hashhash));
      auto& flags = usage[tokens[index].getIdentifierInfo()];
      (operand ? flags.first : flags.second) = true;
      if (operand) {
        continue;
      }
      // 20.7: an expansion of a parameter is parenthesized. Taken as
      // parenthesized: directly between ( and ), or in an argument position
      // between ( or , and ) or ,.
      const bool before = index > 0U && (tokens[index - 1U].is(clang::tok::l_paren) ||
                                         tokens[index - 1U].is(clang::tok::comma));
      const bool after = index + 1U < count && (tokens[index + 1U].is(clang::tok::r_paren) ||
                                                tokens[index + 1U].is(clang::tok::comma));
      if (!(before && after)) {
        report(tokens[index].getLocation(), "misra-c2012-20.7-unparenthesized-parameter");
      }
    }
    for (const auto& entry : usage) {  // 20.12
      if (entry.second.first && entry.second.second) {
        report(definition, "misra-c2012-20.12-operand-and-expanded");
      }
    }
  }

  // 20.9: an identifier in an #if/#elif expression that is neither a macro
  // nor the operand of `defined` silently evaluates to 0.
  void check_condition(const clang::SourceRange range) {
    if (range.isInvalid() || !range.getBegin().isFileID() ||
        !range.getEnd().isFileID()) {
      return;
    }
    // Lex a private, NUL-terminated copy: the raw lexer relies on a terminator
    // that a slice of the file buffer does not have.
    bool invalid = false;
    const llvm::StringRef slice = clang::Lexer::getSourceText(
        clang::CharSourceRange::getTokenRange(range), sm_, lang_, &invalid);
    if (invalid || slice.empty()) {
      return;
    }
    const std::string text = slice.str();
    clang::Lexer lexer(range.getBegin(), lang_, text.data(), text.data(),
                       text.data() + text.size());
    clang::Token token;
    bool after_defined = false;
    for (;;) {
      lexer.LexFromRawLexer(token);
      if (token.is(clang::tok::eof)) {
        break;
      }
      if (!token.is(clang::tok::raw_identifier)) {
        if (!(after_defined && token.is(clang::tok::l_paren))) {
          after_defined = false;
        }
        continue;
      }
      const llvm::StringRef name = token.getRawIdentifier();
      if (name == "defined") {
        after_defined = true;
        continue;
      }
      if (after_defined) {
        after_defined = false;
        continue;
      }
      if (name.starts_with("__has_")) {
        break;  // __has_include(<x>) and friends: the rest is not an expression
      }
      if (!pp_.isMacroDefined(pp_.getIdentifierInfo(name))) {
        report(token.getLocation(), "misra-c2012-20.9-undefined-identifier");
      }
    }
  }

  void note_use(const clang::Token& name_token) {
    if (const clang::IdentifierInfo* const info = name_token.getIdentifierInfo()) {
      shared_.context.program.used_macros.insert(info->getName().str());
    }
  }

  clang::SourceManager& sm_;
  clang::Preprocessor& pp_;
  const clang::LangOptions& lang_;
  Shared& shared_;
  std::set<clang::FileID> scanned_;
};

// Records the compiler's own "type specifier missing" diagnostics (Rule 8.1)
// while still printing every diagnostic as usual.
class DiagnosticRecorder final : public clang::DiagnosticConsumer {
 public:
  explicit DiagnosticRecorder(Shared& shared)
      : shared_(shared),
        options_(new clang::DiagnosticOptions()),
        printer_(llvm::errs(), options_.get()) {}

  void BeginSourceFile(const clang::LangOptions& lang,
                       const clang::Preprocessor* preprocessor) override {
    printer_.BeginSourceFile(lang, preprocessor);
  }
  void EndSourceFile() override { printer_.EndSourceFile(); }
  void finish() override { printer_.finish(); }
  void clear() override {
    clang::DiagnosticConsumer::clear();
    printer_.clear();
  }

  void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                        const clang::Diagnostic& info) override {
    clang::DiagnosticConsumer::HandleDiagnostic(level, info);
    printer_.HandleDiagnostic(level, info);
    const unsigned id = info.getID();
    if ((id == clang::diag::ext_missing_type_specifier ||
         id == clang::diag::ext_param_not_declared) &&
        info.hasSourceManager()) {
      if (const auto location =
              locate(info.getSourceManager(), info.getLocation(), false)) {
        if (shared_.fresh("implicit-int|" + location_text(*location))) {
          shared_.context.syntax.push_back(
              {*location, "misra-c2012-8.1-implicit-int"});
        }
      }
    }
  }

 private:
  Shared& shared_;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> options_;
  clang::TextDiagnosticPrinter printer_;
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
                                               compiler.getPreprocessor(),
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
      {"-Wimplicit-int", "-Wno-error=implicit-function-declaration",
       "-Wno-error=implicit-int",
       "-Wno-error=return-type", "-Wno-error=int-conversion",
       "-Wno-error=incompatible-pointer-types",
       "-Wno-error=incompatible-function-pointer-types"},
      clang::tooling::ArgumentInsertPosition::END));
  FactActionFactory factory(shared);
  DiagnosticRecorder diagnostics(shared);
  tool.setDiagnosticConsumer(&diagnostics);
  if (tool.run(&factory) != 0) {
    result.success = false;
    result.error_message = "Clang could not analyze one or more translation units";
  }
  return result;
}

}  // namespace misra
