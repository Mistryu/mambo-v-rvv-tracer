.section .text
.global rvv_vtype_bug
.type rvv_vtype_bug, @function

rvv_vtype_bug:
   
    li         t0, 8
    vsetvli    zero, t0, e32, m1, ta, ma
    vid.v      v4
    vadd.vi    v4, v4, 1
    # v4 = [1,2,3,4,5,6,7,8]

    li         t0, 8
    vsetvli    zero, t0, e64, m2, ta, ma
    vid.v      v8
    vadd.vi    v8, v8, 10
    # Filled widened destination group v8:v9 with [10..17]


    li         t0, 4 # Correct = 8
    vsetvli    zero, t0, e32, m1, ta, ma   # BUG

    # Widen i32 -> i64 into v8:v9
    # Buggy result:
    #   v8:v9 = [1,2,3,4,14,15,16,17]   (unchanged stale values)
    # Correct result:
    #   v8:v9 = [1,2,3,4,5,6,7,8]
    vwadd.vx   v8, v4, zero

    ret