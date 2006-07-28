#!/bin/sh
python ../../scripts/unittest.py --build=unix --output=gtk simplest.test 
sh build_tests.sh
./run_tests
