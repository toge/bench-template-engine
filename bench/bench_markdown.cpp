#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars/inja_engine.hpp"
#include "glaze/stencil/stencil.hpp"

// === inja ===
static void BM_inja_markdown(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"(# {{ title }}

{{ description }}

- [{{ link1_text }}]({{ link1_url }})
- [{{ link2_text }}]({{ link2_url }})
- [{{ link3_text }}]({{ link3_url }}))";

  nlohmann::json data{
    {"title", "Sample Document"},
    {"description", "This is a sample document for benchmarking."},
    {"link1_text", "Home"}, {"link1_url", "https://example.com/"},
    {"link2_text", "About"}, {"link2_url", "https://example.com/about"},
    {"link3_text", "Contact"}, {"link3_url", "https://example.com/contact"}
  };

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_markdown);

// === glz::stencil ===
struct MdData {
  std::string title;
  std::string description;
  std::string link1_text;
  std::string link1_url;
  std::string link2_text;
  std::string link2_url;
  std::string link3_text;
  std::string link3_url;
};

template <>
struct glz::meta<MdData> {
  static constexpr auto value = glz::object(
    "title", &MdData::title,
    "description", &MdData::description,
    "link1_text", &MdData::link1_text, "link1_url", &MdData::link1_url,
    "link2_text", &MdData::link2_text, "link2_url", &MdData::link2_url,
    "link3_text", &MdData::link3_text, "link3_url", &MdData::link3_url
  );
};

struct MdDataFrozen {
  std::string title;
  std::string description;
  std::string link1_text;
  std::string link1_url;
  std::string link2_text;
  std::string link2_url;
  std::string link3_text;
  std::string link3_url;
};

template <>
struct glz::meta<MdDataFrozen> {
  static constexpr auto value = glz::object(
    "title", &MdDataFrozen::title,
    "description", &MdDataFrozen::description,
    "link1_text", &MdDataFrozen::link1_text, "link1_url", &MdDataFrozen::link1_url,
    "link2_text", &MdDataFrozen::link2_text, "link2_url", &MdDataFrozen::link2_url,
    "link3_text", &MdDataFrozen::link3_text, "link3_url", &MdDataFrozen::link3_url
  );
};

static auto constexpr kFrozenMdTmpl = "# {{ title }}\n\n{{ description }}\n\n- [{{ link1_text }}]({{ link1_url }})\n- [{{ link2_text }}]({{ link2_url }})\n- [{{ link3_text }}]({{ link3_url }})"_fs;

static void BM_frozenchars_markdown(benchmark::State& state) {
  MdDataFrozen data{
    .title = "Sample Document",
    .description = "This is a sample document for benchmarking.",
    .link1_text = "Home", .link1_url = "https://example.com/",
    .link2_text = "About", .link2_url = "https://example.com/about",
    .link3_text = "Contact", .link3_url = "https://example.com/contact"
  };
  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenMdTmpl>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_markdown);

static void BM_glz_stencil_markdown(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"# {{ title }}\n\n{{ description }}\n\n- [{{ link1_text }}]({{ link1_url }})\n- [{{ link2_text }}]({{ link2_url }})\n- [{{ link3_text }}]({{ link3_url }})"};

  MdData data{
    .title = "Sample Document",
    .description = "This is a sample document for benchmarking.",
    .link1_text = "Home", .link1_url = "https://example.com/",
    .link2_text = "About", .link2_url = "https://example.com/about",
    .link3_text = "Contact", .link3_url = "https://example.com/contact"
  };

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_markdown);
