#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <png.h>

static int verbose = 0;
static char *basename = "images";
static char *imagedir = ".";

/*****************************************************************************/

char *
concat (char *a, char *b)
{
  char *buf = 0;
  int i = strlen(a) + strlen(b) + 2;
  buf = (char *)malloc(i);
  sprintf(buf, "%s/%s", a, b);
  return buf;
}

/*****************************************************************************/

typedef struct subimage_struct {
  struct subimage_struct *next;
  char *filename;
  int w, h, max;
  int index;
} subimage_struct;

typedef struct image_struct {
  struct image_struct *next;
  char *name;
  int a, d;
  subimage_struct *sub[3];
  int index;
} image_struct;

image_struct *image_list = 0;
int current_index = 0;

void
scan_image_directory ()
{
  DIR *dir;
  struct dirent *de;
  image_struct *img;
  subimage_struct *sub;

  dir = opendir(imagedir);
  if (!dir) {
    fprintf(stderr, "Unable to scan directory %s for images.\n", imagedir);
    perror("The error was");
    exit(1);
  }
  while (de = readdir(dir)) {
    FILE *f;
    char *dot;
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, type;

    dot = strrchr(de->d_name, '.');
    if (!dot)
      continue;
    if (strcmp(dot, ".png"))
      continue;

    dot = strchr(de->d_name, '.');
    *dot = 0;
    for (img=image_list; img; img=img->next)
      if (strcmp(img->name, de->d_name) == 0)
	break;
    if (!img)
      continue;
    *dot = '.';

    f = fopen(concat(imagedir, de->d_name), "rb");

    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
    info_ptr = png_create_info_struct (png_ptr);

    if (setjmp (png_ptr->jmpbuf)) {
      fclose (f);
      continue;
    }

    png_init_io (png_ptr, f);

    png_read_info (png_ptr, info_ptr);

    png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth,
		  &color_type, &interlace_type, 0, 0);

    if (bit_depth == 1)
      type = 0;
    else if (color_type & PNG_COLOR_MASK_COLOR)
      type = 2;
    else
      type = 1;

    sub = (subimage_struct *)malloc(sizeof(subimage_struct));
    sub->next = img->sub[type];
    img->sub[type] = sub;
    sub->filename = strdup (concat (imagedir, de->d_name));
    sub->w = width;
    sub->h = height;
    sub->index = current_index++;

    fclose (f);
  }
}

static int print_col = 0;
static int print_com = 1;
static void
print_int(FILE *f, int val)
{
  char buf[20];
  sprintf(buf, "%d", val);
  if (print_com) {
    fputc(',', f);
    print_col ++;
  } else {
    fprintf(f, "  ");
    print_col = 2;
  }
  if (print_col + strlen(buf) > 70) {
    fprintf(f, "\n  ");
    print_col = 2;
  }
  fputs(buf, f);
  print_col += strlen(buf);
  print_com = 1;
}

void
dump_images(FILE *o)
{
  image_struct *img;
  subimage_struct *sub;
  int i;
  static char *type_names[] = {"mono", "grey", "color"};

  for (img=image_list; img; img=img->next)
    for (i=0; i<3; i++)
      for (sub=img->sub[i]; sub; sub=sub->next)
	{
	  FILE *f = fopen(sub->filename, "rb");
	  int byte;
	  fprintf(o, "/* %s */\n", sub->filename);
	  fprintf(o, "static const unsigned char data_%d[] = {\n", sub->index);
	  print_com = 0;
	  while ((byte = fgetc (f)) != EOF)
	    print_int(o, byte);
	  fprintf(o, "};\n\n");
	  fclose (f);
	}

  for (img=image_list; img; img=img->next)
    for (i=0; i<3; i++)
      if (img->sub[i])
	{
	  fprintf(o, "static image sub_%d_%s[] = {\n", img->index, type_names[i]);
	  for (sub=img->sub[i]; sub; sub=sub->next)
	    fprintf(o, "  { %d, %d, data_%d },\n", sub->w, sub->h, sub->index);
	  fprintf(o, "  { 0, 0, 0 }\n};\n\n");
	}

  fprintf(o, "static image_list %s[] = {\n", basename);
  for (img=image_list; img; img=img->next)
    {
      char *c = "";
      fprintf(o, "  { \"%s\", %d, %d, { ", img->name, img->a, img->d);
      for (i=0; i<3; i++)
	{
	  if (img->sub[i])
	    fprintf(o, "%ssub_%d_%s", c, img->index, type_names[i]);
	  else
	    fprintf(o, "%s0", c);
	  c = ", ";
	}
      fprintf(o, " } },\n");
    }
  fprintf(o, "  { 0, 0, 0, { 0, 0, 0 } }\n};\n\n");

  fprintf(o, "REGISTER_IMAGE_LIBRARY(%s)\n", basename);
  fprintf(o, "void need_imglib_%s(){}\n", basename);
}

