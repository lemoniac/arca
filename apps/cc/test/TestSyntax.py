import subprocess
import unittest

def runfile(filename):
    """compiles 'filename' and returns the standard output and error"""
    proc = subprocess.Popen(["../cc", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    err = ""
    for line in proc.stderr:
        err += line
    out = ""
    for line in proc.stdout:
        out += line

    return (out, err)


class TestSyntax(unittest.TestCase):
    files = [
        ["test_expressions_000.c", False],
        ["test_for_000.c", False],
        ["test_for_001.c", True],
        ["test_function_000.c", False],
        ["test_globals_000.c", False],
    ]

    def test_parse(self):
        for item in self.files:
            (out, err) = runfile(item[0])
            if (len(err) > 0) != item[1]:
                print item[0], "expected to", item[1], err

if __name__ == '__main__':
    unittest.main()
