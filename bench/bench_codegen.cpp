#include <benchmark/benchmark.h>
#include "common.hpp"
#include <injamm.hpp>
#include <glaze/glaze.hpp>
#include <glaze/stencil/stencil.hpp>
#include <string>
#include <vector>

// ---- HTML data types ----
struct HtmlRow { std::string name; std::string email; int age{}; };
struct HtmlTable { std::vector<HtmlRow> users; };
template <> struct glz::meta<HtmlRow> { static constexpr auto value = glz::object("name", &HtmlRow::name, "email", &HtmlRow::email, "age", &HtmlRow::age); };
template <> struct glz::meta<HtmlTable> { static constexpr auto value = glz::object("users", &HtmlTable::users); };

// ---- CSV data types ----
struct CsvRow { std::string name; std::string email; int age{}; };
struct CsvData { std::vector<CsvRow> users; };
template <> struct glz::meta<CsvRow> { static constexpr auto value = glz::object("name", &CsvRow::name, "email", &CsvRow::email, "age", &CsvRow::age); };
template <> struct glz::meta<CsvData> { static constexpr auto value = glz::object("users", &CsvData::users); };

// ---- URL data types ----
struct UrlData { std::string base_url; std::string p1k; std::string p1v; std::string p2k; std::string p2v; std::string p3k; std::string p3v; std::string p4k; std::string p4v; std::string p5k; std::string p5v; };
template <> struct glz::meta<UrlData> {
  static constexpr auto value = glz::object(
    "base_url", &UrlData::base_url,
    "p1k", &UrlData::p1k, "p1v", &UrlData::p1v,
    "p2k", &UrlData::p2k, "p2v", &UrlData::p2v,
    "p3k", &UrlData::p3k, "p3v", &UrlData::p3v,
    "p4k", &UrlData::p4k, "p4v", &UrlData::p4v,
    "p5k", &UrlData::p5k, "p5v", &UrlData::p5v
  );
};

// ---- JSON data types ----
struct JsonRow { std::string name; std::string email; int age{}; };
struct JsonData { std::vector<JsonRow> users; };
template <> struct glz::meta<JsonRow> { static constexpr auto value = glz::object("name", &JsonRow::name, "email", &JsonRow::email, "age", &JsonRow::age); };
template <> struct glz::meta<JsonData> { static constexpr auto value = glz::object("users", &JsonData::users); };

// ---- Config data types ----
struct ConfigEntry { std::string key; std::string value; };
struct ConfigStencil { std::vector<ConfigEntry> entries; };
template <> struct glz::meta<ConfigEntry> { static constexpr auto value = glz::object("key", &ConfigEntry::key, "value", &ConfigEntry::value); };
template <> struct glz::meta<ConfigStencil> { static constexpr auto value = glz::object("entries", &ConfigStencil::entries); };

// ---- Markdown data types ----
struct MdData { std::string title; std::string description; std::string link1_text; std::string link1_url; std::string link2_text; std::string link2_url; std::string link3_text; std::string link3_url; };
template <> struct glz::meta<MdData> {
  static constexpr auto value = glz::object(
    "title", &MdData::title, "description", &MdData::description,
    "link1_text", &MdData::link1_text, "link1_url", &MdData::link1_url,
    "link2_text", &MdData::link2_text, "link2_url", &MdData::link2_url,
    "link3_text", &MdData::link3_text, "link3_url", &MdData::link3_url
  );
};

// ---- AT vars data types ----
struct BUser { std::string name; int age{}; };
struct BUsersData { std::vector<BUser> users; };
template <> struct glz::meta<BUser> { static constexpr auto value = glz::object("name", &BUser::name, "age", &BUser::age); };
template <> struct glz::meta<BUsersData> { static constexpr auto value = glz::object("users", &BUsersData::users); };

