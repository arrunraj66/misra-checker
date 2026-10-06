#include "misra/clang_observations.hpp"
#include "misra/observation_internal.hpp"

#include <cctype>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SmallString.h"

namespace misra {
namespace {

class ReturnCounter final : public clang::RecursiveASTVisitor<ReturnCounter> {
 public:
  bool VisitReturnStmt(clang::ReturnStmt*) {
    ++count;
    return true;
  }
  unsigned int count = 0U;
};

class CallCollector final : public clang::RecursiveASTVisitor<CallCollector> {
 public:
  bool VisitCallExpr(clang::CallExpr* call) {
    if (const clang::FunctionDecl* callee = call->getDirectCallee()) {
      callees.insert(callee->getCanonicalDecl());
    }
    return true;
  }
  std::set<const clang::FunctionDecl*> callees;
};

class LabelCollector final : public clang::RecursiveASTVisitor<LabelCollector> {
 public:
  bool VisitLabelStmt(clang::LabelStmt* label) {
    defined.push_back(label);
    return true;
  }
  bool VisitGotoStmt(clang::GotoStmt* statement) {
    targeted.insert(statement->getLabel());
    return true;
  }
  bool VisitAddrLabelExpr(clang::AddrLabelExpr* expression) {
    targeted.insert(expression->getLabel());
    return true;
  }
  std::vector<clang::LabelStmt*> defined;
  std::set<const clang::LabelDecl*> targeted;
};

unsigned int pointer_depth(const clang::QualType type) {
  unsigned int depth = 0U;
  const clang::Type* current = type.getTypePtrOrNull();
  while (current != nullptr) {
    const auto* pointer = llvm::dyn_cast<clang::PointerType>(current);
    if (pointer == nullptr) {
      break;
    }
    ++depth;
    current = pointer->getPointeeType().getTypePtrOrNull();
  }
  return depth;
}

// Returns true when every octal/hex escape in a literal spelling is ended by
// the closing quote, another escape, or (octal) its third digit.
bool escapes_terminated(const std::string& spelling) {
  const auto ends_sequence = [&spelling](const std::size_t position) {
    if (position >= spelling.size()) {
      return true;
    }
    const char c = spelling[position];
    return (c == '"') || (c == '\'') || (c == '\\');
  };
  std::size_t i = 0U;
  while (i < spelling.size()) {
    if (spelling[i] != '\\') {
      ++i;
      continue;
    }
    if ((i + 1U) >= spelling.size()) {
      break;
    }
    const char next = spelling[i + 1U];
    std::size_t end = i + 2U;
    if (next == 'x') {
      while ((end < spelling.size()) &&
             (std::isxdigit(static_cast<unsigned char>(spelling[end])) != 0)) {
        ++end;
      }
      if ((end > (i + 2U)) && !ends_sequence(end)) {
        return false;
      }
      i = end;
    } else if ((next >= '0') && (next <= '7')) {
      std::size_t digits = 1U;
      while ((digits < 3U) && (end < spelling.size()) &&
             (spelling[end] >= '0') && (spelling[end] <= '7')) {
        ++end;
        ++digits;
      }
      if ((digits < 3U) && !ends_sequence(end)) {
        return false;
      }
      i = end;
    } else {
      i += 2U;
    }
  }
  return true;
}

class ObservationVisitor final
    : public clang::RecursiveASTVisitor<ObservationVisitor> {
 public:
  ObservationVisitor(clang::ASTContext& ast, const Recorder& recorder)
      : ast_(ast), recorder_(recorder), sm_(ast.getSourceManager()) {}

  void Finish() {
    // Rule 17.2: report each defined function that can reach itself.
    for (const auto& [function, location] : definitions_) {
      std::set<const clang::FunctionDecl*> seen;
      std::vector<const clang::FunctionDecl*> work(
          call_graph_[function].begin(), call_graph_[function].end());
      bool recursive = false;
      while (!work.empty() && !recursive) {
        const clang::FunctionDecl* current = work.back();
        work.pop_back();
        if (current == function) {
          recursive = true;
        } else if (seen.insert(current).second) {
          const auto found = call_graph_.find(current);
          if (found != call_graph_.end()) {
            work.insert(work.end(), found->second.begin(), found->second.end());
          }
        }
      }
      if (recursive) {
        recorder_.add("recursion", location, function->getNameAsString());
      }
    }
  }

  bool VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function->isImplicit() ||
        !sm_.isWrittenInMainFile(sm_.getExpansionLoc(function->getLocation()))) {
      return true;
    }
    const clang::SourceLocation location = function->getLocation();
    if (!function->hasPrototype()) {
      recorder_.add("no-prototype", location);
    }
    for (const clang::ParmVarDecl* parameter : function->parameters()) {
      if (parameter->getName().empty()) {
        recorder_.add("unnamed-parameter", parameter->getBeginLoc());
      }
    }
    if (pointer_depth(function->getReturnType()) > 2U) {
      recorder_.add("pointer-nesting", location);
    }

