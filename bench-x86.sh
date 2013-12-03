#!/bin/bash

function usage {
	echo "usage:   ./bench-x86.sh [ref,x86,sse2,ssse3,avx,xop,avx2] [32,64]"
	echo ""
}

function bench {
	local FN
	case $1 in
		ref) FN=chacha_blocks_$1.c;;
		*) FN=chacha_blocks_$1-$2.S;;
	esac

	if [ ! -f $FN ] ; then
		echo $FN " doesn't exist to bench!"
		exit 1;
	fi

	echo "benching "$1", "$2" bits, single implementation"
	gcc chacha.c bench-x86.c -Dchacha_blocks_impl=chacha_blocks_$1 -Dhchacha_impl=hchacha_$1 $FN -O3 -o chacha_bench_$1 -m$2 2>/dev/null
	local RC=$?
	if [ $RC -ne 0 ]; then
		echo $FN " failed to compile"
		return
	fi
	./chacha_bench_$1
	rm -f chacha_bench_$1
}

case $2 in
	32);;
	64);;
	"") usage; exit 1;;
	*) usage; echo "arch must be 32 or 64 bits!"; exit 1;;
esac

bench $1 $2
