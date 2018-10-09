case $2 in
  */tests/*|uniconf/daemon/uniconfd)
    [ -e "$2.cc" -o -e "$2.c" ] || exit 1
    objs=$2.o
    . ./link.od
    ;;
  *)
    echo "Error: $1: no rule to build this target." >&2
    exit 1
    ;;
esac

