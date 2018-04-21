#include "tubdown.hpp"

void Page::Render(void *userData)  {
  renderer->clear(userData);

  int top = 2;

  int y = top;

  for (auto line : lines) {
    renderer->write_n_string(userData, y, 4, line.c_str(), line.length());
    y += 1;
  }

  int height = renderer->height(userData);
  //renderer->write_string(userData, height-2,0, L"controls: link(unselected) link(selected) - TAB: cycle, SPACE: follow, b: back");
  //renderer->apply_style(userData, height-2, 10, 25-10 +1, 0, TD_COLOR_MAGENTA_ON_WHITE);
  //renderer->apply_style(userData, height-2, 27, 40-27 +1, TD_STYLE_FLAG_REVERSED, TD_COLOR_MAGENTA_ON_WHITE);

  for (auto it = styles.rbegin(); it != styles.rend(); it++) {
    StyleSpan& style = *it;
    uint8_t flags = 0;
    if (style.underlined)
      flags |= TD_STYLE_FLAG_UNDERLINED;
    if (style.bold)
      flags |= TD_STYLE_FLAG_BOLD;

    renderer->apply_style(userData, style.line + top, style.begin + 4, style.end-style.begin, flags, style.color);
  }

  for (auto link : links) {
    renderer->apply_style(userData, link.line + top, link.begin + 4, link.end-link.begin, 0, TD_COLOR_MAGENTA_ON_WHITE);
  }

  if (currentLink != -1) {
    auto& link = links.at(currentLink);
    renderer->apply_style(userData, link.line + top, link.begin + 4, link.end-link.begin, TD_STYLE_FLAG_REVERSED, TD_COLOR_MAGENTA_ON_WHITE);
  }

}


