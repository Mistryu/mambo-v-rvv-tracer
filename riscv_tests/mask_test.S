.section .data
a:
    .word 1,2,3,4,5,6,7,8
b:
    .word 10,10,10,10,10,10,10,10

.section .text
.global mask_test
.type mask_test, @function

mask_test:

    li t0,8

    vsetvli t0, t0, e32, m1

    la t1, a
    vle32.v v1, (t1)

    la t2, b
    vle32.v v2, (t2)

    # mask: a > 4
    vmsgt.vi v0, v1, 4

    # masked add
    vadd.vv v3, v1, v2, v0.t

    # non-masked add
    vadd.vv v4, v1, v2

    ret