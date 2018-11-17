import subprocess
import unittest
import re

def popen(args):
    return subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)


class TestBase(unittest.TestCase):
    def compile_file(self, filename):
        """compiles 'filename' and returns the standard output and error"""
        proc = popen(["../cc", filename])
        self.assertGreaterEqual(proc.wait(), 0, "compiler crash " + filename)
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        return (out, err)


    def assemble(self, code):
        """Call the assembler"""
        open("out.s", "wt").write(code)
        proc = popen(["../../asm/asm", "out.s", "out"])
        self.assertGreaterEqual(proc.wait(), 0, "assembler crashed")
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        return (out, err)


    def run_file(self, filename):
        """run a file in the virtual machine and return the output and the return code"""
        proc = popen(["../../../emu", "--no-gpu", filename])
        self.assertGreaterEqual(proc.wait(), 0, "vm crashed")
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        r1 = parse_vm_output(out)

        return (out, err, r1)


def parse_vm_output(out):
    """Extract the return code"""
    return int(re.search("r1: (\\d+)", out).group(1))
