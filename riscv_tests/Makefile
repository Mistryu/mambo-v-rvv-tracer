CC ?= riscv64-unknown-linux-gnu-gcc
CFLAGS = -march=rv64gcv -mabi=lp64d -O2 -g

all: test_rvv_reg_reg test_rvv_simple rvv_image_processing rvv_signal_processing rvv_simple_filter test_rvv_special mask_test rvv_vtype_bug ternary_incorrect rvv_token


test_rvv_reg_reg: test_rvv_reg_reg.S test_rvv_reg_reg.c
	$(CC) $(CFLAGS) $^ -o $@

test_rvv_simple: test_rvv_simple.S test_rvv_simple.c
	$(CC) $(CFLAGS) $^  -o $@

rvv_image_processing: rvv_image_processing.c
	$(CC) $(CFLAGS) -o $@ $< -lm

rvv_signal_processing: rvv_signal_processing.c
	$(CC) $(CFLAGS) -o $@ $< -lm

rvv_simple_filter: rvv_simple_filter.c
	$(CC) $(CFLAGS) -o $@ $< -lm

test_rvv_special: test_rvv_special.S test_rvv_special.c
	$(CC) $(CFLAGS) $^ -o $@

mask_test: mask_test.S mask_test.c
	$(CC) $(CFLAGS) $^ -o $@

ternary_incorrect: ternary_incorrect.S ternary_incorrect.c
	$(CC) $(CFLAGS) $^ -o $@

rvv_vtype_bug: rvv_vtype_bug.S rvv_vtype_bug.c
	$(CC) $(CFLAGS) $^ -o $@

rvv_token: rvv_token.S rvv_token.c
	$(CC) $(CFLAGS) $^ -o $@ -lm

clean:
	rm -f test_rvv_reg_reg test_rvv_simple rvv_image_processing rvv_simple_filter rvv_signal_processing test_rvv_special mask_test rvv_vtype_bug ternary_incorrect rvv_token

.PHONY: all clean
# cc -march=rv64gcv -mabi=lp64d -O2 -g mask_test.S -o mask_test
