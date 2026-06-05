#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars.hpp"
#include "glaze/stencil/stencil.hpp"

// === frozenchars ===
static auto constexpr kFrozenJson = R"json({"users":[{"name":"Alice Smith","email":"alice@example.com","age":30},{"name":"Bob Johnson","email":"bob@example.com","age":25},{"name":"Carol Williams","email":"carol@example.com","age":35},{"name":"David Brown","email":"david@example.com","age":28},{"name":"Eve Davis","email":"eve@example.com","age":32},{"name":"Frank Miller","email":"frank@example.com","age":40},{"name":"Grace Wilson","email":"grace@example.com","age":27},{"name":"Henry Moore","email":"henry@example.com","age":45},{"name":"Ivy Taylor","email":"ivy@example.com","age":22},{"name":"Jack Anderson","email":"jack@example.com","age":38}]})json"_fs;

static void BM_frozenchars_json(benchmark::State& state) {
  for (auto _ : state) {
    auto sv = kFrozenJson.sv();
    bench::DoNotOptimize(sv);
  }
}
BENCHMARK(BM_frozenchars_json);

// === inja ===
static void BM_inja_json(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"({"users":[{% for user in users %}{"name":"{{ user.name }}","email":"{{ user.email }}","age":{{ user.age }}}{% if not loop.is_last %},{% endif %}{% endfor %}]})";

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
BENCHMARK(BM_inja_json);

// === glz::stencil ===
struct JsonRow {
  std::string name;
  std::string email;
  int age{};
};

struct JsonData {
  std::vector<JsonRow> users;
};

template <>
struct glz::meta<JsonRow> {
  static constexpr auto value = glz::object("name", &JsonRow::name, "email", &JsonRow::email, "age", &JsonRow::age);
};

template <>
struct glz::meta<JsonData> {
  static constexpr auto value = glz::object("users", &JsonData::users);
};

static void BM_glz_stencil_json(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{R"({"users":[{{#users}}{"name":"{{name}}","email":"{{email}}","age":{{age}}}{{#if @last}}{{else}},{{/if}}{{/users}}]})"};

  JsonData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(JsonRow{u.name, u.email, u.age});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_json);
