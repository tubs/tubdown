#pragma once
#include <array>
#include <bitset>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>
#include <iterator>
#include <list>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//C-API definition
#include "tubdown.h"

struct Page;
struct TubdownConfig;

struct StyleSpan {
  int line;
  int begin, end;
  struct {
    bool bold: 1;
    bool underlined: 1;
    bool italic: 1;
    bool header: 1;
    bool quote: 1;
    unsigned color: 4;
  };
};

struct PageLink {
  std::wstring href;
  int anchor;
  int line, begin, end;
};

struct Page {
  std::string title;
  std::vector<std::wstring> lines;
  std::vector<StyleSpan> styles;
  std::vector<PageLink> links;
  int currentLink = -1;
  td_renderer *renderer = nullptr;

  void Render(void *userData);
  Page(TubdownConfig& config, const std::string& fileName);
  Page() {};
};

typedef std::function<void(Page&, const std::vector<std::wstring>&)> CommandCallback;

struct TubdownConfig {
  std::unordered_map<std::wstring, CommandCallback> commands;
  td_renderer *renderer;

  TubdownConfig(td_renderer *renderer) : renderer(renderer) {}
};

