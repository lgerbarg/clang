//== Environment.cpp - Map from Stmt* to Locations/Values -------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defined the Environment and EnvironmentManager classes.
//
//===----------------------------------------------------------------------===//

#include "clang/Analysis/AnalysisContext.h"
#include "clang/Analysis/CFG.h"
#include "clang/Checker/PathSensitive/GRState.h"

using namespace clang;

SVal Environment::GetSVal(const Stmt *E, ValueManager& ValMgr) const {

  for (;;) {

    switch (E->getStmtClass()) {

      case Stmt::AddrLabelExprClass:
        return ValMgr.makeLoc(cast<AddrLabelExpr>(E));

        // ParenExprs are no-ops.

      case Stmt::ParenExprClass:
        E = cast<ParenExpr>(E)->getSubExpr();
        continue;

      case Stmt::CharacterLiteralClass: {
        const CharacterLiteral* C = cast<CharacterLiteral>(E);
        return ValMgr.makeIntVal(C->getValue(), C->getType());
      }

      case Stmt::CXXBoolLiteralExprClass: {
        const SVal *X = ExprBindings.lookup(E);
        if (X) 
          return *X;
        else 
          return ValMgr.makeIntVal(cast<CXXBoolLiteralExpr>(E));
      }
      case Stmt::IntegerLiteralClass: {
        // In C++, this expression may have been bound to a temporary object.
        SVal const *X = ExprBindings.lookup(E);
        if (X)
          return *X;
        else
          return ValMgr.makeIntVal(cast<IntegerLiteral>(E));
      }

      // Casts where the source and target type are the same
      // are no-ops.  We blast through these to get the descendant
      // subexpression that has a value.

      case Stmt::ImplicitCastExprClass:
      case Stmt::CStyleCastExprClass: {
        const CastExpr* C = cast<CastExpr>(E);
        QualType CT = C->getType();

        if (CT->isVoidType())
          return UnknownVal();

        break;
      }

        // Handle all other Stmt* using a lookup.

      default:
        break;
    };

    break;
  }

  return LookupExpr(E);
}

Environment EnvironmentManager::BindExpr(Environment Env, const Stmt *S,
                                         SVal V, bool Invalidate) {
  assert(S);

  if (V.isUnknown()) {
    if (Invalidate)
      return Environment(F.Remove(Env.ExprBindings, S));
    else
      return Env;
  }

  return Environment(F.Add(Env.ExprBindings, S, V));
}

namespace {
class MarkLiveCallback : public SymbolVisitor {
  SymbolReaper &SymReaper;
public:
  MarkLiveCallback(SymbolReaper &symreaper) : SymReaper(symreaper) {}
  bool VisitSymbol(SymbolRef sym) { SymReaper.markLive(sym); return true; }
};
} // end anonymous namespace

static bool isBlockExprInCallers(const Stmt *E, const LocationContext *LC) {
  const LocationContext *ParentLC = LC->getParent();
  while (ParentLC) {
    CFG &C = *ParentLC->getCFG();
    if (C.isBlkExpr(E))
      return true;
    ParentLC = ParentLC->getParent();
  }

  return false;
}


// RemoveDeadBindings:
//  - Remove subexpression bindings.
//  - Remove dead block expression bindings.
//  - Keep live block expression bindings:
//   - Mark their reachable symbols live in SymbolReaper,
//     see ScanReachableSymbols.
//   - Mark the region in DRoots if the binding is a loc::MemRegionVal.

Environment
EnvironmentManager::RemoveDeadBindings(Environment Env,
                                       SymbolReaper &SymReaper,
                                       const GRState *ST,
                              llvm::SmallVectorImpl<const MemRegion*> &DRoots) {

  CFG &C = *SymReaper.getLocationContext()->getCFG();

  // We construct a new Environment object entirely, as this is cheaper than
  // individually removing all the subexpression bindings (which will greatly
  // outnumber block-level expression bindings).
  Environment NewEnv = getInitialEnvironment();

  // Iterate over the block-expr bindings.
  for (Environment::iterator I = Env.begin(), E = Env.end();
       I != E; ++I) {

    const Stmt *BlkExpr = I.getKey();
    const SVal &X = I.getData();

    // Block-level expressions in callers are assumed always live.
    if (isBlockExprInCallers(BlkExpr, SymReaper.getLocationContext())) {
      NewEnv.ExprBindings = F.Add(NewEnv.ExprBindings, BlkExpr, X);

      if (isa<loc::MemRegionVal>(X)) {
        const MemRegion* R = cast<loc::MemRegionVal>(X).getRegion();
        DRoots.push_back(R);
      }

      // Mark all symbols in the block expr's value live.
      MarkLiveCallback cb(SymReaper);
      ST->scanReachableSymbols(X, cb);
      continue;
    }

    // Not a block-level expression?
    if (!C.isBlkExpr(BlkExpr))
      continue;

    if (SymReaper.isLive(BlkExpr)) {
      // Copy the binding to the new map.
      NewEnv.ExprBindings = F.Add(NewEnv.ExprBindings, BlkExpr, X);

      // If the block expr's value is a memory region, then mark that region.
      if (isa<loc::MemRegionVal>(X)) {
        const MemRegion* R = cast<loc::MemRegionVal>(X).getRegion();
        DRoots.push_back(R);
      }

      // Mark all symbols in the block expr's value live.
      MarkLiveCallback cb(SymReaper);
      ST->scanReachableSymbols(X, cb);
      continue;
    }

    // Otherwise the expression is dead with a couple exceptions.
    // Do not misclean LogicalExpr or ConditionalOperator.  It is dead at the
    // beginning of itself, but we need its UndefinedVal to determine its
    // SVal.
    if (X.isUndef() && cast<UndefinedVal>(X).getData())
      NewEnv.ExprBindings = F.Add(NewEnv.ExprBindings, BlkExpr, X);
  }

  return NewEnv;
}
