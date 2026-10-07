#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SmallString.h"
#include "misra/observation_internal.hpp"

namespace misra {
namespace {

// ---------------------------------------------------------------------------
// Essential-type approximation used by the 10.x detectors.
// ---------------------------------------------------------------------------

enum class Category { Boolean, Character, Signed, Unsigned, Enumeration, Floating, Other };

struct Essential {
  Category category;
  unsigned int width;
  const clang::Type* enum_type;  // canonical enum type when category is Enumeration
};

bool is_boolean_like(const clang::Expr* expression) {
  const clang::Expr* stripped = expression->IgnoreParenImpCasts();
  if (stripped->getType()->isBooleanType()) {
    return true;
  }
  if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(stripped)) {
    switch (binary->getOpcode()) {
      case clang::BO_LT:
      case clang::BO_GT:
      case clang::BO_LE:
      case clang::BO_GE:
      case clang::BO_EQ:
      case clang::BO_NE:
      case clang::BO_LAnd:
      case clang::BO_LOr:
        return true;
      default:
        return false;
    }
  }
  if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(stripped)) {
    return unary->getOpcode() == clang::UO_LNot;
  }
  return false;
}

const char* const kStandardNames[] = {
    "memcpy", "memmove", "memset", "memcmp", "memchr", "strlen", "strcpy",
    "strncpy", "strcat", "strncat", "strcmp", "strncmp", "strchr", "strrchr",
    "strstr", "strtok", "printf", "sprintf", "snprintf", "scanf", "puts",
    "putchar", "getchar", "fopen", "fclose", "malloc", "calloc", "realloc",
    "free", "abs", "labs", "exit", "abort", "atoi", "atof", "qsort", "bsearch",
    "rand", "srand", "time", "clock", "sin", "cos", "tan", "sqrt", "pow",
    "floor", "ceil", "fabs", "exp", "log"};

class ExtraVisitor final : public clang::RecursiveASTVisitor<ExtraVisitor> {
 public:
  ExtraVisitor(clang::ASTContext& ast, const Recorder& recorder,
               AnalysisContext& context)
      : ast_(ast), recorder_(recorder), sm_(ast.getSourceManager()),
        context_(context) {
    const clang::PresumedLoc main_loc =
        sm_.getPresumedLoc(sm_.getLocForStartOfFile(sm_.getMainFileID()));
    if (main_loc.isValid()) {
      translation_unit_ = main_loc.getFilename();
    }
    for (const clang::Decl* declaration :
         ast.getTranslationUnitDecl()->decls()) {
      if (const auto* named = llvm::dyn_cast<clang::NamedDecl>(declaration)) {
        if (llvm::isa<clang::VarDecl>(named) ||
            llvm::isa<clang::FunctionDecl>(named) ||
            llvm::isa<clang::TypedefNameDecl>(named)) {
          file_scope_names_.insert(named->getNameAsString());
        }
      }
      if (const auto* enumeration = llvm::dyn_cast<clang::EnumDecl>(declaration)) {
        for (const clang::EnumConstantDecl* constant : enumeration->enumerators()) {
          file_scope_names_.insert(constant->getNameAsString());
        }
      }
    }
  }

  // --- scope tracking (Rule 5.3) and function context (Rule 8.9) -----------

  bool TraverseFunctionDecl(clang::FunctionDecl* function) {
    clang::FunctionDecl* previous = current_function_;
    current_function_ = function;
    scopes_.emplace_back();
    const bool result = RecursiveASTVisitor::TraverseFunctionDecl(function);
    scopes_.pop_back();
    current_function_ = previous;
    return result;
  }

  bool TraverseCompoundStmt(clang::CompoundStmt* block,
                            DataRecursionQueue* = nullptr) {
    scopes_.emplace_back();
    const bool result = RecursiveASTVisitor::TraverseCompoundStmt(block, nullptr);
    scopes_.pop_back();
    return result;
  }

  bool TraverseForStmt(clang::ForStmt* loop, DataRecursionQueue* = nullptr) {
    scopes_.emplace_back();
    const bool result = RecursiveASTVisitor::TraverseForStmt(loop, nullptr);
    scopes_.pop_back();
    return result;
  }

  // --- declarations ---------------------------------------------------------

