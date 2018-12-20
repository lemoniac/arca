#!/usr/bin/env python
from __future__ import print_function
import unittest
import arcacc
import re
import sys
from TestBase import TestBase

class TestSyntax(TestBase):
    files = [
        ["test_array_000.c", False],
        ["test_array_001.c", False],
        ["test_array_002.c", False],
        ["test_expressions_000.c", False],
        ["test_for_000.c", False],
        ["test_for_001.c", True],
        ["test_function_000.c", False],
        ["test_function_001.c", False],
        ["test_if_000.c", False],
        ["test_globals_000.c", False],
        ["test_main_000.c", False],
        ["test_struct_000.c", False],
        ["test_struct_001.c", False],
        ["test_text_000.c", False],
    ]

    def test_parse(self):
        for item in self.files:
            (out, err) = self.compile_file(item[0])
            self.assertEqual(len(err) > 0, item[1], item[0] + " expected " + str(item[1]) + "\n" + err)

    def test_generate(self):
        for item in self.files:
            if not item[1]:
                print(".", sep='', end='')
                sys.stdout.flush()
                (out, err) = self.compile_file(item[0])
                if arcacc.valgrind:
                    self.check_valgrind_error(err, "cc: " + item[0])
                (out, err) = self.assemble(out, item[0])
                if arcacc.valgrind:
                    self.check_valgrind_error(err, "asm: " + item[0])

    def test_generate_valgrind(self):
        arcacc.valgrind = True
        self.test_generate()

    def check_valgrind_error(self, err, filename):
        self.assertEqual(int(re.search("ERROR SUMMARY: (\d+) errors", err).group(1)), 0, "valgrind: " + filename)

if __name__ == '__main__':
    unittest.main()
