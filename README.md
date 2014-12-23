# filterbench

This is a primitive benchmark for the note filters in LMMS. The test is basically filtering 100 seconds of 96k stereo sound, and the reported units/second reflects how many of these units the filter handles in a second.

1. Edit `bench.sh`to select filters to benchmark and set optimization options
2. Run `bench.sh --compile` to generate the following binaries in directory `tests`:
  - `baseline` is the original, with standard LMMS compilation options
  - `optimized` is the original filter code, optimized 
  - `modified` is the modified filter code (in basic_filters_modified.h),  optimized
3. Run `bench.sh --check` to verify the results are identical between original and modified code (*NB* Harder optimizations, like --fast-math seem to sometimes create small differences even in untouched parts of the code, somewhere around the fifth decimal or so)
4. Run `bench.sh --run [N]` to test the selected filters. The optional numeric argument N selects the number of iterations.
