#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars/inja_engine.hpp"

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

// === frozenchars: compile-time JSON generation ===
static auto constexpr kFrozenJsonTmpl = R"({"users":[{% for user in users %}{"name":"{{ user.name }}","email":"{{ user.email }}","age":{{ user.age }}}{% if not loop.is_last %},{% endif %}{% endfor %}]})"_fs;

static void BM_frozenchars_json(benchmark::State& state) {
  JsonData data{};
  for (auto const& u : make_sample_users()) {
    data.users.push_back(JsonRow{u.name, u.email, u.age});
  }
  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenJsonTmpl>(data);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_json);

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
