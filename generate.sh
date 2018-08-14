#!/bin/zsh

function usage() {
  cat <<EOF
Usage: $0 [options] <function>

This tool splits an arbitrary number into IEEE 768 floats.
Options:
  --help|-h        this message

EOF
}
#  -m <bits>        mantissa bits, 23 for sp and 52 for dp (default 23)

bits=23

while (( $# > 0 )); do
  case "$1" in
    --help|-h) usage; exit ;;
    -m)  bits=$2; shift ;;
    -m*) bits=${1#-m} ;;
    *)   fun="$fun $1" ;;
  esac
  shift
done

octave <<EOF
fd=fopen("reference-sincos-dp.dat", "w");
fs=fopen("reference-sincos-sp.dat", "w");
for i = 1:100000
  x=rand * 2 * pi
  fwrite(fd, x, "double");
  fwrite(fd, sin(x), "double");
  fwrite(fd, cos(x), "double");
  fwrite(fs, x, "single");
  fwrite(fs, sin(x), "single");
  fwrite(fs, cos(x), "single");
endfor
for i = 1:100000
  x=rand * 10^(rand * 14)
  fwrite(fd, x, "double");
  fwrite(fd, sin(x), "double");
  fwrite(fd, cos(x), "double");
  fwrite(fs, x, "single");
  fwrite(fs, sin(x), "single");
  fwrite(fs, cos(x), "single");
endfor
#for i = 1:10000
#  x=rand * 10^(14 + rand * 6)
#  fwrite(fd, x, "double");
#  fwrite(fd, sin(x), "double");
#  fwrite(fd, cos(x), "double");
#  fwrite(fs, x, "single");
#  fwrite(fs, sin(x), "single");
#  fwrite(fs, cos(x), "single");
#endfor
EOF

# vim: sw=2 et
