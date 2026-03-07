#!/usr/bin/env python3
import gdb, json, re
from typing import Optional, List

OUTPUT = "trace_gdb_api.json"

# call-skip helpers
_CALL_MNEMONICS = ("jal", "jalr")
HEX_ADDR_RE     = re.compile(r"0x[0-9a-fA-F]+")
FIRST_TOKEN_RE  = re.compile(r"^([^\s]+)\s+")
OF_PATH_RE      = re.compile(r"\bof\s+(\S+)")
SKIP_PREFIX     = "/lib"

def _current_insn_line() -> str:
    return gdb.execute("x/i $pc", to_string=True)

def _looks_like_call(insn_line: str) -> bool:
    asm = insn_line.split(":", 1)[1] if ":" in insn_line else insn_line
    asm = asm.strip().lower()
    return any(re.search(rf"\b{m}\b", asm) for m in _CALL_MNEMONICS)

def _extract_target_addr(insn_line: str):
    addrs = HEX_ADDR_RE.findall(insn_line)
    return int(addrs[-1], 16) if addrs else None

def _info_symbol_text(expr: str):
    try:    return gdb.execute(f"info symbol {expr}", to_string=True).strip()
    except: return None

def _sym_at_addr(addr: int):
    out = _info_symbol_text(f"{addr:#x}")
    if not out: return None
    m = FIRST_TOKEN_RE.match(out)
    return m.group(1) if m else None

def _normalize_sym(sym: str) -> str:
    sym = sym.split("+", 1)[0].replace("@plt", "").split("@@", 1)[0]
    return sym.strip()

def _file_of_symbol(sym: str):
    out = _info_symbol_text(sym)
    if not out: return None
    m = OF_PATH_RE.search(out)
    return m.group(1) if m else None

def _is_stdlib_call(target_addr) -> bool:
    if target_addr is None: return False
    sym = _sym_at_addr(target_addr)
    if not sym: return False
    path = _file_of_symbol(_normalize_sym(sym))
    if not path: return False
    return path.startswith(SKIP_PREFIX)

def safe_stepi():
    """Step one instruction, but skip over stdlib/syscall calls with ni."""
    insn = _current_insn_line()
    if _looks_like_call(insn) and _is_stdlib_call(_extract_target_addr(insn)):
        sym = _sym_at_addr(_extract_target_addr(insn))
        gdb.write(f"  [skip] stepping over {sym}\n")
        gdb.execute("ni", to_string=True)
    else:
        gdb.execute("stepi", to_string=True)

def read_vector_reg(name: str) -> Optional[List[int]]:
    """Read a vector register; returns list of byte-ints or None."""
    try:
        val        = gdb.parse_and_eval("$" + name)
        word_field = val.type["b"]
        values     = val[word_field]
        lo, hi     = values.type.range()
        return [int.from_bytes(bytes(values[i].bytes), "little")
                for i in range(lo, hi + 1)]
    except gdb.error:
        return None

def read_scalar_reg(name: str):
    """Read a scalar/CSR register as int, or None on error."""
    try:
        val    = gdb.parse_and_eval("$" + name)
        nbytes = val.type.sizeof
        ull    = gdb.lookup_type("unsigned long long")
        return int(val.cast(ull))
    except gdb.error:
        try:
            return int(gdb.selected_frame().read_register(name))
        except gdb.error:
            return None

def rvv_classify(inst: int) -> int:
    op, f3 = inst & 0x7F, (inst >> 12) & 0x7
    if op == 0x57:                               return 2 if f3 == 0b111 else 1
    if op in (0x07, 0x27) and f3 in (0,5,6,7):  return 3
    return 0

def read_inst(pc: int) -> int:
    b = bytes(gdb.selected_inferior().read_memory(pc, 4))
    return int.from_bytes(b, "little")

def is_alive() -> bool:
    try:    gdb.selected_frame(); return True
    except: return False


# Main tracer
def trace():
    n, total = 0, 0
    vlenb = None

    with open(OUTPUT, "w") as f:
        f.write("[\n")
        first = True

        while is_alive():
            pc   = read_scalar_reg("pc")
            inst = read_inst(pc)
            total += 1
            typ  = rvv_classify(inst)

            if typ:
                if vlenb is None:
                    vlenb = read_scalar_reg("vlenb") or 32

                vd  = (inst >>  7) & 0x1F
                vs1 = (inst >> 15) & 0x1F
                vs2 = (inst >> 20) & 0x1F

                e = {
                    "n":      n,
                    "pc":     f"0x{pc:016x}",
                    "inst":   f"0x{inst:08x}",
                    "type":   typ,
                    "vl":     read_scalar_reg("vl"),
                    "vtype":  read_scalar_reg("vtype"),
                    "vstart": read_scalar_reg("vstart"),
                    "vlenb":  vlenb,
                }

                if typ == 1:   # vector arithmetic
                    e.update(
                        vd=vd,
                        vs1=vs1, vs1_data=read_vector_reg(f"v{vs1}"),
                        vs2=vs2, vs2_data=read_vector_reg(f"v{vs2}"),
                    )
                elif typ == 3: # vector load/store
                    e.update(
                        rs1=vs1,
                        rs1_value=f"0x{read_scalar_reg(f'x{vs1}'):016x}",
                        vd=vd,
                    )
                elif typ == 2: # vset* / CSR
                    e.update(
                        rd=vd,   rd_value=f"0x{read_scalar_reg(f'x{vd}'):016x}",
                        rs1=vs1, rs1_value=f"0x{read_scalar_reg(f'x{vs1}'):016x}",
                        rs2=vs2, rs2_value=f"0x{read_scalar_reg(f'x{vs2}'):016x}",
                    )

                safe_stepi()

                if typ in (1, 3) and is_alive():
                    e["vd_data_after"] = read_vector_reg(f"v{vd}")

                f.write(("" if first else ",\n") + json.dumps(e))
                f.flush()
                first, n = False, n + 1

            else:
                safe_stepi()

            if total % 1000 == 0:
                gdb.write(f"  {total} stepped, {n} vector instructions traced...\n")

        f.write("\n]\n")

    gdb.write(f"Done: {n} vector / {total} total -> {OUTPUT}\n")


# ── GDB setup ─────────────────────────────────────────────────────────────────
gdb.execute("set debuginfod enabled off")
gdb.execute("set pagination off")
gdb.execute("set confirm off")
gdb.execute("set print inferior-events off")
gdb.execute("break main")
gdb.execute("run")

trace()
gdb.execute("quit")