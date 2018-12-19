import subprocess
import re


def popen(args):
    return subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

def compile_file(filename):
    """compiles 'filename' and returns the standard output and error"""
    proc = popen(["../cc", filename])
    exitcode = proc.wait()
    if exitcode < 0:
        return (None, "compiler crash " + filename)
    if exitcode != 0:
        return (None, "compiler failed: " + filename)
    err = ""
    for line in proc.stderr:
        err += line
    out = ""
    for line in proc.stdout:
        out += line

    return (out, err)


def assemble(code, name = ""):
    """Call the assembler"""
    open("out.s", "wt").write(code)
    proc = popen(["../../asm/asm", "out.s", "out"])
    exitcode = proc.wait()
    if exitcode < 0:
        return (None, "assembler crashed: " + name)
    if exitcode != 0:
        return (None, "assembler failed: " + name)
    err = ""
    for line in proc.stderr:
        err += line
    out = ""
    for line in proc.stdout:
        out += line

    return (out, err)

def run_file(filename, params):
    """run a file in the virtual machine and return the output and the return code"""
    args = ["../../../emu"]
    if "gpu" not in params or not params["gpu"]:
        args.append("--no-gpu")
    if "screenshot" in params:
        args.append("--screenshot")
    args.append(filename)
    proc = popen(args)
    if proc.wait() < 0:
        return (None, "vm crashed", None)
    err = ""
    for line in proc.stderr:
        err += line
    out = ""
    for line in proc.stdout:
        out += line

    r1 = parse_vm_output(out)

    return (out, err, r1)


def diff_files(file1, file2):
    proc = popen(["diff", file1, file2])
    return proc.wait()



def parse_vm_output(out):
    """Extract the return code"""
    return int(re.search("r1: (\\d+)", out).group(1))
