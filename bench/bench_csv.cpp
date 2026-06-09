#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <sstream>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars/inja_engine.hpp"

// === inja ===
static void BM_inja_csv(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"(name,email,age
{% for user in users %}{{ user.name }},{{ user.email }},{{ user.age }}
{% endfor %})";

  auto users_json = nlohmann::json::array();
  for (auto const& u : make_sample_users()) {
    users_json.push_back({{"name", u.name}, {"email", u.email}, {"age", u.age}});
  }
  nlohmann::json data{{"users", users_json}};

  for (auto _ : state) {
    auto result = env.render(kTemplate, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_inja_csv);

// === glz::stencil ===
struct CsvRow {
  std::string name;
  std::string email;
  int age{};
};

struct CsvData {
  std::vector<CsvRow> users;
};

template <>
struct glz::meta<CsvRow> {
  static constexpr auto value = glz::object("name", &CsvRow::name, "email", &CsvRow::email, "age", &CsvRow::age);
};

template <>
struct glz::meta<CsvData> {
  static constexpr auto value = glz::object("users", &CsvData::users);
};

// === frozenchars: compile-time CSV generation ===
static auto constexpr kFrozenCsvTmpl = "name,email,age\n{% for user in users %}{{ user.name }},{{ user.email }},{{ user.age }}\n{% endfor %}"_fs;

static void BM_frozenchars_csv(benchmark::State& state) {
  CsvData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(CsvRow{u.name, u.email, u.age});
  }
  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenCsvTmpl>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_csv);

static void BM_glz_stencil_csv(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}"};

  CsvData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(CsvRow{u.name, u.email, u.age});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_csv);

// === injamm: bytecode VM rendering ===
#include <injamm/escape_hatch.hpp>

static void BM_injamm_csv_bc(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{
      "name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}"};

  CsvData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(CsvRow{u.name, u.email, u.age});
  }
  auto bc = injamm::engine<CsvData>(kLayout);

  for (auto _ : state) {
    auto result = bc.render(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_csv_bc);

// === injamm: NTTP compile-time rendering ===
static void BM_injamm_csv_nttp(benchmark::State& state) {
  CsvData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(CsvRow{u.name, u.email, u.age});
  }
  for (auto _ : state) {
    auto result = injamm::render<"name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}">(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_csv_nttp);
