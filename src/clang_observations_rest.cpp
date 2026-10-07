// Second-wave observations: undefined-behavior indicators (1.3), brace
// elision (9.2), precedence (12.1), constant wrap-around (12.4), evaluation
// order (13.2, 13.3), array arguments (17.5), pointer rules (18.x), overlap
// (19.1), const-correctness of parameters (8.13) and resource rules (22.x).

#include <set>
#include <string>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "llvm/ADT/FoldingSet.h"
#include "misra/observation_internal.hpp"

namespace misra {
namespace {

const clang::VarDecl* referenced_variable(const clang::Expr* expression) {
  const auto* reference =
      llvm::dyn_cast<clang::DeclRefExpr>(expression->IgnoreParenImpCasts());
  return reference == nullptr ? nullptr
                              : llvm::dyn_cast<clang::VarDecl>(reference->getDecl());
}

std::string callee_name(const clang::CallExpr* call) {
  const clang::FunctionDecl* callee = call->getDirectCallee();
  return callee == nullptr ? std::string{} : callee->getNameAsString();
}

bool is_allocator(const std::string& name) {
  return (name == "malloc") || (name == "calloc") || (name == "realloc") ||
         (name == "aligned_alloc") || (name == "fopen") || (name == "freopen") ||
         (name == "tmpfile");
}

int precedence_level(const clang::BinaryOperatorKind code) {
  switch (code) {
    case clang::BO_Mul: case clang::BO_Div: case clang::BO_Rem: return 1;
    case clang::BO_Add: case clang::BO_Sub: return 2;
    case clang::BO_Shl: case clang::BO_Shr: return 3;
    case clang::BO_LT: case clang::BO_GT: case clang::BO_LE: case clang::BO_GE: return 4;
    case clang::BO_EQ: case clang::BO_NE: return 5;
    case clang::BO_And: return 6;
    case clang::BO_Xor: return 7;
    case clang::BO_Or: return 8;
    case clang::BO_LAnd: return 9;
    case clang::BO_LOr: return 10;
    default: return 0;
  }
}

// Collects references to one variable and the calls/assignments around them.
struct VariableEvents final : clang::RecursiveASTVisitor<VariableEvents> {
  explicit VariableEvents(const clang::VarDecl* target) : variable(target) {}
  bool VisitDeclRefExpr(clang::DeclRefExpr* reference) {
    if (reference->getDecl() == variable) {
      references.push_back(reference);
    }
    return true;
  }
  const clang::VarDecl* variable;
  std::vector<const clang::DeclRefExpr*> references;
};

struct AssignmentFinder final : clang::RecursiveASTVisitor<AssignmentFinder> {
  explicit AssignmentFinder(const clang::VarDecl* target) : variable(target) {}
  bool VisitBinaryOperator(clang::BinaryOperator* op) {
    if (op->isAssignmentOp() && (referenced_variable(op->getLHS()) == variable)) {
      assigned = true;
    }
    return true;
  }
  const clang::VarDecl* variable;
  bool assigned = false;
};

struct CallFinder final : clang::RecursiveASTVisitor<CallFinder> {
  bool VisitCallExpr(clang::CallExpr* call) {
    calls.push_back(call);
    return true;
  }
  std::vector<const clang::CallExpr*> calls;
};

class RestVisitor final : public clang::RecursiveASTVisitor<RestVisitor> {
 public:
  RestVisitor(clang::ASTContext& ast, const Recorder& recorder)
      : ast_(ast), recorder_(recorder), sm_(ast.getSourceManager()) {}

  // --- function level -------------------------------------------------------

  bool VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function->isImplicit() || !function->isThisDeclarationADefinition() ||
        (function->getBody() == nullptr) ||
        !sm_.isWrittenInMainFile(sm_.getExpansionLoc(function->getLocation()))) {
      return true;
    }
    check_pointer_parameters(function);
    check_resources(function);
    return true;
  }

  // --- statements -----------------------------------------------------------