  bool VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function->isImplicit()) {
      return true;
    }
    const clang::SourceLocation location = function->getLocation();
    if (function->getDeclContext()->isTranslationUnit() && !function->isMain()) {
      record_symbol(function,
                    function->isThisDeclarationADefinition() ? SymbolRole::Definition
                                                             : SymbolRole::Declaration,
                    location);
    }
    note_named(function, location);
    check_reserved(function, location);
    check_attributes(function);
    if ((function->getPreviousDecl() != nullptr) &&
        (function->getStorageClass() != clang::SC_Static) &&
        !function->isExternallyVisible()) {
      recorder_.add("missing-static-on-redeclaration", location);
    }
    if (function->isThisDeclarationADefinition() && !function->isMain()) {
      const std::string name = function->getNameAsString();
      for (const char* standard : kStandardNames) {
        if (name == standard) {
          recorder_.add("reserved-identifier", location, name);
        }
      }
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl* parameter) {
    const clang::ArrayType* array = ast_.getAsArrayType(parameter->getOriginalType());
    if ((array != nullptr) && (static_cast<int>(array->getSizeModifier()) == 1)) {
      recorder_.add("static-array-parameter", parameter->getLocation());
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* variable) {
    if (variable->isImplicit()) {
      return true;
    }
    const clang::SourceLocation location = variable->getLocation();
    if (variable->isFileVarDecl()) {
      record_symbol(variable,
                    variable->isThisDeclarationADefinition() == clang::VarDecl::DeclarationOnly
                        ? SymbolRole::Declaration
                        : SymbolRole::Definition,
                    location);
    }
    note_named(variable, location);
    check_reserved(variable, location);
    check_attributes(variable);

    const std::string name = variable->getNameAsString();
    if (!name.empty() && !scopes_.empty() &&
        (variable->isLocalVarDecl() || llvm::isa<clang::ParmVarDecl>(variable))) {
      bool hides = file_scope_names_.count(name) != 0U;
      for (std::size_t i = 0U; (i + 1U) < scopes_.size(); ++i) {
        hides = hides || (scopes_[i].count(name) != 0U);
      }
      if (hides && (scopes_.back().count(name) == 0U)) {
        recorder_.add("hides-outer-declaration", location, name);
      }
      scopes_.back().insert(name);
    }

    if ((variable->getPreviousDecl() != nullptr) && variable->isFileVarDecl() &&
        (variable->getStorageClass() != clang::SC_Static) &&
        !variable->isExternallyVisible()) {
      recorder_.add("missing-static-on-redeclaration", location);
    }
    if (variable->isFileVarDecl() &&
        (variable->getStorageClass() == clang::SC_Static) &&
        (variable->getPreviousDecl() == nullptr)) {
      static_objects_.push_back(variable);
    }
    if (variable->hasInit() && (variable->getTypeSourceInfo() != nullptr) &&
        variable->getTypeSourceInfo()->getType()->isIncompleteArrayType()) {
      const auto* list =
          llvm::dyn_cast<clang::InitListExpr>(variable->getInit()->IgnoreParens());
      if (list != nullptr) {
        const clang::InitListExpr* syntactic = syntactic_form(list);
        for (const clang::Expr* init : syntactic->inits()) {
          if (const auto* designated = llvm::dyn_cast<clang::DesignatedInitExpr>(init)) {
            if (designated->getDesignator(0)->isArrayDesignator()) {
              recorder_.add("unsized-array-designated-init", location);
              break;
            }
          }
        }
      }
    }
    if (variable->hasInit() && !llvm::isa<clang::InitListExpr>(variable->getInit()) &&
        !llvm::isa<clang::ParmVarDecl>(variable)) {
      check_assignment(variable->getType(), variable->getInit(), location);
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* field) {
    check_reserved(field, field->getLocation());
    check_attributes(field);
    return true;
  }

  bool VisitTypedefDecl(clang::TypedefDecl* declaration) {
    note_named(declaration, declaration->getLocation());
    check_reserved(declaration, declaration->getLocation());
    check_attributes(declaration);
    if (is_main(declaration->getLocation())) {
      typedefs_.push_back(declaration);
    }
    return true;
  }

  bool VisitTagDecl(clang::TagDecl* tag) {
    if (tag->isImplicit() || tag->getName().empty()) {
      return true;
    }
    note_named(tag, tag->getLocation());
    check_reserved(tag, tag->getLocation());
    if (is_main(tag->getLocation())) {
      tags_.push_back(tag);
    }
    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* enumeration) {
    if (!enumeration->isCompleteDefinition()) {
      return true;
    }
    std::vector<std::pair<const clang::EnumConstantDecl*, llvm::APSInt>> values;
    for (const clang::EnumConstantDecl* constant : enumeration->enumerators()) {
      values.emplace_back(constant, constant->getInitVal());
    }
    for (const auto& [constant, value] : values) {
      if (constant->getInitExpr() != nullptr) {
        continue;
      }
      for (const auto& [other, other_value] : values) {
        if ((other != constant) && llvm::APSInt::isSameValue(value, other_value)) {
          recorder_.add("duplicate-implicit-enumerator", constant->getLocation());
          break;
        }
      }
    }
    for (const clang::EnumConstantDecl* constant : enumeration->enumerators()) {
      note_named(constant, constant->getLocation());
      check_reserved(constant, constant->getLocation());
    }
    return true;
  }

  bool VisitTypedefTypeLoc(clang::TypedefTypeLoc location) {
    typedef_references_.insert(location.getTypedefNameDecl()->getCanonicalDecl());
    return true;
  }

  bool VisitTagTypeLoc(clang::TagTypeLoc location) {
    tag_references_.insert(location.getDecl()->getCanonicalDecl());
    return true;
  }

  bool VisitTypeOfExprTypeLoc(clang::TypeOfExprTypeLoc location) {
    recorder_.add("language-extension", location.getTypeofLoc(), "typeof");
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* reference) {
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl())) {
      object_users_[variable->getCanonicalDecl()].insert(current_function_);
      if (variable->isFileVarDecl()) {
        record_symbol(variable, SymbolRole::Reference, reference->getLocation());
      }
    } else if (const auto* function =
                   llvm::dyn_cast<clang::FunctionDecl>(reference->getDecl())) {
      if (function->getDeclContext()->isTranslationUnit() && !function->isMain()) {
        record_symbol(function, SymbolRole::Reference, reference->getLocation());
      }
    }
    return true;
  }

  // --- statements -----------------------------------------------------------

  bool VisitCompoundStmt(clang::CompoundStmt* block) {
    for (const clang::Stmt* child : block->body()) {
      if (const auto* expression = llvm::dyn_cast<clang::Expr>(child)) {
        const auto* cast = llvm::dyn_cast<clang::CStyleCastExpr>(expression);
        const bool void_cast = (cast != nullptr) && cast->getType()->isVoidType();
        if (!void_cast && !expression->HasSideEffects(ast_)) {
          recorder_.add("no-effect-statement", expression->getBeginLoc());
        }
      }
    }
    return true;
  }

  bool VisitIfStmt(clang::IfStmt* statement) {
    check_condition(statement->getCond(), statement->getIfLoc(), true);
    return true;
  }

  bool VisitWhileStmt(clang::WhileStmt* loop) {
    check_condition(loop->getCond(), loop->getWhileLoc(), false);
    return true;
  }

  bool VisitDoStmt(clang::DoStmt* loop) {
    check_condition(loop->getCond(), loop->getDoLoc(), true);
    return true;
  }

  bool VisitForStmt(clang::ForStmt* loop) {
    check_well_formed(loop);
    if (loop->getCond() != nullptr) {
      check_condition(loop->getCond(), loop->getForLoc(), false);
    }
    if (loop->getInc() != nullptr) {
      FloatCounterFinder finder;
      finder.TraverseStmt(loop->getInc());
      if (finder.found) {
        recorder_.add("float-loop-counter", loop->getForLoc());
      }
    }
    return true;
  }

  bool VisitConditionalOperator(clang::ConditionalOperator* conditional) {
    check_condition(conditional->getCond(), conditional->getQuestionLoc(), false);
    return true;
  }

  bool VisitBinaryConditionalOperator(clang::BinaryConditionalOperator* conditional) {
    recorder_.add("language-extension", conditional->getBeginLoc(), "omitted-operand");
    return true;
  }

  bool VisitStmtExpr(clang::StmtExpr* expression) {
    recorder_.add("language-extension", expression->getBeginLoc(), "statement-expression");
    return true;
  }

  bool VisitAsmStmt(clang::AsmStmt* statement) {
    recorder_.add("language-extension", statement->getBeginLoc(), "inline-assembly");
    return true;
  }

  bool VisitIndirectGotoStmt(clang::IndirectGotoStmt* statement) {
    recorder_.add("language-extension", statement->getBeginLoc(), "computed-goto");
    return true;
  }

  bool VisitAddrLabelExpr(clang::AddrLabelExpr* expression) {
    recorder_.add("language-extension", expression->getBeginLoc(), "label-address");
    return true;
  }

  bool VisitCaseStmt(clang::CaseStmt* statement) {
    if (statement->getRHS() != nullptr) {
      recorder_.add("language-extension", statement->getBeginLoc(), "case-range");
    }
    return true;
  }

  bool VisitSwitchStmt(clang::SwitchStmt* statement) {
    if (is_boolean_like(statement->getCond())) {
      recorder_.add("boolean-switch-expression", statement->getSwitchLoc());
    }
    const auto* body = llvm::dyn_cast_or_null<clang::CompoundStmt>(statement->getBody());
    if (body == nullptr) {
      return true;
    }
    const clang::SwitchCase* head = nullptr;
    const clang::Stmt* last = nullptr;
    const auto close_clause = [&]() {
      if ((head != nullptr) && !ends_with_break(last)) {
        recorder_.add("missing-break", head->getKeywordLoc());
      }
    };
    for (const clang::Stmt* child : body->body()) {
      if (const auto* label = llvm::dyn_cast<clang::SwitchCase>(child)) {
        close_clause();
        head = label;
        const clang::Stmt* current = label;
        while (const auto* inner = llvm::dyn_cast_or_null<clang::SwitchCase>(current)) {
          current = inner->getSubStmt();
        }
        last = current;
      } else {
        last = child;
      }
    }
    close_clause();
    return true;
  }

  bool VisitInitListExpr(clang::InitListExpr* list) {
    const clang::InitListExpr* syntactic = syntactic_form(list);
    if (!seen_lists_.insert(syntactic).second) {
      return true;
    }
    bool designated = false;
    for (const clang::Expr* init : syntactic->inits()) {
      if (init->HasSideEffects(ast_)) {
        recorder_.add("initializer-side-effect", init->getBeginLoc());
      }
      designated = designated || llvm::isa<clang::DesignatedInitExpr>(init);
    }
    if (const clang::ConstantArrayType* array =
            ast_.getAsConstantArrayType(list->getType())) {
      const unsigned long count = syntactic->getNumInits();
      bool zero_only = false;
      if (count == 1U) {
        const auto* literal = llvm::dyn_cast<clang::IntegerLiteral>(
            syntactic->getInit(0)->IgnoreParenImpCasts());
        zero_only = (literal != nullptr) && literal->getValue().isZero();
      }
      if (!designated && (count > 0U) && !zero_only &&
          (count < array->getSize().getZExtValue())) {
        recorder_.add("partial-array-initializer", syntactic->getBeginLoc());
      }
    }
    std::set<std::string> keys;
    for (const clang::Expr* init : syntactic->inits()) {
      const auto* designated_init = llvm::dyn_cast<clang::DesignatedInitExpr>(init);
      if ((designated_init == nullptr) || (designated_init->size() != 1U)) {
        continue;
      }
      const clang::DesignatedInitExpr::Designator* designator =
          designated_init->getDesignator(0);
      std::string key;
      if (designator->isFieldDesignator()) {
        key = "." + designator->getFieldName()->getName().str();
      } else if (designator->isArrayDesignator()) {
        clang::Expr::EvalResult result;
        if (designated_init->getArrayIndex(*designator)->EvaluateAsInt(result, ast_)) {
          key = "[" + std::to_string(result.Val.getInt().getExtValue()) + "]";
        }
      }
      if (!key.empty() && !keys.insert(key).second) {
        recorder_.add("duplicate-initialization", init->getBeginLoc());
      }
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator* op) {
    const clang::SourceLocation location = op->getOperatorLoc();
    const clang::BinaryOperatorKind code = op->getOpcode();
    const Essential left = essential(op->getLHS());
    const Essential right = essential(op->getRHS());
    const bool left_constant = op->getLHS()->isIntegerConstantExpr(ast_);
    const bool right_constant = op->getRHS()->isIntegerConstantExpr(ast_);

    // Rule 10.1 (subset): operands of arithmetic, bitwise and shift operators.
    switch (code) {
      case clang::BO_Add: case clang::BO_Sub: case clang::BO_Mul:
      case clang::BO_Div: case clang::BO_Rem:
      case clang::BO_AddAssign: case clang::BO_SubAssign: case clang::BO_MulAssign:
      case clang::BO_DivAssign: case clang::BO_RemAssign:
        if (inappropriate_arithmetic(left) || inappropriate_arithmetic(right)) {
          recorder_.add("inappropriate-operand-type", location);
        }
        break;
      case clang::BO_And: case clang::BO_Or: case clang::BO_Xor:
      case clang::BO_AndAssign: case clang::BO_OrAssign: case clang::BO_XorAssign:
      case clang::BO_Shl: case clang::BO_Shr:
      case clang::BO_ShlAssign: case clang::BO_ShrAssign:
        if (inappropriate_bitwise(left, left_constant) ||
            ((code != clang::BO_Shl) && (code != clang::BO_Shr) &&
             (code != clang::BO_ShlAssign) && (code != clang::BO_ShrAssign) &&
             inappropriate_bitwise(right, right_constant))) {
          recorder_.add("inappropriate-operand-type", location);
        }
        break;
      default:
        break;
    }

    // Rule 10.4: operands of usual-arithmetic-conversion operators.
    switch (code) {
      case clang::BO_Add: case clang::BO_Sub: case clang::BO_Mul:
      case clang::BO_Div: case clang::BO_Rem: case clang::BO_LT:
      case clang::BO_GT: case clang::BO_LE: case clang::BO_GE:
      case clang::BO_EQ: case clang::BO_NE: case clang::BO_And:
      case clang::BO_Or: case clang::BO_Xor:
        if (!left_constant && !right_constant &&
            (left.category != Category::Other) &&
            (right.category != Category::Other) &&
            ((left.category != right.category) ||
             ((left.category == Category::Enumeration) &&
              (left.enum_type != right.enum_type)))) {
          recorder_.add("essential-type-mismatch", location);
        }
        break;
      default:
        break;
    }

    // Rule 12.2: constant shift counts outside the left operand's width.
    if ((code == clang::BO_Shl) || (code == clang::BO_Shr) ||
        (code == clang::BO_ShlAssign) || (code == clang::BO_ShrAssign)) {
      clang::Expr::EvalResult count;
      if (op->getRHS()->EvaluateAsInt(count, ast_) && (left.width != 0U)) {
        const llvm::APSInt value = count.Val.getInt();
        if (value.isNegative() || (value.getZExtValue() >= left.width)) {
          recorder_.add("shift-out-of-range", location);
        }
      }
    }

    // Rule 10.3 (simple assignment) and Rule 13.4.
    if (code == clang::BO_Assign) {
      check_assignment(op->getLHS()->getType(), op->getRHS(), location);
    }
    if (op->isAssignmentOp() && result_used(op)) {
      recorder_.add("assignment-result-used", location);
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator* op) {
    const Essential operand = essential(op->getSubExpr());
    const bool constant = op->getSubExpr()->isIntegerConstantExpr(ast_);
    if (((op->getOpcode() == clang::UO_Minus) || (op->getOpcode() == clang::UO_Plus)) &&
        (inappropriate_arithmetic(operand) ||
         ((operand.category == Category::Unsigned) && !constant &&
          (op->getOpcode() == clang::UO_Minus)))) {
      recorder_.add("inappropriate-operand-type", op->getOperatorLoc());
    }
    if ((op->getOpcode() == clang::UO_Not) && inappropriate_bitwise(operand, constant)) {
      recorder_.add("inappropriate-operand-type", op->getOperatorLoc());
    }
    if (((op->getOpcode() == clang::UO_PostInc) || (op->getOpcode() == clang::UO_PreInc) ||
         (op->getOpcode() == clang::UO_PostDec) || (op->getOpcode() == clang::UO_PreDec)) &&
        inappropriate_arithmetic(operand)) {
      recorder_.add("inappropriate-operand-type", op->getOperatorLoc());
    }
    if ((op->getOpcode() == clang::UO_Deref) && points_to_file(op->getSubExpr()->getType())) {
      recorder_.add("file-object-dereferenced", op->getOperatorLoc());
    }
    return true;
  }

  bool VisitMemberExpr(clang::MemberExpr* member) {
    if (member->isArrow() && points_to_file(member->getBase()->getType())) {
      recorder_.add("file-object-dereferenced", member->getMemberLoc());
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt* statement) {
    if ((statement->getRetValue() == nullptr) && (current_function_ != nullptr) &&
        !current_function_->getReturnType()->isVoidType()) {
      recorder_.add("missing-return-value", statement->getReturnLoc());
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr* call) {
    const clang::FunctionDecl* callee = call->getDirectCallee();
    if ((callee != nullptr) && callee->isImplicit() && (callee->getBuiltinID() == 0U)) {
      recorder_.add("implicit-function-declaration", call->getBeginLoc(),
                    callee->getNameAsString());
    }
    return true;
  }

  bool VisitCastExpr(clang::CastExpr* cast) {
    const clang::CastKind kind = cast->getCastKind();
    const bool explicit_cast = llvm::isa<clang::ExplicitCastExpr>(cast);
    const clang::QualType source = cast->getSubExpr()->getType();
    const clang::QualType target = cast->getType();
    const clang::SourceLocation location = cast->getBeginLoc();

    if (explicit_cast &&
        ((kind == clang::CK_IntegralToPointer) || (kind == clang::CK_PointerToIntegral)) &&
        (source->isVoidPointerType() || target->isVoidPointerType())) {
      recorder_.add("void-pointer-integer-cast", location);
    }
    if ((kind == clang::CK_BitCast) || (kind == clang::CK_IntegralToPointer) ||
        (kind == clang::CK_PointerToIntegral)) {
      const auto incomplete_pointee = [](const clang::QualType type) {
        return type->isPointerType() && !type->getPointeeType()->isVoidType() &&
               type->getPointeeType()->isIncompleteType() &&
               !type->getPointeeType()->isFunctionType();
      };
      if ((incomplete_pointee(source) || incomplete_pointee(target)) &&
          (ast_.getCanonicalType(source) != ast_.getCanonicalType(target))) {
        recorder_.add("incomplete-pointer-conversion", location);
      }
    }
    if (kind == clang::CK_NullToPointer) {
      const clang::SourceLocation begin = cast->getBeginLoc();
      const std::string name =
          clang::Lexer::getImmediateMacroNameForDiagnostics(begin, sm_, ast_.getLangOpts()).str();
      if (!begin.isMacroID() || (name != "NULL")) {
        recorder_.add("null-pointer-constant-not-null-macro", location);
      }
    }
    if (explicit_cast) {
      const Essential from = essential(cast->getSubExpr());
      const Essential to = essential_of_type(target);
      if ((from.category != Category::Other) && (to.category != Category::Other)) {
        const bool bad =
            ((to.category == Category::Boolean) && (from.category != Category::Boolean)) ||
            ((to.category == Category::Character) && (from.category != Category::Character)) ||
            ((to.category == Category::Enumeration) &&
             !((from.category == Category::Enumeration) && (from.enum_type == to.enum_type))) ||
            ((from.category == Category::Boolean) && (to.category != Category::Boolean)) ||
            ((from.category == Category::Character) && (to.category == Category::Floating));
        if (bad) {
          recorder_.add("inappropriate-cast", location);
        }
      }
    }
    // Rule 7.4: string literal decaying to a pointer to non-const char.
    if ((kind == clang::CK_ArrayToPointerDecay) &&
        llvm::isa<clang::StringLiteral>(cast->getSubExpr()->IgnoreParens()) &&
        !target->getPointeeType().isConstQualified() && !string_decay_is_safe(cast)) {
      recorder_.add("string-literal-to-non-const", location);
    }
    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral* literal) {
    if (literal->getType()->isUnsignedIntegerType()) {
      const std::string text = spelling(literal->getBeginLoc());
      if (text.find_first_of("uU") == std::string::npos) {
        recorder_.add("missing-unsigned-suffix", literal->getBeginLoc());
      }
    }
    return true;
  }

  // --- end of traversal -----------------------------------------------------

  void Finish() {
    for (const clang::TypedefDecl* declaration : typedefs_) {
      if (typedef_references_.count(declaration->getCanonicalDecl()) == 0U) {
        recorder_.add("unused-typedef", declaration->getLocation());
      }
    }
    for (const clang::TagDecl* tag : tags_) {
      if (tag_references_.count(tag->getCanonicalDecl()) == 0U) {
        recorder_.add("unused-tag", tag->getLocation());
      }
    }
    for (const clang::VarDecl* object : static_objects_) {
      const auto found = object_users_.find(object->getCanonicalDecl());
      if ((found != object_users_.end()) && (found->second.size() == 1U) &&
          (*found->second.begin() != nullptr)) {
        recorder_.add("single-function-object", object->getLocation());
      }
    }
    analyze_names();
  }

 private:
  struct FloatCounterFinder final : clang::RecursiveASTVisitor<FloatCounterFinder> {
    bool VisitUnaryOperator(clang::UnaryOperator* op) {
      if (op->isIncrementDecrementOp() && is_float_reference(op->getSubExpr())) {
        found = true;
      }
      return true;
    }
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
      if (op->isAssignmentOp() && is_float_reference(op->getLHS())) {
        found = true;
      }
      return true;
    }
    static bool is_float_reference(const clang::Expr* expression) {
      const auto* reference =
          llvm::dyn_cast<clang::DeclRefExpr>(expression->IgnoreParenImpCasts());
      return (reference != nullptr) && reference->getType()->isFloatingType();
    }
    bool found = false;
  };

  struct NamedEntry final {
    const clang::NamedDecl* declaration;
    char kind;  // 'o' ordinary, 't' typedef, 'g' tag, 'e' enumerator
    bool in_main;
    clang::SourceLocation location;
  };

  [[nodiscard]] bool is_main(const clang::SourceLocation location) const {
    return sm_.isWrittenInMainFile(sm_.getExpansionLoc(location));
  }

  [[nodiscard]] std::string spelling(const clang::SourceLocation location) const {
    llvm::SmallString<64> buffer;
    bool invalid = false;
    const llvm::StringRef text = clang::Lexer::getSpelling(
        sm_.getSpellingLoc(location), buffer, sm_, ast_.getLangOpts(), &invalid);
    return invalid ? std::string{} : text.str();
  }

  [[nodiscard]] static const clang::InitListExpr* syntactic_form(
      const clang::InitListExpr* list) {
    if (list->isSemanticForm() && (list->getSyntacticForm() != nullptr)) {
      return list->getSyntacticForm();
    }
    return list;
  }

  void record_symbol(const clang::NamedDecl* declaration, const SymbolRole role,
                     const clang::SourceLocation spelling) {
    const clang::SourceLocation location = sm_.getExpansionLoc(spelling);
    if (location.isInvalid() || sm_.isInSystemHeader(location) ||
        declaration->getName().empty() || declaration->isImplicit()) {
      return;
    }
    if (const auto* builtin = llvm::dyn_cast<clang::FunctionDecl>(declaration)) {
      if (builtin->getBuiltinID() != 0U) {
        return;
      }
    }
    const clang::PresumedLoc presumed = sm_.getPresumedLoc(location);
    if (presumed.isInvalid()) {
      return;
    }
    SymbolFact fact{declaration->getNameAsString(),
                    llvm::isa<clang::FunctionDecl>(declaration),
                    declaration->isExternallyVisible(),
                    role,
                    {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
                    translation_unit_,
                    {},
                    {}};
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration)) {
      fact.type_text = function->getReturnType().getAsString() + "(";
      for (const clang::ParmVarDecl* parameter : function->parameters()) {
        fact.type_text += parameter->getType().getAsString() + ",";
        if (!fact.param_names.empty()) {
          fact.param_names += ",";
        }
        fact.param_names += parameter->getNameAsString();
      }
      fact.type_text += ")";
    } else if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(declaration)) {
      fact.type_text = variable->getType().getAsString();
    }
    context_.symbols.push_back(std::move(fact));
  }

  void note_named(const clang::NamedDecl* declaration, const clang::SourceLocation location) {
    if (declaration->getName().empty()) {
      return;
    }
    char kind = 'o';
    if (llvm::isa<clang::TypedefNameDecl>(declaration)) {
      kind = 't';
    } else if (llvm::isa<clang::TagDecl>(declaration)) {
      kind = 'g';
    } else if (llvm::isa<clang::EnumConstantDecl>(declaration)) {
      kind = 'e';
    }
    entries_.push_back({declaration, kind, is_main(location), location});
  }

  void check_reserved(const clang::NamedDecl* declaration, const clang::SourceLocation location) {
    const std::string name = declaration->getNameAsString();
    if (name.empty() || (name[0] != '_')) {
      return;
    }
    const bool file_scope = declaration->getDeclContext()->isFileContext();
    const bool reserved_form =
        (name.size() > 1U) && ((name[1] == '_') || (std::isupper(static_cast<unsigned char>(name[1])) != 0));
    if (reserved_form || file_scope) {
      recorder_.add("reserved-identifier", location, name);
    }
  }

  void check_attributes(const clang::Decl* declaration) {
    if (!declaration->hasAttrs()) {
      return;
    }
    for (const clang::Attr* attribute : declaration->attrs()) {
      if (!attribute->isImplicit() && !attribute->isInherited()) {
        recorder_.add("language-extension", attribute->getLocation(), "attribute");
        break;
      }
    }
  }

  [[nodiscard]] Essential essential_of_type(const clang::QualType type) const {
    const clang::QualType canonical = type.getCanonicalType().getUnqualifiedType();
    const unsigned int width = ast_.getTypeSize(canonical);
    if (canonical->isBooleanType()) {
      return {Category::Boolean, width, nullptr};
    }
    if (canonical->isEnumeralType()) {
      return {Category::Enumeration, width, canonical.getTypePtr()};
    }
    if (canonical->isSpecificBuiltinType(clang::BuiltinType::Char_S) ||
        canonical->isSpecificBuiltinType(clang::BuiltinType::Char_U)) {
      return {Category::Character, width, nullptr};
    }
    if (canonical->isSignedIntegerType()) {
      return {Category::Signed, width, nullptr};
    }
    if (canonical->isUnsignedIntegerType()) {
      return {Category::Unsigned, width, nullptr};
    }
    if (canonical->isFloatingType()) {
      return {Category::Floating, width, nullptr};
    }
    return {Category::Other, 0U, nullptr};
  }

  [[nodiscard]] Essential essential(const clang::Expr* expression) const {
    const clang::Expr* stripped = expression->IgnoreParenImpCasts();
    if (is_boolean_like(stripped)) {
      return {Category::Boolean, 1U, nullptr};
    }
    return essential_of_type(stripped->getType());
  }

  [[nodiscard]] static bool inappropriate_arithmetic(const Essential& type) {
    return (type.category == Category::Boolean) || (type.category == Category::Enumeration);
  }

  [[nodiscard]] static bool inappropriate_bitwise(const Essential& type, const bool constant) {
    return (type.category == Category::Boolean) || (type.category == Category::Enumeration) ||
           (type.category == Category::Character) || (type.category == Category::Floating) ||
           ((type.category == Category::Signed) && !constant);
  }

  void check_assignment(const clang::QualType target, const clang::Expr* source,
                        const clang::SourceLocation location) {
    if (source->isIntegerConstantExpr(ast_) || llvm::isa<clang::StringLiteral>(source)) {
      return;
    }
    const Essential to = essential_of_type(target);
    const Essential from = essential(source);
    if ((to.category == Category::Other) || (from.category == Category::Other)) {
      return;
    }
    if ((to.category != from.category) ||
        ((to.category == Category::Enumeration) && (to.enum_type != from.enum_type)) ||
        (from.width > to.width)) {
      recorder_.add("essential-type-narrowing", location);
    }
  }

  void check_condition(const clang::Expr* condition, const clang::SourceLocation location,
                       const bool flag_constant_true) {
    if (!is_boolean_like(condition)) {
      recorder_.add("non-boolean-condition", location);
    }
    bool value = false;
    if (!condition->getBeginLoc().isMacroID() && !condition->isValueDependent() &&
        condition->EvaluateAsBooleanCondition(value, ast_) && (flag_constant_true || !value)) {
      recorder_.add("invariant-condition", location);
    }
  }

  [[nodiscard]] bool string_decay_is_safe(const clang::CastExpr* cast) const {
    for (const clang::DynTypedNode& parent : ast_.getParents(*cast)) {
      if (const auto* outer = parent.get<clang::ImplicitCastExpr>()) {
        if (outer->getType()->isPointerType() &&
            outer->getType()->getPointeeType().isConstQualified()) {
          return true;
        }
      }
      if (parent.get<clang::ArraySubscriptExpr>() != nullptr) {
        return true;
      }
      if (const auto* unary = parent.get<clang::UnaryOperator>()) {
        return unary->getOpcode() == clang::UO_Deref;
      }
      if (const auto* explicit_cast = parent.get<clang::ExplicitCastExpr>()) {
        return explicit_cast->getType()->isPointerType() &&
               explicit_cast->getType()->getPointeeType().isConstQualified();
      }
    }
    return false;
  }

  // Whether the value produced by `expression` feeds an enclosing construct.
  [[nodiscard]] bool result_used(const clang::Expr* expression) const {
    const clang::Stmt* child = expression;
    while (true) {
      const auto parents = ast_.getParents(*child);
      if (parents.empty()) {
        return false;
      }
      const clang::DynTypedNode& node = parents[0];
      if (node.get<clang::Decl>() != nullptr) {
        return true;
      }
      const auto* parent = node.get<clang::Stmt>();
      if (parent == nullptr) {
        return false;
      }
      if (llvm::isa<clang::ParenExpr>(parent)) {
        child = parent;
        continue;
      }
      if (const auto* cast = llvm::dyn_cast<clang::CStyleCastExpr>(parent)) {
        if (cast->getType()->isVoidType()) {
          return false;
        }
      }
      if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(parent)) {
        if ((binary->getOpcode() == clang::BO_Comma) && (binary->getLHS() == child)) {
          return false;
        }
      }
      if (llvm::isa<clang::Expr>(parent)) {
        return true;
      }
      if (const auto* statement = llvm::dyn_cast<clang::IfStmt>(parent)) {
        return statement->getCond() == child;
      }
      if (const auto* statement = llvm::dyn_cast<clang::WhileStmt>(parent)) {
        return statement->getCond() == child;
      }
      if (const auto* statement = llvm::dyn_cast<clang::DoStmt>(parent)) {
        return statement->getCond() == child;
      }
      if (const auto* statement = llvm::dyn_cast<clang::SwitchStmt>(parent)) {
        return statement->getCond() == child;
      }
      if (const auto* statement = llvm::dyn_cast<clang::ForStmt>(parent)) {
        return statement->getCond() == child;
      }
      return llvm::isa<clang::ReturnStmt>(parent);
    }
  }

  // Objects modified or address-taken by an expression subtree.
  struct ModifiedFinder final : clang::RecursiveASTVisitor<ModifiedFinder> {
    bool VisitUnaryOperator(clang::UnaryOperator* op) {
      if (op->isIncrementDecrementOp() || (op->getOpcode() == clang::UO_AddrOf)) {
        note(op->getSubExpr());
      }
      return true;
    }
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
      if (op->isAssignmentOp()) {
        note(op->getLHS());
      }
      return true;
    }
    bool VisitCallExpr(clang::CallExpr*) {
      has_call = true;
      return true;
    }
    void note(const clang::Expr* expression) {
      if (const auto* reference =
              llvm::dyn_cast<clang::DeclRefExpr>(expression->IgnoreParenImpCasts())) {
        if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl())) {
          modified.insert(variable->getCanonicalDecl());
        }
      }
    }
    std::set<const clang::VarDecl*> modified;
    bool has_call = false;
  };

  struct ReferenceFinder final : clang::RecursiveASTVisitor<ReferenceFinder> {
    bool VisitDeclRefExpr(clang::DeclRefExpr* reference) {
      if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(reference->getDecl())) {
        referenced.insert(variable->getCanonicalDecl());
      }
      return true;
    }
    std::set<const clang::VarDecl*> referenced;
  };

  // Rule 14.2 (subset): one loop counter, modified only by the third clause,
  // tested by the second clause, and left alone by the body.
  void check_well_formed(clang::ForStmt* loop) {
    if ((loop->getInit() == nullptr) && (loop->getCond() == nullptr) &&
        (loop->getInc() == nullptr)) {
      return;
    }
    bool well_formed = (loop->getInc() != nullptr) && (loop->getCond() != nullptr);
    const clang::VarDecl* counter = nullptr;
    if (well_formed) {
      ModifiedFinder step;
      step.TraverseStmt(loop->getInc());
      well_formed = (step.modified.size() == 1U) && !step.has_call;
      if (well_formed) {
        counter = *step.modified.begin();
      }
    }
    if (well_formed) {
      ReferenceFinder tested;
      tested.TraverseStmt(loop->getCond());
      ModifiedFinder cond_effects;
      cond_effects.TraverseStmt(loop->getCond());
      well_formed = (tested.referenced.count(counter) != 0U) &&
                    cond_effects.modified.empty() && !cond_effects.has_call;
    }
    if (well_formed && (loop->getInit() != nullptr)) {
      if (const auto* declaration = llvm::dyn_cast<clang::DeclStmt>(loop->getInit())) {
        for (const clang::Decl* decl : declaration->decls()) {
          const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl);
          well_formed = well_formed && (variable != nullptr) &&
                        (variable->getCanonicalDecl() == counter);
        }
      } else {
        ModifiedFinder init;
        init.TraverseStmt(loop->getInit());
        for (const clang::VarDecl* variable : init.modified) {
          well_formed = well_formed && (variable == counter);
        }
      }
    }
    if (well_formed && (loop->getBody() != nullptr)) {
      ModifiedFinder body;
      body.TraverseStmt(loop->getBody());
      well_formed = body.modified.count(counter) == 0U;
    }
    if (!well_formed) {
      recorder_.add("for-loop-not-well-formed", loop->getForLoc());
    }
  }

  [[nodiscard]] static bool ends_with_break(const clang::Stmt* statement) {
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

  [[nodiscard]] static bool points_to_file(const clang::QualType type) {
    if (!type->isPointerType()) {
      return false;
    }
    const clang::QualType pointee = type->getPointeeType();
    if (const auto* typedef_type = llvm::dyn_cast<clang::TypedefType>(pointee.getTypePtr())) {
      return typedef_type->getDecl()->getName() == "FILE";
    }
    return false;
  }

  void analyze_names() {
    // Rules 5.1 / 5.2: identifiers longer than 31 characters that differ only
    // after the 31st.
    std::map<std::string, std::set<std::string>> external_prefixes;
    std::map<std::pair<const clang::DeclContext*, std::string>, std::set<std::string>>
        scope_prefixes;
    for (const NamedEntry& entry : entries_) {
      const std::string name = entry.declaration->getNameAsString();
      if (name.size() <= 31U) {
        continue;
      }
      const std::string prefix = name.substr(0U, 31U);
      const bool object_or_function =
          llvm::isa<clang::VarDecl>(entry.declaration) ||
          llvm::isa<clang::FunctionDecl>(entry.declaration);
      if (object_or_function && entry.declaration->isExternallyVisible()) {
        external_prefixes[prefix].insert(name);
      }
      if (entry.kind != 'g') {
        scope_prefixes[{entry.declaration->getDeclContext(), prefix}].insert(name);
      }
    }
    for (const NamedEntry& entry : entries_) {
      const std::string name = entry.declaration->getNameAsString();
      if (!entry.in_main || (name.size() <= 31U)) {
        continue;
      }
      const std::string prefix = name.substr(0U, 31U);
      const bool object_or_function =
          llvm::isa<clang::VarDecl>(entry.declaration) ||
          llvm::isa<clang::FunctionDecl>(entry.declaration);
      if (object_or_function && entry.declaration->isExternallyVisible() &&
          (external_prefixes[prefix].size() > 1U)) {
        recorder_.add("external-identifiers-not-distinct", entry.location, name);
      }
      if ((entry.kind != 'g') &&
          (scope_prefixes[{entry.declaration->getDeclContext(), prefix}].size() > 1U)) {
        recorder_.add("scope-identifiers-not-distinct", entry.location, name);
      }
    }

    // Rules 5.6 / 5.7: typedef and tag names must be unique.
    for (const NamedEntry& entry : entries_) {
      if (!entry.in_main) {
        continue;
      }
      const std::string name = entry.declaration->getNameAsString();
      for (const NamedEntry& other : entries_) {
        if ((other.declaration == entry.declaration) ||
            (other.declaration->getName() != name) ||
            (other.declaration->getCanonicalDecl() == entry.declaration->getCanonicalDecl())) {
          continue;
        }
        const auto same_type = [this](const NamedEntry& a, const NamedEntry& b) {
          const NamedEntry& t = (a.kind == 't') ? a : b;
          const NamedEntry& g = (a.kind == 't') ? b : a;
          const auto* typedef_decl = llvm::cast<clang::TypedefNameDecl>(t.declaration);
          const auto* tag = llvm::cast<clang::TagDecl>(g.declaration);
          return ast_.getCanonicalType(typedef_decl->getUnderlyingType()) ==
                 ast_.getCanonicalType(ast_.getTypeDeclType(tag));
        };
        if (entry.kind == 't') {
          if ((other.kind == 'g') && same_type(entry, other)) {
            continue;
          }
          if ((other.kind == 't') && (other.declaration->getDeclContext() == entry.declaration->getDeclContext()) &&
              (ast_.getCanonicalType(llvm::cast<clang::TypedefNameDecl>(other.declaration)->getUnderlyingType()) ==
               ast_.getCanonicalType(llvm::cast<clang::TypedefNameDecl>(entry.declaration)->getUnderlyingType()))) {
            continue;
          }
          recorder_.add("typedef-name-not-unique", entry.location, name);
          break;
        }
        if (entry.kind == 'g') {
          if ((other.kind == 't') && same_type(entry, other)) {
            continue;
          }
          if ((other.kind == 'g') && (other.declaration->getKind() == entry.declaration->getKind()) &&
              false) {
            continue;
          }
          recorder_.add("tag-name-not-unique", entry.location, name);
          break;
        }
      }
    }

    // Rule 5.5 is evaluated against the macro list in the collection step.
    for (const NamedEntry& entry : entries_) {
      if (entry.in_main && (entry.kind != 'g')) {
        declared_names_.push_back({entry.declaration->getNameAsString(), entry.location});
      }
    }
  }

 public:
  std::vector<std::pair<std::string, clang::SourceLocation>> declared_names_;

 private:
  clang::ASTContext& ast_;
  const Recorder& recorder_;
  clang::SourceManager& sm_;
  clang::FunctionDecl* current_function_ = nullptr;
  AnalysisContext& context_;
  std::string translation_unit_;
  std::vector<std::set<std::string>> scopes_;
  std::set<std::string> file_scope_names_;
  std::vector<NamedEntry> entries_;
  std::vector<const clang::TypedefDecl*> typedefs_;
  std::vector<const clang::TagDecl*> tags_;
  std::set<const clang::TypedefNameDecl*> typedef_references_;
  std::set<const clang::TagDecl*> tag_references_;
  std::vector<const clang::VarDecl*> static_objects_;
  std::map<const clang::VarDecl*, std::set<const clang::FunctionDecl*>> object_users_;
  std::set<const clang::InitListExpr*> seen_lists_;
};

