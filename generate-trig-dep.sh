#!/bin/bash

c++ -x c++ -Ofast -o to_binary - <<EOF
#include <iostream>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
  if (argc != 2) { return 1; }
  const int total = std::atoi(argv[1]);
  long double tmp;
  std::cin >> tmp;
  int i = 0;
  while(!std::cin.eof()) {
    std::fwrite(&tmp, sizeof(tmp), 1, stdout);
    std::cin >> tmp;
    std::fprintf(stderr, "\033[4D%3i%%", ++i*100/total);
  }
  std::fprintf(stderr, "\033[4D%3i%%\n", 100);
  std::fflush(stdout);
  return 0;
}
EOF

export BC_LINE_LENGTH=0
if [[ ! -f reference-sincos-dep.dat ]]; then
  echo -n "Generating 100000 dep sincos values [0, 2π]     " 1>&2
  { bc -l <<EOF
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
  }|./to_binary $((3*100000)) > reference-sincos-dep.dat
fi

if false && [[ ! -f reference-asin-dep.dat ]]; then
  echo -n "Generating 112909 dep asin values [0, 1]     " 1>&2
  { bc <<EOF
    define asin(x) {
      r=x
      scale *= 2
      x2=x*x
      prev=0
      term=x
      n=1
      while (r != prev) {
        term = term * x2 * n^2 / ((n+1) * (n+2))
        n += 2
        scale *= .5
        prev = r
        r += term
        scale *= 2
      }
      print n
      scale *= .5
      return (r*2+10^-scale)/2
    }

    scale=80
    print 0, " ", asin(0)
    print 1, " ", asin(1)
    for (n=0; n < 112909; n++) {
      scale=0
      exp = n * 90 / 112909 - 90
      scale=80
      mant = 2^63 + n + n * 2^17 + n * 2^31 + n * 2^44
      x = mant*(2^exp)
      print x, " ", asin(x), "\n"
    }
EOF
  }|./to_binary $((2*112909)) > reference-asin-dep.dat
fi

rm to_binary
