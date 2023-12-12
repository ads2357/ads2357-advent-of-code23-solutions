#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef __SIZEOF_INT128__
#error "Need __int128__!"
#endif

void populate_masks(char *s, int *mask_broken, int *mask_mystery) {
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

void init_mask(int *mask, int len, size_t offset) {
  *mask = 0;
  while (len --> 0) { //whimsy
    *mask |= 1<<(len+offset);
  }
}

int check_mask_fits(int maskb, int maskm, int run_mask) {
  // To fit, all of run mask must be in maskb|maskm,
  // And all of maskb must be in run mask.

  return (
    ((run_mask & (maskb | maskm))==run_mask) // none of run_mask outside of b|m
    //    && ((maskb & run_mask)==maskb) // none of b outside run_mask.
          );
}

int check_mask_really_fits(int maskb, int maskm, int run_mask) {
  // To fit, all of run mask must be in maskb|maskm,
  // And all of maskb must be in run mask.

  return (
    ((run_mask & (maskb | maskm))==run_mask) // none of run_mask outside of b|m
    && ((maskb & run_mask)==maskb) // none of b outside run_mask.
          );
}

void print_mask_in_binary_reversed(int len, int mask) {
  //while (len--) {
  for (int ii=0; ii<len; ++ii) {
    putchar(mask & (1<<ii) ? '1' : '0');
  }
}

int count_arrangements_r(size_t len, int maskb, int maskm, char *sruns, size_t offset, int *mask_runs) {
  if (sruns[0] == '\0') {
    if (!check_mask_really_fits(maskb, maskm, *mask_runs))
        return 0;

    printf("succeeded with %x versus %x|%x=%x\n", *mask_runs, maskb, maskm, maskb|maskm);
    print_mask_in_binary_reversed(len, *mask_runs);
    puts("");
    return 1;
  }

  //puts(sruns);

  int run_len;
  int run_mask;
  int arrange_count = 0;
  int recurse_count = 0;
  int srun_num_chars = 0; // `,'

  sscanf(sruns, "%d", &run_len);
  for (int n=1; n<1000; n=n*10) {
    if (run_len / n)
      ++srun_num_chars; // ew
  }

  if (sruns[srun_num_chars] == ',')
    ++ srun_num_chars ;
  else
    assert (sruns[srun_num_chars] == '\0');

  init_mask(&run_mask, run_len, offset);

  for (int shift = 0; shift + offset + run_len-1 < len; ++shift) {
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

  /* int total_mask = run_mask | *mask_runs; */

  /* // 1) don't overlap with mask runs (what?) */
  /* // 2) wtf? */
  /* //while((run_mask & *mask_runs)==0 && ((run_mask & (1<<shift)) == 0)) { */

  /*   arrange_count += count_arrangements_r(len, maskb, maskm, sruns + srun_num_chars /\*yikes*\/, */
  /*                                         offset + run_len + shift + 1, &total_mask); */
  /*   total_mask = (run_mask << ++shift) | *mask_runs; */
  /* } */

  return arrange_count;
}

int count_arrangements(int len, int maskb, int maskm, char *sruns) {
  int mask_runs = 0;
  return count_arrangements_r(len, maskb, maskm, sruns, 0, &mask_runs);
}


int main(int argc, char **argv)
{
  char gears_cbuf[512]; // danger!
  char runs_cbuf[512];

  int acc = 0;

  if (argc < 2) {
    return 1;
  }

  FILE *infile = fopen(argv[1], "r");

  while (fscanf(infile, "%s %s\n", gears_cbuf, runs_cbuf) != EOF) {
    /* puts(gears_cbuf); */
    /* puts(runs_cbuf); */

    int mask_broken;
    int mask_mystery;
    int listlen;

    listlen = strlen(gears_cbuf);
    populate_masks(gears_cbuf, &mask_broken, &mask_mystery);

    printf("broken = %x, mystery = %x, length=%d, runs=%s\n", mask_broken, mask_mystery, listlen, runs_cbuf);


    int narrs = count_arrangements(listlen, mask_broken, mask_mystery, runs_cbuf);
    printf("num arrs = %d\n------------\n", narrs);
    acc += narrs;
  }

  // maybe we finished, maybe something bad happened to scanf.

  printf("%d", acc);

  return 0;
}
