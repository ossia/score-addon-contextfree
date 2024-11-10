#pragma once

#include <halp/audio.hpp>
#include <halp/controls.hpp>
#include <halp/meta.hpp>
#include <halp/texture.hpp>

namespace vo
{
class ContextFree
{
public:
  halp_meta(name, "Context-Free")
  halp_meta(category, "Visuals")
  halp_meta(c_name, "contextfree")
  halp_meta(author, "Context-Free Art authors, Jean-Michaël Celerier")
  halp_meta(manual_url, "https://ossia.io/score-docs/processes/context-free-art.html")
  halp_meta(uuid, "2350a51b-9128-4035-bab5-e532a10d9404")

  struct
  {
    struct : halp::lineedit<"Program", "">
    {
      halp_meta(language, "contextfree")
      void update(ContextFree& g)
      {
        if(g.current_program != this->value)
          g.update_all();
      }
    } program;

    struct : halp::spinbox_i32<"Variation", halp::range{-INT_MAX, INT_MAX, 0}>
    {
      void update(ContextFree& g)
      {
        if(g.current_variation != this->value)
          g.update_all();
      }
    } variation;
    halp::spinbox_i32<"Width", halp::range{1, 2000, 640}> width;
    halp::spinbox_i32<"Heigh", halp::range{1, 2000, 360}> height;
  } inputs;

  struct
  {
    halp::texture_output<"Out", halp::rgb_texture> image;
  } outputs;

  void operator()();

  void update_all()
  {
    current_program = inputs.program;
    current_variation = inputs.variation;
    worker.request(inputs.program, inputs.width, inputs.height, inputs.variation);
  }

  struct worker
  {
    std::function<void(std::string, int w, int h, int var)> request;
    static std::function<void(ContextFree&)>
    work(std::string_view s, int w, int h, int var);
  } worker;

  std::string current_program;
  int current_variation{};
};

}
