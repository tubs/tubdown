#define TD_UNCURSED
#include "uncursed.h"
#include "tubdown.h"

static void insert_item_description(td_page page, td_arglist args, int arg_count) {
  td_page_append_line(page, L"This is an item description");
}

int main(int argc, char **argv) {
  initialize_uncursed(&argc, argv);

  curs_set(0);
  start_color();
  init_pair(512, 0, 0);

  initscr();

  void *window_data = td_uncursed_new(stdscr, 0);
  td_renderer renderer = td_uncursed();

  td_config config = td_config_new(&renderer);
  td_config_register_command(config, L"item", insert_item_description);

  td_page start = td_page_new(config, L"home");
  td_page current = start;

  td_history history = td_history_new();
  td_map pages = td_map_new();

  td_page_render(current, window_data);

  while (1) {
    int k = getch();
    switch (k) {
      case 'Q':
      case KEY_ESCAPE:
        goto end;
      case 4:
        td_page_scroll(current, window_data, renderer.height(window_data) / 2);
        break;
      case 21:
        td_page_scroll(current, window_data, -renderer.height(window_data) / 2);
        break;
      case 'j':
        td_page_scroll(current, window_data, 1);
        break;
      case 'k':
        td_page_scroll(current, window_data, -1);
        break;
      case '\t':
        td_page_cycle_link(current);
        break;
      case ' ':
        {
          const wchar_t* link = td_page_get_link(current);
          if (link) {
            td_history_push(history, current);
            current = td_map_find(pages, link);
            if (!current.ptr) {
              current = td_page_new(config, link);
              td_map_insert_page(pages, link, current);
            }
          }
        }
        break;
      case 'b':
        {
          td_page back = td_history_pop(history);
          if (back.ptr)
            current = back;
        }
        break;
      default:
        printf("%x\n", k);
    }
    td_page_render(current, window_data);

  }

end:
  endwin();
  return 0;

}