// ---- Paths data types ----
struct BAddress { std::string city; std::string country; };
struct BFounder { std::string name; BAddress address; };
struct BCompany { std::string name; BFounder founder; };
template <> struct glz::meta<BAddress> { static constexpr auto value = glz::object("city", &BAddress::city, "country", &BAddress::country); };
template <> struct glz::meta<BFounder> { static constexpr auto value = glz::object("name", &BFounder::name, "address", &BFounder::address); };
template <> struct glz::meta<BCompany> { static constexpr auto value = glz::object("name", &BCompany::name, "founder", &BCompany::founder); };

// ---- codegen render headers ----
#include "codegen/render_html.hpp"
#include "codegen/render_csv.hpp"
#include "codegen/render_url.hpp"
#include "codegen/render_json.hpp"
#include "codegen/render_config.hpp"
#include "codegen/render_markdown.hpp"
#include "codegen/render_at_index.hpp"
#include "codegen/render_at_last.hpp"
#include "codegen/render_at_if_else.hpp"
#include "codegen/render_large.hpp"
#include "codegen/render_path_2level.hpp"
#include "codegen/render_path_3level.hpp"

// ---- helpers ----
static auto make_html_table() -> HtmlTable {
  HtmlTable t;
  for (auto const& u : make_sample_users()) t.users.push_back(HtmlRow{u.name, u.email, u.age});
  return t;
}
static auto make_csv_data() -> CsvData {
  CsvData d;
  for (auto const& u : make_sample_users()) d.users.push_back(CsvRow{u.name, u.email, u.age});
  return d;
}
static auto make_json_data() -> JsonData {
  JsonData d;
  for (auto const& u : make_sample_users()) d.users.push_back(JsonRow{u.name, u.email, u.age});
  return d;
}
static auto make_users_data() -> BUsersData {
  BUsersData d;
  for (auto const& u : make_sample_users()) d.users.push_back(BUser{u.name, u.age});
  return d;
}
static auto make_large_data() -> BUsersData {
  BUsersData d;
  for (int i = 0; i < 1000; ++i) d.users.push_back(BUser{"user" + std::to_string(i), 20 + (i % 50)});
  return d;
}
static auto make_company() -> BCompany {
  return {"Acme", BFounder{"Alice", BAddress{"Tokyo", "JP"}}};
}