    const bool is_definition = function->isThisDeclarationADefinition();
    if (is_definition && function->isExternallyVisible() && !function->isMain() &&
        (function->getPreviousDecl() == nullptr)) {
      recorder_.add("no-prior-declaration", location);
    }
    if (is_definition && function->isInlineSpecified() &&
        (function->getStorageClass() != clang::SC_Static)) {
      recorder_.add("external-inline", location);
    }
    if (!is_definition || (function->getBody() == nullptr)) {
      return true;
    }

    clang::Stmt* body = function->getBody();
    ReturnCounter returns;
    returns.TraverseStmt(body);
    bool last_is_return = false;
    if (const auto* block = llvm::dyn_cast<clang::CompoundStmt>(body)) {
      last_is_return = !block->body_empty() &&
                       llvm::isa<clang::ReturnStmt>(block->body_back());
    }
    if ((returns.count > 1U) || ((returns.count == 1U) && !last_is_return)) {
      recorder_.add("multiple-exit", location);
    }

    CallCollector calls;
    calls.TraverseStmt(body);
    const clang::FunctionDecl* canonical = function->getCanonicalDecl();
    call_graph_[canonical] = calls.callees;
    definitions_[canonical] = location;

    LabelCollector labels;
    labels.TraverseStmt(body);
    for (const clang::LabelStmt* label : labels.defined) {
      if (labels.targeted.count(label->getDecl()) == 0U) {
        recorder_.add("unused-label", label->getIdentLoc());
      }
    }

