#!/bin/sh

echo "Building tester executable"
gcc -m32 -o tester tester.c tester.s
echo "Build finished, run ./tester file"
