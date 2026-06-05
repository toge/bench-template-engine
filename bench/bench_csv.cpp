#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <sstream>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars.hpp"
#include "glaze/stencil/stencil.hpp"

// === frozenchars ===
static auto constexpr kFrozenCsv = R"csv(name,email,age
Alice Smith,alice@example.com,30
Bob Johnson,bob@example.com,25
Carol Williams,carol@example.com,35
David Brown,david@example.com,28
Eve Davis,eve@example.com,32
Frank Miller,frank@example.com,40
Grace Wilson,grace@example.com,27
Henry Moore,henry@example.com,45
Ivy Taylor,ivy@example.com,22
Jack Anderson,jack@example.com,38
)csv"_fs;

static void BM_frozenchars_csv(benchmark::State& state) {
  for (auto _ : state) {
    auto sv = kFrozenCsv.sv();
    bench::DoNotOptimize(sv);
  }
}
BENCHMARK(BM_frozenchars_csv);

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

static void BM_glz_stencil_csv(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{R"(name,email,age
{{#users}}{{name}},{{email}},{{age}}
{{/users}})"};

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
