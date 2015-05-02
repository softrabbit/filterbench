default: original optimized sse

bench: original optimized sse
	@/usr/bin/time -f "%U %C" ./original
	@/usr/bin/time -f "%U %C" ./sse
	@/usr/bin/time -f "%U %C" ./optimized

compare: original optimized sse
	./original --output > original.out
	./sse --output > sse.out
	./original --output > original.out
	./optimized --output > optimized.out
	/usr/bin/diff -s sse.out optimized.out 
	/usr/bin/diff -s original.out optimized.out 
clean: 
	rm original optimized sse original.out optimized.out sse.out

original: main.c vocoder_original.c
	gcc -std=c99 -O2 -ftree-vectorize -fopt-info-vec-optimized -DORIGINAL main.c -lm -o original

sse: main.c vocoder_original.c
	gcc -std=c99 -O2 -msse2 -mfpmath=sse -ftree-vectorize -fopt-info-vec-optimized -DORIGINAL main.c -lm -o sse

optimized: main.c vocoder_optimized.c
	gcc -std=c99 -O2 -msse2 -mfpmath=sse -ftree-vectorize -fopt-info-vec-optimized main.c -lm -o optimized

