#include <benchmark/benchmark.h>
#include "common.hpp"
#include <injamm.hpp>
#include "frozenchars/inja_engine.hpp"
#include <glaze/glaze.hpp>
#include <glaze/stencil/stencil.hpp>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <vector>

// ---- ベンチマーク用データ型 ----

struct BUser {
  std::string name;
  int age{};
};

struct BUsersData {
  std::vector<BUser> users;
};

template <>
struct glz::meta<BUser> {
  static constexpr auto value = glz::object("name", &BUser::name, "age", &BUser::age);
};

template <>
struct glz::meta<BUsersData> {
  static constexpr auto value = glz::object("users", &BUsersData::users);
};

// @index/@first/@last マイクロベンチマーク

/// @brief runtime パース込みの @vars ベンチマーク（比較用）
static void BM_injamm_at_index_runtime(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto bc = injamm::engine<BUsersData>("{{#users}}{{@index}}:{{name}};{{/users}}");
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_index_runtime);

// === NTTP (compile-time parse) variants ===

/// @brief NTTP: コンパイル時パース済み @index ベンチマーク
static void BM_injamm_at_index_nttp(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = injamm::render<"{{#users}}{{@index}}:{{name}};{{/users}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_index_nttp);

/// @brief NTTP: コンパイル時パース済み @last セクション ベンチマーク
static void BM_injamm_at_last_section_nttp(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = injamm::render<"{{#users}}{{name}}{{#@last}}.{{/@last}}{{^@last}},{{/@last}}{{/users}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_last_section_nttp);

/// @brief NTTP: コンパイル時パース済み if/else ベンチマーク
static void BM_injamm_at_vars_if_else_nttp(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = injamm::render<"{{#users}}[{{@index}}]{{name}}{{#if @last}}.{{else}},{{/if}}{{/users}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_vars_if_else_nttp);

// === Bytecode VM variants ===

/// @brief バイトコードVM: @index ベンチマーク
static void BM_injamm_at_index_bc(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }
  auto bc = injamm::engine<BUsersData>("{{#users}}{{@index}}:{{name}};{{/users}}");

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_index_bc);

/// @brief バイトコードVM: @last セクション ベンチマーク
static void BM_injamm_at_last_section_bc(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }
  auto bc = injamm::engine<BUsersData>("{{#users}}{{name}}{{#@last}}.{{/@last}}{{^@last}},{{/@last}}{{/users}}");

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_last_section_bc);

/// @brief バイトコードVM: if/else ベンチマーク
static void BM_injamm_at_vars_if_else_bc(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }
  auto bc = injamm::engine<BUsersData>("{{#users}}[{{@index}}]{{name}}{{#if @last}}.{{else}},{{/if}}{{/users}}");

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_at_vars_if_else_bc);

// === 大規模ベンチマーク ===

/// @brief 大規模データ: バイトコードVM
static void BM_injamm_large_data_bc(benchmark::State& state) {
  BUsersData data;
  for (int i = 0; i < 1000; ++i) {
    data.users.push_back(BUser{"user" + std::to_string(i), 20 + (i % 50)});
  }
  auto bc = injamm::engine<BUsersData>("{{#users}}{{@index}}:{{name}}({{age}});{{/users}}");

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_large_data_bc);

/// @brief 大規模データ: NTTP
static void BM_injamm_large_data_nttp(benchmark::State& state) {
  BUsersData data;
  for (int i = 0; i < 1000; ++i) {
    data.users.push_back(BUser{"user" + std::to_string(i), 20 + (i % 50)});
  }

  for (auto _ : state) {
    auto result = injamm::render<"{{#users}}{{@index}}:{{name}}({{age}});{{/users}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_large_data_nttp);

/// @brief コンパイルコスト込み: バイトコードVM（1000回繰り返し）
static void BM_injamm_compile_cost_bc(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    // コンパイル + 実行を1000回繰り返す
    for (int i = 0; i < 1000; ++i) {
      auto bc = injamm::engine<BUsersData>("{{#users}}{{@index}}:{{name}};{{/users}}");
      auto result = bc.render(data);
      bench::DoNotOptimize(result);
    }
  }
}
BENCHMARK(BM_injamm_compile_cost_bc);

/// @brief 長いテンプレート: バイトコードVM
static void BM_injamm_long_template_bc(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }
  // 長いテンプレートを生成（50個のプレースホルダー）
  std::string long_tmpl = "{{#users}}";
  for (int i = 0; i < 25; ++i) {
    long_tmpl += "{{name}}-{{age}}|";
  }
  long_tmpl += "{{/users}}";
  auto bc = injamm::engine<BUsersData>(long_tmpl);

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_long_template_bc);

// === inja (pantor/inja) benchmarks ===

/// @brief inja: @index 相当（loop.index）
static void BM_inja_at_index(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = "{% for user in users %}{{ loop.index }}:{{ user.name }};{% endfor %}";

  auto users_json = nlohmann::json::array();
  for (auto const& u : make_sample_users()) {
    users_json.push_back({{"name", u.name}, {"age", u.age}});
  }
  nlohmann::json data{{"users", users_json}};

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_at_index);

/// @brief inja: @last 相当（loop.is_last）
static void BM_inja_at_last(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = "{% for user in users %}{{ user.name }}{% if loop.is_last %}.{% else %},{% endif %}{% endfor %}";

  auto users_json = nlohmann::json::array();
  for (auto const& u : make_sample_users()) {
    users_json.push_back({{"name", u.name}, {"age", u.age}});
  }
  nlohmann::json data{{"users", users_json}};

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_at_last);

/// @brief inja: if/else with loop.last
static void BM_inja_if_else(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = "{% for user in users %}[{{ loop.index }}]{{ user.name }}{% if loop.is_last %}.{% else %},{% endif %}{% endfor %}";

  auto users_json = nlohmann::json::array();
  for (auto const& u : make_sample_users()) {
    users_json.push_back({{"name", u.name}, {"age", u.age}});
  }
  nlohmann::json data{{"users", users_json}};

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_if_else);

// === glz::stencil benchmarks ===

/// @brief glz::stencil: @index 相当（loop.index）
static void BM_glz_stencil_at_index(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"{{#users}}{{loop.index}}:{{name}};{{/users}}"};

  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_at_index);

/// @brief glz::stencil: @last 相当（loop.is_last）
static void BM_glz_stencil_at_last(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"{{#users}}{{name}}{{#loop.is_last}}.{{/loop.is_last}}{{^loop.is_last}},{{/loop.is_last}}{{/users}}"};

  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_at_last);

// === frozenchars benchmarks ===

/// @brief frozenchars: @index 相当
static auto constexpr kFrozenAtIdx = "{% for user in users %}{{loop.index}}:{{user.name}};{% endfor %}"_fs;

static void BM_frozenchars_at_index(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenAtIdx>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_at_index);

/// @brief frozenchars: @last 相当
static auto constexpr kFrozenAtLast = "{% for user in users %}{{user.name}}{% if loop.is_last %}.{% else %},{% endif %}{% endfor %}"_fs;

static void BM_frozenchars_at_last(benchmark::State& state) {
  BUsersData data;
  for (auto const& u : make_sample_users()) {
    data.users.push_back(BUser{u.name, u.age});
  }

  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenAtLast>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_at_last);
