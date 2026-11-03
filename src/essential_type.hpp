#pragma once

#include <optional>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "llvm/ADT/APSInt.h"

// Essential type model used by the Section 10 rules. This is an independent
// engineering model written from the published rule intent; it must be
// validated against the licensed MISRA text before any compliance claim.
namespace misra::essential {

enum class Category {
  Boolean,
  Character,
  Enum,
  Signed,
  Unsigned,
  Floating,
  Other,  // pointers, aggregates, void, wide characters: not modelled
};

struct Type final {
  Category category = Category::Other;
  unsigned width = 0U;                         // bits
  const clang::EnumDecl* enumeration = nullptr;  // for Category::Enum
  std::optional<llvm::APSInt> constant;        // integer constant expression

  [[nodiscard]] bool is_constant() const { return constant.has_value(); }
  [[nodiscard]] bool is_arithmetic() const {
    return category == Category::Signed || category == Category::Unsigned ||
           category == Category::Floating;
  }
};

// Operand positions with a row in the appropriateness table of Rule 10.1.
enum class Operand {
  Subscript,
  UnaryPlus,
  UnaryMinus,
  AddSub,        // also ++ and --
  MulDiv,
  Modulo,
  Relational,
  Equality,
  Logical,       // !, &&, || and the first operand of ?:
  LeftShift,     // left operand of << and >>, ~ and & | ^
  RightShift,    // right operand of << and >>
};

Type of_type(clang::QualType type, clang::ASTContext& context);
Type of_expression(const clang::Expr* expression, clang::ASTContext& context);

// A non-constant result of an arithmetic, bitwise or shift operator.
bool is_composite(const clang::Expr* expression, clang::ASTContext& context);

bool fits(const llvm::APSInt& value, Category category, unsigned width);

// Rule 10.1 table lookup. Unmodelled operands (Category::Other) are accepted.
bool appropriate(Operand position, const Type& operand);

// Rule 10.2: character operands in addition or subtraction.
bool character_arithmetic_misused(bool subtraction, const Type& left,
                                  const Type& right);

// Rule 10.4: operands of usual-arithmetic-conversion operators must share an
// essential category (same enumeration for enums).
bool categories_differ(const Type& left, const Type& right);

// Rule 10.3: assignment to a narrower type or a different category.
bool assignment_violates_category_or_width(const Type& destination,
                                           const Type& source);

}  // namespace misra::essential
