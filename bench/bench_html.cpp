#include <benchmark/benchmark.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "common.hpp"
#include "frozenchars.hpp"
#include "glaze/stencil/stencil.hpp"

// === frozenchars: compile-time HTML generation ===
static auto constexpr kFrozenHtml = R"html(
<table>
<tr><td>Alice Smith</td><td>alice@example.com</td><td>30</td></tr>
<tr><td>Bob Johnson</td><td>bob@example.com</td><td>25</td></tr>
<tr><td>Carol Williams</td><td>carol@example.com</td><td>35</td></tr>
<tr><td>David Brown</td><td>david@example.com</td><td>28</td></tr>
<tr><td>Eve Davis</td><td>eve@example.com</td><td>32</td></tr>
<tr><td>Frank Miller</td><td>frank@example.com</td><td>40</td></tr>
<tr><td>Grace Wilson</td><td>grace@example.com</td><td>27</td></tr>
<tr><td>Henry Moore</td><td>henry@example.com</td><td>45</td></tr>
<tr><td>Ivy Taylor</td><td>ivy@example.com</td><td>22</td></tr>
<tr><td>Jack Anderson</td><td>jack@example.com</td><td>38</td></tr>
</table>
)html"_fs;

static void BM_frozenchars_html(benchmark::State& state) {
  for (auto _ : state) {
    auto sv = kFrozenHtml.sv();
    bench::DoNotOptimize(sv);
  }
}
BENCHMARK(BM_frozenchars_html);

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

static void BM_glz_stencil_html(benchmark::State& state) {
  static auto constexpr kLayout = std::string_view{R"(<table>
{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>
{{/users}}</table>)"};

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
