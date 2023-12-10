// Use GNU (for getline, see `man 3 getline')
#define _POSIX_C_SOURCE (200809L)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


// Solve the puzzle by:
//
// 1) tracing the pipeline (producing a length and thus the furthest
// tile from the start)
//
// 2) traversing tile edges from every tile corner on the perimeter of
// the map (producing a count of which tiles are reachable this way,
// and thus which are not i.e. are enclosed by the pipeline)

// macros for generating linear offsets from 2d coords
#define SERXY(x,y,in) ((y)*(in->size_x)+(x))
#define SERXYSTRIDE(x,y,s) ((y)*(s)+(x))

// ANSI terminal colour commands (for visualisation)
const char *TCOL_CLR = "\x1b[0m";
const char *TCOL_YEL = "\x1b[0;33m";
const char *TCOL_MYS = "\x1b[0;34m"; // Hope that these codes are for
                                     // more colours,
const char *TCOL_TRY = "\x1b[0;35m"; // I only looked up the one (yellow)

static int g_verbose = 0;

// Use conn_e to encode both complete connections (two bits set) and
// half connections (one bit set).  Whilst convienient, there would
// arguably be some utility in instead using type checking to protect
// against confusion between the two forms. (etc.)
typedef enum {N=1, E=2, W=4, S=8, START=16, NONE=0, FILE_NOT_FOUND=32 /* <- inside joke */} conn_e;
char *conn_e_names[] = {
  "N",
  "E",
  "W",
  "S",
};

// Reverse a single-ended connection.
static inline conn_e reverse_conn(conn_e con) {
  assert (__builtin_popcount(con) == 1);

  return con == N ? S
    : con == S ? N
    : con == W ? E
    : con == E ? W
    : con;
}

typedef struct {
  char *tiles;
  size_t size_x;
  size_t size_y;
  int start_x;
  int start_y;
} s_input_t;

// End this program whenever there is a problem.
// No grace, just pure panic.
static void solution_abort(char *what, int line) {
  fprintf(stderr, "Error at line %d ", line);
  fputs(what, stderr);
  fputs("\n", stderr);
  abort();
}

static conn_e get_conn(char c) {
  return
    c == '|' ? N|S
    : c == '-' ? E|W
    : c == 'L' ? N|E
    : c == 'J' ? N|W
    : c == 'F' ? E|S
    : c == '7' ? W|S
    : c == '.' ? 0
    : c == 'S' ? START
    : '!';
}

static char get_c_from_conn(conn_e con) {
  return
    con == (N|S) ? '|'
    : con == (E|W) ? '-'
    : con == (N|E) ? 'L'
    : con == (N|W) ? 'J'
    : con == (E|S) ? 'F'
    : con == (W|S) ? '7'
    : con == 0 ? '.'
    : '?';
}

// Get conn from the map at (x,y)

// The re-encoding of original chars to conn_e's is questionable but
// was convenient for a quick puzzle solution.
static int get_mapconn(s_input_t *in, const int x, const int y, conn_e *con) {
  if (y < 0 || y >= (int)in->size_y
      || x < 0 || x >= (int)in->size_x
      || in->tiles[y * in->size_x + x] == NONE
      )
    {
      *con = 0;
      return 0;
    }

  *con = (conn_e)in->tiles[y * in->size_x + x];
  return 1;
}

// Encapsulated `free' function for the associated struct.
// In C++, this would be done as RAII instead.
void free_input(s_input_t *p) {
  free(p->tiles);
  free(p);
}

void print_input(FILE *fp, s_input_t in) {
  fprintf(fp, "size=(%lu,%lu), S@(%d,%d)\n", in.size_x, in.size_y, in.start_x, in.start_y);
  for (size_t yy=0; yy < in.size_y; ++yy) {
    for (size_t xx=0; xx < in.size_x; ++xx) {
      conn_e con;
      get_mapconn(&in, xx, yy, &con);
      char c = get_c_from_conn(con);
      fputc(c, stdout);
    }
    fputc('\n', stdout);
  }
}

