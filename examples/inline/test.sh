#!/bin/sh
rm run_tests
python ../../scripts/unittest.py --build=unix --output=gtk FixedPoint.h 
sh build_tests.sh
./run_tests