/*****************************************************************************/

char *
tokenize(char *string)
{
  static char *next;
  char *rv;
  if (string) {
    next = string;
    return;
  }
  while (*next && !isgraph(*next)) next++;
  if (!*next) return 0;
  rv = next;
  while (*next && isgraph(*next)) next++;
  if (*next)
    *next++ = 0;
  return rv;
}

void
give_help()
{
  fprintf(stderr, "make-imgdir [options] infile outfile [pngs]\n");
  fprintf(stderr, "  -v               verbose\n");
  fprintf(stderr, "  -h               print this help message\n");
  fprintf(stderr, "  -n basename      specify basename for symbols\n");
  fprintf(stderr, "  -i imagedir      location of images\n");
  fprintf(stderr, "  -d dependencies  write dependencies to this file\n");
  fprintf(stderr, "  infile format: <image> [across down]\n");
  fprintf(stderr, "  if infile is `-' image names are taken from the command line.\n");
  exit(1);
}

/*****************************************************************************/

int
main(int argc, char **argv)
{
  struct stat s;
  int x, y, o;
  FILE *inf, *outf, *depfile;
  char *inbuf, *outfname, *tok;
  char *imagename;
  int across, down;
  image_struct *img;

  while (1)
    switch (getopt(argc, argv, "vhn:i:d:m:")) {
    case '?':
    case 'h':
      give_help();
    case 'v':
      verbose++;
      break;
    case 'n':
      basename = optarg;
      break;
    case 'i':
      imagedir = optarg;
      break;
    case 'd':
      depfile = fopen(optarg, "w");
      if (!depfile) {
	fprintf(stderr, "Unable to open dependency file %s for writing\n", depfile);
	perror("The error was");
	exit(1);
      }
      break;
    case -1:
      goto done_args;
    }
 done_args:

  if (optind > argc-1)
    give_help();

  if (strcmp (argv[optind], "-") == 0)
    {
      int i;
      char *dot;
      for (i=optind+2; i<argc; i++)
	{
	  printf("image %s\n", argv[i]);
	  dot = strrchr(argv[i], '.');
	  if (dot) *dot = 0;

	  img = (image_struct *)malloc(sizeof(image_struct));
	  memset(img, 0, sizeof(*img));
	  img->name = strdup (argv[i]);
	  img->a = img->d = 1;
	  img->index = current_index++;

	  img->next = image_list;
	  image_list = img;
	}
    }
  else
    {
      inf = fopen(argv[optind], "r");
      if (!inf) {
	fprintf(stderr, "Unable to open %s for reading\n", argv[optind]);
	perror("The error was");
	exit(1);
      }
      fstat(fileno(inf), &s);
      inbuf = (char *)malloc(s.st_size+1);

      while (fgets(inbuf, s.st_size, inf)) {

	inbuf[strlen(inbuf)] = 0;
	tokenize(inbuf);

	imagename = tokenize(0);
	if (!imagename) continue;

	img = (image_struct *)malloc(sizeof(image_struct));
	memset(img, 0, sizeof(*img));
	img->name = strdup (imagename);
	img->a = img->d = 1;
	img->index = current_index++;

	tok = tokenize(0);
	if (tok)
	  img->a = atoi(tok);
	tok = tokenize(0);
	if (tok)
	  img->d = atoi(tok);

	img->next = image_list;
	image_list = img;
      }
    }

  scan_image_directory();

  if (optind > argc-2)
    outfname = 0;
  else
    outfname = argv[optind+1];

  if (outfname)
    outf = fopen(outfname, "w");
  else
    outf = stdout;
  if (!outf) {
    fprintf(stderr, "Unable to open %s for writing\n", outfname);
    perror("The error was");
    exit(1);
  }

  fprintf(outf, "/* Automatically generated, do not edit */\n");
  fprintf(outf, "#include \"imagelib.h\"\n\n");

  dump_images(outf);

  if (outfname) fclose(outf);

  exit(0);
}