  bool VisitCompoundStmt(clang::CompoundStmt* block) {
    std::vector<const clang::Stmt*> children(block->body_begin(), block->body_end());
    for (std::size_t i = 0U; i < children.size(); ++i) {
      const auto* statement_expr = llvm::dyn_cast<clang::Expr>(children[i]);
      const auto* call = statement_expr == nullptr
                             ? nullptr
                             : llvm::dyn_cast<clang::CallExpr>(statement_expr->IgnoreParenCasts());
      if ((call == nullptr) || (call->getNumArgs() < 1U)) {
        continue;
      }
      const std::string name = callee_name(call);
      if ((name != "free") && (name != "fclose")) {
        continue;
      }
      const clang::VarDecl* variable = referenced_variable(call->getArg(0));
      if (variable == nullptr) {
        continue;
      }
      for (std::size_t j = i + 1U; j < children.size(); ++j) {
        AssignmentFinder assignment(variable);
        assignment.TraverseStmt(const_cast<clang::Stmt*>(children[j]));
        if (assignment.assigned) {
          break;
        }
        if (name == "free") {
          CallFinder calls;
          calls.TraverseStmt(const_cast<clang::Stmt*>(children[j]));
          bool reported = false;
          for (const clang::CallExpr* later : calls.calls) {
            if ((callee_name(later) == "free") && (later->getNumArgs() > 0U) &&
                (referenced_variable(later->getArg(0)) == variable)) {
              recorder_.add("double-free", later->getBeginLoc());
              reported = true;
            }
          }
          if (reported) {
            break;
          }
        } else {
          VariableEvents events(variable);
          events.TraverseStmt(const_cast<clang::Stmt*>(children[j]));
          if (!events.references.empty()) {
            recorder_.add("use-after-close", events.references.front()->getLocation());
            break;
          }
        }
      }
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt* statement) {
    if ((statement->getRetValue() != nullptr) &&
        address_of_automatic(statement->getRetValue())) {
      recorder_.add("address-of-local-escapes", statement->getReturnLoc());
    }
    return true;
  }

  bool VisitInitListExpr(clang::InitListExpr* list) {
    const clang::InitListExpr* syntactic =
        (list->isSemanticForm() && (list->getSyntacticForm() != nullptr))
            ? list->getSyntacticForm()
            : list;
    if (!seen_lists_.insert(syntactic).second) {
      return true;
    }
    const clang::QualType type = list->getType();
    const auto aggregate = [](const clang::QualType candidate) {
      return candidate->isArrayType() || candidate->isRecordType();
    };
    const auto elided = [&](const clang::QualType element, const clang::Expr* init) {
      const clang::Expr* stripped = init->IgnoreParenImpCasts();
      if (!aggregate(element) || llvm::isa<clang::InitListExpr>(stripped) ||
          llvm::isa<clang::DesignatedInitExpr>(stripped) ||
          llvm::isa<clang::ImplicitValueInitExpr>(stripped)) {
        return false;
      }
      if (llvm::isa<clang::StringLiteral>(stripped) && element->isArrayType()) {
        return false;
      }
      return ast_.getCanonicalType(stripped->getType()) != ast_.getCanonicalType(element);
    };
    const auto is_zero = [&](const clang::Expr* init) {
      const auto* literal = llvm::dyn_cast<clang::IntegerLiteral>(init->IgnoreParenImpCasts());
      return (literal != nullptr) && literal->getValue().isZero();
    };
    if ((syntactic->getNumInits() == 1U) && is_zero(syntactic->getInit(0))) {
      return true;  // the {0} idiom
    }
    if (const clang::ArrayType* array = ast_.getAsArrayType(type)) {
      for (const clang::Expr* init : syntactic->inits()) {
        if (elided(array->getElementType(), init)) {
          recorder_.add("brace-elision", init->getBeginLoc());
          break;
        }
      }
    } else if (const auto* record = type->getAsRecordDecl()) {
      auto field = record->field_begin();
      for (const clang::Expr* init : syntactic->inits()) {
        while ((field != record->field_end()) && field->isUnnamedBitfield()) {
          ++field;
        }
        if (field == record->field_end()) {
          break;
        }
        if (elided(field->getType(), init)) {
          recorder_.add("brace-elision", init->getBeginLoc());
          break;
        }
        ++field;
      }
    }
    return true;
  }

  bool VisitExpr(clang::Expr* expression) {
    const auto parents = ast_.getParents(*expression);
    if (parents.empty() || (parents[0].get<clang::Expr>() != nullptr)) {
      return true;
    }
    if (!sm_.isWrittenInMainFile(sm_.getExpansionLoc(expression->getBeginLoc()))) {
      return true;
    }
    check_sequencing(expression);
    return true;
  }

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* subscript) {
    const clang::ConstantArrayType* array =
        ast_.getAsConstantArrayType(subscript->getBase()->IgnoreParenImpCasts()->getType());
    clang::Expr::EvalResult index;
    if ((array != nullptr) && subscript->getIdx()->EvaluateAsInt(index, ast_)) {
      const llvm::APSInt value = index.Val.getInt();
      if (value.isNegative() || (value.getZExtValue() >= array->getSize().getZExtValue())) {
        recorder_.add("array-index-constant-out-of-range", subscript->getExprLoc());
        recorder_.add("pointer-arithmetic-out-of-bounds", subscript->getExprLoc());
      }
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator* op) {
    if ((op->getOpcode() == clang::UO_Deref) &&
        op->getSubExpr()->isNullPointerConstant(ast_, clang::Expr::NPC_ValueDependentIsNull) !=
            clang::Expr::NPCK_NotNull) {
      recorder_.add("null-pointer-dereference", op->getOperatorLoc());
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator* op) {
    const clang::SourceLocation location = op->getOperatorLoc();
    const clang::BinaryOperatorKind code = op->getOpcode();

    // Rule 1.3: constant division by zero.
    if (((code == clang::BO_Div) || (code == clang::BO_Rem) ||
         (code == clang::BO_DivAssign) || (code == clang::BO_RemAssign)) &&
        op->getRHS()->getType()->isIntegerType()) {
      clang::Expr::EvalResult divisor;
      if (op->getRHS()->EvaluateAsInt(divisor, ast_) && divisor.Val.getInt().isZero()) {
        recorder_.add("division-by-zero", location);
      }
    }

    // Rule 12.1: unparenthesized operands with a different precedence level.
    const int level = precedence_level(code);
    if (level != 0) {
      for (const clang::Expr* operand : {op->getLHS(), op->getRHS()}) {
        const auto* child = llvm::dyn_cast<clang::BinaryOperator>(operand->IgnoreImpCasts());
        if ((child != nullptr) && (precedence_level(child->getOpcode()) != 0) &&
            (precedence_level(child->getOpcode()) != level)) {
          recorder_.add("implicit-precedence", location);
          break;
        }
      }
    }

    // Rule 12.4: constant unsigned expressions that wrap around.
    if (((code == clang::BO_Add) || (code == clang::BO_Sub) || (code == clang::BO_Mul)) &&
        op->getType()->isUnsignedIntegerType() && op->isIntegerConstantExpr(ast_)) {
      clang::Expr::EvalResult left;
      clang::Expr::EvalResult right;
      if (op->getLHS()->EvaluateAsInt(left, ast_) && op->getRHS()->EvaluateAsInt(right, ast_)) {
        const unsigned int width = ast_.getTypeSize(op->getType());
        llvm::APSInt a = left.Val.getInt().extOrTrunc(256);
        llvm::APSInt b = right.Val.getInt().extOrTrunc(256);
        a.setIsSigned(true);
        b.setIsSigned(true);
        llvm::APSInt exact(256, false);
        bool computable = true;
        if (code == clang::BO_Add) exact = a + b;
        else if (code == clang::BO_Sub) exact = a - b;
        else if (code == clang::BO_Mul) exact = a * b;
        else if (b.getExtValue() < 128) exact = a << static_cast<unsigned>(b.getExtValue());
        else computable = false;
        exact.setIsSigned(true);
        const llvm::APSInt limit = llvm::APSInt(llvm::APInt::getOneBitSet(256, width), false);
        if (computable && (exact.isNegative() || (exact >= llvm::APSInt(limit, false)))) {
          recorder_.add("constant-wraparound", location);
        }
      }
    }

    // Rule 18.1: constant offsets outside an array (pointer arithmetic).
    if ((code == clang::BO_Add) || (code == clang::BO_Sub) ||
        (code == clang::BO_AddAssign) || (code == clang::BO_SubAssign)) {
      const clang::ConstantArrayType* array =
          ast_.getAsConstantArrayType(op->getLHS()->IgnoreParenImpCasts()->getType());
      clang::Expr::EvalResult offset;
      if ((array != nullptr) && op->getRHS()->EvaluateAsInt(offset, ast_)) {
        long long value = offset.Val.getInt().getExtValue();
        if ((code == clang::BO_Sub) || (code == clang::BO_SubAssign)) {
          value = -value;
        }
        if ((value < 0) || (static_cast<unsigned long long>(value) > array->getSize().getZExtValue())) {
          recorder_.add("pointer-arithmetic-out-of-bounds", location);
        }
      }
    }

    // Rules 18.2 and 18.3: pointers into two different declared objects.
    if ((code == clang::BO_Sub) || (code == clang::BO_LT) || (code == clang::BO_GT) ||
        (code == clang::BO_LE) || (code == clang::BO_GE)) {
      if (op->getLHS()->getType()->isPointerType() && op->getRHS()->getType()->isPointerType()) {
        const clang::VarDecl* left = pointed_object(op->getLHS());
        const clang::VarDecl* right = pointed_object(op->getRHS());
        if ((left != nullptr) && (right != nullptr) && (left != right)) {
          recorder_.add(code == clang::BO_Sub ? "pointer-subtraction-different-objects"
                                              : "pointer-comparison-different-objects",
                        location);
        }
      }
    }

    // Rule 19.1 (self assignment) and Rule 18.6 (escaping local address).
    if (code == clang::BO_Assign) {
      if (same_expression(op->getLHS(), op->getRHS())) {
        recorder_.add("overlapping-copy", location);
      }
      if (address_of_automatic(op->getRHS()) && !is_local_target(op->getLHS())) {
        recorder_.add("address-of-local-escapes", location);
      }
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr* call) {
    const clang::FunctionDecl* callee = call->getDirectCallee();
    if (callee == nullptr) {
      return true;
    }
    const std::string name = callee->getNameAsString();

    // Rule 17.5: array parameters of a stated size.
    for (unsigned int i = 0U; (i < call->getNumArgs()) && (i < callee->getNumParams()); ++i) {
      const clang::ConstantArrayType* parameter =
          ast_.getAsConstantArrayType(callee->getParamDecl(i)->getOriginalType());
      if (parameter == nullptr) {
        continue;
      }
      const clang::Expr* argument = call->getArg(i);
      const clang::ConstantArrayType* actual =
          ast_.getAsConstantArrayType(argument->IgnoreParenImpCasts()->getType());
      const bool null_argument =
          argument->isNullPointerConstant(ast_, clang::Expr::NPC_ValueDependentIsNull) !=
          clang::Expr::NPCK_NotNull;
      if (null_argument ||
          ((actual != nullptr) && (actual->getSize().getZExtValue() < parameter->getSize().getZExtValue()))) {
        recorder_.add("array-argument-too-small", argument->getBeginLoc());
      }
    }

    // Rule 19.1: identical source and destination of a copy.
    if (((name == "memcpy") || (name == "strcpy") || (name == "strncpy") ||
         (name == "strcat") || (name == "strncat") || (name == "wmemcpy")) &&
        (call->getNumArgs() >= 2U) && same_expression(call->getArg(0), call->getArg(1))) {
      recorder_.add("overlapping-copy", call->getBeginLoc());
    }

    // Rule 22.2: freeing something that was not allocated.
    if ((name == "free") && (call->getNumArgs() == 1U)) {
      const clang::Expr* argument = call->getArg(0)->IgnoreParenImpCasts();
      const auto* address = llvm::dyn_cast<clang::UnaryOperator>(argument);
      const clang::VarDecl* variable = referenced_variable(argument);
      const bool array_object =
          (variable != nullptr) && variable->getType()->isArrayType();
      if (((address != nullptr) && (address->getOpcode() == clang::UO_AddrOf)) || array_object ||
          llvm::isa<clang::StringLiteral>(argument)) {
        recorder_.add("free-of-non-heap-object", call->getBeginLoc());
      }
    }
    return true;
  }

 private:
  // --- helpers --------------------------------------------------------------

  [[nodiscard]] bool same_expression(const clang::Expr* a, const clang::Expr* b) const {
    llvm::FoldingSetNodeID first;
    llvm::FoldingSetNodeID second;
    a->IgnoreParenImpCasts()->Profile(first, ast_, true);
    b->IgnoreParenImpCasts()->Profile(second, ast_, true);
    return first == second;
  }

  // The declared object a pointer expression is derived from, when obvious.
  [[nodiscard]] const clang::VarDecl* pointed_object(const clang::Expr* expression) const {
    const clang::Expr* stripped = expression->IgnoreParenImpCasts();
    if (const auto* variable = referenced_variable(stripped)) {
      return variable->getType()->isArrayType() ? variable : nullptr;
    }
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(stripped)) {
      if (unary->getOpcode() == clang::UO_AddrOf) {
        const clang::Expr* operand = unary->getSubExpr()->IgnoreParenImpCasts();
        if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(operand)) {
          return pointed_object(subscript->getBase());
        }
        return referenced_variable(operand);
      }
    }
    if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(stripped)) {
      if (((binary->getOpcode() == clang::BO_Add) || (binary->getOpcode() == clang::BO_Sub)) &&
          binary->getLHS()->getType()->isPointerType()) {
        return pointed_object(binary->getLHS());
      }
    }
    return nullptr;
  }

