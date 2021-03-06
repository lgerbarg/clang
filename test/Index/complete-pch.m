// Note: the run lines follow their respective tests, since line/column
// matter in this test.

@interface C
- (int)instanceMethod3:(int)x;
+ (int)classMethod3:(float)f;
@end

void msg_id(id x) {
  [id classMethod1:1.0];
  [x instanceMethod1:5];
}

// Build the precompiled header
// RUN: %clang -x objective-c-header -o %t.h.pch %S/Inputs/complete-pch.h

// Run the actual tests
// RUN: c-index-test -code-completion-at=%s:10:7 -include %t.h %s | FileCheck -check-prefix=CHECK-CC1 %s
// CHECK-CC1: ObjCClassMethodDecl:{ResultType int}{TypedText classMethod1:}{Placeholder (double)d}
// CHECK-CC1: ObjCClassMethodDecl:{ResultType int}{TypedText classMethod2:}{Placeholder (float)f}
// CHECK-CC1: ObjCClassMethodDecl:{ResultType int}{TypedText classMethod3:}{Placeholder (float)f}

// RUN: c-index-test -code-completion-at=%s:11:6 -include %t.h %s | FileCheck -check-prefix=CHECK-CC2 %s
// CHECK-CC2: ObjCInstanceMethodDecl:{ResultType int}{TypedText instanceMethod1:}{Placeholder (int)x}
// CHECK-CC2: ObjCInstanceMethodDecl:{ResultType int}{TypedText instanceMethod2:}{Placeholder (int)x}
// CHECK-CC2: ObjCInstanceMethodDecl:{ResultType int}{TypedText instanceMethod3:}{Placeholder (int)x}