// ---------------------------------------------------------------------------
// Raw text scan of the main file: Rules 20.1, 20.2, 20.13 and 21.11.
// ---------------------------------------------------------------------------

struct LogicalLine {
  std::size_t offset;
  std::string raw;       // physical text with continuations joined
  std::string stripped;  // comments replaced by spaces
};

std::vector<LogicalLine> split_lines(const std::string& text) {
  std::vector<LogicalLine> lines;
  std::size_t i = 0U;
  bool in_block_comment = false;
  while (i < text.size()) {
    LogicalLine line{i, {}, {}};
    bool in_string = false;
    char quote = '\0';
    while (i < text.size()) {
      const char c = text[i];
      if ((c == '\\') && ((i + 1U) < text.size()) && (text[i + 1U] == '\n')) {
        i += 2U;
        continue;
      }
      if (c == '\n') {
        ++i;
        break;
      }
      line.raw.push_back(c);
      if (in_block_comment) {
        if ((c == '*') && ((i + 1U) < text.size()) && (text[i + 1U] == '/')) {
          in_block_comment = false;
          line.raw.push_back('/');
          ++i;
        }
        line.stripped.push_back(' ');
      } else if (in_string) {
        line.stripped.push_back(c);
        if ((c == '\\') && ((i + 1U) < text.size())) {
          line.raw.push_back(text[i + 1U]);
          line.stripped.push_back(text[i + 1U]);
          ++i;
        } else if (c == quote) {
          in_string = false;
        }
      } else if ((c == '/') && ((i + 1U) < text.size()) && (text[i + 1U] == '*')) {
        in_block_comment = true;
        line.raw.push_back('*');
        line.stripped += "  ";
        ++i;
      } else if ((c == '/') && ((i + 1U) < text.size()) && (text[i + 1U] == '/')) {
        while ((i < text.size()) && (text[i] != '\n')) {
          line.raw.push_back(text[i]);
          line.stripped.push_back(' ');
          ++i;
        }
        continue;
      } else {
        if ((c == '"') || (c == '\'')) {
          in_string = true;
          quote = c;
        }
        line.stripped.push_back(c);
      }
      ++i;
    }
    lines.push_back(std::move(line));
  }
  return lines;
}

