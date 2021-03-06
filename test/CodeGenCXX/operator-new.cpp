// RUN: %clang_cc1 -triple i686-pc-linux-gnu -emit-llvm -o %t-1.ll %s
// RUN: FileCheck -check-prefix SANE --input-file=%t-1.ll %s
// RUN: %clang_cc1 -triple i686-pc-linux-gnu -emit-llvm -fno-assume-sane-operator-new -o %t-2.ll %s
// RUN: FileCheck -check-prefix SANENOT --input-file=%t-2.ll %s


class teste {
  int A;
public:
  teste() : A(2) {}
};

void f1() {
  // SANE: declare noalias i8* @_Znwj(
  // SANENOT: declare i8* @_Znwj(
  new teste();
}


// rdar://5739832 - operator new should check for overflow in multiply.
void *f2(long N) {
  return new int[N];
  
// SANE: call{{.*}}@llvm.umul.with.overflow
// SANE: extractvalue
// SANE: br i1
// SANE: = phi {{.*}} [ {{.*}} ], [ -1,
// SANE:  call noalias i8* @_Znaj(
}
