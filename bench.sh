#!/bin/bash

# Benchmarking filters from LMMS.
# Lots of hardcoded paths and stuff in here, so you may need to tweak stuff,
# but at least it works on my Ubuntu system. (should make a Makefile or something, 
# but this kinda grew from a simple loop...)


################################################################

source defaults

# Where's gnuplot? comment this out to use cat instead
GNUPLOT=$(which gnuplot)


ALL_FILTERS="LowPass HiPass BandPass_CSG BandPass_CZPG Notch AllPass Moog DoubleLowPass Lowpass_RC12 Bandpass_RC12 Highpass_RC12 Lowpass_RC24 Bandpass_RC24 Highpass_RC24 Formantfilter DoubleMoog Lowpass_SV Bandpass_SV Highpass_SV Notch_SV FastFormant Tripole"

if [ "$FILTERS" == "" ] ; then
    FILTERS=$ALL_FILTERS
fi

BINDIR=tests
OUTDIR=output
UNAME=$(uname -rs)
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
    g++ -DBASELINE -o $BINDIR/baseline filterdriver.cpp $BASELINE $DEFINES
    # Baseline with compile-time optimization
    g++ -DBASELINE -o $BINDIR/optimized filterdriver.cpp $OPTIMIZE $DEFINES
fi

if [ "$1" == "--compile" -o "$1" == "--mod" ] ; then 
    mkdir $BINDIR
    echo "Compiling modified ( g++ -o $BINDIR/modified filterdriver.cpp $OPTIMIZE $EXTRAS $DEFINES)"
    g++ -o $BINDIR/modified filterdriver.cpp $OPTIMIZE $EXTRAS $DEFINES
fi

if [ "$1" == "--run" -o "$1" == "--coeffs" -o "$1" == "--denormal" ] ; then 
    echo -e "#filter\tbase\topt\tmod\t%change" >$OUTDIR/results.dat
    for F in $FILTERS ; do
		>&2 echo $F
	ARGSTR=""
	if [ $1 == "--coeffs" ] ; then 
	    ARGSTR="coeffs"
	elif [ $1 == "--denormal" ] ; then 
	    ARGSTR="denormal"
	fi
	echo -n $F >>$OUTDIR/results.dat
	( for BINARY in baseline optimized modified ; do 

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
             END { avg=sum/n; printf(" %f", 1/avg); }' 
	done ) | awk '{printf("\t%.2f\t%.2f\t%.2f\t%.2f\n", $1, $2, $3, ($3/$2)*100-100);}' >> $OUTDIR/results.dat
    done
	if [ "$ARGSTR" == "" ] ; then 
	    ARGSTR="filtering"
	fi
	if [ $GNUPLOT ] ; then 
	    $GNUPLOT -p <<EOF
set title "$ARGSTR - $GCC\n$UNAME - $CPU"
set tic scale 0
set yrange [0:]
set ylabel "units/s"
set ytics
set y2range [-25:100]
set y2label "percent change"
set grid y2tics
set y2tics
set key outside bottom center horizontal
set style fill solid 1.00  
set style data histograms 
set boxwidth 1.0
set xtics nomirror rotate by -45
plot "$OUTDIR/results.dat" \
    using 2:xtic(stringcolumn(1)." ".stringcolumn(5)."\%") \
    title "$BASELINE" lt rgb "#ff8080",\
  "" using 3 title "$OPTIMIZE" lt rgb "#80ff80", \
  "" using 4 title "mod $OPTIMIZE" lt rgb "#8080ff", \
  "" using 5 axes x1y2 title "change" lt rgb "#000000"
EOF

	else
		echo ""
	    cat $OUTDIR/results.dat
		echo -e "(bigger is better)\n"
	fi
fi

if [ "$1" == "--check" ] ; then 
    # Compare output of optimized and modified, doesn't work too well with -ffast-math
    # For detailed comparison: 
    # paste $OUTDIR/$FILTER-opt $OUTDIR/$FILTER-mod |awk '{print ($1 - $2)}'

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
