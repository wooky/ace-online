/* The Ace of Penguins - make-cards.c
   Copyright (C) 1998 DJ Delorie

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>

#include "gd.h"

gdImagePtr blank;
gdImagePtr pjack;
gdImagePtr pqueen;
gdImagePtr pking;
gdImagePtr a2k;
gdImagePtr suits;
gdImagePtr bigsuits;
gdImagePtr smsuits;
gdImagePtr penguin;
int  w, h;

char *suit_letters = "cdsh";
char *num_letters = "a234567890jqk";
#define SPADE 2

char *icons[] = {
  "52",
  "1292",
  "125292",
  "11139193",
  "1113529193",
  "111351539193",
  "11133251539193",
  "1113:1:3;1;39193",
  "1113:1:352;1;39193",
  "11132241436163829193"
};

int xx[] = { 0, 15, 30, 45 };
int yy[] = { 0, 12, 23, 27, 34, 42, 50, 57, 61, 72, 32, 52 };

gdImagePtr
load_image(char *filename)
{
  gdImagePtr rv;
  FILE *f = fopen(filename, "rb");
  if (!f)
  {
    perror(filename);
    exit(0);
  }
  rv = gdImageCreateFromGif(f);
  fclose(f);
  return rv;
}

void
load_gifs()
{
  blank = load_image("blank.gif");
  pjack = load_image("pjack.gif");
  pqueen = load_image("pqueen.gif");
  pking = load_image("pking.gif");
  a2k = load_image("a-k.gif");
  suits = load_image("suits.gif");
  smsuits = load_image("smsuits.gif");
  bigsuits = load_image("bigsuits.gif");
  penguin = load_image("penguin.gif");
}

void
C13(gdImagePtr d, int x, int y, gdImagePtr s, int sx, int sy)
{
  int ax, ay, tp;
  gdImageCopy(d, s, x, y, (sx)*13, (sy)*13, 13, 13);
  if (y > 42)
    for (ax=0; ax<13; ax++)
      for (ay=0; ay<13; ay++)
      {
	tp = gdImageGetPixel(d, x+ax, y+ay);
	gdImageSetPixel(d, x+ax, y+ay, gdImageGetPixel(d, x+12-ax, y+12-ay));
	gdImageSetPixel(d, x+12-ax, y+12-ay, tp);
	if (ax == 6 && ay == 6)
	  return;
      }
}

void
C9(gdImagePtr d, int x, int y, gdImagePtr s, int sx, int sy)
{
  int ax, ay, tp;
  gdImageCopy(d, s, x, y, (sx)*9, (sy)*9, 9, 9);
  if (y > 42)
    for (ax=0; ax<9; ax++)
      for (ay=0; ay<9; ay++)
      {
	tp = gdImageGetPixel(d, x+ax, y+ay);
	gdImageSetPixel(d, x+ax, y+ay, gdImageGetPixel(d, x+8-ax, y+8-ay));
	gdImageSetPixel(d, x+8-ax, y+8-ay, tp);
	if (ax == 4 && ay == 4)
	  return;
      }
}

gdImagePtr
start_with(gdImagePtr src, int s, int n)
{
  gdImagePtr gd = gdImageCreate(w, h);
  gdImageCopy(gd, src, 0, 0, 0, 0, w, h);
  C13(gd, 1, 2, a2k, s&1, n);
  C9(gd, 3, 17, smsuits, 0, s);
  C13(gd, 59, 82, a2k, s&1, n);
  C9(gd, 61, 71, smsuits, 0, s);
  return gd;
}

void
save(gdImagePtr gd, int n, int s)
{
  FILE *f;
  char name[30];
  sprintf(name, "card/%c%c.gif", num_letters[n], suit_letters[s]);
  f = fopen(name, "wb");
  gdImageGif(gd, f);
  fclose(f);
  gdImageDestroy(gd);
}

void
face_card(gdImagePtr src, int n, int s)
{
  gdImagePtr gd = start_with(src, s, n);
  C13(gd, 17, 14, suits, 0, s);
  C13(gd, 43, 70, suits, 0, s);
  save(gd, n, s);
}

int
main()
{
  int s, n, i;
  gdImagePtr gd;

  mkdir("card", 0755);

  load_gifs();

  w = gdImageSX(blank);
  h = gdImageSY(blank);

  for (s=0; s<4; s++)
  {
    gd = start_with(blank, s, 0);
    if (s == SPADE)
      gdImageCopy(gd, penguin,
		  gdImageSX(gd)/2-gdImageSX(penguin)/2,
		  gdImageSY(gd)/2-gdImageSY(penguin)/2,
		  0, 0, gdImageSX(penguin), gdImageSY(penguin));
    else
      gdImageCopy(gd, bigsuits, 30-13, 42-13, 0, s*39, 39, 39);
    save(gd, 0, s);

    for (n=1; n<10; n++)
    {
      gd = start_with(blank, s, n);
      for (i=0; i<strlen(icons[n]); i+=2)
      {
	int y = icons[n][i] - '0';
	int x = icons[n][i+1] - '0';
	C13(gd, xx[x], yy[y], suits, 0, s);
      }
      save(gd, n, s);
    }

    face_card(pjack, 10, s);
    face_card(pqueen, 11, s);
    face_card(pking, 12, s);
  }

  return 0;
}
