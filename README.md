# mambo-thesis (Fork of MAMBO-V)

This repository is a fork of **MAMBO-V** (itself based on MAMBO), adapted for RVV-focused tracing and analysis workflows used in thesis experiments.

## What this fork is

`mambo-thesis` keeps the core dynamic binary modification/runtime translation model from MAMBO, and adds RVV-oriented plugin to make vector instruction behavior easier to inspect and debug.

## How it works

At runtime, MAMBO executes target binaries through its DBM engine and lets plugins instrument translated code.

High-level flow:

1. Launch program through `dbm`
2. Translation happens block-by-block
3. Registered plugin callbacks inject instrumentation
4. Runtime records state/events (e.g., RVV instruction data)
5. Traces are exported for post-processing/visualization

This is especially useful for RVV debugging where issues in `vl`, `vtype` (`vsew`, `vlmul`), and register grouping are hard to spot in traditional debugger stepping.

## Basic usage

Run a program under MAMBO:

```bash
./dbm <path_to_executable> [args...]
```

Example:

```bash
./dbm program_to_trace
```

## RVV tracing workflow

Build and run an RVV test under tracing/instrumentation, then inspect trace output.

Example commands (adapt to your test paths):

```bash
./dbm /riscv_tests/rvv_vtype_bug
```

## Example RVV tests in this fork

- `riscv_tests/rvv_image_processing.S`
- `riscv_tests/test_rvv_simple.S`

The tests were created to contain as many RVV instructions as possible. Additionally there are a few test for specific functionality like masking or vtype bug to illustrate the usefulness of the trace in debugging

## Upstream references

- MAMBO: https://github.com/beehive-lab/mambo
- MAMBO-V work follows the same DBM/plugin model with RISC-V focus

## Related work

It is advised to use https://github.com/Mistryu/GUI-for-RVV-tracer for trace visualization.