  [[nodiscard]] static bool is_automatic(const clang::VarDecl* variable) {
    return (variable != nullptr) && variable->hasLocalStorage() &&
           !llvm::isa<clang::ParmVarDecl>(variable);
  }

  // The address of an automatic object (directly or via array decay).
  [[nodiscard]] bool address_of_automatic(const clang::Expr* expression) const {
    const clang::Expr* stripped = expression->IgnoreParenCasts();
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(stripped)) {
      if (unary->getOpcode() == clang::UO_AddrOf) {
        const clang::Expr* operand = unary->getSubExpr()->IgnoreParenImpCasts();
        while (true) {
          if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(operand)) {
            if (member->isArrow()) {
              return false;
            }
            operand = member->getBase()->IgnoreParenImpCasts();
          } else if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(operand)) {
            operand = subscript->getBase()->IgnoreParenImpCasts();
            if (!operand->getType()->isArrayType()) {
              return false;
            }
          } else {
            break;
          }
        }
        return is_automatic(referenced_variable(operand));
      }
    }
    if (const auto* variable = referenced_variable(stripped)) {
      return is_automatic(variable) && variable->getType()->isArrayType();
    }
    if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(stripped)) {
      if (((binary->getOpcode() == clang::BO_Add) || (binary->getOpcode() == clang::BO_Sub)) &&
          binary->getLHS()->getType()->isPointerType()) {
        return address_of_automatic(binary->getLHS());
      }
    }
    return false;
  }

  // Whether the assignment target has automatic storage duration.
  [[nodiscard]] bool is_local_target(const clang::Expr* target) const {
    const clang::Expr* current = target->IgnoreParenImpCasts();
    while (true) {
      if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(current)) {
        if (member->isArrow()) {
          return false;
        }
        current = member->getBase()->IgnoreParenImpCasts();
      } else if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(current)) {
        current = subscript->getBase()->IgnoreParenImpCasts();
        if (!current->getType()->isArrayType()) {
          return false;
        }
      } else {
        break;
      }
    }
    const clang::VarDecl* variable = referenced_variable(current);
    return (variable != nullptr) && variable->hasLocalStorage();
  }

  // --- Rule 8.13 ------------------------------------------------------------

  [[nodiscard]] bool lvalue_read_only(const clang::Stmt* lvalue) const {
    const auto parents = ast_.getParents(*lvalue);
    if (parents.size() != 1U) {
      return false;
    }
    const clang::Stmt* parent = parents[0].get<clang::Stmt>();
    if (parent == nullptr) {
      return false;
    }
    if (const auto* cast = llvm::dyn_cast<clang::ImplicitCastExpr>(parent)) {
      return cast->getCastKind() == clang::CK_LValueToRValue;
    }
    if (llvm::isa<clang::UnaryExprOrTypeTraitExpr>(parent)) {
      return true;
    }
    if (llvm::isa<clang::ParenExpr>(parent)) {
      return lvalue_read_only(parent);
    }
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(parent)) {
      return !member->isArrow() && lvalue_read_only(parent);
    }
    if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(parent)) {
      return (subscript->getBase()->IgnoreParenImpCasts() == lvalue) &&
             lvalue_read_only(parent);
    }
    return false;
  }

  [[nodiscard]] bool pointer_use_read_only(const clang::Stmt* reference) const {
    const clang::Stmt* current = reference;
    while (true) {
      const auto parents = ast_.getParents(*current);
      if (parents.size() != 1U) {
        return false;
      }
      const clang::Stmt* parent = parents[0].get<clang::Stmt>();
      if (parent == nullptr) {
        return false;
      }
      if (const auto* cast = llvm::dyn_cast<clang::ImplicitCastExpr>(parent)) {
        if ((cast->getCastKind() == clang::CK_LValueToRValue) ||
            (cast->getCastKind() == clang::CK_NoOp)) {
          current = parent;
          continue;
        }
        return cast->getCastKind() == clang::CK_PointerToBoolean;
      }
      if (llvm::isa<clang::ParenExpr>(parent)) {
        current = parent;
        continue;
      }
      if (const auto* subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(parent)) {
        return (subscript->getBase()->IgnoreParenImpCasts() == reference) &&
               lvalue_read_only(parent);
      }
      if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(parent)) {
        if (unary->getOpcode() == clang::UO_Deref) {
          return lvalue_read_only(parent);
        }
        return unary->getOpcode() == clang::UO_LNot;
      }
      if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(parent)) {
        return member->isArrow() && lvalue_read_only(parent);
      }
      if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(parent)) {
        switch (binary->getOpcode()) {
          case clang::BO_EQ: case clang::BO_NE: case clang::BO_LT: case clang::BO_GT:
          case clang::BO_LE: case clang::BO_GE: case clang::BO_LAnd: case clang::BO_LOr:
            return true;
          default:
            return false;
        }
      }
      if (const auto* call = llvm::dyn_cast<clang::CallExpr>(parent)) {
        const clang::FunctionDecl* callee = call->getDirectCallee();
        if (callee == nullptr) {
          return false;
        }
        for (unsigned int i = 0U; (i < call->getNumArgs()) && (i < callee->getNumParams()); ++i) {
          if (static_cast<const clang::Stmt*>(call->getArg(i)->IgnoreParenImpCasts()) ==
              reference) {
            const clang::QualType type = callee->getParamDecl(i)->getType();
            return type->isPointerType() && type->getPointeeType().isConstQualified();
          }
        }
        return false;
      }
      return false;
    }
  }

  void check_pointer_parameters(clang::FunctionDecl* function) {
    for (const clang::ParmVarDecl* parameter : function->parameters()) {
      const clang::QualType type = parameter->getType();
      if (!type->isPointerType() || parameter->getName().empty()) {
        continue;
      }
      const clang::QualType pointee = type->getPointeeType();
      if (pointee.isConstQualified() || pointee->isVoidType() || pointee->isFunctionType() ||
          pointee->isPointerType()) {
        continue;
      }
      VariableEvents events(parameter);
      events.TraverseStmt(function->getBody());
      if (events.references.empty()) {
        continue;
      }
      bool read_only = true;
      for (const clang::DeclRefExpr* reference : events.references) {
        read_only = read_only && pointer_use_read_only(reference);
      }
      if (read_only) {
        recorder_.add("pointer-could-be-const", parameter->getLocation(),
                      parameter->getNameAsString());
      }
    }
  }

  // --- Rules 22.1, 22.3, 22.4 -----------------------------------------------

  void check_resources(clang::FunctionDecl* function) {
    struct Allocation {
      const clang::VarDecl* variable;
      clang::SourceLocation location;
    };
    struct Finder final : clang::RecursiveASTVisitor<Finder> {
      bool VisitVarDecl(clang::VarDecl* declaration) {
        if (declaration->hasInit()) {
          note(declaration, declaration->getInit(), declaration->getLocation());
        }
        return true;
      }
      bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (op->getOpcode() == clang::BO_Assign) {
          note(referenced_variable(op->getLHS()), op->getRHS(), op->getOperatorLoc());
        }
        return true;
      }
      bool VisitCallExpr(clang::CallExpr* call) {
        calls.push_back(call);
        return true;
      }
      void note(const clang::VarDecl* variable, const clang::Expr* value,
                clang::SourceLocation location) {
        if (variable == nullptr) {
          return;
        }
        if (const auto* call = llvm::dyn_cast<clang::CallExpr>(value->IgnoreParenCasts())) {
          if (is_allocator(callee_name(call))) {
            allocations.push_back({variable, location, call});
          }
        }
      }
      struct Entry {
        const clang::VarDecl* variable;
        clang::SourceLocation location;
        const clang::CallExpr* call;
      };
      std::vector<Entry> allocations;
      std::vector<const clang::CallExpr*> calls;
    } finder;
    finder.TraverseStmt(function->getBody());

    // Rule 22.1: an allocation that is neither released nor handed on.
    std::set<const clang::VarDecl*> reported;
    for (const auto& allocation : finder.allocations) {
      if (!reported.insert(allocation.variable).second) {
        continue;
      }
      VariableEvents events(allocation.variable);
      events.TraverseStmt(function->getBody());
      bool released_or_escaped = false;
      for (const clang::DeclRefExpr* reference : events.references) {
        const clang::Stmt* current = reference;
        bool decided = false;
        while (!decided) {
          const auto parents = ast_.getParents(*current);
          const clang::Stmt* parent = parents.empty() ? nullptr : parents[0].get<clang::Stmt>();
          if (parent == nullptr) {
            released_or_escaped = true;  // initializer of another declaration
            break;
          }
          if (llvm::isa<clang::ParenExpr>(parent) ||
              (llvm::isa<clang::ImplicitCastExpr>(parent) &&
               (llvm::cast<clang::ImplicitCastExpr>(parent)->getCastKind() ==
                clang::CK_LValueToRValue))) {
            current = parent;
            continue;
          }
          decided = true;
          if (const auto* cast = llvm::dyn_cast<clang::CStyleCastExpr>(parent);
              (cast != nullptr) && cast->getType()->isVoidType()) {
            continue;  // (void)p is not an escape
          }
          if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(parent)) {
            const bool comparison =
                (binary->getOpcode() == clang::BO_EQ) || (binary->getOpcode() == clang::BO_NE);
            const bool assigned_to =
                binary->isAssignmentOp() && (referenced_variable(binary->getLHS()) == allocation.variable) &&
                (binary->getLHS()->IgnoreParenImpCasts() == reference);
            released_or_escaped = released_or_escaped || (!comparison && !assigned_to);
          } else if (llvm::isa<clang::UnaryOperator>(parent) || llvm::isa<clang::MemberExpr>(parent) ||
                     llvm::isa<clang::ArraySubscriptExpr>(parent) ||
                     llvm::isa<clang::ImplicitCastExpr>(parent) || llvm::isa<clang::IfStmt>(parent)) {
            const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(parent);
            released_or_escaped = released_or_escaped ||
                                  ((unary != nullptr) && (unary->getOpcode() == clang::UO_AddrOf));
          } else {
            released_or_escaped = true;  // call argument, return, init list, ...
          }
        }
      }
      if (!released_or_escaped) {
        recorder_.add("resource-not-released", allocation.location,
                      callee_name(allocation.call));
      }
    }

    // Rule 22.3: the same file opened twice with writing involved.
    std::map<std::string, std::vector<std::pair<bool, clang::SourceLocation>>> files;
    for (const clang::CallExpr* call : finder.calls) {
      if ((callee_name(call) != "fopen") || (call->getNumArgs() < 2U)) {
        continue;
      }
      const auto* path = llvm::dyn_cast<clang::StringLiteral>(call->getArg(0)->IgnoreParenImpCasts());
      const auto* mode = llvm::dyn_cast<clang::StringLiteral>(call->getArg(1)->IgnoreParenImpCasts());
      if ((path == nullptr) || (mode == nullptr)) {
        continue;
      }
      const std::string text = mode->getString().str();
      const bool writes = text.find_first_of("wa+") != std::string::npos;
      files[path->getString().str()].emplace_back(writes, call->getBeginLoc());
    }
    for (const auto& [path, opens] : files) {
      bool any_write = false;
      for (const auto& open : opens) {
        any_write = any_write || open.first;
      }
      if ((opens.size() > 1U) && any_write) {
        recorder_.add("file-opened-twice", opens[1].second, path);
      }
    }

    // Rule 22.4: a stream opened only for reading is written to.
    std::map<const clang::VarDecl*, bool> read_only_stream;
    for (const auto& allocation : finder.allocations) {
      if (callee_name(allocation.call) != "fopen" || allocation.call->getNumArgs() < 2U) {
        continue;
      }
      const auto* mode = llvm::dyn_cast<clang::StringLiteral>(
          allocation.call->getArg(1)->IgnoreParenImpCasts());
      const bool reading_only = (mode != nullptr) &&
                                (mode->getString().str().find_first_of("wa+") == std::string::npos);
      const auto found = read_only_stream.find(allocation.variable);
      read_only_stream[allocation.variable] =
          (found == read_only_stream.end()) ? reading_only : (found->second && reading_only);
    }
    for (const clang::CallExpr* call : finder.calls) {
      const std::string name = callee_name(call);
      int stream_index = -1;
      if ((name == "fprintf") || (name == "vfprintf")) stream_index = 0;
      else if ((name == "fputs") || (name == "fputc") || (name == "putc")) stream_index = 1;
      else if (name == "fwrite") stream_index = 3;
      if ((stream_index < 0) || (call->getNumArgs() <= static_cast<unsigned>(stream_index))) {
        continue;
      }
      const clang::VarDecl* stream = referenced_variable(call->getArg(static_cast<unsigned>(stream_index)));
      const auto found = read_only_stream.find(stream);
      if ((stream != nullptr) && (found != read_only_stream.end()) && found->second) {
        recorder_.add("write-to-read-only-stream", call->getBeginLoc());
      }
    }
  }

  // --- Rules 13.2 and 13.3 --------------------------------------------------

  struct Access {
    const clang::VarDecl* variable;
    bool write;
    int begin;
    int end;
    std::vector<std::pair<const void*, int>> path;
    clang::SourceLocation location;
  };

  struct Collector {
    explicit Collector(const RestVisitor& owner) : self(owner) {}

    void collect(const clang::Stmt* statement) {
      if (statement == nullptr) {
        return;
      }
      const int begin = counter++;
      if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(statement)) {
        const auto code = binary->getOpcode();
        if ((code == clang::BO_LAnd) || (code == clang::BO_LOr) || (code == clang::BO_Comma)) {
          path.emplace_back(statement, 0);
          collect(binary->getLHS());
          path.back().second = 1;
          collect(binary->getRHS());
          path.pop_back();
          return;
        }
        if (binary->isAssignmentOp()) {
          const clang::VarDecl* variable = referenced_variable(binary->getLHS());
          const std::size_t index = accesses.size();
          if (variable != nullptr) {
            accesses.push_back({variable, true, begin, 0, path, binary->getOperatorLoc()});
          }
          effects.push_back(binary);
          collect(binary->getRHS());
          if (variable == nullptr) {
            collect(binary->getLHS());
          }
          if (variable != nullptr) {
            accesses[index].end = counter;
          }
          return;
        }
      }
      if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(statement)) {
        if (unary->isIncrementDecrementOp()) {
          const clang::VarDecl* variable = referenced_variable(unary->getSubExpr());
          effects.push_back(unary);
          increments.push_back(unary);
          if (variable != nullptr) {
            accesses.push_back({variable, true, begin, counter + 2, path, unary->getOperatorLoc()});
            counter += 2;
            return;
          }
        }
      }
      if (const auto* conditional = llvm::dyn_cast<clang::ConditionalOperator>(statement)) {
        path.emplace_back(statement, 0);
        collect(conditional->getCond());
        path.back().second = 1;
        collect(conditional->getTrueExpr());
        path.back().second = 2;
        collect(conditional->getFalseExpr());
        path.pop_back();
        return;
      }
      if (llvm::isa<clang::CallExpr>(statement)) {
        effects.push_back(statement);
      }
      if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(statement)) {
        if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl())) {
          if (!variable->getType().isVolatileQualified() && !variable->getType()->isArrayType()) {
            accesses.push_back({variable, false, begin, begin + 1, path, reference->getLocation()});
          }
        }
        return;
      }
      for (const clang::Stmt* child : statement->children()) {
        collect(child);
      }
    }

    const RestVisitor& self;
    int counter = 0;
    std::vector<std::pair<const void*, int>> path;
    std::vector<Access> accesses;
    std::vector<const clang::Stmt*> effects;
    std::vector<const clang::UnaryOperator*> increments;
  };

  static bool ordered(const Access& a, const Access& b) {
    for (const auto& left : a.path) {
      for (const auto& right : b.path) {
        if ((left.first == right.first) && (left.second != right.second)) {
          return true;
        }
      }
    }
    return false;
  }

  void check_sequencing(const clang::Expr* root) {
    Collector collector(*this);
    collector.collect(root);
    bool reported = false;
    for (std::size_t i = 0U; (i < collector.accesses.size()) && !reported; ++i) {
      const Access& write = collector.accesses[i];
      if (!write.write) {
        continue;
      }
      for (std::size_t j = 0U; j < collector.accesses.size(); ++j) {
        const Access& other = collector.accesses[j];
        if ((i == j) || (other.variable != write.variable) || ordered(write, other)) {
          continue;
        }
        const bool inside_own_operands = !other.write && (other.begin >= write.begin) &&
                                         (other.end <= write.end);
        if (!inside_own_operands && (other.write ? (j > i) : true)) {
          recorder_.add("unsequenced-access", write.location, write.variable->getNameAsString());
          reported = true;
          break;
        }
      }
    }
    const clang::Expr* stripped = root->IgnoreParens();
    for (const clang::UnaryOperator* increment : collector.increments) {
      if ((increment != stripped) && (collector.effects.size() >= 2U)) {
        recorder_.add("increment-with-side-effect", increment->getOperatorLoc());
      }
    }
  }

  clang::ASTContext& ast_;
  const Recorder& recorder_;
  clang::SourceManager& sm_;
  std::set<const clang::InitListExpr*> seen_lists_;
};

}  // namespace

void collect_rest_observations(clang::ASTContext& ast, const Recorder& recorder) {
  RestVisitor visitor(ast, recorder);
  visitor.TraverseDecl(ast.getTranslationUnitDecl());
}

}  // namespace misra
