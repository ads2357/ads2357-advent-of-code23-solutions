#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef __SIZEOF_INT128__
#error "Need __int128__!"
#endif

const int NUM_UNFOLD = 5;

void populate_masks(char *s, __int128 *mask_broken, __int128 *mask_mystery) {
  size_t len = strlen(s); // don't care about iterating the string twice or crashing

  *mask_broken = 0;
  *mask_mystery = 0;

  for (size_t ii=0; ii < len; ++ii) {
    //*mask_broken |= (s[ii] == '#') << (len-ii-1) ;
    //*mask_mystery |= (s[ii] == '?') << (len-ii-1) ;
    *mask_broken |= (s[ii] == '#') << ii ;
    *mask_mystery |= (s[ii] == '?') << ii ;
  }
}

void init_mask(__int128 *mask, __int128 len, size_t offset) {
  *mask = 0;
  while (len --> 0) { //whimsy
    *mask |= 1<<(len+offset);
  }
}

__int128 check_mask_fits(__int128 maskb, __int128 maskm, __int128 run_mask) {
  // To fit, all of run mask must be in maskb|maskm,
  // And all of maskb must be in run mask.

  return (
    ((run_mask & (maskb | maskm))==run_mask) // none of run_mask outside of b|m
    //    && ((maskb & run_mask)==maskb) // none of b outside run_mask.
          );
}

__int128 check_mask_really_fits(__int128 maskb, __int128 maskm, __int128 run_mask) {
  // To fit, all of run mask must be in maskb|maskm,
  // And all of maskb must be in run mask.

  return (
    ((run_mask & (maskb | maskm))==run_mask) // none of run_mask outside of b|m
    && ((maskb & run_mask)==maskb) // none of b outside run_mask.
          );
}

void print_mask_in_binary_reversed(__int128 len, __int128 mask) {
  //while (len--) {
  for (__int128 ii=0; ii<len; ++ii) {
    putchar(mask & (1<<ii) ? '1' : '0');
  }
}

__int128 count_arrangements_r(size_t len, __int128 maskb, __int128 maskm, char *sruns, size_t offset, __int128 *mask_runs) {
  if (sruns[0] == '\0') {
    if (!check_mask_really_fits(maskb, maskm, *mask_runs))
        return 0;

    //printf("succeeded with %x versus %x|%x=%x\n", *mask_runs, maskb, maskm, maskb|maskm);
    print_mask_in_binary_reversed(len, *mask_runs);
    //puts("");
    return 1;
  }

  //puts(sruns);

  int run_len;
  __int128 run_mask;
  int arrange_count = 0;
  int recurse_count = 0;
  int srun_num_chars = 0; // `,'

  sscanf(sruns, "%d", &run_len);
  for (__int128 n=1; n<1000; n=n*10) {
    if (run_len / n)
      ++srun_num_chars; // ew
  }

  if (sruns[srun_num_chars] == ',')
    ++ srun_num_chars ;
  else
    assert (sruns[srun_num_chars] == '\0');

  init_mask(&run_mask, run_len, offset);

  for (__int128 shift = 0; shift + offset + run_len-1 < len; ++shift) {
    // if proposed mask doesn't fit description, don't recurse further
    if (!check_mask_fits(maskb, maskm, run_mask << shift))
      continue;

    *mask_runs |= run_mask << shift;

    recurse_count = count_arrangements_r(
      len, maskb, maskm, sruns + srun_num_chars /*yikes*/,
      offset + run_len + 1 + shift,
      mask_runs);

    *mask_runs &= ~(run_mask << shift);

    arrange_count += recurse_count;
  }

  //puts("");

  /* __int128 total_mask = run_mask | *mask_runs; */

  /* // 1) don't overlap with mask runs (what?) */
  /* // 2) wtf? */
  /* //while((run_mask & *mask_runs)==0 && ((run_mask & (1<<shift)) == 0)) { */

  /*   arrange_count += count_arrangements_r(len, maskb, maskm, sruns + srun_num_chars /\*yikes*\/, */
  /*                                         offset + run_len + shift + 1, &total_mask); */
  /*   total_mask = (run_mask << ++shift) | *mask_runs; */
  /* } */

  return arrange_count;
}

__int128 count_arrangements(__int128 len, __int128 maskb, __int128 maskm, char *sruns) {
  __int128 mask_runs = 0;
  return count_arrangements_r(len, maskb, maskm, sruns, 0, &mask_runs);
}


int main(int argc, char **argv)
{
  char gears_cbuf[512]; // danger!
  char runs_cbuf[512];
  int part = 1;

  __int128 acc = 0;

  if (argc < 2) {
    return 1;
  }

  if (argc >2) {
    part = 2;
  }

  FILE *infile = fopen(argv[1], "r");


  while (fscanf(infile, "%s %s\n", gears_cbuf, runs_cbuf) != EOF) {
    __int128 mask_broken;
    __int128 mask_mystery;
    int listlen;

    listlen = strlen(gears_cbuf);

    if (part==2) {
      for (int ii=0; ii<NUM_UNFOLD; ++ii) {
        memcpy(runs_cbuf + ii * listlen, runs_cbuf, listlen);
        //mask_broken |= (mask_broken << (listlen * ii));
        //mask_mystery |= (mask_mystery << (listlen * ii));
      }
    }

    listlen *= NUM_UNFOLD * (part-1);

    populate_masks(gears_cbuf, &mask_broken, &mask_mystery);

    if (part==2) {
      for (int ii=0; ii<NUM_UNFOLD; ++ii) {
        //memcpy(runs_cbuf + ii * listlen, runs_cbuf, listlen);
        mask_broken |= (mask_broken << (listlen * ii));
        mask_mystery |= (mask_mystery << (listlen * ii));
      }
    }

    printf("broken = %x, mystery = %x, length=%d, runs=%s\n", mask_broken, mask_mystery, listlen, runs_cbuf);


    __int128 narrs = count_arrangements(listlen, mask_broken, mask_mystery, runs_cbuf);
    printf("num arrs = %d\n------------\n", narrs);
    acc += narrs;
  }

  // maybe we finished, maybe something bad happened to scanf.

  printf("%d", acc);

  return 0;
}
