//== SymbolManager.h - Management of Symbolic Values ------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines SymbolManager, a class that manages symbolic values
//  created for use by GRExprEngine and related classes.
//
//===----------------------------------------------------------------------===//

#include "clang/Checker/PathSensitive/SymbolManager.h"
#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/Checker/PathSensitive/MemRegion.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

void SymExpr::dump() const {
  dumpToStream(llvm::errs());
}

static void print(llvm::raw_ostream& os, BinaryOperator::Opcode Op) {
  switch (Op) {
    default:
      assert(false && "operator printing not implemented");
      break;
    case BinaryOperator::Mul: os << '*'  ; break;
    case BinaryOperator::Div: os << '/'  ; break;
    case BinaryOperator::Rem: os << '%'  ; break;
    case BinaryOperator::Add: os << '+'  ; break;
    case BinaryOperator::Sub: os << '-'  ; break;
    case BinaryOperator::Shl: os << "<<" ; break;
    case BinaryOperator::Shr: os << ">>" ; break;
    case BinaryOperator::LT:  os << "<"  ; break;
    case BinaryOperator::GT:  os << '>'  ; break;
    case BinaryOperator::LE:  os << "<=" ; break;
    case BinaryOperator::GE:  os << ">=" ; break;
    case BinaryOperator::EQ:  os << "==" ; break;
    case BinaryOperator::NE:  os << "!=" ; break;
    case BinaryOperator::And: os << '&'  ; break;
    case BinaryOperator::Xor: os << '^'  ; break;
    case BinaryOperator::Or:  os << '|'  ; break;
  }
}

void SymIntExpr::dumpToStream(llvm::raw_ostream& os) const {
  os << '(';
  getLHS()->dumpToStream(os);
  os << ") ";
  print(os, getOpcode());
  os << ' ' << getRHS().getZExtValue();
  if (getRHS().isUnsigned()) os << 'U';
}

void SymSymExpr::dumpToStream(llvm::raw_ostream& os) const {
  os << '(';
  getLHS()->dumpToStream(os);
  os << ") ";
  os << '(';
  getRHS()->dumpToStream(os);
  os << ')';
}

void SymbolConjured::dumpToStream(llvm::raw_ostream& os) const {
  os << "conj_$" << getSymbolID() << '{' << T.getAsString() << '}';
}

void SymbolDerived::dumpToStream(llvm::raw_ostream& os) const {
  os << "derived_$" << getSymbolID() << '{'
     << getParentSymbol() << ',' << getRegion() << '}';
}

void SymbolExtent::dumpToStream(llvm::raw_ostream& os) const {
  os << "extent_$" << getSymbolID() << '{' << getRegion() << '}';
}

void SymbolRegionValue::dumpToStream(llvm::raw_ostream& os) const {
  os << "reg_$" << getSymbolID() << "<" << R << ">";
}

const SymbolRegionValue*
SymbolManager::getRegionValueSymbol(const TypedRegion* R) {
  llvm::FoldingSetNodeID profile;
  SymbolRegionValue::Profile(profile, R);
  void* InsertPos;
  SymExpr *SD = DataSet.FindNodeOrInsertPos(profile, InsertPos);
  if (!SD) {
    SD = (SymExpr*) BPAlloc.Allocate<SymbolRegionValue>();
    new (SD) SymbolRegionValue(SymbolCounter, R);
    DataSet.InsertNode(SD, InsertPos);
    ++SymbolCounter;
  }

  return cast<SymbolRegionValue>(SD);
}

const SymbolConjured*
SymbolManager::getConjuredSymbol(const Stmt* E, QualType T, unsigned Count,
                                 const void* SymbolTag) {

  llvm::FoldingSetNodeID profile;
  SymbolConjured::Profile(profile, E, T, Count, SymbolTag);
  void* InsertPos;
  SymExpr *SD = DataSet.FindNodeOrInsertPos(profile, InsertPos);
  if (!SD) {
    SD = (SymExpr*) BPAlloc.Allocate<SymbolConjured>();
    new (SD) SymbolConjured(SymbolCounter, E, T, Count, SymbolTag);
    DataSet.InsertNode(SD, InsertPos);
    ++SymbolCounter;
  }

  return cast<SymbolConjured>(SD);
}

const SymbolDerived*
SymbolManager::getDerivedSymbol(SymbolRef parentSymbol,
                                const TypedRegion *R) {

  llvm::FoldingSetNodeID profile;
  SymbolDerived::Profile(profile, parentSymbol, R);
  void* InsertPos;
  SymExpr *SD = DataSet.FindNodeOrInsertPos(profile, InsertPos);
  if (!SD) {
    SD = (SymExpr*) BPAlloc.Allocate<SymbolDerived>();
    new (SD) SymbolDerived(SymbolCounter, parentSymbol, R);
    DataSet.InsertNode(SD, InsertPos);
    ++SymbolCounter;
  }

  return cast<SymbolDerived>(SD);
}

