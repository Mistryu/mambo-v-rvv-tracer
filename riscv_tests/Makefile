CC ?= riscv64-unknown-linux-gnu-gcc
CFLAGS = -march=rv64gcv -mabi=lp64d -O2 -g
LDFLAGS = 

all: test_rvv_reg_reg test_rvv_simple test_vs1r test_vs1r_simple test_rvv_v0 test_rvv_v1 rvv_image_processing rvv_signal_processing rvv_simple_filter


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

rvv_image_processing: rvv_image_processing.c
	$(CC) $(CFLAGS) -o $@ $< -lm

rvv_signal_processing: rvv_signal_processing.c
	$(CC) $(CFLAGS) -o $@ $< -lm

rvv_simple_filter: rvv_simple_filter.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_rvv_reg_reg test_rvv_simple test_vs1r test_rvv_v0 test_rvv_v1 rvv_image_processing test_vs1r_simple rvv_signal_processing rvv_simple_filter
.PHONY: all clean

