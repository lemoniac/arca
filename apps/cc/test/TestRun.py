#!/usr/bin/env python
import unittest
from TestBase import TestBase

class TestRun(TestBase):
    files = [
        ["test_for_000.c", 90, {}],
        ["test_function_001.c", 24, {}],
        ["test_main_000.c", 123, {}],
        #["test_array_001.c", 45, {}],
        ["test_array_002.c", 45, {}],
        ["test_struct_001.c", 50, {}],
        ["test_text_000.c", 0, {"gpu": True, "screenshot": "test_text_000.png"}],
    ]

    def test_run(self):
        for item in self.files:
            (out, err) = self.compile_file(item[0], True)
            self.assemble(out, item[0])
            (out, err, retcode) = self.run_file("out", item[2])
            self.assertEqual(item[1], retcode, "return value " + str(item[1]) + " != " + str(retcode))

            if "screenshot" in item[2]:
                self.assertEqual( self.diff_files("screenshot.png", item[2]["screenshot"]), 0)


if __name__ == '__main__':
    unittest.main()
