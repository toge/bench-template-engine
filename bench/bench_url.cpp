#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"

// === inja ===
static void BM_inja_url(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"({{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}})";

  nlohmann::json data{
    {"base_url", "https://example.com/api/search"},
    {"p1k", "q"}, {"p1v", "hello world"},
    {"p2k", "page"}, {"p2v", "1"},
    {"p3k", "limit"}, {"p3v", "10"},
    {"p4k", "sort"}, {"p4v", "name"},
    {"p5k", "order"}, {"p5v", "asc"}
  };

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_url);

// === glz::stencil ===
struct UrlData {
  std::string base_url;
  std::string p1k; std::string p1v;
  std::string p2k; std::string p2v;
  std::string p3k; std::string p3v;
  std::string p4k; std::string p4v;
  std::string p5k; std::string p5v;
};

template <>
struct glz::meta<UrlData> {
  static constexpr auto value = glz::object(
    "base_url", &UrlData::base_url,
    "p1k", &UrlData::p1k, "p1v", &UrlData::p1v,
    "p2k", &UrlData::p2k, "p2v", &UrlData::p2v,
    "p3k", &UrlData::p3k, "p3v", &UrlData::p3v,
    "p4k", &UrlData::p4k, "p4v", &UrlData::p4v,
    "p5k", &UrlData::p5k, "p5v", &UrlData::p5v
  );
};

static void BM_glz_stencil_url(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}"};

  UrlData data{
    .base_url = "https://example.com/api/search",
    .p1k = "q", .p1v = "hello world",
    .p2k = "page", .p2v = "1",
    .p3k = "limit", .p3v = "10",
    .p4k = "sort", .p4v = "name",
    .p5k = "order", .p5v = "asc"
  };

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_url);

// === injamm: bytecode VM rendering ===
#include <injamm/escape_hatch.hpp>

static void BM_injamm_url_bc(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{
      "{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}"};

  UrlData data{
      .base_url = "https://example.com/api/search",
      .p1k = "q", .p1v = "hello world",
      .p2k = "page", .p2v = "1",
      .p3k = "limit", .p3v = "10",
      .p4k = "sort", .p4v = "name",
      .p5k = "order", .p5v = "asc"};
  auto bc = injamm::engine<UrlData>(kLayout);

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_url_bc);

// === injamm: NTTP compile-time rendering ===
static void BM_injamm_url_nttp(benchmark::State& state) {
  UrlData data{
      .base_url = "https://example.com/api/search",
      .p1k = "q", .p1v = "hello world",
      .p2k = "page", .p2v = "1",
      .p3k = "limit", .p3v = "10",
      .p4k = "sort", .p4v = "name",
      .p5k = "order", .p5v = "asc"};
  for (auto _ : state) {
    auto result = injamm::render<"{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_url_nttp);
