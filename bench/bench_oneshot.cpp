#include <benchmark/benchmark.h>

#include "common.hpp"
#include <injamm.hpp>

#include <string>
#include <vector>

// 1回ずつ異なるテンプレートを描画する「ワンショット」シナリオ。
// 既存ベンチは BC のコンパイルを SetUp で相殺しているため、BC が実行時コンパイル
// コストを支払う現実の使い方（多数の異テンプレートを1回ずつ描画）での不利が見えない。
// ここでは BC は毎 op コンパイルし、NTTP はビルド時コンパイル済み（実行時0）で比較する。

struct OTable {
  std::vector<User> users;
};

template <>
struct glz::meta<OTable> {
  static constexpr auto value = glz::object("users", &OTable::users);
};

// 16個の「構造的に同等だが異なる」テンプレート。
// NTTP はコンパイル時パース、BC は実行時パースの同じ文字列を使う。
#define EACH_TMPL(X) \
  X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) \
  X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15)

#define TMPL_LITERAL(i) "T" #i "{{#users}}<tr><td>{{name}}</td><td>{{age}}</td></tr>{{/users}}"

// BC用: 実行時文字列として同一テンプレートを構築
[[nodiscard]] static auto make_tmpl(std::size_t i) -> std::string {
  return std::string("T") + std::to_string(i) +
         "{{#users}}<tr><td>{{name}}</td><td>{{age}}</td></tr>{{/users}}";
}

// NTTP用: i 番目のリテラルで描画（ビルド時に全てパース済み）
[[nodiscard]] static auto nttp_render_nth(std::size_t i, OTable const& d) -> std::string {
  switch (i) {
#define CASE(i) case i: return injamm::render<TMPL_LITERAL(i)>(d).value();
    EACH_TMPL(CASE)
#undef CASE
    default:
      return {};
  }
}

static void fill(OTable& data) {
  for (auto const& u : make_sample_users()) {
    data.users.push_back(u);
  }
}

// BC: 毎 op コンパイル + 描画（ワンショット、コンパイルコストが相殺されない）
static void BM_oneshot_bc(benchmark::State& state) {
  auto const N = static_cast<std::size_t>(state.range(0));
  OTable data;
  fill(data);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      auto bc = injamm::engine<OTable>(make_tmpl(i));
      auto r = bc.render(data);
      bench::DoNotOptimize(r);
    }
  }
}
BENCHMARK(BM_oneshot_bc)->Arg(1)->Arg(4)->Arg(16);

// NTTP: ビルド時パース済みテンプレートを描画のみ（実行時コンパイルコスト = 0）
static void BM_oneshot_nttp(benchmark::State& state) {
  auto const N = static_cast<std::size_t>(state.range(0));
  OTable data;
  fill(data);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      auto r = nttp_render_nth(i, data);
      bench::DoNotOptimize(r);
    }
  }
}
BENCHMARK(BM_oneshot_nttp)->Arg(1)->Arg(4)->Arg(16);

// 参考: BC が事前コンパイル（ウォーム）した場合。既存ベンチと同条件。
static void BM_oneshot_bc_warm(benchmark::State& state) {
  auto const N = static_cast<std::size_t>(state.range(0));
  OTable data;
  fill(data);

  std::vector<injamm::engine<OTable>> engines;
  engines.reserve(N);
  for (std::size_t i = 0; i < N; ++i) {
    engines.emplace_back(make_tmpl(i));
  }

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      auto r = engines[i].render(data);
      bench::DoNotOptimize(r);
    }
  }
}
BENCHMARK(BM_oneshot_bc_warm)->Arg(1)->Arg(4)->Arg(16);
