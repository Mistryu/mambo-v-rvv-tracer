#!/usr/bin/env python3
import subprocess, time

PROGRAMS = [
    ("test_rvv_simple",       "Simple test"),
    ("rvv_simple_filter",     "Basic loop"),
    ("rvv_image_processing",  "Larger program"),
    ("rvv_signal_processing", "Very large program"),
]

def run(cmd):
    start = time.perf_counter()
    r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elapsed = time.perf_counter() - start
    return elapsed, r.returncode

def run_native(prog):
    start = time.perf_counter()
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(prog, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elapsed = time.perf_counter() - start
    return elapsed

def run_dbm(prog):
    start = time.perf_counter()
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["../../dbm", prog], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elapsed = time.perf_counter() - start
    return elapsed

def p(s=""):
    out.write(s + "\n")
    out.flush()

def main():
    global out
    with open("output.txt", "w") as out:

        p("--- Native ---")
        p(f"{'Program':<25} {'Description':<20} {'Time (s)':>10}")
        p("-" * 60)
        native_t = {}
        for prog, desc in PROGRAMS:
            t = run_native([f"../{prog}"])
            native_t[prog] = t / 100  # We run each program 100 times in run_native, so we divide by 100 to get the average time
            p(f"{prog:<25} {desc:<20} {f'{native_t[prog]:.3f}':>10}")
        p("-" * 60)


        p("\n--- DBM ---")
        p(f"{'Program':<25} {'Description':<20} {'Time (s)':>10}")
        p("-" * 60)
        dbm_t = {}
        for prog, desc in PROGRAMS:
            t = run_dbm(f"../{prog}")
            dbm_t[prog] = t / 100  # We run each program 100 times in run_dbm, so we divide by 100 to get the average time
            p(f"{prog:<25} {desc:<20} {f'{dbm_t[prog]:.3f}':>10}")
        p("-" * 60)


        p("\n--- GDB API Tracer ---")
        p(f"{'Program':<25} {'Description':<20} {'Time (s)':>10}")
        p("-" * 60)
        api_times = {}
        for prog, desc in PROGRAMS:
            t, ret = run(["gdb", "-batch", "-x", "./gdb_api_test.py", f"../{prog}"])
            api_times[prog] = t
            t_str       = f"{t:.3f}" if ret == 0 else "FAILED"
            p(f"{prog:<25} {desc:<20} {t_str:>10}")
        p("-" * 60)
        
        
        p("\n--- GDB Basic Tracer ---")
        p(f"{'Program':<25} {'Description':<20} {'Time (s)':>10}")
        p("-" * 60)
        basic_t = {}
        for prog, desc in PROGRAMS:
            if prog == "rvv_signal_processing":
                # This one takes very long with the basic tracer, so we skip it
                basic_t[prog] = -1
                p(f"{prog:<25} {desc:<20} {'SKIPPED':>10}")
                continue
            t, ret = run(["python3", "basic_test.py", f"../{prog}"])
            basic_t[prog] = t
            t_str       = f"{t:.3f}" if ret == 0 else "FAILED"
            p(f"{prog:<25} {desc:<20} {t_str:>10}")
        p("-" * 60)

        p("\n\n\n -----    Main comparison    -----")
        
        p(f"{'Program':<25} {'Description':<20} {'Native (s)':>12} {'DBM (s)':>10} {'GDB Basic (s)':>15} {'DBM/Native':>12} {'Basic/Native':>12} {'Basic/DBM':>12}")
        p("-" * 100)
        for prog, desc in PROGRAMS:
            p(f"{prog:<25} {desc:<20} {f'{native_t[prog]:.3f}':>12} {f'{dbm_t[prog]:.3f}':>10} {f'{basic_t[prog]:.3f}':>15} {f'{dbm_t[prog]/native_t[prog]:.2f}x':>12} {f'{basic_t[prog]/native_t[prog]:.2f}x':>12} {f'{basic_t[prog]/dbm_t[prog]:.2f}x':>10}")
            
       
        p('\n\n\n' )
        p(f"{'Program':<25} {'Description':<20} {'Native (s)':>12} {'DBM (s)':>10}  {'GDB Tracer API (s)':>15} {'DBM/Native':>12} {'API/Native':>12} {'API/DBM':>10} ")
        p("-" * 100)
        for prog, desc in PROGRAMS:
            p(f"{prog:<25} {desc:<20} {f'{native_t[prog]:.3f}':>12} {f'{dbm_t[prog]:.3f}':>10} {f'{api_times[prog]:.3f}':>15} {f'{dbm_t[prog]/native_t[prog]:.2f}x':>12} {f'{api_times[prog]/native_t[prog]:.2f}x':>12} {f'{api_times[prog]/dbm_t[prog]:.2f}x':>10}")
            

        p("\n\n\n --- GDB Basic vs GDB API ---")
        p(f"{'Program':<25} {'Description':<20} {'GDB Basic (s)':>12} {'GDB Tracer API (s)':>10} {'GDB/API':>12}")
        p("-" * 100)
        for prog, desc in PROGRAMS:
            p(f"{prog:<25} {desc:<20} {f'{basic_t[prog]:.3f}':>12} {f'{api_times[prog]:.3f}':>15} {f'{basic_t[prog]/api_times[prog]:.2f}x':>12} ")
        p("-" * 100)

if __name__ == "__main__":
    main()