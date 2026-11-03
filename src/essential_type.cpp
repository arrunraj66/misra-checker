#include "essential_type.hpp"

#include "clang/Lex/Lexer.h"

namespace misra::essential {
namespace {

using clang::BinaryOperator;
using clang::UnaryOperator;

const clang::Expr* strip(const clang::Expr* expression) {
  for (;;) {
    if (const auto* paren = llvm::dyn_cast<clang::ParenExpr>(expression)) {
      expression = paren->getSubExpr();
    } else if (const auto* implicit =
                   llvm::dyn_cast<clang::ImplicitCastExpr>(expression)) {
      expression = implicit->getSubExpr();
    } else if (const auto* constant =
                   llvm::dyn_cast<clang::ConstantExpr>(expression)) {
      expression = constant->getSubExpr();
    } else {
      return expression;
    }
  }
}

Type enumeration_type(const clang::EnumDecl* declaration,
                      clang::ASTContext& context) {
  Type result;
  const clang::QualType underlying = declaration->getIntegerType();
  if (declaration->getIdentifier() != nullptr ||
      declaration->getTypedefNameForAnonDecl() != nullptr) {
    result.category = Category::Enum;
    result.enumeration = declaration->getCanonicalDecl();
    result.width = underlying.isNull() ? 32U : context.getTypeSize(underlying);
  } else {
    // Constants of an unnamed enumeration have type int in C, whatever
    // underlying type the compiler chose for the enumeration itself.
    result.category = Category::Signed;
    result.width = context.getTypeSize(context.IntTy);
  }
  return result;
}

// Result of combining two operands of an arithmetic or bitwise operator.
Type combine(const Type& left, const Type& right, const clang::Expr* whole,
             clang::ASTContext& context) {
  if (left.category == Category::Boolean && right.category == Category::Boolean) {
    Type result = left;  // for example flag ? true : false
    result.constant.reset();
    return result;
  }
  if (left.is_constant() && right.is_constant()) {
    return of_type(whole->getType(), context);
  }
  if (left.is_constant()) {
    Type result = right;
    result.constant.reset();
    return result;
  }
  if (right.is_constant()) {
    Type result = left;
    result.constant.reset();
    return result;
  }
  if (left.category == right.category && left.category != Category::Enum &&
      left.category != Category::Other) {
    Type result = left;
    result.width = left.width > right.width ? left.width : right.width;
    return result;
  }
  return of_type(whole->getType(), context);
}

bool is_arithmetic_or_bitwise(const clang::BinaryOperatorKind op) {
  switch (op) {
    case clang::BO_Mul:
    case clang::BO_Div:
    case clang::BO_Rem:
    case clang::BO_Add:
    case clang::BO_Sub:
    case clang::BO_And:
    case clang::BO_Xor:
    case clang::BO_Or:
      return true;
    default:
      return false;
  }
}

}  // namespace

Type of_type(clang::QualType type, clang::ASTContext& context) {
  Type result;
  if (type.isNull() || type->isDependentType()) {
    return result;
  }
  const clang::QualType canonical = type.getCanonicalType().getUnqualifiedType();
  if (canonical->isBooleanType()) {
    result.category = Category::Boolean;
    result.width = context.getTypeSize(canonical);
    return result;
  }
  if (const auto* enumeration = canonical->getAs<clang::EnumType>()) {
    return enumeration_type(enumeration->getDecl(), context);
  }
  if (const auto* builtin = canonical->getAs<clang::BuiltinType>()) {
    switch (builtin->getKind()) {
      case clang::BuiltinType::Char_S:
      case clang::BuiltinType::Char_U:
        result.category = Category::Character;
        break;
      case clang::BuiltinType::SChar:
      case clang::BuiltinType::Short:
      case clang::BuiltinType::Int:
      case clang::BuiltinType::Long:
      case clang::BuiltinType::LongLong:
      case clang::BuiltinType::Int128:
        result.category = Category::Signed;
        break;
      case clang::BuiltinType::UChar:
      case clang::BuiltinType::UShort:
      case clang::BuiltinType::UInt:
      case clang::BuiltinType::ULong:
      case clang::BuiltinType::ULongLong:
      case clang::BuiltinType::UInt128:
        result.category = Category::Unsigned;
        break;
      default:
        if (builtin->isFloatingPoint()) {
          result.category = Category::Floating;
        }
        break;
    }
    if (result.category != Category::Other) {
      result.width = context.getTypeSize(canonical);
    }
  }
  return result;
}

Type of_expression(const clang::Expr* original, clang::ASTContext& context) {
  const clang::Expr* const expression = strip(original);
  Type result;

  if (const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(expression)) {
    if (const auto* enumerator =
            llvm::dyn_cast<clang::EnumConstantDecl>(reference->getDecl())) {
      const auto* declaration =
          llvm::cast<clang::EnumDecl>(enumerator->getDeclContext());
      result = enumeration_type(declaration, context);
      result.constant = enumerator->getInitVal();
      return result;
    }
  }
  // <stdbool.h> true/false expand to the integer constants 1 and 0.
  if (llvm::isa<clang::IntegerLiteral>(expression) &&
      expression->getBeginLoc().isMacroID()) {
    const llvm::StringRef macro = clang::Lexer::getImmediateMacroName(
        expression->getBeginLoc(), context.getSourceManager(),
        context.getLangOpts());
    if (macro == "true" || macro == "false") {
      result.category = Category::Boolean;
      result.width = 1U;
      result.constant = llvm::cast<clang::IntegerLiteral>(expression)->getValue();
      result.constant->setIsUnsigned(true);
      return result;
    }
  }
  if (const auto* character =
          llvm::dyn_cast<clang::CharacterLiteral>(expression)) {
    result.category = Category::Character;
    result.width = 8U;
    result.constant = llvm::APSInt(llvm::APInt(32U, character->getValue(), true),
                                   false);
    return result;
  }

  if (const auto* cast = llvm::dyn_cast<clang::ExplicitCastExpr>(expression)) {
    result = of_type(cast->getType(), context);
  } else if (const auto* binary =
                 llvm::dyn_cast<clang::BinaryOperator>(expression)) {
    const clang::BinaryOperatorKind op = binary->getOpcode();
    if (binary->isComparisonOp() || binary->isLogicalOp()) {
      result.category = Category::Boolean;
      result.width = 1U;
      return result;
    }
    if (op == clang::BO_Comma) {
      return of_expression(binary->getRHS(), context);
    }
    if (binary->isAssignmentOp()) {
      return of_expression(binary->getLHS(), context);
    }
    if (op == clang::BO_Shl || op == clang::BO_Shr) {
      result = of_expression(binary->getLHS(), context);
    } else if (is_arithmetic_or_bitwise(op)) {
      result = combine(of_expression(binary->getLHS(), context),
                       of_expression(binary->getRHS(), context), expression,
                       context);
    } else {
      result = of_type(expression->getType(), context);
    }
  } else if (const auto* unary =
                 llvm::dyn_cast<clang::UnaryOperator>(expression)) {
    switch (unary->getOpcode()) {
      case clang::UO_LNot:
        result.category = Category::Boolean;
        result.width = 1U;
        return result;
      case clang::UO_Plus:
      case clang::UO_Minus:
      case clang::UO_Not:
      case clang::UO_PreInc:
      case clang::UO_PostInc:
      case clang::UO_PreDec:
      case clang::UO_PostDec:
        result = of_expression(unary->getSubExpr(), context);
        break;
      default:
        result = of_type(expression->getType(), context);
        break;
    }
  } else if (const auto* conditional =
                 llvm::dyn_cast<clang::ConditionalOperator>(expression)) {
    const Type a = of_expression(conditional->getTrueExpr(), context);
    const Type b = of_expression(conditional->getFalseExpr(), context);
    result = (a.category == b.category && a.category != Category::Other)
                 ? combine(a, b, expression, context)
                 : of_type(expression->getType(), context);
  } else {
    result = of_type(expression->getType(), context);
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(expression)) {
      if (const auto* field =
              llvm::dyn_cast<clang::FieldDecl>(member->getMemberDecl())) {
        if (field->isBitField() && result.category != Category::Other) {
          result.width = field->getBitWidthValue(context);
        }
      }
    }
  }

