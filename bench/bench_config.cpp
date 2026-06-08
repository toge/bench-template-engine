#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars/inja_engine.hpp"
#include "glaze/stencil/stencil.hpp"

// === inja ===
static void BM_inja_config(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"({% for entry in entries %}{{ entry.key }}={{ entry.value }}
{% endfor %})";

  auto entries_json = nlohmann::json::array();
  for (auto const& [k, v] : make_sample_config().entries) {
    entries_json.push_back({{"key", k}, {"value", v}});
  }
  nlohmann::json data{{"entries", entries_json}};

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_config);

// === glz::stencil ===
struct ConfigEntry {
  std::string key;
  std::string value;
};

struct ConfigStencil {
  std::vector<ConfigEntry> entries;
};

template <>
struct glz::meta<ConfigEntry> {
  static constexpr auto value = glz::object("key", &ConfigEntry::key, "value", &ConfigEntry::value);
};

template <>
struct glz::meta<ConfigStencil> {
  static constexpr auto value = glz::object("entries", &ConfigStencil::entries);
};

struct ConfigEntryFrozen {
  std::string key;
  std::string value;
};

struct ConfigStencilFrozen {
  std::vector<ConfigEntryFrozen> entries;
};

template <>
struct glz::meta<ConfigEntryFrozen> {
  static constexpr auto value = glz::object("key", &ConfigEntryFrozen::key, "value", &ConfigEntryFrozen::value);
};

template <>
struct glz::meta<ConfigStencilFrozen> {
  static constexpr auto value = glz::object("entries", &ConfigStencilFrozen::entries);
};

static auto constexpr kFrozenConfigTmpl = "{% for entry in entries %}{{ entry.key }}={{ entry.value }}\n{% endfor %}"_fs;

static void BM_frozenchars_config(benchmark::State& state) {
  ConfigStencilFrozen data{};
  for (auto const& [k, v] : make_sample_config().entries) {
    data.entries.push_back(ConfigEntryFrozen{k, v});
  }
  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenConfigTmpl>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_config);

static void BM_glz_stencil_config(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{R"({{#entries}}{{key}}={{value}}
{{/entries}})"};

  ConfigStencil data{};
  for (auto const& [k, v] : make_sample_config().entries) {
    data.entries.push_back(ConfigEntry{k, v});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_config);

// === injamm: bytecode VM rendering ===
#include <injamm/escape_hatch.hpp>

static void BM_injamm_config_bc(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"{{#entries}}{{key}}={{value}}\n{{/entries}}"};

  ConfigStencil data{};
  for (auto const& [k, v] : make_sample_config().entries) {
    data.entries.push_back(ConfigEntry{k, v});
  }
  auto bc = injamm::bc_template<ConfigStencil>(kLayout);

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_config_bc);
