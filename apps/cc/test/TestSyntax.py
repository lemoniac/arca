import unittest
from TestBase import TestBase

class TestSyntax(TestBase):
    files = [
        ["test_array_000.c", False],
        ["test_array_001.c", False],
        ["test_expressions_000.c", False],
        ["test_for_000.c", False],
        ["test_for_001.c", True],
        ["test_function_000.c", False],
        ["test_globals_000.c", False],
        ["test_main_000.c", False],
        ["test_struct_000.c", False],
    ]

    def test_parse(self):
        for item in self.files:
            (out, err) = self.compile_file(item[0])
            self.assertEqual(len(err) > 0, item[1], item[0] + " expected " + str(item[1]) + "\n" + err)

if __name__ == '__main__':
    unittest.main()
