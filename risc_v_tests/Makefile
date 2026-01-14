CC ?= riscv64-unknown-linux-gnu-gcc
CFLAGS = -march=rv64gcv -mabi=lp64d -O2 -g
LDFLAGS = 

all: test_rvv_reg_reg test_rvv_simple test_vs1r test_vs1r_simple test_rvv_v0 test_rvv_v1

test_rvv_reg_reg: test_rvv_reg_reg.S test_rvv_reg_reg.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_rvv_simple: test_rvv_simple.S test_rvv_simple.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_vs1r: test_vs1r.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_vs1r_simple: test_vs1r_simple.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_rvv_v0: test_rvv_v0.S test_rvv_v0.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_rvv_v1: test_rvv_v1.S test_rvv_v1.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f test_rvv_reg_reg test_rvv_simple test_vs1r test_rvv_v0 test_rvv_v1
	rm -f test_vs1r_simple
.PHONY: all clean

