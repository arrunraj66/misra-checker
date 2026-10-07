// Control-flow-graph based observations: unreachable code (2.1), dead stores
// (2.2), reads of uninitialized objects (9.1) and paths that leave a non-void
// function without a return value (17.4).

#include <set>
#include <string>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/Analysis/Analyses/UninitializedValues.h"
#include "clang/Analysis/AnalysisDeclContext.h"
#include "clang/Analysis/CFG.h"
#include "misra/observation_internal.hpp"

namespace misra {
namespace {

std::set<const clang::CFGBlock*> reachable_blocks(const clang::CFG& cfg) {
  std::set<const clang::CFGBlock*> seen;
  std::vector<const clang::CFGBlock*> work{&cfg.getEntry()};
  while (!work.empty()) {
    const clang::CFGBlock* block = work.back();
    work.pop_back();
    if (!seen.insert(block).second) {
      continue;
    }
    for (const clang::CFGBlock::AdjacentBlock& successor : block->succs()) {
      if (const clang::CFGBlock* next = successor.getReachableBlock()) {
        work.push_back(next);
      }
    }
  }
  return seen;
}

// First statement in a block that counts as code (null statements, bare
// declarations without initializer and label-only blocks do not).
const clang::Stmt* first_code_statement(const clang::CFGBlock& block) {
  for (const clang::CFGElement& element : block) {
    const auto statement = element.getAs<clang::CFGStmt>();
    if (!statement) {
      continue;
    }
    const clang::Stmt* stmt = statement->getStmt();
    if (llvm::isa<clang::NullStmt>(stmt)) {
      continue;
    }
    if (const auto* declaration = llvm::dyn_cast<clang::DeclStmt>(stmt)) {
      bool initialized = false;
      for (const clang::Decl* decl : declaration->decls()) {
        if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl)) {
          initialized = initialized || variable->hasInit();
        }
      }
      if (!initialized) {
        continue;
      }
    }
    if (stmt->getBeginLoc().isValid()) {
      return stmt;
    }
  }
  return nullptr;
}

bool ends_in_return_or_noreturn(const clang::CFGBlock& block) {
  for (auto it = block.rbegin(); it != block.rend(); ++it) {
    const auto statement = it->getAs<clang::CFGStmt>();
    if (!statement) {
      continue;
    }
    const clang::Stmt* stmt = statement->getStmt();
    if (llvm::isa<clang::ReturnStmt>(stmt)) {
      return true;
    }
    if (const auto* call = llvm::dyn_cast<clang::CallExpr>(stmt)) {
      const clang::FunctionDecl* callee = call->getDirectCallee();
      return (callee != nullptr) && callee->isNoReturn();
    }
    return false;
  }
  return false;
}

class DeadStoreObserver final : public clang::LiveVariables::Observer {
 public:
  explicit DeadStoreObserver(const Recorder& recorder) : recorder_(recorder) {}

  void observerKill(const clang::DeclRefExpr*) override {}

  void observeStmt(const clang::Stmt* statement, const clang::CFGBlock*,
                   const clang::LiveVariables::LivenessValues& live) override {
    const auto* assignment = llvm::dyn_cast<clang::BinaryOperator>(statement);
    if ((assignment == nullptr) || (assignment->getOpcode() != clang::BO_Assign)) {
      return;
    }
    const auto* target =
        llvm::dyn_cast<clang::DeclRefExpr>(assignment->getLHS()->IgnoreParenCasts());
    if (target == nullptr) {
      return;
    }
    const auto* variable = llvm::dyn_cast<clang::VarDecl>(target->getDecl());
    if ((variable != nullptr) && variable->hasLocalStorage() &&
        !variable->getType()->isReferenceType() &&
        !variable->getType().isVolatileQualified() && !live.isLive(variable)) {
      recorder_.add("dead-store", assignment->getOperatorLoc(),
                    variable->getNameAsString());
    }
  }

 private:
  const Recorder& recorder_;
};

class UninitHandler final : public clang::UninitVariablesHandler {
 public:
  explicit UninitHandler(const Recorder& recorder) : recorder_(recorder) {}

