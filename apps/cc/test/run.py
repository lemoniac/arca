#!/usr/bin/env python
import sys
import arcacc

(out, err) = arcacc.compile_file(sys.argv[1])
if out is None:
    print err
    sys.exit(1)

arcacc.assemble(out, sys.argv[1])
(out, err, retcode) = arcacc.run_file("out", {})
if out is None:
    print err
    sys.exit(1)

print arcacc.parse_vm_output(out)