  // The value of the whole expression (for example -1) replaces any value
  // inherited from an operand.
  if (result.category != Category::Other && !expression->isValueDependent() &&
      expression->getType()->isIntegerType()) {
    result.constant = expression->getIntegerConstantExpr(context);
  }
  return result;
}

bool is_composite(const clang::Expr* original, clang::ASTContext& context) {
  const clang::Expr* const expression = strip(original);
  bool operator_result = false;
  if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(expression)) {
    const clang::BinaryOperatorKind op = binary->getOpcode();
    operator_result = is_arithmetic_or_bitwise(op) || op == clang::BO_Shl ||
                      op == clang::BO_Shr;
  } else if (const auto* unary =
                 llvm::dyn_cast<clang::UnaryOperator>(expression)) {
    operator_result = unary->getOpcode() == clang::UO_Plus ||
                      unary->getOpcode() == clang::UO_Minus ||
                      unary->getOpcode() == clang::UO_Not;
  }
  return operator_result && !of_expression(expression, context).is_constant();
}

bool fits(const llvm::APSInt& value, const Category category,
          const unsigned width) {
  const bool negative = value.isSigned() && value.isNegative();
  switch (category) {
    case Category::Boolean:
      return !negative && value.getActiveBits() <= 1U;
    case Category::Unsigned:
      return !negative && value.getActiveBits() <= width;
    case Category::Signed:
      return negative ? value.getSignificantBits() <= width
                      : value.getActiveBits() < width;
    case Category::Character:
      return negative ? value.getSignificantBits() <= width
                      : value.getActiveBits() <= width;
    default:
      return false;
  }
}

