import unittest
from TestBase import TestBase

class TestRun(TestBase):
    files = [
        ["test_main_000.c", 123]
    ]

    def test_run(self):
        for item in self.files:
            (out, err) = self.compile_file(item[0])
            self.assemble(out)
            (out, err, retcode) = self.run_file("out")
            self.assertEqual(item[1], retcode, "return value " + str(item[1]) + " != " + str(retcode))

if __name__ == '__main__':
    unittest.main()