s_input_t *parse_input(FILE *infilep)
{
  // don't bother dynamically allocating memory for the file, just
  // specify a max size for this exercise.
  const int EXPECT_NUM_LINES = 1024;
  size_t *line_starts;
  char *line = NULL;
  char *lines = NULL;
  ssize_t line_len = 0;
  size_t buf_size = 0;
  ssize_t first_line_len;
  int start_x = -1;
  int start_y = -1;

  // obtain first line length and allocate width accordingly.
  if ((line_len = getline(&line, &buf_size, infilep)) == -1)
    solution_abort("IO Error or EOF", __LINE__);

  first_line_len = line_len;

  if ((line_starts = calloc(EXPECT_NUM_LINES+1, sizeof(int))) == NULL)
    solution_abort("alloc", __LINE__);

  if ((lines = malloc(EXPECT_NUM_LINES * line_len * sizeof(int))) == NULL)
    solution_abort("alloc", __LINE__);

  line_starts[0] = 0;

  for (int ii=0; ii<first_line_len; ++ii) {
    if (line[ii] == 'S') {
      start_x = ii;
      start_y = 0;
      lines[ii] = line[ii];
    } else {
      lines[ii] = get_conn(line[ii]);
    }
  }

  line_starts[1] = first_line_len-1;

  int ii;
  for (ii=1; ii<EXPECT_NUM_LINES; ++ii) {
    char *linestart = &(lines[line_starts[ii]]);
    line = linestart;

    // Use the existing buffer to read another line.  Assume lines are
    // the same length (or decreasing).  Will realloc if bigger, so
    // detect this and abort.  This style is maybe a bit inelegant and
    // vulnerable to bugs.
    if ((line_len = getline(&line, &buf_size, infilep)) == -1) {
      break; // end of file, hopefully.
    }

    if (line != linestart)
      solution_abort("re-alloc", __LINE__);

    // chain together line offsets
    line_starts[ii+1] = line_starts[ii] + line_len -1;

    // convert chars to conns (except S).
    // (Discussed above.)
    for (size_t jj=line_starts[ii]; jj<line_starts[ii+1]; ++jj) {
      if (lines[jj] == 'S') {
        start_x = jj - line_starts[ii];
        start_y = ii;
      } else {
        lines[jj] = get_conn(lines[jj]);
      }
    }
  }

  int line_count = ii;
  if (line_count == EXPECT_NUM_LINES) {
    solution_abort("Too many input lines.", __LINE__);
  }

  if (start_x == -1 || start_y == -1)
    solution_abort("S not found.", __LINE__);

  s_input_t *retval = malloc(sizeof(s_input_t));
  retval->start_x = start_x;
  retval->start_y = start_y;
  retval->size_x = line_starts[1] - line_starts[0];
  retval->size_y = line_count;
  retval->tiles = lines;
  return retval;
}

// change x and y, then get subsequent connection.
static int get_next_position(s_input_t *input, int *x, int *y, conn_e *con) {
  const int dx[] = { [E] = 1, [W] = -1 };
  const int dy[] = { [N] = -1, [S] = 1 };
  conn_e next_con;
  int rv;

  assert(*con == E || *con == N || *con == S || *con == W);

  *x += dx[*con];
  *y += dy[*con];

  rv = get_mapconn(input, *x, *y, &next_con);

  next_con &=~ reverse_conn(*con); // mask out reverse of previous con (i.e. source direction)
  *con = next_con;

  return rv;
}

// from input->start, find a connecting pipesection and return its
// coords and where it points to.
static int find_first_pipesection(s_input_t *input, int *x, int *y, conn_e *con) {
  const conn_e con_sn[] = {N,S};
  const conn_e con_we[] = {E,W};

  conn_e retval = NONE;
  conn_e tmpcon = NONE;

  for (int dx=-1; dx <2; dx += 2) {
    *x = input->start_x+dx;
    *y = input->start_y;
    if (get_mapconn(input, *x, *y, &tmpcon)) {
      if (tmpcon & con_we[(dx+1)/2]) {
          retval = tmpcon & ~con_we[(dx+1)/2];
          *con = retval;
          return 1;
      }
    }
  }

  for (int dy=-1; dy <2; dy += 2) {
    *x = input->start_x;
    *y = input->start_y+dy;
    if (get_mapconn(input, *x, *y, &tmpcon)) {
      if (tmpcon & con_sn[(dy+1)/2]) {
        retval = tmpcon & ~con_sn[(dy+1)/2];
        *con = retval;
        return 1;
      }
    }
  }

  return 0;
}

static char g_conn_s_buf[16] = {0};
// non thread safe representation of a conn_e
const char *conn_to_s(conn_e con) {
  char *rv = g_conn_s_buf;
  memset(rv, '.', 4);
  rv[4] = '\0';
  for (size_t ii=0; ii<sizeof(conn_e_names)/sizeof(conn_e_names[0]); ++ii) {
    if (con & (1<<ii)) {
      rv[ii] = conn_e_names[ii][0];
    }
  }
  return rv;
}

