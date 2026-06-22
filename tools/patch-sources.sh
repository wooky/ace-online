#!/bin/sh
# Patch the fetched Ace of Penguins sources for the combined launcher build
# (issue #160).  Run by CMake's FetchContent PATCH_COMMAND in the populated
# source directory.  Transformations:
#
#   * games/<g>.c : rename main()        -> <g>_main()
#   * games/<g>.c : redirect exit(...)   -> ace_return_to_launcher(...)
#   * lib/funcs.c : redirect exit(...)   -> ace_return_to_launcher(...)
#                   (the default key handler) and add ace_reset_funcs()
#   * lib/table.c : add ace_reset_table() to reset the one-shot init guard
#                   and related persistent state
#
# Idempotent: guarded by a marker file so re-running (or re-configuring) is a
# no-op and never double-rewrites exit()/main().
set -e

marker=".launcher-patched"
if [ -f "$marker" ]; then
  echo "patch-sources.sh: sources already patched, skipping"
  exit 0
fi

games="canfield freecell golf mastermind merlin minesweeper pegged solitaire spider taipedit taipei thornq"

# The exit() redirect matches the libc call (exit preceded by a non-identifier
# character) so it never touches identifiers like atexit/_exit.
exit_re='s/\([^A-Za-z0-9_]\)exit *(/\1ace_return_to_launcher(/g'

for g in $games; do
  f="games/$g.c"
  sed -i "s/main(int argc, char \*\*argv)/${g}_main(int argc, char **argv)/" "$f"
  sed -i "$exit_re" "$f"
done

sed -i "$exit_re" lib/funcs.c

# Make the X11 layer reusable so every game (and the returning launcher) shares
# ONE display connection and ONE window instead of creating new ones each time.
#   * xwin_init  : if the display is already open, just refresh the window-title
#                  name and return (don't reopen the display / re-create the GC,
#                  font, colours, ...).
#   * xwin_create: if the window already exists, resize it in place instead of
#                  creating a brand new window.  The size hints are reset to
#                  PSize first: the launcher (and fixed-size games) pin the
#                  window with PMinSize==PMaxSize via xwin_fixed_size, which
#                  would otherwise make the WM refuse the next game's resize.
#                  The whole window is then cleared to the table background so
#                  the new game never shows the previous game's pixels in
#                  regions it doesn't repaint (some compositors don't deliver
#                  the post-resize expose the framework would otherwise rely
#                  on).  display_width/height covers any size regardless of
#                  when the async resize actually lands.
sed -i "s|  name = argv\[0\];|  name = argv[0]; { char *aop_sl = strrchr(name, '/'); if (aop_sl) name = aop_sl + 1; } if (display) return 0;|" lib/xwin.c
sed -i "s|  XSetWindowAttributes attributes;|  XSetWindowAttributes attributes; if (window) { XSizeHints aop_sh; aop_sh.flags = PSize; aop_sh.width = width; aop_sh.height = height; XSetWMNormalHints(display, window, \&aop_sh); XResizeWindow(display, window, width, height); static_display_image.width = width; static_display_image.height = height; XSetClipMask(display, gc, None); XSetForeground(display, gc, table_background); XFillRectangle(display, window, gc, 0, 0, display_width, display_height); XFlush(display); return; }|" lib/xwin.c

cat >> lib/funcs.c <<'EOF'

/* Added for the combined launcher build (issue #160): restore every callback
   to its default so a freshly launched game starts from a clean slate. */
void
ace_reset_funcs(void)
{
  click_cb = default_click_cb;
  drag_cb = default_drag_cb;
  redraw_cb = default_redraw_cb;
  init_cb = default_init_cb;
  drop_cb = default_drop_cb;
  key_cb = default_key_cb;
  resize_cb = default_resize_cb;
  double_click_cb = default_double_click_cb;
}
EOF

cat >> lib/stack.c <<'EOF'

/* Added for the combined launcher build (issue #160): free every card stack
   and reset drag/undo state.  The stack list is a file-global; without this a
   freshly launched card game inherits the previous game's stacks and
   stack_redraw() paints their cards onto the new game. */
void
ace_reset_stacks(void)
{
  while (stacks)
  {
    Stack *s = stacks;
    stacks = s->next;
    if (s != dragging_s)   /* dragging_s->cards aliases a real stack's array */
      free(s->cards);
    free(s);
  }
  dragging_s = 0;
  dragging_os = 0;
  dragging_n = 0;
  num_undo = 0;
  nodrop_showing = 0;
}
EOF

cat >> lib/table.c <<'EOF'

/* Added for the combined launcher build (issue #160): reset the one-shot init
   guard and persistent table state between games. */
void
ace_reset_table(void)
{
  initted = 0;
  no_resize = 0;
  centered_pic = 0;
  help_is_showing = 0;
  /* Zero the size so the next game computes its own preferred dimensions,
     exactly as it would as a fresh process (otherwise it inherits whatever
     the launcher / previous game left here). */
  table_width = 0;
  table_height = 0;
}
EOF

touch "$marker"
echo "patch-sources.sh: done"