Page::Page(TubdownConfig &config, const std::string &fileName) : renderer(config.renderer) {
  std::list<StyleSpan> styleStack;

  int leadingWhitespace = 0;
  bool firstNonWhitespaceInLine = true;
  int colNo = 0;
  int rowNo = 1;


  std::wstring indent = L"                                ";
  wchar_t bullet[] = {
    L'•',
    L'◦',
    L'‐',
  };

  int listIndex = 0;
  bool skipWS = false;
  bool parseCommand = false;

  std::wstring command;
  std::vector<std::wstring> commandArgs;

  std::unordered_map<std::wstring, unsigned int> colors = {
    {L"red", 1},
    {L"blue", 2},
    {L"green", 3},
    {L"yellow", 4},
    {L"magenta", 5},
    {L"cyan", 6},
  };


  CommandCallback currentCommandCallback = nullptr;
  bool inList = false;
  PageLink link;

  std::wifstream inputStream(fileName);
  
  styleStack.push_front({});
  wchar_t c;

  std::wstring line;

  while (inputStream.get(c)) {
    colNo += 1;
    if (c == '\n') {

      if (parseCommand) {
        __builtin_abort();
      }

      lines.push_back(line);

      line = L"";

      inList= false;
      colNo = 1;
      firstNonWhitespaceInLine = true;
      leadingWhitespace = 0;

      rowNo += 1;
      continue;
    }

    //we don't have to handle newlines here since we already 
    //parsed them out
    if (std::iswblank(c) && skipWS)
      continue;

    //as soon as we get past the previous point we're in regular-parse-mode
    skipWS = false;

    //If we are parsing a command don't break the "firstwhitespace", for the case of:
    //\red{ * This is a red list item}
    if (parseCommand) {
      if (std::iswblank(c))
        __builtin_abort();
      if (c != '{') { //make sure we stop parsing the command when we're supposed to...
        command.append(&c, 1);
        continue;
      }
    }

    if (std::iswblank(c) && firstNonWhitespaceInLine) {
      leadingWhitespace += 1;
      continue;
    }

    if (firstNonWhitespaceInLine && (c == '*' || c == '=' || c == '#')) {
      if (c == '*' || c == '#') {
        if (leadingWhitespace >= indent.length())
          __builtin_abort();

        //indent
        line.append(indent.c_str(), leadingWhitespace);

        //add the bullet or list index
        if (c == '*') {
          line.append(&bullet[leadingWhitespace], 1);
          line += L" ";
        } else {
          line.append(std::to_wstring(++listIndex));
          inList = true;
          line += L" ";
        }
      }
      skipWS = true;
      continue;
    }

    //take a copy not a reference.
    StyleSpan currentStyle = styleStack.front();

    if (firstNonWhitespaceInLine && !inList)
      listIndex = 0;

    //bit wank how we do / _ *, but want named thing so eh.
    switch  (c) {
      case '\\':
        parseCommand = true;
        continue;
      case '{':
        currentStyle.line = lines.size();
        currentStyle.begin = line.length();
        if (command == L"quote")
          currentStyle.quote = true;
        else if (std::wcsncmp(command.c_str(), L"link:", 5) == 0) {
          link.line = lines.size();
          link.begin = line.length();
          link.href = command.substr(5);
        } else {
          std::wstring token;
          std::wistringstream tokenStream(command);
          std::wstring commandName;
          if (command.find(L':') != std::wstring::npos) {
            std::getline(tokenStream, commandName, L':');
            while (std::getline(tokenStream, token, L','))
              commandArgs.push_back(token);
            currentCommandCallback = config.commands.at(commandName);
          } else {
            currentStyle.color = colors.at(command);
          }
        }
        styleStack.push_front(currentStyle);

        command = L"";
        parseCommand= false;

        continue;
      case '}':
        if (currentCommandCallback) {
          currentCommandCallback(*this, commandArgs);
          currentCommandCallback = nullptr;
          commandArgs.clear();
        } else if (!link.href.empty()) {
          link.end = line.length();
          links.push_back(link);
          link.href = L"";
        } else {
          styleStack.pop_front();
          currentStyle.end = line.length();
          styles.push_back(currentStyle);
        }
        continue;
      case '/':
        if (currentStyle.italic) {
          styleStack.pop_front();
          currentStyle.end = line.length();
          styles.push_back(currentStyle);
        } else {
          currentStyle.line = lines.size();
          currentStyle.begin = line.length();
          currentStyle.italic = true;
          styleStack.push_front(currentStyle);
        }
        continue;
      case '_':
        if (currentStyle.underlined) {
          styleStack.pop_front();
          currentStyle.end = line.length();
          styles.push_back(currentStyle);
        } else {
          currentStyle.line = lines.size();
          currentStyle.begin = line.length();
          currentStyle.underlined = true;
          styleStack.push_front(currentStyle);
        }
        continue;
      case '*':
        if (currentStyle.bold) {
          styleStack.pop_front();
          currentStyle.end = line.length();
          styles.push_back(currentStyle);
        } else {
          currentStyle.line = lines.size();
          currentStyle.begin = line.length();
          currentStyle.bold = true;
          styleStack.push_front(currentStyle);
        }
        continue;
      default:
        firstNonWhitespaceInLine = false;
        line += c;

        if (line.length() > 70 && c != ' ') {
          int split = line.find_last_of(' ', 70);
          lines.push_back(line.substr(0, split));
          line = line.substr(split);
        }

        break;
    }
  }
}


