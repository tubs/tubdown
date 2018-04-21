#include "tubdown.h"
#include "uncursed.h"
#include <stdlib.h>

typedef struct td_uncursed_data {
  WINDOW *window;
  int base_color_pair;
} td_uncursed_data;

static int write_n_string(void *user_data, int y, int x, const wchar_t *text, int length) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;
  return mvwaddwnstr(data->window, y, x, text, length);
}

static int write_string(void *user_data, int y, int x, const wchar_t *text) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;
  return mvwaddwstr(data->window, y, x, text);
}

static int apply_style(void *user_data, int y, int x, int length, uint8_t flags, uint8_t color) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;

  unsigned long attr = 0;
  if (flags & TD_STYLE_FLAG_BOLD)
    attr |= A_BOLD;
  if (flags & TD_STYLE_FLAG_UNDERLINED)
    attr |= A_UNDERLINE;
  if (flags & TD_STYLE_FLAG_REVERSED)
    attr |= A_REVERSE;

  attr |= COLOR_PAIR(color + data->base_color_pair);

  return mvwchgat(data->window, y, x, length, attr, 0, NULL);
}

static int height(void *user_data) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;
  return data->window->maxy + 1;
}

static int td_uncursed_clear(void *user_data) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;
  return wclear(data->window);
}

static int td_uncursed_refresh(void *user_data) {
  td_uncursed_data *data = (td_uncursed_data *)user_data;
  return wrefresh(data->window);
}

void *td_uncursed_new(WINDOW *window, int base_color_pair) {
  init_pair(base_color_pair + TD_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(base_color_pair + TD_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);

  init_pair(base_color_pair + TD_COLOR_MAGENTA_ON_WHITE, COLOR_MAGENTA, COLOR_WHITE);

  td_uncursed_data *data = (td_uncursed_data *)malloc(sizeof(*data));
  if (data) {
    data->window = window;
    data->base_color_pair = base_color_pair;
  }
  return data;
}

td_renderer td_uncursed() {
  td_renderer r = {
    .write_string = write_string,
    .write_n_string = write_n_string,
    .apply_style = apply_style,
    .height = height,
    .clear = td_uncursed_clear,
    .refresh = td_uncursed_refresh
  };
  return r;
}


