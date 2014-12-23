#!/bin/bash

# Benchmarking filters from LMMS.
# Lots of hardcoded paths and stuff in here, so you may need to tweak stuff,
# but at least it works on my Ubuntu system. (should make a Makefile or something, 
# but this kinda grew from a simple loop...)

# Select filters to test, see below for list of all
FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass DoubleLowPass Tripole Moog DoubleMoog"

# Where's gnuplot? comment this out to use cat instead
GNUPLOT=/usr/bin/gnuplot

# How to optimize
BASELINE="-O2 -fno-exceptions"
OPTIMIZE="-O2 -fno-exceptions -ftree-vectorize"


################################################################

ALL_FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass Moog DoubleLowPass Lowpass_RC12 Bandpass_RC12 Highpass_RC12 Lowpass_RC24 Bandpass_RC24 Highpass_RC24 Formantfilter DoubleMoog Lowpass_SV Bandpass_SV Highpass_SV Notch_SV FastFormant Tripole"

if [ "$FILTERS" == "" ] ; then
    FILTERS=$ALL_FILTERS
fi

BINDIR=tests
OUTDIR=output
UNAME=$(uname -rps)
CPU=$(cat /proc/cpuinfo |awk '/^model name/ { $1=""; $2=""; $3=""; sub(/[[:space:]]+/,""); print ;exit}')
GCC=$(gcc --version |head -n1)
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
    g++ -DBASELINE -o $BINDIR/baseline filterdriver.cpp $BASELINE
    # Baseline with compile-time optimization
    g++ -DBASELINE -o $BINDIR/optimized filterdriver.cpp $OPTIMIZE
fi

if [ "$1" == "--compile" -o "$1" == "--mod" ] ; then 
    mkdir $BINDIR
    echo "Compiling modified ( $OPTIMIZE )"
    g++ -o $BINDIR/modified filterdriver.cpp $OPTIMIZE
fi

if [ "$1" == "--run" -o "$1" == "--coeffs" -o "$1" == "--denormal" ] ; then 
    echo -e "#filter\tbase\topt\tmod" >output/results.dat
    for F in $FILTERS ; do
	ARGSTR=""
	if [ $1 == "--coeffs" ] ; then 
	    ARGSTR="coeffs"
	elif [ $1 == "--denormal" ] ; then 
	    ARGSTR="denormal"
	fi
	echo -n $F >>output/results.dat
	for BINARY in baseline optimized modified ; do 

	    if [ $# -gt 1 ] ; then 
		COUNT=$(($2+0))
	    else
		COUNT=1
	    fi

	    # Output in gnuplot-friendly format... units/s, i.e. the inverse of average time
	    ( while [ $((COUNT--)) -gt 0 ] ; do
		/usr/bin/time -f "%U %C" $BINDIR/$BINARY $F $ARGSTR 
	    done ) |& /usr/bin/awk \
	    'BEGIN {sum=0; n=0; } \
                   {sum+=$1; n++; }\
             END { avg=sum/n; printf("\t%.2f", 1/avg); }' >> output/results.dat
	done
	echo "" >> output/results.dat
    done
	if [ "$ARGSTR" == "" ] ; then 
	    ARGSTR="filtering"
	fi
	if [ $GNUPLOT ] ; then 
	    $GNUPLOT -p <<EOF
set title "filterbench - $ARGSTR - units/s, bigger is better\n$UNAME ($CPU), $GCC"
set tic scale 0
set yrange [0:]
set grid ytics
set key outside bottom center horizontal
set style fill solid 1.00  
set style data histograms 
set boxwidth 1.0
set xtics nomirror rotate by -45
plot "output/results.dat" using 2:xtic(1) title "$BASELINE" lt rgb "#800000", "" using 3 title "$OPTIMIZE" lt rgb "#008000", "" using 4 title "mod $OPTIMIZE" lt rgb "#000080"
EOF

	else
	    cat output/results.dat
	fi
fi

if [ "$1" == "--check" ] ; then 
    # Compare output of optimized and modified, doesn't work too well with -ffast-math
    mkdir $OUTDIR
    for F in $ALL_FILTERS ; do 
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
