#!/bin/bash

c++ -x c++ -Ofast -o to_binary - <<EOF
#include <iostream>
#include <cstdio>

int main() {
  long double tmp;
  while(!std::cin.eof()) {
    std::cin >> tmp;
    std::fwrite(&tmp, sizeof(tmp), 1, stdout);
  }
  return 0;
}
EOF

export BC_LINE_LENGTH=0
{
bc -l <<EOF
scale=80
print 0, " ", s(0), " ", c(0)
for (n=0; n < 100000; n++) {
  scale=0
  exp = n * 180 / 100000 - 90
  scale=80
  mant = 2^63 + n + n * 2^17 + n * 2^31 + n * 2^44
  x = mant*(2^exp)
  print x, " ", s(x), " ", c(x), "\n"
}
EOF
}|./to_binary > reference-sincos-dep.dat

rm to_binary
