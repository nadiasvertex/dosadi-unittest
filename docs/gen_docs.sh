#!/bin/sh
cd /projects/unittest/docs
doxygen
rm -f unittest_docs.tar.bz2
tar -cf unittest_docs.tar html/*
bzip2 unittest_docs.tar


