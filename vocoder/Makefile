default: original optimized

bench: original optimized
	@/usr/bin/time -f "%U %C" ./original
	@/usr/bin/time -f "%U %C" ./optimized

compare: original optimized
	./original --output > original.out
	./optimized --output > optimized.out
	/usr/bin/diff -s original.out optimized.out 
clean: 
	rm original optimized original.out optimized.out

original: main.c vocoder_original.c
	gcc -std=c99 -O2 -ftree-vectorize -fopt-info-vec-optimized -DORIGINAL main.c -lm -o original

optimized: main.c vocoder_optimized.c
	gcc -std=c99 -O2 -ftree-vectorize -fopt-info-vec-optimized main.c -lm -o optimized

