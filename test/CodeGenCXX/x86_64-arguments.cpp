// RUN: %clang_cc1 -triple x86_64-unknown-unknown -emit-llvm -o - %s | FileCheck %s

// Basic base class test.
struct f0_s0 { unsigned a; };
struct f0_s1 : public f0_s0 { void *b; };
// CHECK: define void @_Z2f05f0_s1(i64 %a0.coerce0, i8* %a0.coerce1)
void f0(f0_s1 a0) { }

// Check with two eight-bytes in base class.
struct f1_s0 { unsigned a; unsigned b; float c; };
struct f1_s1 : public f1_s0 { float d;};
// CHECK: define void @_Z2f15f1_s1(i64 %a0.coerce0, double %a0.coerce1)
void f1(f1_s1 a0) { }

// Check with two eight-bytes in base class and merge.
struct f2_s0 { unsigned a; unsigned b; float c; };
struct f2_s1 : public f2_s0 { char d;};
// CHECK: define void @_Z2f25f2_s1(i64 %a0.coerce0, i64 %a0.coerce1)
void f2(f2_s1 a0) { }

// PR5831
// CHECK: define void @_Z2f34s3_1(i64 %x.coerce)
struct s3_0 {};
struct s3_1 { struct s3_0 a; long b; };
void f3(struct s3_1 x) {}

// CHECK: define i64 @_Z4f4_0M2s4i(i64 %a)
// CHECK: define {{.*}} @_Z4f4_1M2s4FivE(i64 %a.coerce0, i64 %a.coerce1)
struct s4 {};
typedef int s4::* s4_mdp;
typedef int (s4::*s4_mfp)();
s4_mdp f4_0(s4_mdp a) { return a; }
s4_mfp f4_1(s4_mfp a) { return a; }


namespace PR7523 {
struct StringRef {
  char *a;
};

void AddKeyword(StringRef, int x);

void foo() {
  // CHECK: define void @_ZN6PR75233fooEv()
  // CHECK: call void @_ZN6PR752310AddKeywordENS_9StringRefEi(i8* {{.*}}, i32 4)
  AddKeyword(StringRef(), 4);
}
}

namespace PR7742 { // Also rdar://8250764
  struct s2 {
    float a[2];
  };
  
  struct c2 : public s2 {};
  
  // CHECK: define double @_ZN6PR77423fooEPNS_2c2E(%"struct.PR7742::c2"* %P)
  c2 foo(c2 *P) {
  }
  
}

namespace PR5179 {
  struct B {};

  struct B1 : B {
    int* pa;
  };

  struct B2 : B {
    B1 b1;
  };

  // CHECK: define i8* @_ZN6PR51793barENS_2B2E(i32* %b2.coerce)
  const void *bar(B2 b2) {
    return b2.b1.pa;
  }
}
