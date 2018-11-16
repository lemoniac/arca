import subprocess
import unittest
import re


class TestBase(unittest.TestCase):
    def compile_file(self, filename):
        """compiles 'filename' and returns the standard output and error"""
        proc = subprocess.Popen(["../cc", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertGreaterEqual(proc.wait(), 0, "compiler crash " + filename)
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        return (out, err)


    def assemble(self, code):
        file("out.s", "wt").write(code)
        proc = subprocess.Popen(["../../asm/asm", "out.s", "out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertGreaterEqual(proc.wait(), 0, "assembler crashed")
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        return (out, err)


    def run_file(self, filename):
        proc = subprocess.Popen(["../../../emu", "--no-gpu", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertGreaterEqual(proc.wait(), 0, "vm crashed")
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        r1 = self.parse_vm_output(out)

        return (out, err, r1)


    def parse_vm_output(self, out):
        return int(re.search("r1: (\d+)", out).group(1))