const SymbolExtent*
SymbolManager::getExtentSymbol(const SubRegion *R) {
  llvm::FoldingSetNodeID profile;
  SymbolExtent::Profile(profile, R);
  void* InsertPos;
  SymExpr *SD = DataSet.FindNodeOrInsertPos(profile, InsertPos);
  if (!SD) {
    SD = (SymExpr*) BPAlloc.Allocate<SymbolExtent>();
    new (SD) SymbolExtent(SymbolCounter, R);
    DataSet.InsertNode(SD, InsertPos);
    ++SymbolCounter;
  }

  return cast<SymbolExtent>(SD);
}

const SymIntExpr *SymbolManager::getSymIntExpr(const SymExpr *lhs,
                                               BinaryOperator::Opcode op,
                                               const llvm::APSInt& v,
                                               QualType t) {
  llvm::FoldingSetNodeID ID;
  SymIntExpr::Profile(ID, lhs, op, v, t);
  void *InsertPos;
  SymExpr *data = DataSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!data) {
    data = (SymIntExpr*) BPAlloc.Allocate<SymIntExpr>();
    new (data) SymIntExpr(lhs, op, v, t);
    DataSet.InsertNode(data, InsertPos);
  }

  return cast<SymIntExpr>(data);
}

const SymSymExpr *SymbolManager::getSymSymExpr(const SymExpr *lhs,
                                               BinaryOperator::Opcode op,
                                               const SymExpr *rhs,
                                               QualType t) {
  llvm::FoldingSetNodeID ID;
  SymSymExpr::Profile(ID, lhs, op, rhs, t);
  void *InsertPos;
  SymExpr *data = DataSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!data) {
    data = (SymSymExpr*) BPAlloc.Allocate<SymSymExpr>();
    new (data) SymSymExpr(lhs, op, rhs, t);
    DataSet.InsertNode(data, InsertPos);
  }

  return cast<SymSymExpr>(data);
}

QualType SymbolConjured::getType(ASTContext&) const {
  return T;
}

QualType SymbolDerived::getType(ASTContext& Ctx) const {
  return R->getValueType();
}

QualType SymbolExtent::getType(ASTContext& Ctx) const {
  return Ctx.getSizeType();
}

QualType SymbolRegionValue::getType(ASTContext& C) const {
  return R->getValueType();
}

SymbolManager::~SymbolManager() {}

bool SymbolManager::canSymbolicate(QualType T) {
  if (Loc::IsLocType(T))
    return true;

  if (T->isIntegerType())
    return T->isScalarType();

  if (T->isRecordType())
    return true;

  return false;
}

void SymbolReaper::markLive(SymbolRef sym) {
  TheLiving.insert(sym);
  TheDead.erase(sym);
}

bool SymbolReaper::maybeDead(SymbolRef sym) {
  if (isLive(sym))
    return false;

  TheDead.insert(sym);
  return true;
}

bool SymbolReaper::isLive(SymbolRef sym) {
  if (TheLiving.count(sym))
    return true;

  if (const SymbolDerived *derived = dyn_cast<SymbolDerived>(sym)) {
    if (isLive(derived->getParentSymbol())) {
      markLive(sym);
      return true;
    }
    return false;
  }

  if (const SymbolExtent *extent = dyn_cast<SymbolExtent>(sym)) {
    const MemRegion *Base = extent->getRegion()->getBaseRegion();
    if (const VarRegion *VR = dyn_cast<VarRegion>(Base))
      return isLive(VR);
    if (const SymbolicRegion *SR = dyn_cast<SymbolicRegion>(Base))
      return isLive(SR->getSymbol());
    return false;
  }

  // Interogate the symbol.  It may derive from an input value to
  // the analyzed function/method.
  return isa<SymbolRegionValue>(sym);
}

bool SymbolReaper::isLive(const Stmt* ExprVal) const {
  return LCtx->getLiveVariables()->isLive(Loc, ExprVal);
}

bool SymbolReaper::isLive(const VarRegion *VR) const {
  const StackFrameContext *SFC = VR->getStackFrame();

  if (SFC == LCtx->getCurrentStackFrame())
    return LCtx->getLiveVariables()->isLive(Loc, VR->getDecl());
  else
    return SFC->isParentOf(LCtx->getCurrentStackFrame());
}

SymbolVisitor::~SymbolVisitor() {}
