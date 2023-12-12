#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef __SIZEOF_INT128__
#error "Need __int128__!"
#endif

#define NUM_UNFOLD (5)

// use 128b type for masking broken and unknown gears (inputs all < 128 units long even after part 2)
static const __int128 ONE = (__int128)1;

static int g_verbose = 0;

// convert mask string to broken '#' and unknown '?' int128 masks
void populate_masks(char *s, __int128 *mask_broken, __int128 *mask_mystery) {
  size_t len = strlen(s); // don't care about iterating the string twice or crashing

  *mask_broken = 0;
  *mask_mystery = 0;

  for (size_t ii=0; ii < len; ++ii) {
    *mask_broken |= (s[ii] == '#') << ii ;
    *mask_mystery |= (s[ii] == '?') << ii ;
  }
}

// fill mask with 1's between offset and offset+len-1, the leisurely way.
void init_mask(__int128 *mask, __int128 len, size_t offset) {
  *mask = 0;
  while (len --> 0) { //<-- whimsy
    *mask |= ONE<<(len+offset);
  }
}

// maskb: broken mask; maskm: mystery mask; run_mask: proposed run of broken gears.
__int128 check_mask_fits_range(__int128 maskb, __int128 maskm, __int128 run_mask, int left_lsb, int right_msb) {
  // To fit, all of run mask must be in maskb|maskm,
  // And all of maskb must be in run mask.

  __int128 cmp_mask = ((ONE<<(right_msb+1))-1) & ~ ((ONE<<left_lsb)-1); // 1=active

  __int128 allowed_locations_mask = ~cmp_mask | maskb | maskm;

  // none of run mask outside of b and m
  if (run_mask & ~ allowed_locations_mask)
    return 0;

  // none of b (in cmp range) outside of run_mask
  if (maskb & ~run_mask & cmp_mask)
    return 0;

  return 1;
}

void print_mask_in_binary_reversed(__int128 len, __int128 mask) {
  for (__int128 ii=0; ii<len; ++ii) {
    putchar(mask & (ONE<<ii) ? '1' : '0');
  }
  putchar('\n');
}

// TODO: bundle constant fields into a const struct *


// Only non constant args* are the offset and the length of the run.
// Both are constrained to the input length.  Memoise the
// function. (Reset on new problem. Would encapsulate this better in a
// serious project.)
//
// *per-line, i.e. per-problem
static int_fast64_t count_arrangements_r_memo(size_t len, __int128 maskb, __int128 maskm,
                                              size_t offset, __int128 mask_runs_dbg, int *lengths,
                                              size_t lengths_off, int combined_len);

int_fast64_t count_arrangements_r(size_t len, __int128 maskb, __int128 maskm,
                                  size_t offset, __int128 mask_runs_dbg, int *lengths, size_t lengths_off, int combined_len)
{
  // base case: no more runs of broken gears to fit
  if (lengths[lengths_off] == 0) {
    if (g_verbose) {
      printf("succeeded.\n");
      printf("- mask_runs    = ");
      print_mask_in_binary_reversed(len, mask_runs_dbg);
      printf("- mask_broken  = ");
      print_mask_in_binary_reversed(len, maskb);
      printf("- mask_mystery = ");
      print_mask_in_binary_reversed(len, maskm);
      puts("");
    }

    // count this valid solution
    return 1;
  }

  int run_len;
  __int128 run_mask;
  int_fast64_t arrange_count = 0;
  int_fast64_t recurse_count = 0;

  // sanity check mask so far
  __int128 mask_to_offset = ~(((__int128)-1) >> (len - 1 - offset));
  assert((mask_runs_dbg & mask_to_offset) == 0);

  run_len = lengths[lengths_off];
  init_mask(&run_mask, run_len, offset);

  // iterate all possible offsets of this run (with recusion down the
  // tree of possibilities for later runs in the list)
  for (__int128 shift = 0; shift + offset + run_len-1 < len; ++shift) {
    __int128 n_mask_runs_dbg = mask_runs_dbg | run_mask << shift;
    int recurse_offset = offset + shift + run_len + 1; // +1 for mandated gap

    // weed out illegal runs of broken gears
    if (!check_mask_fits_range(maskb, maskm, run_mask << shift, offset, recurse_offset-1))
      continue;

    if (lengths[lengths_off+1] == 0) {
      // check whole thing is OK to the end of the `broken' string
      if (!check_mask_fits_range(maskb, maskm, run_mask << shift, offset, len-1))
        continue;
    }

    recurse_count = count_arrangements_r_memo(
      len, maskb, maskm,
      recurse_offset,
      n_mask_runs_dbg,
      lengths, lengths_off+1, combined_len-lengths[0]
                                              );
    // had integer overflows :(
    assert (arrange_count >= 0);
    assert (recurse_count >= 0);

    arrange_count += recurse_count;

    assert (arrange_count >= 0);
  }

  return arrange_count;
}

#define LONGEST_LINE (512) // favourite number
//#define SLOW_DEBUG (1) // Do. Not. Zero. To. Disable. ;)

int_fast64_t count_arrangements_r_memo_val[NUM_UNFOLD * LONGEST_LINE * NUM_UNFOLD * LONGEST_LINE];
int_fast64_t count_arrangements_r_memo_set[NUM_UNFOLD * LONGEST_LINE * NUM_UNFOLD * LONGEST_LINE] = {0};

