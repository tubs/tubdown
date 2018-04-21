#ifndef TUBDOWN_H
#define TUBDOWN_H

#include <stdint.h>
#include <string.h>

#if !defined(__cplusplus)
#include <wchar.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif


#define TD_STYLE_FLAG_BOLD       (1u << 0)
#define TD_STYLE_FLAG_UNDERLINED (1u << 1)
#define TD_STYLE_FLAG_ITALIC     (1u << 2)
#define TD_STYLE_FLAG_HEADER     (1u << 3)
#define TD_STYLE_FLAG_QUOTE      (1u << 4)
#define TD_STYLE_FLAG_REVERSED   (1u << 5)

#define TD_COLOR_WHITE 0
#define TD_COLOR_RED 1
#define TD_COLOR_BLUE 2
#define TD_COLOR_GREEN 3
#define TD_COLOR_YELLOW 4
#define TD_COLOR_MAGENTA 5
#define TD_COLOR_CYAN 6
#define TD_COLOR_MAGENTA_ON_WHITE 7

typedef struct td_renderer {
  int (*write_n_string)(void *user_data, int y, int x, const wchar_t *text, int length);
  int (*write_string)(void *user_data, int y, int x, const wchar_t *text);
  int (*apply_style)(void *user_data, int y, int x, int length, uint8_t flags, uint8_t color);
  int (*height)(void *user_data);
  int (*clear)(void *user_data);
  int (*refresh)(void *user_data);
} td_renderer;

typedef struct td_page { void *ptr; } td_page;
typedef struct td_config { void *ptr; } td_config;
typedef struct td_arglist { void *ptr; } td_arglist;
typedef struct td_history { void *ptr; } td_history;
typedef struct td_map { void *ptr; } td_map;

typedef void (*tb_command_callback)(td_page page, td_arglist args, int argCount);

td_config td_config_new(td_renderer *renderer);
void td_config_delete(td_config config);
void td_config_register_command(td_config config, const wchar_t* command, tb_command_callback callback);

const wchar_t *td_arglist_get(td_arglist args, int index);

td_page td_page_new(td_config config, const wchar_t *file);
void td_page_delete(td_page page);

void td_page_append_line(td_page page, const wchar_t *line);

void td_page_append_style(td_page page, int line, int begin, int end,
    uint8_t flags, uint8_t color);

void td_page_append_link(td_page page, int line, int begin, int end,
    const wchar_t *href, int anchor);

void td_page_get_current_location(td_page page, int *line, int *col);

void td_page_render(td_page page, void *user_data);
void td_page_cycle_link(td_page page);
const wchar_t* td_page_get_link(td_page page);

td_history td_history_new();
void td_history_delete(td_history history);
td_page td_history_pop(td_history history);
void td_history_push(td_history, td_page page);

td_map td_map_new();
void td_map_delete(td_map map);
void td_map_insert_page(td_map map, const wchar_t *name, td_page page);
td_page td_map_find(td_map map, const wchar_t *name);

#if defined(TD_UNCURSED)
void *td_uncursed_new(WINDOW *window, int base_color_pair);
td_renderer td_uncursed();
#endif

#if defined(__cplusplus)
}
#endif

#endif