void trace_path(s_input_t *input, int *len, int **is_pline) {
  int x, y;
  conn_e con;

  int num_tiles = input->size_x * input->size_y;
  // grid of just the pipeline tiles.
  // wasteful of memory but simple and easy to visualise.
  // alternatively could count tiles outside, and only visit them once.
  int *pipeline_tiles = calloc(num_tiles, sizeof(pipeline_tiles[0]));
  *is_pline = pipeline_tiles;

  // initialise x, y and forward connection to the first (non-start) tile.
  // start tile connections can be calculated later.
  if (find_first_pipesection(input, &x, &y, &con) == 0)
    solution_abort("first not found", __LINE__);

  // set half of start connection by reversing the next tile's
  // connection and masking out the onward connection.
  conn_e start_con;
  conn_e xy_con;
  get_mapconn(input, x, y, &xy_con);
  conn_e camefrom = xy_con & ~con; // remove onward component
  start_con = reverse_conn(camefrom);

  // debug
  int tracex[99999];
  int tracey[99999];

  int count = 0;

  // trace the pipe back to the start
  while (x != input->start_x || y != input->start_y) {
    //printf("%d,%d\n", x, y);

    get_mapconn(input, x, y, &xy_con);
    pipeline_tiles[SERXY(x, y, input)] = xy_con;

    tracex[count] = x;
    tracey[count] = y;

    ++ count;
    if (!get_next_position(input, &x, &y, &con)) {
      tracex[count] = x;
      tracey[count] = y;
      solution_abort("next position not found!", __LINE__);
    }
  }

  // populate other half of start connection from the last connection
  // before reaching the start again.
  conn_e last_con;
  if (tracex[count-1] < input->start_x)
    last_con = E;
  else if (tracex[count-1] > input->start_x)
    last_con = W;
  else if (tracey[count-1] < input->start_y)
    last_con = S;
  else
    last_con = N;
  start_con |= reverse_conn(last_con);

  // fill in S with calculated start connection.
  pipeline_tiles[SERXY(input->start_x, input->start_y, input)] = start_con;
  input->tiles[SERXY(input->start_x, input->start_y, input)] = start_con;

  *len = count;
}

// (Part 2:) return 1 if it is impossible to traverse the edge of a tile (because ac onnection is in the way)
// accepts a corner coordinate (= top-left of corresponding tile) and a direction (dx,dy)
static int is_tile_edge_blocked(s_input_t *in, int *pipeline_tiles, int x, int y, int dx, int dy) {
  if ((x+dx < 0 || x+dx >= in->size_x) || (y+dy < 0 || y+dy >= in->size_y))
    return 1;

  // x+1
  if (dx==1 && dy==0)
    return (pipeline_tiles[SERXY(x,y-1,in)] & S) && (pipeline_tiles[SERXY(x,y,in)] & N);
  // x-1
  else if (dx==-1 && dy==0)
    return (pipeline_tiles[SERXY(x-1,y,in)] & N) && (pipeline_tiles[SERXY(x-1,y-1,in)] & S);
  // y+1
  else if (dy==1 && dx==0)
    return (pipeline_tiles[SERXY(x,y,in)] & W) && (pipeline_tiles[SERXY(x-1,y,in)] & E);
  // y-1
  else if (dy==-1 && dx==0)
    return (pipeline_tiles[SERXY(x,y-1,in)] & W) && (pipeline_tiles[SERXY(x-1,y-1,in)] & E);
  else
    assert(0);
  return 1;
}

void print_enclosed(s_input_t *in, int *pipeline_tiles, int *is_outside) {
  for (int yy=0; yy<in->size_y; ++yy) {
    for (int xx=0; xx<in->size_x; ++xx) {
      int idx = SERXY(xx, yy, in);
      if (pipeline_tiles[idx]) {
        fputs(TCOL_YEL, stdout);
        putchar(get_c_from_conn(in->tiles[idx]));
        fputs(TCOL_CLR, stdout);
      }
      else if (is_outside[idx]) {
        fputs(TCOL_MYS, stdout);
        putchar(get_c_from_conn(in->tiles[idx]));
        fputs(TCOL_CLR, stdout);
      }
      else {
        fputs(TCOL_TRY, stdout);
        putchar(get_c_from_conn(in->tiles[idx]));
        fputs(TCOL_CLR, stdout);
      }
    }
    putchar('\n');
  }
}