void scan_main_file_text(clang::ASTContext& ast, const Recorder& recorder) {
  clang::SourceManager& sm = ast.getSourceManager();
  const clang::FileID main_file = sm.getMainFileID();
  const std::string text = sm.getBufferData(main_file).str();
  const auto location_of = [&](const std::size_t offset) {
    return sm.getLocForStartOfFile(main_file).getLocWithOffset(static_cast<int>(offset));
  };

  static const std::set<std::string> kDirectives{
      "define", "undef", "include", "if", "ifdef", "ifndef", "elif", "else",
      "endif", "line", "error", "pragma", "warning", "include_next", "ident",
      "elifdef", "elifndef", "embed", "import"};
  bool seen_code = false;
  for (const LogicalLine& line : split_lines(text)) {
    std::size_t start = line.stripped.find_first_not_of(" \t\r");
    if (start == std::string::npos) {
      continue;
    }
    if (line.stripped[start] != '#') {
      seen_code = true;
      continue;
    }
    std::size_t name_begin = line.stripped.find_first_not_of(" \t", start + 1U);
    if (name_begin == std::string::npos) {
      continue;  // null directive
    }
    std::size_t name_end = name_begin;
    while ((name_end < line.stripped.size()) &&
           ((std::isalnum(static_cast<unsigned char>(line.stripped[name_end])) != 0) ||
            (line.stripped[name_end] == '_'))) {
      ++name_end;
    }
    const std::string name = line.stripped.substr(name_begin, name_end - name_begin);
    if (name.empty()) {
      if (std::isdigit(static_cast<unsigned char>(line.stripped[name_begin])) == 0) {
        recorder.add("invalid-directive", location_of(line.offset));
      }
      continue;
    }
    if (kDirectives.count(name) == 0U) {
      recorder.add("invalid-directive", location_of(line.offset), name);
      continue;
    }
    if (name != "include") {
      continue;
    }
    if (seen_code) {
      recorder.add("include-after-code", location_of(line.offset));
    }
    const std::size_t raw_hash = line.raw.find('#');
    std::size_t operand = line.raw.find("include", raw_hash);
    operand = line.raw.find_first_of("<\"", operand);
    if (operand == std::string::npos) {
      continue;
    }
    const char closer = (line.raw[operand] == '<') ? '>' : '"';
    const std::size_t close = line.raw.find(closer, operand + 1U);
    if (close == std::string::npos) {
      continue;
    }
    const std::string header = line.raw.substr(operand + 1U, close - operand - 1U);
    const bool suspicious = (header.find('\'') != std::string::npos) ||
                            (header.find('\\') != std::string::npos) ||
                            (header.find("/*") != std::string::npos) ||
                            (header.find("//") != std::string::npos) ||
                            ((closer == '>') && (header.find('"') != std::string::npos));
    if (suspicious) {
      recorder.add("invalid-header-name-characters", location_of(line.offset), header);
    }
    if ((header == "tgmath.h") ||
        ((header.size() > 9U) && (header.compare(header.size() - 9U, 9U, "/tgmath.h") == 0))) {
      recorder.add("tgmath-include", location_of(line.offset));
    }
  }
}

