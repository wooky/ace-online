/* The Ace of Penguins - launcher.c
   Combined launcher for all the Ace of Penguins games (see issue #160).

   This file contains the only main() in the combined executable.  It is an
   X11 program built on the same table/xwin framework as the games: it draws
   the list of games in white text on the standard green background, and when
   a game is clicked it runs that game in the same X11 window.

   Each game's main() has been renamed to <game>_main() and each game's exit()
   redirected to ace_return_to_launcher() by the FetchContent patch
   (tools/patch-sources.sh), so a game "quitting" longjmps back here instead of
   terminating the process. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "imagelib.h"
#include "table.h"

/* --- framework bits not exposed in the public headers ----------------- */
/* From xwin.c: draws white text on the green background at a baseline,
   fills a rectangle with the background, and flushes to the server. */
extern void text(char *t, int x, int y);
extern void clear(int x, int y, int w, int h);
extern void flush(void);
extern int font_width, font_height;

/* Added to lib/table.c and lib/funcs.c by tools/patch-sources.sh: reset the
   one-shot init guard / persistent state and the callback pointers so each
   launched game (and the returning launcher) starts from a clean slate. */
extern void ace_reset_table(void);
extern void ace_reset_funcs(void);
extern void ace_reset_stacks(void);

/* --- the games -------------------------------------------------------- */
/* main() of each game, renamed by the patch. */
#define DECL(n) extern int n##_main(int argc, char **argv);
DECL(canfield) DECL(freecell) DECL(golf) DECL(mastermind) DECL(merlin)
DECL(minesweeper) DECL(pegged) DECL(solitaire) DECL(spider)
DECL(taipedit) DECL(taipei) DECL(thornq)
#undef DECL

typedef int (*game_main_fn)(int, char **);
typedef struct {
  char *label;        /* shown in the launcher          */
  char *name;         /* passed as argv[0] to the game  */
  game_main_fn fn;
} GameEntry;

static GameEntry games[] = {
  { "Canfield",     "canfield",    canfield_main    },
  { "Freecell",     "freecell",    freecell_main    },
  { "Golf",         "golf",        golf_main        },
  { "Mastermind",   "mastermind",  mastermind_main  },
  { "Merlin",       "merlin",      merlin_main      },
  { "Minesweeper",  "minesweeper", minesweeper_main },
  { "Pegged",       "pegged",      pegged_main      },
  { "Solitaire",    "solitaire",   solitaire_main   },
  { "Spider",       "spider",      spider_main      },
  { "Taipei",       "taipei",      taipei_main      },
  { "Taipei Edit",  "taipedit",    taipedit_main    },
  { "Thornq",       "thornq",      thornq_main      },
};
#define NUM_GAMES ((int)(sizeof(games) / sizeof(games[0])))

/* --- layout ----------------------------------------------------------- */
#define LAUNCHER_W   300
#define MARGIN       24
#define TITLE_Y      32
#define SUBTITLE_Y   56
#define FIRST_ROW_Y  88
#define ROW_H        26
#define LAUNCHER_H   (FIRST_ROW_Y + NUM_GAMES * ROW_H + MARGIN)

/* --- launcher <-> game control flow ----------------------------------- */
static int    saved_argc;
static char **saved_argv;

static jmp_buf launcher_jb;
static int     in_game = 0;

/* Replaces exit() in the games and in the framework's default key handler.
   While a game is running, jump back to the launcher; otherwise really quit
   (e.g. the launcher's own quit, or a WM-close arriving outside a game). */
void
ace_return_to_launcher(int code)
{
  if (in_game)
    longjmp(launcher_jb, 1);
  exit(code);
}

/* --- launcher callbacks ----------------------------------------------- */
static void launch_game(int i);

static void
launcher_redraw(void)
{
  int i;
  text("The Ace of Penguins", MARGIN, TITLE_Y);
  text("Click a game to play:", MARGIN, SUBTITLE_Y);
  for (i = 0; i < NUM_GAMES; i++)
    text(games[i].label, MARGIN + 2 * font_width, FIRST_ROW_Y + i * ROW_H);
}

static void
launcher_click(int x, int y, int b)
{
  int i;
  for (i = 0; i < NUM_GAMES; i++)
    {
      int top = FIRST_ROW_Y + i * ROW_H - font_height;
      if (y >= top && y < top + ROW_H)
	{
	  launch_game(i);
	  return;
	}
    }
}

static void
launcher_key(int k, int x, int y)
{
  if (k == 'q' || k == 'Q' || k == 27 || k == 3)
    exit(0);
}

static void
launcher_resize(int w, int h)
{
  table_no_resize();
}

static void launcher_nop(void) {}

static FunctionMapping launcher_fmap[] = {
  { "init",         (void *) launcher_nop    },
  { "redraw",       (void *) launcher_redraw },
  { "click",        (void *) launcher_click  },
  { "drag",         (void *) launcher_nop    },
  { "drop",         (void *) launcher_nop    },
  { "key",          (void *) launcher_key    },
  { "resize",       (void *) launcher_resize },
  { "double_click", (void *) launcher_nop    },
  { 0, 0 }
};

static void
install_launcher(void)
{
  init_ace(saved_argc, saved_argv, launcher_fmap);
  ace_reset_table();
  init_table(LAUNCHER_W, LAUNCHER_H);

  /* Paint the menu directly rather than via invalidate(): shrinking the
     window back to launcher size produces no expose event, and the low-level
     clear()/text() primitives draw unconditionally (invalidate() is gated by
     the framework's graphics-disabled flag). */
  clear(0, 0, table_width, table_height);
  launcher_redraw();
  flush();
}

static void
launch_game(int i)
{
  char *gargv[2];

  /* Wipe the launcher's text first (while table_width/height still hold the
     launcher size) so nothing shows through in regions a smaller game won't
     repaint. */
  clear(0, 0, table_width, table_height);
  flush();

  /* Start the game from a clean framework state, just like a fresh process
     (this zeroes table_width/height so the game sizes its own window, and
     frees the previous game's card stacks so they aren't redrawn here). */
  ace_reset_funcs();
  ace_reset_table();
  ace_reset_stacks();

  in_game = 1;
  if (setjmp(launcher_jb) == 0)
    {
      gargv[0] = games[i].name;
      gargv[1] = 0;
      games[i].fn(1, gargv);   /* runs its own event loop until it "exits" */
    }
  in_game = 0;

  /* The game longjmped back here on quit; restore the launcher in the same
     window. */
  install_launcher();
}

int
main(int argc, char **argv)
{
  saved_argc = argc;
  saved_argv = argv;

  init_ace(argc, argv, launcher_fmap);
  init_table(LAUNCHER_W, LAUNCHER_H);
  table_loop();
  return 0;
}
