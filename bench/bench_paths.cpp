#include <benchmark/benchmark.h>
#include "common.hpp"
#include <injamm.hpp>
#include <glaze/glaze.hpp>

// ---- ベンチマーク用データ型（ネストパス） ----

struct BAddress {
  std::string city;
  std::string country;
};

struct BFounder {
  std::string name;
  BAddress address;
};

struct BCompany {
  std::string name;
  BFounder founder;
};

template <>
struct glz::meta<BAddress> {
  static constexpr auto value = glz::object("city", &BAddress::city, "country", &BAddress::country);
};

template <>
struct glz::meta<BFounder> {
  static constexpr auto value =
      glz::object("name", &BFounder::name, "address", &BFounder::address);
};

template <>
struct glz::meta<BCompany> {
  static constexpr auto value =
      glz::object("name", &BCompany::name, "founder", &BCompany::founder);
};

// ネストパスのマイクロベンチマーク

/// @brief 2 レベルのネストパスアクセスベンチマーク
static void BM_injamm_path_2level_bc(benchmark::State& state) {
  BCompany company{
      "Acme",
      BFounder{"Alice", BAddress{"Tokyo", "JP"}}};
  static auto constexpr kTmpl = "{{name}} by {{founder.name}} in {{founder.address.city}}";
  auto bc = injamm::engine<BCompany>(kTmpl);

  for (auto _ : state) {
    auto result = bc.render(company);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_path_2level_bc);

/// @brief 3 レベルのネストパスアクセスベンチマーク
static void BM_injamm_path_3level_bc(benchmark::State& state) {
  BCompany company{
      "Acme",
      BFounder{"Alice", BAddress{"Tokyo", "JP"}}};
  static auto constexpr kTmpl = "{{founder.address.country}}";
  auto bc = injamm::engine<BCompany>(kTmpl);

  for (auto _ : state) {
    auto result = bc.render(company);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_path_3level_bc);

/// @brief runtime パース込みのネストパスベンチマーク
static void BM_injamm_path_runtime(benchmark::State& state) {
  BCompany company{
      "Acme",
      BFounder{"Alice", BAddress{"Tokyo", "JP"}}};

  for (auto _ : state) {
    auto bc = injamm::engine<BCompany>("{{name}} / {{founder.name}} / {{founder.address.city}}");
    auto result = bc.render(company);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_path_runtime);

// === NTTP (compile-time parse) variants ===

/// @brief NTTP: 2 レベルのネストパス
static void BM_injamm_path_2level_nttp(benchmark::State& state) {
  BCompany company{
      "Acme",
      BFounder{"Alice", BAddress{"Tokyo", "JP"}}};

  for (auto _ : state) {
    auto result = injamm::render<"{{name}} by {{founder.name}} in {{founder.address.city}}">(company);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_path_2level_nttp);

/// @brief NTTP: 3 レベルのネストパス
static void BM_injamm_path_3level_nttp(benchmark::State& state) {
  BCompany company{
      "Acme",
      BFounder{"Alice", BAddress{"Tokyo", "JP"}}};

  for (auto _ : state) {
    auto result = injamm::render<"{{founder.address.country}}">(company);
    bench::DoNotOptimize(result);
  }
}
BENCHMARK(BM_injamm_path_3level_nttp);