// ============================================================
//  HTML benchmarks
// ============================================================
static void BM_injamm_html_bc(benchmark::State& state) {
  auto table = make_html_table();
  auto bc = injamm::engine<HtmlTable>("<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>");
  for (auto _ : state) { auto r = bc.render(table); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_html_bc);

static void BM_injamm_html_nttp(benchmark::State& state) {
  auto table = make_html_table();
  for (auto _ : state) { auto r = injamm::render<"<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>">(table); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_html_nttp);

static void BM_injamm_html_codegen(benchmark::State& state) {
  auto table = make_html_table();
  for (auto _ : state) { auto r = generated::render_html(table); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_html_codegen);

// ============================================================
//  CSV benchmarks
// ============================================================
static void BM_injamm_csv_bc(benchmark::State& state) {
  auto d = make_csv_data();
  auto bc = injamm::engine<CsvData>("name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_csv_bc);

static void BM_injamm_csv_nttp(benchmark::State& state) {
  auto d = make_csv_data();
  for (auto _ : state) { auto r = injamm::render<"name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_csv_nttp);

static void BM_injamm_csv_codegen(benchmark::State& state) {
  auto d = make_csv_data();
  for (auto _ : state) { auto r = generated::render_csv(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_csv_codegen);

// ============================================================
//  URL benchmarks
// ============================================================
static UrlData make_url_data() {
  return UrlData{
    "https://example.com/api/search",
    "q", "hello world", "page", "1", "limit", "10", "sort", "name", "order", "asc"
  };
}

static void BM_injamm_url_bc(benchmark::State& state) {
  auto d = make_url_data();
  auto bc = injamm::engine<UrlData>("{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_url_bc);

static void BM_injamm_url_nttp(benchmark::State& state) {
  auto d = make_url_data();
  for (auto _ : state) { auto r = injamm::render<"{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_url_nttp);

static void BM_injamm_url_codegen(benchmark::State& state) {
  auto d = make_url_data();
  for (auto _ : state) { auto r = generated::render_url(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_url_codegen);

// ============================================================
//  JSON benchmarks
// ============================================================
static void BM_injamm_json_bc(benchmark::State& state) {
  auto d = make_json_data();
  auto bc = injamm::engine<JsonData>(R"({"users":[{{#users}}{"name":"{{name}}","email":"{{email}}","age":{{age}}}{{#if loop.is_last}}{{else}},{{/if}}{{/users}}]})");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_json_bc);

static void BM_injamm_json_nttp(benchmark::State& state) {
  auto d = make_json_data();
  for (auto _ : state) { auto r = injamm::render<R"({"users":[{{#users}}{"name":"{{name}}","email":"{{email}}","age":{{age}}}{{#if loop.is_last}}{{else}},{{/if}}{{/users}}]})">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_json_nttp);

static void BM_injamm_json_codegen(benchmark::State& state) {
  auto d = make_json_data();
  for (auto _ : state) { auto r = generated::render_json(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_json_codegen);

// ============================================================
//  Config benchmarks
// ============================================================
static ConfigStencil make_config_data() {
  ConfigStencil d;
  for (auto const& [k, v] : make_sample_config().entries) d.entries.push_back(ConfigEntry{k, v});
  return d;
}

static void BM_injamm_config_bc(benchmark::State& state) {
  auto d = make_config_data();
  auto bc = injamm::engine<ConfigStencil>("{{#entries}}{{key}}={{value}}\n{{/entries}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_config_bc);

static void BM_injamm_config_nttp(benchmark::State& state) {
  auto d = make_config_data();
  for (auto _ : state) { auto r = injamm::render<"{{#entries}}{{key}}={{value}}\n{{/entries}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_config_nttp);

static void BM_injamm_config_codegen(benchmark::State& state) {
  auto d = make_config_data();
  for (auto _ : state) { auto r = generated::render_config(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_config_codegen);

// ============================================================
//  Markdown benchmarks
// ============================================================
static MdData make_md_data() {
  return MdData{
    "Sample Document",
    "This is a sample document for benchmarking.",
    "Home", "https://example.com/",
    "About", "https://example.com/about",
    "Contact", "https://example.com/contact"
  };
}

static void BM_injamm_markdown_bc(benchmark::State& state) {
  auto d = make_md_data();
  auto bc = injamm::engine<MdData>("# {{title}}\n\n{{description}}\n\n- [{{link1_text}}]({{link1_url}})\n- [{{link2_text}}]({{link2_url}})\n- [{{link3_text}}]({{link3_url}})");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_markdown_bc);

static void BM_injamm_markdown_nttp(benchmark::State& state) {
  auto d = make_md_data();
  for (auto _ : state) { auto r = injamm::render<"# {{title}}\n\n{{description}}\n\n- [{{link1_text}}]({{link1_url}})\n- [{{link2_text}}]({{link2_url}})\n- [{{link3_text}}]({{link3_url}})">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_markdown_nttp);

static void BM_injamm_markdown_codegen(benchmark::State& state) {
  auto d = make_md_data();
  for (auto _ : state) { auto r = generated::render_markdown(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_injamm_markdown_codegen);

// ============================================================
//  @index benchmark
// ============================================================
static void BM_codegen_at_index_bc(benchmark::State& state) {
  auto d = make_users_data();
  auto bc = injamm::engine<BUsersData>("{{#users}}{{loop.index}}:{{name}};{{/users}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_index_bc);

static void BM_codegen_at_index_nttp(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = injamm::render<"{{#users}}{{loop.index}}:{{name}};{{/users}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_index_nttp);

static void BM_codegen_at_index_codegen(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = generated::render_at_index(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_index_codegen);

// ============================================================
//  @last benchmark
// ============================================================
static void BM_codegen_at_last_bc(benchmark::State& state) {
  auto d = make_users_data();
  auto bc = injamm::engine<BUsersData>("{{#users}}{{name}}{{#loop.is_last}}.{{/loop.is_last}}{{^loop.is_last}},{{/loop.is_last}}{{/users}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_last_bc);

static void BM_codegen_at_last_nttp(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = injamm::render<"{{#users}}{{name}}{{#loop.is_last}}.{{/loop.is_last}}{{^loop.is_last}},{{/loop.is_last}}{{/users}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_last_nttp);

static void BM_codegen_at_last_codegen(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = generated::render_at_last(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_at_last_codegen);

// ============================================================
//  if/else benchmark
// ============================================================
static void BM_codegen_if_else_bc(benchmark::State& state) {
  auto d = make_users_data();
  auto bc = injamm::engine<BUsersData>("{{#users}}[{{loop.index}}]{{name}}{{#if loop.is_last}}.{{else}},{{/if}}{{/users}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_if_else_bc);

static void BM_codegen_if_else_nttp(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = injamm::render<"{{#users}}[{{loop.index}}]{{name}}{{#if loop.is_last}}.{{else}},{{/if}}{{/users}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_if_else_nttp);

static void BM_codegen_if_else_codegen(benchmark::State& state) {
  auto d = make_users_data();
  for (auto _ : state) { auto r = generated::render_at_if_else(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_if_else_codegen);

// ============================================================
//  Large data benchmark
// ============================================================
static void BM_codegen_large_bc(benchmark::State& state) {
  auto d = make_large_data();
  auto bc = injamm::engine<BUsersData>("{{#users}}{{loop.index}}:{{name}}({{age}});{{/users}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_large_bc);

static void BM_codegen_large_nttp(benchmark::State& state) {
  auto d = make_large_data();
  for (auto _ : state) { auto r = injamm::render<"{{#users}}{{loop.index}}:{{name}}({{age}});{{/users}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_large_nttp);

static void BM_codegen_large_codegen(benchmark::State& state) {
  auto d = make_large_data();
  for (auto _ : state) { auto r = generated::render_large(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_large_codegen);

// ============================================================
//  Path 2-level benchmark
// ============================================================
static void BM_codegen_path_2level_bc(benchmark::State& state) {
  auto d = make_company();
  auto bc = injamm::engine<BCompany>("{{name}} by {{founder.name}} in {{founder.address.city}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_2level_bc);

static void BM_codegen_path_2level_nttp(benchmark::State& state) {
  auto d = make_company();
  for (auto _ : state) { auto r = injamm::render<"{{name}} by {{founder.name}} in {{founder.address.city}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_2level_nttp);

static void BM_codegen_path_2level_codegen(benchmark::State& state) {
  auto d = make_company();
  for (auto _ : state) { auto r = generated::render_path_2level(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_2level_codegen);

// ============================================================
//  Path 3-level benchmark
// ============================================================
static void BM_codegen_path_3level_bc(benchmark::State& state) {
  auto d = make_company();
  auto bc = injamm::engine<BCompany>("{{founder.address.country}}");
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_3level_bc);

static void BM_codegen_path_3level_nttp(benchmark::State& state) {
  auto d = make_company();
  for (auto _ : state) { auto r = injamm::render<"{{founder.address.country}}">(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_3level_nttp);

static void BM_codegen_path_3level_codegen(benchmark::State& state) {
  auto d = make_company();
  for (auto _ : state) { auto r = generated::render_path_3level(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_path_3level_codegen);

// ============================================================
//  Long template benchmark (codegen only)
// ============================================================
static void BM_codegen_long_template_bc(benchmark::State& state) {
  auto d = make_users_data();
  std::string long_tmpl = "{{#users}}";
  for (int i = 0; i < 25; ++i) long_tmpl += "{{name}}-{{age}}|";
  long_tmpl += "{{/users}}";
  auto bc = injamm::engine<BUsersData>(long_tmpl);
  for (auto _ : state) { auto r = bc.render(d); bench::DoNotOptimize(r); }
}
BENCHMARK(BM_codegen_long_template_bc);
