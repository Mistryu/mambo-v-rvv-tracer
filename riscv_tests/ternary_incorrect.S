
# For each element i:
#   s[i]   = a[i] + b[i]
#   p[i]   = s[i] * w[i]
#   out[i] = (p[i] > 100) ? p[i] : 0
#
# Equivalent one-liner:
#   out[i] = (((a[i] + b[i]) * w[i]) > 100) ? ((a[i] + b[i]) * w[i]) : 0


.section .text
.global ternary_incorrect
.type ternary_incorrect, @function

ternary_incorrect:
    la      a0, A
    la      a1, B
    la      a2, W

    # VL=8, SEW=32
    vsetivli zero, 8, e32, m1, ta, ma

    # v1=a, v2=b, v3=w
    vle32.v   v1, (a0)
    vle32.v   v2, (a1)
    vle32.v   v3, (a2)

    # v4 = a+b
    vadd.vv   v4, v1, v2

    # v5 = (a+b)*w
    vmul.vv   v5, v4, v3

    # mask v0 = (v5 > 100)
    li        t0, 100
    vmsgt.vx  v0, v5, t0

    # v7 = (v5 > 100) ? v5 : 0
    vmv.v.i   v6, 0 # Clear v6 to use as the false value
    vmerge.vvm v7, v6, v5, v0

    vmv.v.i   v8, 0 # Clear v8 to accumulate the sum of outputs
    vredsum.vs v8, v1, v8  # Error 
    ret

.section .data
.align 4
A:          .word 1,2,3,4,5,6,7,8
B:          .word 8,7,6,5,4,3,2,1
W:          .word 10,11,12,13,14,15,16,17