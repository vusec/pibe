; ModuleID = 'basic.ll'
source_filename = "basic.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZTV1A = constant i8 0, !type !0, !type !1
@_ZTV1B = constant i8 0, !type !0, !type !1, !type !2, !type !3

define signext i8 @f11() !type !5 !type !6 {
entry:
  ret i8 1
}

define signext i8 @f12() !type !5 !type !6 {
entry:
  ret i8 2
}

define signext i8 @f13() !type !5 !type !6 {
entry:
  ret i8 3
}

define i32 @f21() !type !7 !type !8 {
entry:
  ret i32 4
}

define i32 @f22() !type !7 !type !8 {
entry:
  ret i32 5
}

define weak_odr hidden void @__cfi_check_fail(i8* %0, i8* %1) {
entry:
  ret void
}

define void @__cfi_check(i64 %CallSiteTypeId, i8* %Addr, i8* %CFICheckFailData) align 4096 {
entry:
  switch i64 %CallSiteTypeId, label %fail [
    i64 111, label %test
    i64 222, label %test1
    i64 333, label %test2
    i64 444, label %test3
  ]

exit:                                             ; preds = %test3, %test2, %test1, %test, %fail
  ret void

fail:                                             ; preds = %test3, %test2, %test1, %test, %entry
  call void @__cfi_check_fail(i8* %CFICheckFailData, i8* %Addr)
  br label %exit

test:                                             ; preds = %entry
  %0 = call i1 @llvm.type.test(i8* %Addr, metadata i64 111)
  br i1 %0, label %exit, label %fail, !prof !9

test1:                                            ; preds = %entry
  %1 = call i1 @llvm.type.test(i8* %Addr, metadata i64 222)
  br i1 %1, label %exit, label %fail, !prof !9

test2:                                            ; preds = %entry
  %2 = call i1 @llvm.type.test(i8* %Addr, metadata i64 333)
  br i1 %2, label %exit, label %fail, !prof !9

test3:                                            ; preds = %entry
  %3 = call i1 @llvm.type.test(i8* %Addr, metadata i64 444)
  br i1 %3, label %exit, label %fail, !prof !9
}

; Function Attrs: nounwind readnone willreturn
declare i1 @llvm.type.test(i8*, metadata) #0

attributes #0 = { nounwind readnone willreturn }

!llvm.module.flags = !{!4}

!0 = !{i64 16, !"_ZTS1A"}
!1 = !{i64 16, i64 333}
!2 = !{i64 16, !"_ZTS1B"}
!3 = !{i64 16, i64 444}
!4 = !{i32 4, !"Cross-DSO CFI", i32 1}
!5 = !{i64 0, !"_ZTSFcvE"}
!6 = !{i64 0, i64 111}
!7 = !{i64 0, !"_ZTSFivE"}
!8 = !{i64 0, i64 222}
!9 = !{!"branch_weights", i32 1048575, i32 1}