int_fast64_t count_arrangements_r_memo(size_t len, __int128 maskb, __int128 maskm,
                              size_t offset, __int128 mask_runs, int *lengths, size_t lengths_off, int combined_len)
{
  size_t idx = offset * LONGEST_LINE + lengths_off;

  assert(idx < sizeof(count_arrangements_r_memo_set)/sizeof(count_arrangements_r_memo_set[0]));

  if (!count_arrangements_r_memo_set[idx]) {
    printf("miss: %lu, %lu\n", offset, lengths_off);
    count_arrangements_r_memo_set[idx] = 1;
    count_arrangements_r_memo_val[idx] = count_arrangements_r(
      len, maskb, maskm, offset, mask_runs, lengths, lengths_off, combined_len);
  } else {
#ifdef SLOW_DEBUG
    // disable memoisation
    assert (count_arrangements_r_memo_val[idx] == count_arrangements_r(
              len, maskb, maskm, offset, mask_runs, lengths, lengths_off, combined_len));
#endif
  }

  return count_arrangements_r_memo_val[idx];
}

int_fast64_t count_arrangements(__int128 len, __int128 maskb, __int128 maskm, int *lengths) {
  __int128 mask_runs = 0;
  int combined_len = 0;
  for (int ii=0; ii<len; ++ii) { // TODO (TO-UNDO): awful naming scheme.
    combined_len += lengths[ii] + 1; // +1 for at least one space between runs
  }
  combined_len -= 1; // ... except the last;

  memset(count_arrangements_r_memo_set, 0, sizeof(count_arrangements_r_memo_set));

  return count_arrangements_r_memo(len, maskb, maskm, 0, mask_runs, lengths, 0, combined_len);
}

void print_records(FILE *outf, int len, __int128 maskb, __int128 maskm, int *runs, int run_count) {
  for (int ii=0; ii<len; ++ii) {
    if ((ONE<<ii) & maskb) {
      putc('#', outf);
      assert(((ONE<<ii) & maskm)==0);
    }
    else if ((ONE<<ii) & maskm) {
      putc('?', outf);
    }
    else {
      putc('.', outf);
    }
  }
  putc(' ', outf);
  for (int ii=0; ii<run_count; ++ii) {
    fprintf(outf, "%d", runs[ii]);
    if (ii+1<run_count)
      putc(',', outf);
  }
  putc('\n', outf);
}

int main(int argc, char **argv)
{
  char gears_cbuf[512]; // Danger!
  char runs_cbuf[512]; // Using fscanf %s !
  // Beware malicious elves!

  int part = 1;
  int_fast64_t acc = 0;

  if (argc < 2) {
    return 1;
  }

  if (argc >2) {
    part = 2;
  }

  FILE *infile = fopen(argv[1], "r");

  // process one line at a time, reading into char buffers, until
  // something happens to make fscanf return EOF (end of file, bad
  // parse, ...)
  while (fscanf(infile, "%s %s\n", gears_cbuf, runs_cbuf) != EOF) {
    __int128 mask_broken;
    __int128 mask_mystery;
    int listlen;
    int runlen;

    listlen = strlen(gears_cbuf);
    runlen = strlen(runs_cbuf);

    populate_masks(gears_cbuf, &mask_broken, &mask_mystery);

    if (part==2) {
      // Repeat the 'broken machine runs' list.  It was a mistake to
      // do this to the string when it could have been done to the
      // resulting array.
      for (int ii=0; ii<NUM_UNFOLD; ++ii) {
        for (int jj=0; jj<runlen; ++jj) {
          // +1 to add ','
          runs_cbuf[ii*(runlen+1)+jj] = runs_cbuf[jj];
        }
        runs_cbuf[ii*(runlen+1)+runlen] = ',';
      }
      runs_cbuf[(NUM_UNFOLD-1)*(runlen+1)+runlen] = '\0';

      // Repeat masks.
      mask_mystery |= 1 << listlen; // append '?'
      __int128 maskb_dup = 0;
      __int128 maskm_dup = 0;
      for (int ii=0; ii<NUM_UNFOLD; ++ii) {
        // +1: leave space for extra ? spring
        maskb_dup |= (mask_broken << ((listlen+1) * ii));
        maskm_dup |= (mask_mystery << ((listlen+1) * ii));
      }

      mask_mystery = maskm_dup;
      mask_broken = maskb_dup;

      listlen *= NUM_UNFOLD;
      listlen += NUM_UNFOLD-1; // add joining gears
    }

    // Parse run lengths.  Could have been done in one go rather than
    // traversing the input twice.
    int lengths[512] = {0}; // What could go wrong?
    int run_count=0;
    char *runs = runs_cbuf;
    do {
      if (EOF==sscanf(runs, "%d", &lengths[run_count]))
        break;
      for (int n=1; n-1 < lengths[run_count] ; n=n*10)
        ++ runs;
      ++ run_count;
    } while (sscanf(runs++, ",") != EOF);

    lengths[run_count] = 0;

    fprintf(stdout, "Record:\n");
    fprintf(stdout, "- list length : %d\n", listlen);
    // dump reconstructed input to stderr to check parse.
    print_records(stderr, listlen, mask_broken, mask_mystery, lengths, run_count);

    int_fast64_t narrs = 0;
    if (!getenv("REPRODUCE_INPUT_ONLY")) {
      narrs = count_arrangements(listlen, mask_broken, mask_mystery, lengths);
    } else {
      fputs("WARNING: skipping computation.", stderr);
    }

    assert (narrs >= 0);

    printf("num arrs = %lu\n------------\n", narrs);
    acc += narrs;
  }

  // maybe we finished, maybe something bad happened to scanf.

  printf("%lu\n", acc);

  return 0;
}
