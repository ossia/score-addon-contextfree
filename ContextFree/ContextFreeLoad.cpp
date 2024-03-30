#include <abstractPngCanvas.h>
#include <astexpression.h>
#include <cfdg.h>
#include <commandLineSystem.h>
#include <prettyint.h>

struct options
{
  std::string input;
  std::string definitions;
  int width = 1280;
  int height = 720;
  int minSize = 0;
  int borderSize = 1;
  int animationFrames = 0;
};

struct OssiaCanvas : abstractPngCanvas
{
  using abstractPngCanvas::abstractPngCanvas;
  void output(const char* outfilename, int frame = -1) override { }

  using abstractPngCanvas::mData;
};

struct RenderResult
{
  std::vector<unsigned char> bytes;
  QImage::Format fmt{};
  int width = 0;
  int height = 0;
};

RenderResult contextfree_render_file(std::string_view path, int w, int h, int variation)
{
  options opts;
  opts.input = path;
  opts.width = w;
  opts.height = h;

  CommandLineSystem system{true};
  auto design
      = CFDG::ParseFile(opts.input.c_str(), &system, variation, opts.definitions);
  if(!design)
    return {};

  design->usesAlpha = true;
  design->usesColor = true;

  auto renderer = design->renderer(
      design, opts.width, opts.height, opts.minSize, variation, opts.borderSize);
  if(!renderer)
    return {};

  opts.width = renderer->m_width;
  opts.height = renderer->m_height;

  renderer->run(nullptr, false);

  auto canvas = std::make_unique<OssiaCanvas>(
      "", true, opts.width, opts.height, aggCanvas::PixelFormat::RGBA8_Blend, false,
      opts.animationFrames, variation, false, renderer.get(), 1, 1);

  if(canvas->mWidth != opts.width || canvas->mHeight != opts.height)
  {
    renderer->resetSize(canvas->mWidth, canvas->mHeight);
    opts.width = renderer->m_width;
    opts.height = renderer->m_height;
  }
  renderer->draw(canvas.get());

  return {
      .bytes = std::move(canvas->mData),
      .fmt = QImage::Format::Format_ARGB32,
      .width = opts.width,
      .height = opts.height};
}
