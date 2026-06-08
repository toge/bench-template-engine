#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include <injamm/injamm.hpp>
#include <injamm/escape_hatch.hpp>
#include "frozenchars/inja_engine.hpp"

// === inja: runtime template rendering ===
static void BM_inja_html(benchmark::State& state) {
  auto env = inja::Environment{};
  static auto const kTemplate = R"(<table>
{% for user in users %}
<tr><td>{{ user.name }}</td><td>{{ user.email }}</td><td>{{ user.age }}</td></tr>
{% endfor %}
</table>)";

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
BENCHMARK(BM_inja_html);

// === glz::stencil: runtime stencil rendering ===
struct HtmlRow {
  std::string name;
  std::string email;
  int age{};
};

struct HtmlTable {
  std::vector<HtmlRow> users;
};

template <>
struct glz::meta<HtmlTable> {
  static constexpr auto value = glz::object("users", &HtmlTable::users);
};

template <>
struct glz::meta<HtmlRow> {
  static constexpr auto value = glz::object("name", &HtmlRow::name, "email", &HtmlRow::email, "age", &HtmlRow::age);
};

// === frozenchars: compile-time HTML generation ===
static auto constexpr kFrozenHtmlTmpl = "<table>{% for user in users %}<tr><td>{{ user.name }}</td><td>{{ user.email }}</td><td>{{ user.age }}</td></tr>{% endfor %}</table>"_fs;

static void BM_frozenchars_html(benchmark::State& state) {
  HtmlTable table{};
  for (auto const& u : make_sample_users()) {
    table.users.push_back(HtmlRow{u.name, u.email, u.age});
  }
  for (auto _ : state) {
    auto result = frozenchars::inja::render<kFrozenHtmlTmpl>(table);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_frozenchars_html);

static void BM_glz_stencil_html(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{"<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>"};

  HtmlTable table{};
  for (auto const& u : make_sample_users()) {
    table.users.push_back(HtmlRow{u.name, u.email, u.age});
  }

  for (auto _ : state) {
    auto result = glz::stencil(kLayout, table);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_glz_stencil_html);

// === injamm: bytecode VM rendering ===
static void BM_injamm_html_bc(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{
      "<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>"};

  HtmlTable table{};
  for (auto const& u : make_sample_users()) {
    table.users.push_back(HtmlRow{u.name, u.email, u.age});
  }
  auto bc = injamm::bc_template<HtmlTable>(kLayout);

  for (auto _ : state) {
    auto result = bc.render(table);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_html_bc);

// === injamm: NTTP compile-time rendering ===
static void BM_injamm_html_nttp(benchmark::State& state) {
  HtmlTable table{};
  for (auto const& u : make_sample_users()) {
    table.users.push_back(HtmlRow{u.name, u.email, u.age});
  }
  for (auto _ : state) {
    auto result = injamm::render<"<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>">(table);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_html_nttp);
