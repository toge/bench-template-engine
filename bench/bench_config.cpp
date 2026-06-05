#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars.hpp"
#include "glaze/stencil/stencil.hpp"

// === frozenchars ===
static auto constexpr kFrozenConfig = R"ini(server.host=localhost
server.port=8080
database.driver=sqlite3
database.path=/data/app.db
cache.enabled=true
cache.ttl=3600
log.level=info
log.file=/var/log/app.log
auth.secret=changeme
auth.token_ttl=86400
)ini"_fs;

static void BM_frozenchars_config(benchmark::State& state) {
  for (auto _ : state) {
    auto sv = kFrozenConfig.sv();
    bench::DoNotOptimize(sv);
  }
}
BENCHMARK(BM_frozenchars_config);

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