  void handleUseOfUninitVariable(const clang::VarDecl* variable,
                                 const clang::UninitUse& use) override {
    const bool always = (use.branch_begin() == use.branch_end()) &&
                        (use.getKind() != clang::UninitUse::Maybe);
    recorder_.add(always ? "uninitialized-use" : "maybe-uninitialized-use",
                  use.getUser()->getBeginLoc(), variable->getNameAsString());
  }

 private:
  const Recorder& recorder_;
};

}  // namespace

void collect_cfg_observations(clang::ASTContext& ast, const Recorder& recorder) {
  clang::AnalysisDeclContextManager manager(ast);
  clang::SourceManager& sm = ast.getSourceManager();
  for (const clang::Decl* declaration : ast.getTranslationUnitDecl()->decls()) {
    const auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration);
    if ((function == nullptr) || !function->isThisDeclarationADefinition() ||
        (function->getBody() == nullptr) ||
        !sm.isWrittenInMainFile(sm.getExpansionLoc(function->getLocation()))) {
      continue;
    }
    clang::AnalysisDeclContext* context = manager.getContext(function);
    context->getCFGBuildOptions().setAllAlwaysAdd();
    const clang::CFG* cfg = context->getCFG();
    if (cfg == nullptr) {
      continue;
    }
    const std::set<const clang::CFGBlock*> reachable = reachable_blocks(*cfg);

    // Rule 2.1: report the first code statement of each unreachable region.
    std::set<const clang::CFGBlock*> visited;
    std::vector<std::pair<const clang::CFGBlock*, bool>> work;
    for (const clang::CFGBlock* block : *cfg) {
      if ((reachable.count(block) != 0U) || (block == &cfg->getExit())) {
        continue;
      }
      bool has_unreachable_predecessor = false;
      for (const clang::CFGBlock::AdjacentBlock& predecessor : block->preds()) {
        const clang::CFGBlock* pred = predecessor.getPossiblyUnreachableBlock();
        has_unreachable_predecessor =
            has_unreachable_predecessor ||
            ((pred != nullptr) && (reachable.count(pred) == 0U));
      }
      if (!has_unreachable_predecessor) {
        work.emplace_back(block, false);
      }
    }
    while (!work.empty()) {
      auto [block, covered] = work.back();
      work.pop_back();
      if (!visited.insert(block).second) {
        continue;
      }
      if (!covered) {
        if (const clang::Stmt* statement = first_code_statement(*block)) {
          recorder.add("unreachable-code", statement->getBeginLoc());
          covered = true;
        }
      }
      for (const clang::CFGBlock::AdjacentBlock& successor : block->succs()) {
        const clang::CFGBlock* next = successor.getPossiblyUnreachableBlock();
        if ((next != nullptr) && (reachable.count(next) == 0U) &&
            (next != &cfg->getExit())) {
          work.emplace_back(next, covered);
        }
      }
    }

    // Rule 17.4: a reachable path ends without a return statement.
    if (!function->getReturnType()->isVoidType() && !function->isMain() &&
        !function->isNoReturn()) {
      for (const clang::CFGBlock::AdjacentBlock& predecessor : cfg->getExit().preds()) {
        const clang::CFGBlock* block = predecessor.getReachableBlock();
        if ((block != nullptr) && (reachable.count(block) != 0U) &&
            !ends_in_return_or_noreturn(*block)) {
          recorder.add("missing-return-value", function->getLocation());
          break;
        }
      }
    }

    // Rule 9.1: uninitialized reads.
    UninitHandler uninit(recorder);
    clang::UninitVariablesAnalysisStats stats{};
    clang::runUninitializedVariablesAnalysis(*function, *cfg, *context, uninit, stats);

    // Rule 2.2: assignments whose value is never read.
    if (clang::LiveVariables* liveness = context->getAnalysis<clang::LiveVariables>()) {
      DeadStoreObserver observer(recorder);
      liveness->runOnAllBlocks(observer);
    }
  }
}

}  // namespace misra
