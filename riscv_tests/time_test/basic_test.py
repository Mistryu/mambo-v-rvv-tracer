#!/usr/bin/env python3
import subprocess, json, re, sys

OUTPUT  = "trace_basic.json"

def rvv_classify(inst):
    op, f3 = inst & 0x7F, (inst >> 12) & 0x7
    if op == 0x57: return 2 if f3 == 0b111 else 1
    if op in (0x07, 0x27) and f3 in (0,5,6,7): return 3
    return 0

def send(g, cmd):
    g.stdin.write(cmd + "\n")
    g.stdin.flush()
    lines = []
    while True:
        l = g.stdout.readline()
        if not l: break
        l = l.rstrip()
        lines.append(l)
        if l.startswith("^"): break
    return "\n".join(lines)

def wait_stop(g):
    while True:
        l = g.stdout.readline()
        if not l: return False
        if "*stopped" in l:
            return "exited" not in l
        if "^error" in l: return False

def read_reg(g, name):
    raw = send(g, f"-interpreter-exec console \"info registers {name}\"")
    m = re.search(rf'{name}\s+0x([0-9a-fA-F]+)', raw)
    if m: return int(m.group(1), 16)
    return 0

def read_inst(g):
    raw = send(g, "-data-read-memory-bytes $pc 4")
    m = re.search(r'contents="([0-9a-fA-F]+)"', raw)
    if not m: return None
    data = m.group(1)
    return int("".join(reversed([data[i:i+2] for i in range(0,8,2)])), 16)

def vreg(g, n, vlenb):
    raw = send(g, f"-data-read-memory-bytes $v{n} {vlenb}")
    m = re.search(r'contents="([0-9a-fA-F]+)"', raw)
    if not m:
        return "00" * vlenb
    return m.group(1).lower().ljust(vlenb * 2, "0")

def main():
    if len(sys.argv) != 2:
        sys.exit(f"Usage: {sys.argv[0]} <program>")

    g = subprocess.Popen(
        ["gdb", "--interpreter=mi3", "-q", "--nx", sys.argv[1]],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, bufsize=1)

    # Waiting for gdb to be ready
    for _ in range(30):
        l = g.stdout.readline().rstrip()
        if l.startswith("(gdb)"): break

    # Disabling anything that could interfere with tracing
    send(g, "-gdb-set debuginfod enabled off")
    send(g, "-gdb-set pagination off")
    send(g, "-gdb-set confirm off")
    send(g, "-break-insert main")
    send(g, "-exec-run")
    wait_stop(g)

    vlenb = read_reg(g, "vlenb") or 32

    n, total = 0, 0
    with open(OUTPUT, "w") as f:
        f.write("[\n")
        first = True
        while True:
            inst = read_inst(g)
            if inst is None: break
            total += 1

            typ = rvv_classify(inst)
            if typ != 0:
                pc     = read_reg(g, "pc")
                vl     = read_reg(g, "vl")
                vtype  = read_reg(g, "vtype")
                vstart = read_reg(g, "vstart")
                vd, vs1, vs2 = (inst>>7)&0x1F, (inst>>15)&0x1F, (inst>>20)&0x1F

                e = {"n": n, "pc": f"0x{pc:016x}", "inst": f"0x{inst:08x}", "type": typ,
                    "vl": vl, "vtype": vtype, "vstart": vstart, "vlenb": vlenb}

                if typ == 1:
                    e.update(vd=vd,   vd_data=vreg(g,vd,vlenb),
                            vs1=vs1, vs1_data=vreg(g,vs1,vlenb),
                            vs2=vs2, vs2_data=vreg(g,vs2,vlenb))
                elif typ == 3:
                    e.update(rs1=vs1, rs1_value=f"0x{read_reg(g,f'x{vs1}'):016x}",
                            vd=vd,   vd_data=vreg(g,vd,vlenb))
                elif typ == 2:
                    e.update(rd=vd,   rd_value=f"0x{read_reg(g,f'x{vd}'):016x}",
                            rs1=vs1, rs1_value=f"0x{read_reg(g,f'x{vs1}'):016x}",
                            rs2=vs2, rs2_value=f"0x{read_reg(g,f'x{vs2}'):016x}")

                send(g, "-exec-step-instruction")
                if not wait_stop(g):
                    prefix = "" if first else ","
                    f.write(prefix + json.dumps(e) + "\n")
                    f.flush()
                    first = False
                    n += 1
                    break

                if typ in (1, 3):
                    e["vd_data_after"] = vreg(g, vd, vlenb)

                prefix = "" if first else ","
                f.write(prefix + json.dumps(e) + "\n"); f.flush()
                first = False; n += 1
            else:
                send(g, "-exec-step-instruction")
                if not wait_stop(g): break

            if total % 1000 == 0:
                print(f"  {total} instructions, {n} vector so far", file=sys.stderr)

        f.write("]\n")

    print(f"Done: {n} vector / {total} total -> {OUTPUT}", file=sys.stderr)
    send(g, "-gdb-exit")

if __name__ == "__main__":
    main()