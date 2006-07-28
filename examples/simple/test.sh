#!/bin/sh
python ../../scripts/unittest.py simple.test --build=unix
sh build_tests.sh
./run_tests
