/* The Ace of Penguins - text2c.c
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

int
main(int argc, char **argv)
{
  int w=0, c;
  char *base = "text";
  if (argc > 1)
    base = argv[1];

  printf("/* This is a generated file - DO NOT EDIT */\n");
  printf("/* Copyright for this file is copied from the source from */\n");
  printf("/* which it was generated */\n\n");
  printf("char %s[] = {\n", base);
  while ((c = getchar()) != EOF)
  {
    if (w == 16)
    {
      putchar('\n');
      w = 0;
    }
    printf(" %d,", c);
    w++;
  }
  printf("\n 0 };\n");
  exit(0);
}
