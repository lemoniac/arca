import unittest
import arcacc


class TestBase(unittest.TestCase):
    def compile_file(self, filename, assert_if_fail = False):
        """compiles 'filename' and returns the standard output and error"""
        (out, err) = arcacc.compile_file(filename)

        self.assertNotEqual(out, None, err)

        return (out, err)

    def assemble(self, code, name = ""):
        """Call the assembler"""
        (out, err) = arcacc.assemble(code, name)
        self.assertNotEqual(out, None, err)

        return (out, err)


    def run_file(self, filename, params):
        """run a file in the virtual machine and return the output and the return code"""
        (out, err, r1) = arcacc.run_file(filename, params)
        self.assertNotEqual(out, None, err)

        return (out, err, r1)
