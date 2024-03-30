#pragma once
#include <QImage>

#include <string_view>
#include <vector>

struct RenderResult
{
  std::vector<unsigned char> bytes;
  QImage::Format fmt{};
  int width = 0;
  int height = 0;
};

RenderResult contextfree_render_file(std::string_view path, int w, int h, int variation);