extern "C" {
td_config td_config_new(td_renderer *renderer) {
  TubdownConfig *config = new TubdownConfig(renderer);
  return {config};
}
void td_config_delete(td_config config) {
  delete static_cast<TubdownConfig*>(config.ptr);
}
void td_config_register_command(td_config config, const wchar_t* command, tb_command_callback callback) {
  (static_cast<TubdownConfig*>(config.ptr))->commands[std::wstring(command)] = [&config, callback]
  (Page& page, const std::vector<std::wstring>& args){
    callback({&page}, {(void *)&args}, args.size());
  };

}

const wchar_t *td_arglist_get(td_arglist args, int index) {
  return static_cast<const std::vector<std::wstring>*>(args.ptr)->at(index).c_str();
}

td_page td_page_new(td_config config, const wchar_t *file) {
  std::wstring wide = file;
  Page *page = new Page(*static_cast<TubdownConfig*>(config.ptr), std::string(wide.begin(), wide.end()));
  return {page};
}

void td_page_delete(td_page page) {
  delete static_cast<Page*>(page.ptr);
}

void td_page_append_line(td_page p, const wchar_t *line) {
  static_cast<Page*>(p.ptr)->lines.push_back(std::wstring(line));
}

#define TD_STYLE_FLAG_BOLD       (1u << 0)
#define TD_STYLE_FLAG_UNDERLINED (1u << 1)
#define TD_STYLE_FLAG_ITALIC     (1u << 2)
#define TD_STYLE_FLAG_HEADER     (1u << 3)
#define TD_STYLE_FLAG_QUOTE      (1u << 4)

static bool hasFlag(uint8_t flags, uint8_t bit) {
  return (flags & bit) != 0;
}

void td_page_append_style(td_page p, int line, int begin, int end,
    uint8_t flags, uint8_t color) {
  static_cast<Page*>(p.ptr)->styles.push_back({line, begin, end, {
    hasFlag(flags, TD_STYLE_FLAG_BOLD),
    hasFlag(flags, TD_STYLE_FLAG_UNDERLINED),
    hasFlag(flags, TD_STYLE_FLAG_ITALIC),
    hasFlag(flags, TD_STYLE_FLAG_HEADER),
    hasFlag(flags, TD_STYLE_FLAG_QUOTE),
    color
  }});
}

void td_page_append_link(td_page p, int line, int begin, int end,
    const wchar_t *href, int anchor) {
  static_cast<Page*>(p.ptr)->links.push_back({std::wstring(href), anchor, line, begin, end});
}

void td_page_get_current_location(td_page p, int *line, int *col) {
  Page& page = *static_cast<Page*>(p.ptr);
  *line = page.lines.size();
  if (page.lines.empty())
    *col = 0;
  else
    *col = page.lines.back().length();
}

void td_page_render(td_page p, void *userData) {
  static_cast<Page*>(p.ptr)->Render(userData);
}

void td_page_cycle_link(td_page p) {
  Page& page = *static_cast<Page*>(p.ptr);
  if (!page.links.empty()) {
    int nextTab = page.currentLink + 1;
    if (nextTab == page.links.size()) {
      nextTab = 0;
    }
    page.currentLink = nextTab;
  }
}
const wchar_t* td_page_get_link(td_page p) {
  Page& page = *static_cast<Page*>(p.ptr);
  if (page.links.empty() || page.currentLink == -1)
    return nullptr;
  return page.links.at(page.currentLink).href.c_str();
}

td_history td_history_new() {
  return {new std::list<td_page>()};
}
void td_history_delete(td_history history) {
  delete static_cast<std::list<td_page>*>(history.ptr);

}
td_page td_history_pop(td_history history) {
  auto list = static_cast<std::list<td_page>*>(history.ptr);
  if (list->empty())
    return {nullptr};

  td_page page = list->front();
  list->pop_front();
  return page;
}
void td_history_push(td_history history, td_page page) {
  static_cast<std::list<td_page>*>(history.ptr)->push_front(page);
}

td_map td_map_new() {
  return {new std::unordered_map<std::wstring, td_page>()};
}
void td_map_delete(td_map map) {
  delete static_cast<std::unordered_map<std::wstring, td_page>*>(map.ptr);
}
void td_map_insert_page(td_map map, const wchar_t *name, td_page page) {
  static_cast<std::unordered_map<std::wstring, td_page>*>(map.ptr)->emplace(name, page);
}
td_page td_map_find(td_map map, const wchar_t *name) {
  auto actualMap = static_cast<std::unordered_map<std::wstring, td_page> *>(map.ptr);
  auto page = actualMap->find(name);
  if (page == actualMap->end())
    return {nullptr};
  return page->second;
}


} //extern C