    for (const clang::ParmVarDecl* parameter : function->parameters()) {
      if (!parameter->getName().empty() && !parameter->isReferenced() &&
          !parameter->isUsed(false)) {
        recorder_.add("unused-parameter", parameter->getLocation());
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* variable) {
    if (variable->isImplicit()) {
      return true;
    }
    const clang::SourceLocation location = variable->getLocation();
    const clang::QualType type = variable->getType();
    if (type.isRestrictQualified()) {
      recorder_.add("restrict-qualifier", location);
    }
    if (pointer_depth(type) > 2U) {
      recorder_.add("pointer-nesting", location);
    }
    if (type->isVariableArrayType()) {
      recorder_.add("variable-length-array", location);
    }
    if (variable->isFileVarDecl()) {
      if ((variable->isThisDeclarationADefinition() !=
           clang::VarDecl::DeclarationOnly) &&
          variable->isExternallyVisible() &&
          (variable->getPreviousDecl() == nullptr)) {
        recorder_.add("no-prior-declaration", location);
      }
      if (variable->hasExternalStorage() &&
          (variable->isThisDeclarationADefinition() ==
           clang::VarDecl::DeclarationOnly) &&
          type->isIncompleteArrayType()) {
        recorder_.add("extern-array-no-size", location);
      }
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* field) {
    const clang::SourceLocation location = field->getLocation();
    const clang::QualType type = field->getType();
    if (type.isRestrictQualified()) {
      recorder_.add("restrict-qualifier", location);
    }
    if (pointer_depth(type) > 2U) {
      recorder_.add("pointer-nesting", location);
    }
    if (type->isVariableArrayType()) {
      recorder_.add("variable-length-array", location);
    }
    if (field->isBitField()) {
      const clang::QualType canonical = type.getCanonicalType();
      const bool allowed =
          canonical->isSpecificBuiltinType(clang::BuiltinType::Int) ||
          canonical->isSpecificBuiltinType(clang::BuiltinType::UInt) ||
          canonical->isSpecificBuiltinType(clang::BuiltinType::Bool);
      if (!allowed) {
        recorder_.add("bitfield-type", location);
      }
      if ((field->getIdentifier() != nullptr) &&
          (field->getBitWidthValue(ast_) == 1U) &&
          canonical->isSignedIntegerType()) {
        recorder_.add("signed-single-bit-field", location);
      }
    }
    return true;
  }

  bool VisitTypedefDecl(clang::TypedefDecl* declaration) {
    const clang::QualType type = declaration->getUnderlyingType();
    if (pointer_depth(type) > 2U) {
      recorder_.add("pointer-nesting", declaration->getLocation());
    }
    if (type->isVariableArrayType()) {
      recorder_.add("variable-length-array", declaration->getLocation());
    }
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* record) {
    if (!record->isCompleteDefinition()) {
      return true;
    }
    if (record->isUnion()) {
      recorder_.add("union-declared", record->getLocation());
    }
    if (record->isStruct()) {
      const clang::FieldDecl* last = nullptr;
      for (const clang::FieldDecl* field : record->fields()) {
        last = field;
      }
      if ((last != nullptr) && last->getType()->isIncompleteArrayType()) {
        recorder_.add("flexible-array-member", last->getLocation());
      }
    }
    return true;
  }

  bool VisitCompoundStmt(clang::CompoundStmt* block) {
    for (const clang::Stmt* child : block->body()) {
      if (const auto* call = llvm::dyn_cast<clang::CallExpr>(child)) {
        if (!call->getType()->isVoidType()) {
          const clang::FunctionDecl* callee = call->getDirectCallee();
          recorder_.add("unused-call-result", call->getBeginLoc(),
                        callee != nullptr ? callee->getNameAsString() : "");
        }
      }
    }
    return true;
  }

  bool VisitIfStmt(clang::IfStmt* statement) {
    if (!llvm::isa<clang::CompoundStmt>(statement->getThen())) {
      recorder_.add("body-not-compound", statement->getIfLoc());
    }
    const clang::Stmt* otherwise = statement->getElse();
    if ((otherwise != nullptr) && !llvm::isa<clang::CompoundStmt>(otherwise) &&
        !llvm::isa<clang::IfStmt>(otherwise)) {
      recorder_.add("body-not-compound", statement->getElseLoc());
    }
    if (else_if_members_.count(statement) == 0U) {
      const clang::IfStmt* tail = statement;
      bool chained = false;
      while ((tail->getElse() != nullptr) &&
             llvm::isa<clang::IfStmt>(tail->getElse())) {
        tail = llvm::cast<clang::IfStmt>(tail->getElse());
        else_if_members_.insert(tail);
        chained = true;
      }
      if (chained && (tail->getElse() == nullptr)) {
        recorder_.add("missing-final-else", statement->getIfLoc());
      }
    }
    return true;
  }

  bool VisitForStmt(clang::ForStmt* loop) {
    if (!llvm::isa<clang::CompoundStmt>(loop->getBody())) {
      recorder_.add("body-not-compound", loop->getForLoc());
    }
    return true;
  }

  bool VisitWhileStmt(clang::WhileStmt* loop) {
    if (!llvm::isa<clang::CompoundStmt>(loop->getBody())) {
      recorder_.add("body-not-compound", loop->getWhileLoc());
    }
    return true;
  }

  bool VisitDoStmt(clang::DoStmt* loop) {
    if (!llvm::isa<clang::CompoundStmt>(loop->getBody())) {
      recorder_.add("body-not-compound", loop->getDoLoc());
    }
    return true;
  }

  bool VisitSwitchStmt(clang::SwitchStmt* statement) {
    const auto* body = llvm::dyn_cast_or_null<clang::CompoundStmt>(statement->getBody());
    if (body == nullptr) {
      recorder_.add("body-not-compound", statement->getSwitchLoc());
      return true;
    }
    unsigned int clauses = 0U;
    int default_index = -1;
    for (const clang::Stmt* child : body->body()) {
      const clang::Stmt* current = child;
      bool is_head = false;
      while (const auto* label = llvm::dyn_cast_or_null<clang::SwitchCase>(current)) {
        is_head = true;
        top_level_cases_.insert(label);
        if (llvm::isa<clang::DefaultStmt>(label)) {
          default_index = static_cast<int>(clauses);
        }
        current = label->getSubStmt();
      }
      if (is_head) {
        ++clauses;
      }
    }
    if (default_index < 0) {
      recorder_.add("switch-no-default", statement->getSwitchLoc());
    } else if ((default_index != 0) &&
               (default_index != static_cast<int>(clauses) - 1)) {
      recorder_.add("default-not-first-or-last", statement->getSwitchLoc());
    }
    if (clauses < 2U) {
      recorder_.add("switch-too-few-clauses", statement->getSwitchLoc());
    }
    return true;
  }

  bool VisitSwitchCase(clang::SwitchCase* label) {
    if (top_level_cases_.count(label) == 0U) {
      recorder_.add("nested-switch-label", label->getKeywordLoc());
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator* op) {
    const clang::SourceLocation location = op->getOperatorLoc();
    if (op->getOpcode() == clang::BO_Comma) {
      recorder_.add("comma-operator", location);
    }
    if (((op->getOpcode() == clang::BO_LAnd) ||
         (op->getOpcode() == clang::BO_LOr)) &&
        op->getRHS()->HasSideEffects(ast_)) {
      recorder_.add("side-effect-in-logical-rhs", location);
    }
    if (op->isAssignmentOp() && modifies_parameter(op->getLHS())) {
      recorder_.add("parameter-modified", location);
    }
    switch (op->getOpcode()) {
      case clang::BO_Add:
      case clang::BO_Sub:
      case clang::BO_AddAssign:
      case clang::BO_SubAssign:
        if (op->getLHS()->getType()->isPointerType() ||
            op->getRHS()->getType()->isPointerType()) {
          recorder_.add("pointer-arithmetic", location);
        }
        break;
      default:
        break;
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator* op) {
    if (op->isIncrementDecrementOp() && modifies_parameter(op->getSubExpr())) {
      recorder_.add("parameter-modified", op->getOperatorLoc());
    }
    return true;
  }

  bool VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr* expression) {
    if ((expression->getKind() == clang::UETT_SizeOf) &&
        !expression->isArgumentType() &&
        expression->getArgumentExpr()->HasSideEffects(ast_)) {
      recorder_.add("sizeof-side-effect", expression->getOperatorLoc());
    }
    return true;
  }

  bool VisitVAArgExpr(clang::VAArgExpr* expression) {
    recorder_.add("stdarg-use", expression->getBuiltinLoc());
    return true;
  }

  bool VisitCallExpr(clang::CallExpr* call) {
    const clang::FunctionDecl* callee = call->getDirectCallee();
    if (callee == nullptr) {
      return true;
    }
    const std::string name = callee->getNameAsString();
    recorder_.add("call", call->getBeginLoc(), name);
    if (name.rfind("__builtin_va_", 0U) == 0U) {
      recorder_.add("stdarg-use", call->getBeginLoc(), name);
    }
    return true;
  }

  bool VisitCastExpr(clang::CastExpr* cast) {
    const clang::CastKind kind = cast->getCastKind();
    const bool explicit_cast = llvm::isa<clang::ExplicitCastExpr>(cast);
    const clang::QualType source = cast->getSubExpr()->getType();
    const clang::QualType target = cast->getType();
    const clang::SourceLocation location = cast->getBeginLoc();

    if (explicit_cast && ((kind == clang::CK_IntegralToPointer) ||
                          (kind == clang::CK_PointerToIntegral))) {
      recorder_.add("pointer-integer-cast", location);
    }
    if (kind == clang::CK_BitCast || kind == clang::CK_IntegralToPointer ||
        kind == clang::CK_PointerToIntegral) {
      const bool source_function = source->isFunctionPointerType();
      const bool target_function = target->isFunctionPointerType();
      if ((source_function || target_function) &&
          (ast_.getCanonicalType(source) != ast_.getCanonicalType(target))) {
        recorder_.add("function-pointer-conversion", location);
      }
    }
    if (((kind == clang::CK_BitCast) || (kind == clang::CK_NoOp)) &&
        source->isPointerType() && target->isPointerType()) {
      const clang::QualType from = source->getPointeeType();
      const clang::QualType to = target->getPointeeType();
      const bool null_constant = cast->getSubExpr()->isNullPointerConstant(
          ast_, clang::Expr::NPC_ValueDependentIsNull) !=
          clang::Expr::NPCK_NotNull;
      if ((kind == clang::CK_BitCast) && from->isVoidType() &&
          !to->isVoidType() && !to->isFunctionType() && !null_constant) {
        recorder_.add("void-pointer-to-object", location);
      }
      if (explicit_cast) {
        if ((from.isConstQualified() && !to.isConstQualified()) ||
            (from.isVolatileQualified() && !to.isVolatileQualified())) {
          recorder_.add("cast-removes-qualifier", location);
        }
        if ((kind == clang::CK_BitCast) && !from->isVoidType() && !to->isVoidType() &&
            !from->isFunctionType() && !to->isFunctionType() &&
            (ast_.getCanonicalType(from.getUnqualifiedType()) !=
             ast_.getCanonicalType(to.getUnqualifiedType()))) {
          recorder_.add("pointer-type-mismatch-cast", location);
        }
      }
    }
    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral* literal) {
    const std::string text = spelling(literal->getBeginLoc());
    if ((text.size() > 1U) && (text[0] == '0') &&
        (std::isdigit(static_cast<unsigned char>(text[1])) != 0)) {
      recorder_.add("octal-constant", literal->getBeginLoc());
    }
    if (text.find('l') != std::string::npos) {
      recorder_.add("lowercase-l-suffix", literal->getBeginLoc());
    }
    return true;
  }

  bool VisitFloatingLiteral(clang::FloatingLiteral* literal) {
    if (spelling(literal->getBeginLoc()).find('l') != std::string::npos) {
      recorder_.add("lowercase-l-suffix", literal->getBeginLoc());
    }
    return true;
  }

  bool VisitStringLiteral(clang::StringLiteral* literal) {
    for (unsigned int index = 0U; index < literal->getNumConcatenated(); ++index) {
      const clang::SourceLocation location = literal->getStrTokenLoc(index);
      if (!escapes_terminated(spelling(location))) {
        recorder_.add("unterminated-escape", location);
      }
    }
    return true;
  }

  bool VisitCharacterLiteral(clang::CharacterLiteral* literal) {
    if (!escapes_terminated(spelling(literal->getLocation()))) {
      recorder_.add("unterminated-escape", literal->getLocation());
    }
    return true;
  }

 private:
  [[nodiscard]] std::string spelling(const clang::SourceLocation location) const {
    llvm::SmallString<64> buffer;
    bool invalid = false;
    const llvm::StringRef text = clang::Lexer::getSpelling(
        sm_.getSpellingLoc(location), buffer, sm_, ast_.getLangOpts(), &invalid);
    return invalid ? std::string{} : text.str();
  }

  [[nodiscard]] static bool modifies_parameter(const clang::Expr* target) {
    const auto* reference =
        llvm::dyn_cast<clang::DeclRefExpr>(target->IgnoreParenImpCasts());
    return (reference != nullptr) &&
           llvm::isa<clang::ParmVarDecl>(reference->getDecl());
  }

  clang::ASTContext& ast_;
  const Recorder& recorder_;
  clang::SourceManager& sm_;
  std::map<const clang::FunctionDecl*, std::set<const clang::FunctionDecl*>>
      call_graph_;
  std::map<const clang::FunctionDecl*, clang::SourceLocation> definitions_;
  std::set<const clang::IfStmt*> else_if_members_;
  std::set<const clang::SwitchCase*> top_level_cases_;
};

class MacroObserver final : public clang::PPCallbacks {
 public:
  MacroObserver(const Recorder& recorder) : recorder_(recorder) {}

  void MacroDefined(const clang::Token& name,
                    const clang::MacroDirective* directive) override {
    const clang::MacroInfo* info = directive->getMacroInfo();
    for (const clang::Token& token : info->tokens()) {
      if (token.is(clang::tok::hashhash) ||
          (token.is(clang::tok::hash) && info->isFunctionLike())) {
        recorder_.add("stringify-or-paste-operator", name.getLocation());
        break;
      }
    }
  }

  void MacroUndefined(const clang::Token& name, const clang::MacroDefinition&,
                      const clang::MacroDirective*) override {
    recorder_.add("undef", name.getLocation(),
                  name.getIdentifierInfo() != nullptr
                      ? name.getIdentifierInfo()->getName().str()
                      : std::string{});
  }

 private:
  const Recorder& recorder_;
};

class CommentObserver final : public clang::CommentHandler {
 public:
  CommentObserver(clang::SourceManager& source_manager, AnalysisContext& context)
      : recorder_(source_manager, context) {}

  [[nodiscard]] const Recorder& recorder() const { return recorder_; }

  bool HandleComment(clang::Preprocessor& preprocessor,
                     clang::SourceRange range) override {
    const clang::SourceManager& sm = preprocessor.getSourceManager();
    bool invalid = false;
    const llvm::StringRef text = clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(range), sm,
        preprocessor.getLangOpts(), &invalid);
    if (invalid || (text.size() < 2U)) {
      return false;
    }
    const std::string whole = text.str();
    const bool block = whole.compare(0U, 2U, "/*") == 0;
    const bool closed = block && (whole.size() >= 4U) &&
                        (whole.compare(whole.size() - 2U, 2U, "*/") == 0);
    const std::string body_text =
        whole.substr(2U, block ? whole.size() - 2U - (closed ? 2U : 0U)
                               : std::string::npos);
    const llvm::StringRef body{body_text};
    if ((body.find("/*") != llvm::StringRef::npos) ||
        (body.find("//") != llvm::StringRef::npos)) {
      recorder_.add("comment-nested-marker", range.getBegin());
    }
    if (!block && ((text.find("\\\n") != llvm::StringRef::npos) ||
                   (text.find("\\\r\n") != llvm::StringRef::npos))) {
      recorder_.add("comment-line-splice", range.getBegin());
    }
    return false;
  }

 private:
  Recorder recorder_;
};

}  // namespace

std::unique_ptr<clang::CommentHandler> install_preprocessor_observers(
    clang::CompilerInstance& compiler, AnalysisContext& context) {
  auto handler = std::make_unique<CommentObserver>(compiler.getSourceManager(),
                                                   context);
  compiler.getPreprocessor().addPPCallbacks(
      std::make_unique<MacroObserver>(handler->recorder()));
  compiler.getPreprocessor().addPPCallbacks(make_extra_macro_observer(
      handler->recorder(), compiler.getLangOpts()));
  compiler.getPreprocessor().addCommentHandler(handler.get());
  return handler;
}

void collect_ast_observations(clang::ASTContext& ast, AnalysisContext& context) {
  // Invalid ASTs (recovery expressions, null types) are not safe to analyze;
  // the frontend reports the translation unit as failed anyway.
  if (ast.getDiagnostics().hasErrorOccurred()) {
    return;
  }
  const Recorder recorder(ast.getSourceManager(), context);
  ObservationVisitor visitor(ast, recorder);
  visitor.TraverseDecl(ast.getTranslationUnitDecl());
  visitor.Finish();
  collect_extra_observations(ast, recorder, context);

  // Rule 4.2: raw scan of the main file for trigraph sequences.
  clang::SourceManager& sm = ast.getSourceManager();
  const clang::FileID main_file = sm.getMainFileID();
  const llvm::StringRef buffer = sm.getBufferData(main_file);
  for (std::size_t offset = buffer.find("??"); offset != llvm::StringRef::npos;
       offset = buffer.find("??", offset + 1U)) {
    if (((offset + 2U) < buffer.size()) &&
        (std::string{"=/'()!<>-"}.find(buffer[offset + 2U]) != std::string::npos)) {
      recorder.add("trigraph", sm.getLocForStartOfFile(main_file)
                                   .getLocWithOffset(static_cast<int>(offset)));
    }
  }
}

}  // namespace misra
