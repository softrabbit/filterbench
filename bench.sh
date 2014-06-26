#!/bin/bash

# Benchmarking filters from LMMS.
# Lots of hardcoded paths and stuff in here, so you may need to tweak stuff
# (should make a Makefile or something, but this kinda grew from a simple loop...)

# All filters
FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass Moog DoubleLowPass Lowpass_RC12 Bandpass_RC12	Highpass_RC12 Lowpass_RC24 Bandpass_RC24 Highpass_RC24 Formantfilter"
# "faster" filters, the ones not 4x oversampled internally
#FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass"
#FILTERS="Formantfilter"

# How to optimize
OPTIMIZE="-O2 -msse2 -mfpmath=sse"

BINDIR=tests
OUTDIR=output

if [ $# -eq 0 ] ; then
    echo "Usage: $0 [options]"
    echo -e "\t--compile   \tBuild all versions"
    echo -e "\t--mod       \tBuild modified version"

    echo -e "\t--run [N]   \tRun benchmarks N times"
    echo -e "\t--coeffs [N]\tRun coefficient calculations N times"
    echo -e "\t--check     \tCheck output for regressions"
    echo -e "\t--clean     \tHousekeeping"
    echo -e "\t\tSee script for further details and filter selection."
    exit 0
fi 

if [ "$1" == "--compile" ] ; then 
    mkdir $BINDIR
    echo "Compiling baseline"
    # Baseline
    g++ -DBASELINE -o $BINDIR/baseline filterdriver.cpp -O2
    # Baseline with compile-time optimization
    g++ -DBASELINE -o $BINDIR/optimized filterdriver.cpp $OPTIMIZE
fi

if [ "$1" == "--compile" -o "$1" == "--mod" ] ; then 
    mkdir $BINDIR
    echo "Compiling modified ( $OPTIMIZE )"
    g++ -o $BINDIR/modified filterdriver.cpp $OPTIMIZE
fi

if [ "$1" == "--run" -o "$1" == "--coeffs" -o "$1" == "--denormal" ] ; then 
    for F in $FILTERS ; do
	for BINARY in baseline optimized modified ; do 

	    if [ $# -gt 1 ] ; then 
		COUNT=$(($2+0))
	    else
		COUNT=1
	    fi

	    ARGSTR=""
	    if [ $1 == "--coeffs" ] ; then 
		ARGSTR="coeffs"
	    elif [ $1 == "--denormal" ] ; then 
		ARGSTR="denormal"
	    fi

	    ( while [ $((COUNT--)) -gt 0 ] ; do
		/usr/bin/time -f "%U %C" $BINDIR/$BINARY $F $ARGSTR 
	    done ) |& /usr/bin/awk \
	    'BEGIN {sum=0; n=0; } \
                   {sum+=$1; n++; print; }\
             END { avg=sum/n; printf("Average: %.2f seconds, %.2f units/s\n", avg, 1/avg); }'
	    echo ""
	done
	echo "-------------------"
    done
fi

if [ "$1" == "--check" ] ; then 
    # Compare output of optimized and modified, doesn't work too well with -ffast-math
    mkdir $OUTDIR
    for F in $FILTERS ; do 
	$BINDIR/optimized $F output > $OUTDIR/$F-opt
	$BINDIR/modified $F output > $OUTDIR/$F-mod
	if diff -sq $OUTDIR/$F-opt $OUTDIR/$F-mod >/dev/null ; then
	    echo -e "OK    \t" $F
	else
	    echo -e "differ\t" $F
	fi
    done
fi

if [ "$1" == "--clean" ] ; then 
    rm $BINDIR/*
    rm $OUTDIR/*
fi