// ---------------------------------------------------------------------------
// Macro observer: Rules 2.5, 5.4/5.5 inputs, 20.4, 20.7, 20.11, 20.12, 21.1.
// ---------------------------------------------------------------------------

class ExtraMacroObserver final : public clang::PPCallbacks {
 public:
  ExtraMacroObserver(const Recorder& recorder, const clang::LangOptions& language)
      : recorder_(recorder), language_(language) {}

  void MacroDefined(const clang::Token& name_token,
                    const clang::MacroDirective* directive) override {
    const clang::IdentifierInfo* identifier = name_token.getIdentifierInfo();
    if ((identifier == nullptr) ||
        !recorder_.source_manager().isWrittenInMainFile(
            recorder_.source_manager().getExpansionLoc(name_token.getLocation()))) {
      return;
    }
    const std::string name = identifier->getName().str();
    const clang::MacroInfo* info = directive->getMacroInfo();
    defined_[name] = name_token.getLocation();
    recorder_.add("macro-definition", name_token.getLocation(), name);
    if (identifier->isKeyword(language_)) {
      recorder_.add("macro-named-keyword", name_token.getLocation(), name);
    }
    check_reserved_macro(name, name_token.getLocation());

    if (!info->isFunctionLike()) {
      return;
    }
    const auto is_parameter = [&](const clang::Token& token) {
      if (!token.is(clang::tok::identifier)) {
        return false;
      }
      for (const clang::IdentifierInfo* parameter : info->params()) {
        if (parameter == token.getIdentifierInfo()) {
          return true;
        }
      }
      return false;
    };
    const auto tokens = info->tokens();
    std::map<const clang::IdentifierInfo*, std::pair<unsigned int, unsigned int>> uses;
    bool unparenthesized = false;
    for (std::size_t i = 0U; i < tokens.size(); ++i) {
      if (!is_parameter(tokens[i])) {
        continue;
      }
      const bool after_operator =
          (i > 0U) && (tokens[i - 1U].is(clang::tok::hash) || tokens[i - 1U].is(clang::tok::hashhash));
      const bool before_paste = ((i + 1U) < tokens.size()) && tokens[i + 1U].is(clang::tok::hashhash);
      if (after_operator || before_paste) {
        ++uses[tokens[i].getIdentifierInfo()].first;
        if ((i > 0U) && tokens[i - 1U].is(clang::tok::hash) && before_paste) {
          recorder_.add("stringify-before-paste", name_token.getLocation(), name);
        }
        continue;
      }
      ++uses[tokens[i].getIdentifierInfo()].second;
      const bool member_name =
          (i > 0U) && (tokens[i - 1U].is(clang::tok::period) || tokens[i - 1U].is(clang::tok::arrow));
      const bool open_before = (i > 0U) && (tokens[i - 1U].is(clang::tok::l_paren) ||
                                            tokens[i - 1U].is(clang::tok::comma));
      const bool close_after = ((i + 1U) < tokens.size()) &&
                               (tokens[i + 1U].is(clang::tok::r_paren) ||
                                tokens[i + 1U].is(clang::tok::comma));
      if (!member_name && !(open_before && close_after)) {
        unparenthesized = true;
      }
    }
    if (unparenthesized) {
      recorder_.add("macro-parameter-unparenthesized", name_token.getLocation(), name);
    }
    for (const auto& entry : uses) {
      if ((entry.second.first > 0U) && (entry.second.second > 0U)) {
        recorder_.add("macro-parameter-operand-and-expanded", name_token.getLocation(), name);
        break;
      }
    }
  }