// it would be easy enough to count outside tiles on the fly
// (excluding pipeline tiles), but since we are already printing the
// grid, we might as well count everything again.
int count_enclosed(s_input_t *in, int *pipeline_tiles, int *is_outside) {
  int enclosed_count = 0;
  for (int yy=0; yy<in->size_y; ++yy) {
    for (int xx=0; xx<in->size_x; ++xx) {
      int idx = SERXY(xx, yy, in);
      if (pipeline_tiles[idx]) {
      }
      else if (is_outside[idx]) {
      }
      else {
        ++ enclosed_count;
      }
    }
  }
  return enclosed_count;
}

// recursive depth first search of tiles reachable from x y (without crossing the pipeline)
static int tile_dfs_r(s_input_t *in, int *pipeline_tiles, int *is_outside, int x, int y, int *acc, int *is_visited) {
  if (g_verbose) {
    printf("@%d,%d\n", x, y);
    print_enclosed(in, pipeline_tiles, is_outside);
  }

  if (is_visited[SERXYSTRIDE(x,y,in->size_x+1)])
    return 0;

  is_visited[SERXYSTRIDE(x,y,in->size_x+1)] = 1;

  // mark reached tiles outside
  is_outside[SERXY(x,y,in)] = 1;
  if (x-1>0) {
    is_outside[SERXY(x-1,y,in)] = 1;
    if (y-1>0)
      is_outside[SERXY(x-1,y-1,in)] = 1;
  }
  if (y-1>0)
    is_outside[SERXY(x,y-1,in)] = 1;

  if (!is_tile_edge_blocked(in, pipeline_tiles, x, y, 1, 0)) {
    if (g_verbose)
      printf("@%d,%d ->", x, y);
    tile_dfs_r(in, pipeline_tiles, is_outside, x+1, y, acc, is_visited);
    ++*acc;
  }

  if (!is_tile_edge_blocked(in, pipeline_tiles, x, y, -1, 0)) {
    if (g_verbose)
      printf("@%d,%d ->", x, y);
    tile_dfs_r(in, pipeline_tiles, is_outside, x-1, y, acc, is_visited);
    ++*acc;
  }

  if (!is_tile_edge_blocked(in, pipeline_tiles, x, y, 0, 1)) {
    if (g_verbose)
      printf("@%d,%d ->", x, y);
    tile_dfs_r(in, pipeline_tiles, is_outside, x, y+1, acc, is_visited);
    ++*acc;
  }

  if (!is_tile_edge_blocked(in, pipeline_tiles, x, y, 0, -1)) {
    if (g_verbose)
      printf("@%d,%d ->", x, y);
    tile_dfs_r(in, pipeline_tiles, is_outside, x, y-1, acc, is_visited);
    ++*acc;
  }

  return 0;
}

int tile_dfs(s_input_t *in, int *pipeline_tiles, int **is_out) {
  int *is_outside = calloc(in->size_x * in->size_y, sizeof(int));
  int *corner_is_visited = calloc((in->size_x+1) * (in->size_y+1), sizeof(int));
  *is_out = is_outside;
  int acc = 0;

  for (int xx=0; xx < in->size_x; ++xx) {
    tile_dfs_r(in, pipeline_tiles, is_outside, xx, 0, &acc, corner_is_visited);
    tile_dfs_r(in, pipeline_tiles, is_outside, xx, in->size_y-0, &acc, corner_is_visited);
  }

  for (int yy=0; yy < in->size_y; ++yy) {
    tile_dfs_r(in, pipeline_tiles, is_outside, 0,            yy, &acc, corner_is_visited);
    tile_dfs_r(in, pipeline_tiles, is_outside, in->size_x-0, yy, &acc, corner_is_visited);
  }

  printf("visited %d corners\n", acc);
  return acc;
}

int main(int argc, char *argv[]) {

  printf("%c[0;33mHello, world!%c[0m\n", 27, 27); /* Yellowish color */

  if (argc < 2) {
    return 1;
  }

  FILE *infilep;
  if (( infilep = fopen(argv[1], "r")) == NULL)
    solution_abort("problem opening file", __LINE__);

  s_input_t *in = parse_input(infilep);
  print_input(stdout, *in);

  int len;
  int *pipeline_tiles;
  int *is_outside;

  trace_path(in, &len, &pipeline_tiles);
  tile_dfs(in, pipeline_tiles, &is_outside);

  int furthest = (len+1)/2;
  printf("furthest = %d\n", furthest);

  int enclosed_count = 0;

  print_enclosed(in, pipeline_tiles, is_outside);
  enclosed_count = count_enclosed(in, pipeline_tiles, is_outside);

  printf("furthest = %d\n", furthest);
  printf("enclosed: %d\n", enclosed_count);

  return 0;
}
