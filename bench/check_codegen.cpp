#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "common.hpp"
#include <injamm.hpp>
#include <glaze/glaze.hpp>
#include "codegen/render_html.hpp"
#include "codegen/render_csv.hpp"
#include "codegen/render_url.hpp"
#include "codegen/render_json.hpp"
#include "codegen/render_config.hpp"
#include "codegen/render_markdown.hpp"
#include "codegen/render_path_2level.hpp"
#include "codegen/render_path_3level.hpp"
#include "codegen/render_at_index.hpp"
#include "codegen/render_at_last.hpp"
#include "codegen/render_at_if_else.hpp"
#include "codegen/render_large.hpp"

// ---- Match types from bench_codegen.cpp exactly ----
struct HtmlRow { std::string name; std::string email; int age{}; };
struct HtmlTable { std::vector<HtmlRow> users; };
template <> struct glz::meta<HtmlRow> { static constexpr auto value = glz::object("name", &HtmlRow::name, "email", &HtmlRow::email, "age", &HtmlRow::age); };
template <> struct glz::meta<HtmlTable> { static constexpr auto value = glz::object("users", &HtmlTable::users); };

struct CsvRow { std::string name; std::string email; int age{}; };
struct CsvData { std::vector<CsvRow> users; };
template <> struct glz::meta<CsvRow> { static constexpr auto value = glz::object("name", &CsvRow::name, "email", &CsvRow::email, "age", &CsvRow::age); };
template <> struct glz::meta<CsvData> { static constexpr auto value = glz::object("users", &CsvData::users); };

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

struct JsonRow { std::string name; std::string email; int age{}; };
struct JsonData { std::vector<JsonRow> users; };
template <> struct glz::meta<JsonRow> { static constexpr auto value = glz::object("name", &JsonRow::name, "email", &JsonRow::email, "age", &JsonRow::age); };
template <> struct glz::meta<JsonData> { static constexpr auto value = glz::object("users", &JsonData::users); };

struct ConfigEntry { std::string key; std::string value; };
struct ConfigStencil { std::vector<ConfigEntry> entries; };
template <> struct glz::meta<ConfigEntry> { static constexpr auto value = glz::object("key", &ConfigEntry::key, "value", &ConfigEntry::value); };
template <> struct glz::meta<ConfigStencil> { static constexpr auto value = glz::object("entries", &ConfigStencil::entries); };

struct MdData { std::string title; std::string description; std::string link1_text; std::string link1_url; std::string link2_text; std::string link2_url; std::string link3_text; std::string link3_url; };
template <> struct glz::meta<MdData> {
  static constexpr auto value = glz::object(
    "title", &MdData::title, "description", &MdData::description,
    "link1_text", &MdData::link1_text, "link1_url", &MdData::link1_url,
    "link2_text", &MdData::link2_text, "link2_url", &MdData::link2_url,
    "link3_text", &MdData::link3_text, "link3_url", &MdData::link3_url
  );
};

struct BUser { std::string name; int age{}; };
struct BUsersData { std::vector<BUser> users; };
template <> struct glz::meta<BUser> { static constexpr auto value = glz::object("name", &BUser::name, "age", &BUser::age); };
template <> struct glz::meta<BUsersData> { static constexpr auto value = glz::object("users", &BUsersData::users); };

struct BAddress { std::string city; std::string country; };
struct BFounder { std::string name; BAddress address; };
struct BCompany { std::string name; BFounder founder; };
template <> struct glz::meta<BAddress> { static constexpr auto value = glz::object("city", &BAddress::city, "country", &BAddress::country); };
template <> struct glz::meta<BFounder> { static constexpr auto value = glz::object("name", &BFounder::name, "address", &BFounder::address); };
template <> struct glz::meta<BCompany> { static constexpr auto value = glz::object("name", &BCompany::name, "founder", &BCompany::founder); };

