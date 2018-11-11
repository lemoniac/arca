import subprocess
import unittest

class TestSyntax(unittest.TestCase):
    files = [
        ["test_array_000.c", False],
        ["test_expressions_000.c", False],
        ["test_for_000.c", False],
        ["test_for_001.c", True],
        ["test_function_000.c", False],
        ["test_globals_000.c", False],
        ["test_struct_000.c", False],
    ]

    def runfile(self, filename):
        """compiles 'filename' and returns the standard output and error"""
        proc = subprocess.Popen(["../cc", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertGreaterEqual(proc.wait(), 0, "compiler crash")
        err = ""
        for line in proc.stderr:
            err += line
        out = ""
        for line in proc.stdout:
            out += line

        return (out, err)


    def test_parse(self):
        for item in self.files:
            (out, err) = self.runfile(item[0])
            self.assertEqual(len(err) > 0, item[1], item[0] + " expected " + str(item[1]) + "\n" + err)

if __name__ == '__main__':
    unittest.main()