  void MacroUndefined(const clang::Token& name_token, const clang::MacroDefinition&,
                      const clang::MacroDirective*) override {
    if (const clang::IdentifierInfo* identifier = name_token.getIdentifierInfo()) {
      check_reserved_macro(identifier->getName().str(), name_token.getLocation());
    }
  }

  void MacroExpands(const clang::Token& name_token, const clang::MacroDefinition&,
                    clang::SourceRange, const clang::MacroArgs*) override {
    mark_used(name_token);
  }

  void Ifdef(clang::SourceLocation, const clang::Token& name_token,
             const clang::MacroDefinition&) override {
    mark_used(name_token);
  }

  void Ifndef(clang::SourceLocation, const clang::Token& name_token,
              const clang::MacroDefinition&) override {
    mark_used(name_token);
  }

  void Defined(const clang::Token& name_token, const clang::MacroDefinition&,
               clang::SourceRange) override {
    mark_used(name_token);
  }

  void EndOfMainFile() override {
    for (const auto& entry : defined_) {
      if (used_.count(entry.first) == 0U) {
        recorder_.add("unused-macro", entry.second, entry.first);
      }
    }
  }

 private:
  void mark_used(const clang::Token& name_token) {
    if (const clang::IdentifierInfo* identifier = name_token.getIdentifierInfo()) {
      used_.insert(identifier->getName().str());
    }
  }

