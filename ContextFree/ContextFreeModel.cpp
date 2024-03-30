#include "ContextFreeModel.hpp"

#include <QTemporaryFile>

#include <ContextFree/ContextFreeLoad.hpp>

namespace vo
{
void ContextFree::operator()() { }

std::function<void(ContextFree&)>
ContextFree::worker::work(std::string_view in, int w, int h, int var)
{
  if(in.empty())
    return {};

  QTemporaryFile temp;
  if(!temp.open())
    return {};
  temp.write(in.data(), in.size());
  temp.close();

  auto path = temp.fileName().toStdString();

  return [rendered = contextfree_render_file(path, w, h, var)](ContextFree& f) mutable {
    f.outputs.image.create(rendered.width, rendered.height);
    halp::rgb_texture& tex = f.outputs.image.texture;
    tex.update(rendered.bytes.data(), rendered.width, rendered.height);
    f.outputs.image.upload();
  };
}
}