bool appropriate(const Operand position, const Type& operand) {
  using C = Category;
  const C c = operand.category;
  if (c == C::Other) {
    return true;
  }
  const bool nonnegative_constant =
      operand.is_constant() && c == C::Signed &&
      !(operand.constant->isSigned() && operand.constant->isNegative());
  switch (position) {
    case Operand::Subscript:
      return c == C::Character || c == C::Signed || c == C::Unsigned;
    case Operand::UnaryPlus:
      return c == C::Signed || c == C::Unsigned || c == C::Floating;
    case Operand::UnaryMinus:
      return c == C::Signed || c == C::Floating;
    case Operand::AddSub:
      return c == C::Character || c == C::Signed || c == C::Unsigned ||
             c == C::Floating;
    case Operand::MulDiv:
      return c == C::Signed || c == C::Unsigned || c == C::Floating;
    case Operand::Modulo:
      return c == C::Signed || c == C::Unsigned;
    case Operand::Relational:
      return c != C::Boolean;
    case Operand::Equality:
      return true;
    case Operand::Logical:
      return c == C::Boolean;
    case Operand::LeftShift:
    case Operand::RightShift:
      // A non-negative signed constant is accepted (documented leniency).
      return c == C::Unsigned || nonnegative_constant;
  }
  return true;
}

bool character_arithmetic_misused(const bool subtraction, const Type& left,
                                  const Type& right) {
  using C = Category;
  if (left.category == C::Other || right.category == C::Other) {
    return false;
  }
  const bool left_char = left.category == C::Character;
  const bool right_char = right.category == C::Character;
  if (!left_char && !right_char) {
    return false;
  }
  const auto integer = [](const Type& t) {
    return t.category == C::Signed || t.category == C::Unsigned;
  };
  if (subtraction) {
    // char - integer and char - char are fine; integer - char is not.
    return !(left_char && (right_char || integer(right)));
  }
  // char + integer and integer + char are fine; char + char is not.
  return !((left_char && integer(right)) || (right_char && integer(left)));
}

bool categories_differ(const Type& left, const Type& right) {
  if (left.category == Category::Other || right.category == Category::Other) {
    return false;
  }
  if (left.category == right.category) {
    return left.category == Category::Enum &&
           left.enumeration != right.enumeration;
  }
  const auto allowed_constant = [](const Type& constant, const Type& other) {
    return constant.category == Category::Signed && constant.is_constant() &&
           other.category == Category::Unsigned &&
           fits(*constant.constant, Category::Unsigned, other.width);
  };
  return !(allowed_constant(left, right) || allowed_constant(right, left));
}

bool assignment_violates_category_or_width(const Type& destination,
                                           const Type& source) {
  using C = Category;
  if (destination.category == C::Other || source.category == C::Other) {
    return false;
  }
  if (destination.category == C::Enum) {
    return !(source.category == C::Enum &&
             source.enumeration == destination.enumeration);
  }
  if (source.is_constant()) {
    if (source.category == destination.category) {
      return !fits(*source.constant, destination.category, destination.width);
    }
    const bool signed_to_unsigned =
        destination.category == C::Unsigned && source.category == C::Signed &&
        fits(*source.constant, C::Unsigned, destination.width);
    return !signed_to_unsigned;
  }
  return source.category != destination.category ||
         source.width > destination.width;
}

}  // namespace misra::essential
