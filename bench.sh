#!/bin/bash

# All filters
FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass Moog DoubleLowPass Lowpass_RC12 Bandpass_RC12	Highpass_RC12 Lowpass_RC24 Bandpass_RC24 Highpass_RC24 Formantfilter"
# "faster" filters, the ones not 4x oversampled internally
#FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass"
FILTERS="Lowpass_RC24"

BINDIR=tests
OUTDIR=output

if [ $# -eq 0 ] ; then
    echo "Usage: $0 [options]"
    echo -e "\t--compile\tBuild all versions"
    echo -e "\t--mod    \tBuild modified version"
    echo -e "\t--run N  \tRun benchmarks N times"
    echo -e "\t--check  \tCheck output for regressions"
    echo -e "\t--clean  \tHousekeeping"
    echo -e "\tSee script for details and filter selection."
    exit 0
fi 

if [ "$1" == "--compile" ] ; then 
mkdir $BINDIR
for F in $FILTERS ; do 
    echo "Compiling baseline $F"
    # Baseline
    g++ -DBASELINE -DFILTER=$F -o $BINDIR/$F-base filterdriver.cpp   
    # Baseline with compile-time optimization
    g++ -DBASELINE -DFILTER=$F -o $BINDIR/$F-opt filterdriver.cpp -O3 -msse2 -ftree-vectorize -ffast-math
done
fi

if [ "$1" == "--compile" -o "$1" == "--mod" ] ; then 
mkdir $BINDIR
for F in $FILTERS ; do 
    echo "Compiling modified $F"
    # Modified version, optimized compile
    g++ -DFILTER=$F -o $BINDIR/$F-mod filterdriver.cpp -O3 -msse2 -ftree-vectorize -ffast-math
done
fi

if [ "$1" == "--run" ] ; then 
    for F in $FILTERS ; do
	for SUFFIX in base opt mod ; do 
	    if [ $# -gt 1 ] ; then 
		COUNT=$(($2+0))
	    else
		COUNT=1
	    fi
	    while [ $((COUNT--)) -gt 0 ] ; do
		/usr/bin/time -f "%U %C" $BINDIR/$F-$SUFFIX
	    done
	    echo ""
	done
	echo "-------------------"
    done
fi

if [ "$1" == "--check" ] ; then 
    # Compare output of optimized and modified
    mkdir $OUTDIR
    for F in $FILTERS ; do 
	$BINDIR/$F-opt output > $OUTDIR/$F-opt
	$BINDIR/$F-mod output > $OUTDIR/$F-mod
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