int main() {
  // HTML
  HtmlTable ht;
  for (auto const& u : make_sample_users()) ht.users.push_back(HtmlRow{u.name, u.email, u.age});
  auto bc_h = injamm::engine<HtmlTable>("<table>{{#users}}<tr><td>{{name}}</td><td>{{email}}</td><td>{{age}}</td></tr>{{/users}}</table>");
  auto r_h_bc = bc_h.render(ht);
  auto r_h_cg = generated::render_html(ht);
  assert(r_h_bc.has_value());
  assert(r_h_cg.has_value());
  assert(r_h_bc.value() == r_h_cg.value());
  std::cout << "HTML: " << r_h_bc.value().size() << " bytes OK\n";

  // CSV
  CsvData ct;
  for (auto const& u : make_sample_users()) ct.users.push_back(CsvRow{u.name, u.email, u.age});
  auto bc_c = injamm::engine<CsvData>("name,email,age\n{{#users}}{{name}},{{email}},{{age}}\n{{/users}}");
  auto r_c_bc = bc_c.render(ct);
  auto r_c_cg = generated::render_csv(ct);
  assert(r_c_bc.has_value() && r_c_cg.has_value());
  assert(r_c_bc.value() == r_c_cg.value());
  std::cout << "CSV: " << r_c_bc.value().size() << " bytes OK\n";

  // URL
  UrlData ud{"https://example.com/api/search", "q", "hello world", "page", "1", "limit", "10", "sort", "name", "order", "asc"};
  auto bc_u = injamm::engine<UrlData>("{{base_url}}?{{p1k}}={{p1v}}&{{p2k}}={{p2v}}&{{p3k}}={{p3v}}&{{p4k}}={{p4v}}&{{p5k}}={{p5v}}");
  auto r_u_bc = bc_u.render(ud);
  auto r_u_cg = generated::render_url(ud);
  assert(r_u_bc.has_value() && r_u_cg.has_value());
  assert(r_u_bc.value() == r_u_cg.value());
  std::cout << "URL: " << r_u_bc.value().size() << " bytes OK\n";

  // JSON
  JsonData jt;
  for (auto const& u : make_sample_users()) jt.users.push_back(JsonRow{u.name, u.email, u.age});
  auto bc_j = injamm::engine<JsonData>(R"({"users":[{{#users}}{"name":"{{name}}","email":"{{email}}","age":{{age}}}{{#if loop.is_last}}{{else}},{{/if}}{{/users}}]})");
  auto r_j_bc = bc_j.render(jt);
  auto r_j_cg = generated::render_json(jt);
  assert(r_j_bc.has_value() && r_j_cg.has_value());
  assert(r_j_bc.value() == r_j_cg.value());
  std::cout << "JSON: " << r_j_bc.value().size() << " bytes OK\n";

  // Config
  ConfigStencil cft;
  for (auto const& [k, v] : make_sample_config().entries) cft.entries.push_back(ConfigEntry{k, v});
  auto bc_cf = injamm::engine<ConfigStencil>("{{#entries}}{{key}}={{value}}\n{{/entries}}");
  auto r_cf_bc = bc_cf.render(cft);
  auto r_cf_cg = generated::render_config(cft);
  assert(r_cf_bc.has_value() && r_cf_cg.has_value());
  assert(r_cf_bc.value() == r_cf_cg.value());
  std::cout << "Config: " << r_cf_bc.value().size() << " bytes OK\n";

  // Markdown
  MdData md{"Sample Document", "This is a sample document for benchmarking.",
    "Home", "https://example.com/", "About", "https://example.com/about", "Contact", "https://example.com/contact"};
  auto bc_md = injamm::engine<MdData>("# {{title}}\n\n{{description}}\n\n- [{{link1_text}}]({{link1_url}})\n- [{{link2_text}}]({{link2_url}})\n- [{{link3_text}}]({{link3_url}})");
  auto r_md_bc = bc_md.render(md);
  auto r_md_cg = generated::render_markdown(md);
  assert(r_md_bc.has_value() && r_md_cg.has_value());
  assert(r_md_bc.value() == r_md_cg.value());
  std::cout << "Markdown: " << r_md_bc.value().size() << " bytes OK\n";

  // @index
  BUsersData iud;
  for (auto const& u : make_sample_users()) iud.users.push_back(BUser{u.name, u.age});
  auto bc_idx = injamm::engine<BUsersData>("{{#users}}{{loop.index}}:{{name}};{{/users}}");
  auto r_idx_bc = bc_idx.render(iud);
  auto r_idx_cg = generated::render_at_index(iud);
  assert(r_idx_bc.has_value() && r_idx_cg.has_value());
  assert(r_idx_bc.value() == r_idx_cg.value());
  std::cout << "@index: " << r_idx_bc.value().size() << " bytes OK\n";

  // @last
  auto bc_last = injamm::engine<BUsersData>("{{#users}}{{name}}{{#loop.is_last}}.{{/loop.is_last}}{{^loop.is_last}},{{/loop.is_last}}{{/users}}");
  auto r_last_bc = bc_last.render(iud);
  auto r_last_cg = generated::render_at_last(iud);
  assert(r_last_bc.has_value() && r_last_cg.has_value());
  assert(r_last_bc.value() == r_last_cg.value());
  std::cout << "@last: " << r_last_bc.value().size() << " bytes OK\n";

  // if/else
  auto bc_ie = injamm::engine<BUsersData>("{{#users}}[{{loop.index}}]{{name}}{{#if loop.is_last}}.{{else}},{{/if}}{{/users}}");
  auto r_ie_bc = bc_ie.render(iud);
  auto r_ie_cg = generated::render_at_if_else(iud);
  assert(r_ie_bc.has_value() && r_ie_cg.has_value());
  assert(r_ie_bc.value() == r_ie_cg.value());
  std::cout << "if/else: " << r_ie_bc.value().size() << " bytes OK\n";

  // Large (10 users, not 1000)
  BUsersData lud;
  for (int i = 0; i < 10; ++i) lud.users.push_back(BUser{"user" + std::to_string(i), 20 + (i % 50)});
  auto bc_lg = injamm::engine<BUsersData>("{{#users}}{{loop.index}}:{{name}}({{age}});{{/users}}");
  auto r_lg_bc = bc_lg.render(lud);
  auto r_lg_cg = generated::render_large(lud);
  assert(r_lg_bc.has_value() && r_lg_cg.has_value());
  assert(r_lg_bc.value() == r_lg_cg.value());
  std::cout << "Large: " << r_lg_bc.value().size() << " bytes OK\n";

  // Path 2-level
  BCompany cmp{"Acme", BFounder{"Alice", BAddress{"Tokyo", "JP"}}};
  auto bc_p2 = injamm::engine<BCompany>("{{name}} by {{founder.name}} in {{founder.address.city}}");
  auto r_p2_bc = bc_p2.render(cmp);
  auto r_p2_cg = generated::render_path_2level(cmp);
  assert(r_p2_bc.has_value() && r_p2_cg.has_value());
  assert(r_p2_bc.value() == r_p2_cg.value());
  std::cout << "Path2: " << r_p2_bc.value() << " OK\n";

  // Path 3-level
  auto bc_p3 = injamm::engine<BCompany>("{{founder.address.country}}");
  auto r_p3_bc = bc_p3.render(cmp);
  auto r_p3_cg = generated::render_path_3level(cmp);
  assert(r_p3_bc.has_value() && r_p3_cg.has_value());
  assert(r_p3_bc.value() == r_p3_cg.value());
  std::cout << "Path3: " << r_p3_bc.value() << " OK\n";

  std::cout << "\nAll checks passed!\n";
  return 0;
}