  void check_reserved_macro(const std::string& name, const clang::SourceLocation location) const {
    const bool reserved =
        (name == "defined") ||
        ((name.size() > 1U) && (name[0] == '_') &&
         ((name[1] == '_') || (std::isupper(static_cast<unsigned char>(name[1])) != 0)));
    if (reserved) {
      recorder_.add("reserved-macro-name", location, name);
    }
  }

  const Recorder& recorder_;
  const clang::LangOptions& language_;
  std::map<std::string, clang::SourceLocation> defined_;
  std::set<std::string> used_;
};

}  // namespace

std::unique_ptr<clang::PPCallbacks> make_extra_macro_observer(
    const Recorder& recorder, const clang::LangOptions& language) {
  return std::make_unique<ExtraMacroObserver>(recorder, language);
}

void collect_extra_observations(clang::ASTContext& ast, const Recorder& recorder,
                                AnalysisContext& context) {
  ExtraVisitor visitor(ast, recorder, context);
  const clang::PresumedLoc main_loc = ast.getSourceManager().getPresumedLoc(
      ast.getSourceManager().getLocForStartOfFile(ast.getSourceManager().getMainFileID()));
  if (main_loc.isValid()) {
    context.translation_units.push_back(main_loc.getFilename());
  }
  visitor.TraverseDecl(ast.getTranslationUnitDecl());
  visitor.Finish();
  scan_main_file_text(ast, recorder);
  collect_cfg_observations(ast, recorder);

  // Rules 5.4 / 5.5 from the recorded macro names, then drop the helper rows.
  std::map<std::string, std::set<std::string>> prefixes;
  std::vector<const Observation*> macros;
  for (const Observation& observation : context.observations) {
    if ((observation.kind == "macro-definition") && (observation.detail.size() > 31U)) {
      prefixes[observation.detail.substr(0U, 31U)].insert(observation.detail);
      macros.push_back(&observation);
    }
  }
  std::vector<Observation> derived;
  for (const Observation* macro : macros) {
    if (prefixes[macro->detail.substr(0U, 31U)].size() > 1U) {
      derived.push_back({"macro-name-not-distinct", macro->location, macro->detail});
    }
  }
  std::set<std::string> macro_names;
  for (const Observation& observation : context.observations) {
    if (observation.kind == "macro-definition") {
      macro_names.insert(observation.detail);
    }
  }
  for (const auto& [name, location] : visitor.declared_names_) {
    if (macro_names.count(name) != 0U) {
      recorder.add("identifier-matches-macro-name", location, name);
    }
  }
  for (Observation& observation : derived) {
    context.observations.push_back(std::move(observation));
  }
  std::vector<Observation> kept;
  for (Observation& observation : context.observations) {
    if (observation.kind != "macro-definition") {
      kept.push_back(std::move(observation));
    }
  }
  context.observations = std::move(kept);
}

}  // namespace misra
