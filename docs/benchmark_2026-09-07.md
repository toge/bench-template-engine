# Benchmark 2026-09-07 — glaze 8.3.0 / injamm 2026-09-05 更新後再計測

glaze `8.0.0` → `8.3.0`、injamm `caed3b6` (2026-08-27) → `e12b19f` (2026-09-05) に更新後の再計測。
vcpkg builtin-baseline `04a9d8e`、my-vcpkg-ports baseline `e4273ed` (injamm port `2026-09-05`) を使用。

## 更新内容（perf 関連）

injamm `caed3b6..e12b19f` (40 commits) の主な変更：

- OOM を `std::expected` で伝搬しランタイムパスを `noexcept` 化（`serialize_value` fallible 化、`try_*` ヘルパー、`out_of_memory` / `invalid_format` 追加）
- フィルタ追加：`sort` / `join` / `round`（+ codegen パリティ対応、bytecode v5）
- Mustache 互換の親スタック解決（暗黙参照）を追加
- ホット emit パスの見直し（直接 append、手続きの整理）、コールドな OOM/format ヘルパーに `gnu::cold`
- `codegen_helpers.hpp` を Buffer ジェネリック化（`html_escape_append` / `append_number` 等が `std::string` 専用 → 任意 Buffer）
- glaze 8.3.0（8.2.0 経由）：stencil への影響は軽微（下表 ±5% 以内、実質横ばい）

codegen 側の対応：`codegen_helpers.hpp` を injamm `test_codegen/` の新版に同期し、
`render_*.hpp` 12 件を新 `injamm_bc` + `injamm_codegen` で再生成。
生成ボディは旧版と実質同一（`render_path_3level.hpp` の `reserve(32)` 削除のみ）。
`check_codegen`（BC vs Codegen 出力一致 assert）で全12パターン一致を確認。

## 環境

- CPU: AMD Ryzen 7 7700 8-Core, OS: Linux (Fedora), governor: `powersave`（08-28 計測と同一条件、絶対値はノイジー・相対比で評価）
- Compiler: GCC 16.2.1 + `-O3 -DNDEBUG -march=native`（CMake Release）
- glaze 8.3.0, benchmark 1.9.5, inja 3.5.0（不変）, nlohmann-json 3.12.0, fmt（不変）, mstch 1.0.2#5 / kainjow-mustache 2025-06-15 / jinja2cpp 2026-03-10（port 不変）
- injamm: `e12b19f7adbc243cf210badef3486a09b2deac9f` @2026-09-05 (HEAD)
- 計測: 各ベンチ `--benchmark_repetitions=3` の median CPU time (ns/op) を2 run 取得し、run 間中央値の**平均**を採用。
  run1 終了時 load 1.22 / run2 終了時 load 2.19。全148セルの run 間乖離は中央値 0.8%（最大 13.4% は 30ns 未満の極小ベンチのみ）。

## 結果サマリ

### 1. テンプレートレンダリング（6種） — `bench_codegen` 3 reps median × 2 runs

> inja/glz は単独ベンチ (`bench_html` 等) の値。BC/NTTP/Codegen は `bench_codegen`（同一データ・同一テンプレートの apple-to-apple）。

| テンプレート | inja | glz::stencil | injamm BC | injamm NTTP | injamm Codegen |
| ------------ | ----: | -----------: | --------: | ----------: | -------------: |
| HTML         | 10648 |         1853 |       930 |         954 |        **444** |
| CSV          | 10314 |         1371 |       773 |         786 |        **405** |
| URL          |  4002 |          471 |       248 |     **119** |     **122** |
| JSON         | 13068 |         1717 |       984 |         999 |        **439** |
| Config       |  7732 |          990 |       518 |         530 |        **290** |
| Markdown     |  3256 |          420 |       245 |     **133** |     **133** |

旧値（08-28）との比較：

| テンプレート | 旧 BC | 新 BC | 旧 NTTP | 新 NTTP | 旧 CG | 新 CG | BC 変化 | NTTP 変化 | CG 変化 |
| ------------ | ----: | ----: | ------: | ------: | ----: | ----: | ------: | --------: | ------: |
| HTML         |   853 |   930 |     955 |     954 |   509 |   444 |   +9% |      0% |  **-13%** |
| CSV          |   729 |   773 |     744 |     786 |   462 |   405 |   +6% |     +6% |  **-12%** |
| URL          |   245 |   248 |     124 |     119 |   125 |   122 |   +1% |     -4% |      -2% |
| JSON         |   937 |   984 |     974 |     999 |   502 |   439 |   +5% |     +3% |  **-13%** |
| Config       |   533 |   518 |     505 |     530 |   294 |   290 |   -3% |     +5% |      -1% |
| Markdown     |   254 |   245 |     138 |     133 |   139 |   133 |   -4% |     -4% |      -4% |

- **Codegen は HTML/CSV/JSON で 12–13% 改善**（Buffer ジェネリック化した helpers + noexcept 化の恩恵）。BC/NTTP 比では **2.0–2.3倍** に拡大。
- **BC は HTML/CSV/JSON で +5–9% の小幅回帰**（OOM-expected 伝搬・分岐追加の影響とみられる）。NTTP はほぼ横ばい（±6%、Config inja/glz も同日 +6% のため日間ノイズを含む）。
- URL/Markdown は NTTP と Codegen が互角のまま。inja/glz はバージョン不変（glaze stencil は ±5% 以内で横ばい）。

### 2. ネストパス解決 — `bench_codegen`

| パターン | 方式 | 新 CPU (ns) | 旧 CPU (ns) |
| -------- | ---- | ----------: | ----------: |
| 2レベル  | BC | 102.6 | 103.9 |
| 2レベル  | NTTP | 59.8 | 61.3 |
| 2レベル  | **Codegen** | **35.3** | 36.0 |
| 3レベル  | BC | 46.7 | 47.5 |
| 3レベル  | NTTP | 24.2 | 25.3 |
| 3レベル  | **Codegen** | **9.1** | 15.1 |

ネストパスはほぼ横ばいだが、**3レベル Codegen が 15.1 → 9.1 ns（-40%）** と大幅改善。BC 比 5.1倍、NTTP 比 2.7倍。

### 3. @index/@last/if-else/large — `bench_codegen`

| パターン | 方式 | 新 CPU (ns) | 旧 CPU (ns) |
| -------- | ---- | ----------: | ----------: |
| @index | BC / NTTP / **Codegen** | 388 / 390 / **209** | 395 / 367 / 216 |
| @last section | BC / NTTP / **Codegen** | 408 / 397 / **158** | 410 / 382 / 158 |
| @vars if/else | BC / NTTP / **Codegen** | 473 / 458 / **209** | 457 / 435 / 215 |
| Large 1000users | BC / NTTP / **Codegen** | 62357 / 58862 / **24346** | 60471 / 54394 / 28863 |
| Long template 50pl | BC | 9892 | 10175 |

Codegen は @last（2.5–2.6倍）、if/else（2.2倍）、large（**2.4–2.6倍**、旧 1.9–2.1倍から拡大）を維持・拡大。
large NTTP の +8%（54394 → 58862）は両 run で再現するが、同日 inja/glz も +3–6% のため日間ノイズの可能性あり。

### 4. ワンショット描画 — `bench_oneshot`

| 異なるテンプレート数 N | BC コールド | NTTP | BC ウォーム | 旧コールド / 旧NTTP / 旧ウォーム |
| ---------------------: | ----------: | ---: | ----------: | -------------------------------: |
| 1 | 1514 | 602 | 639 | 1466 / 617 / 602 |
| 4 | 6129 | 2420 | 2513 | 6020 / 2548 / 2405 |
| 16 | 24422 | 9756 | 10137 | 23626 / 9910 / 9632 |

傾向は不変：BC コールドは NTTP の約2.5倍（実行時コンパイルコスト約1500ns/テンプレート）。NTTP の純描画は BC ウォームと互角。

### 5. mstch / kainjow-mustache / jinja2cpp（バージョン不変、再計測）

| テンプレート | inja | glz::stencil | injamm BC | injamm NTTP | injamm Codegen | mstch | kainjow | jinja2cpp |
| ------------ | ----: | -----------: | --------: | ----------: | -------------: | ----: | ------: | --------: |
| HTML         | 10648 |         1853 |       930 |         954 |            444 | 10541 |    1737 |      8957 |
| CSV          | 10314 |         1371 |       773 |         786 |            405 |  9988 |    1695 |      8764 |
| URL          |  4002 |          471 |       248 |         119 |            122 |  3045 |     631 |       974 |
| JSON         | 13068 |         1717 |       984 |         999 |            439 | 13650 |    2060 |     10842 |
| Config       |  7732 |          990 |       518 |         530 |            290 |  6417 |    1162 |      7061 |
| Markdown     |  3256 |          420 |       245 |         133 |            133 |  2651 |     740 |       869 |

3 ライブラリとも port 不変のため 08-30 単発値とほぼ一致（±3% 以内）。結論は不変：kainjow が Mustache 系最速だが injamm BC/NTTP には 1.5–2倍及ばない。

| パターン | 方式 | CPU time (ns) |
| -------- | ---- | ------------: |
| 2レベル  | mstch | 1304 |
| 3レベル  | mstch | 745 |
| 2レベル  | kainjow | 594 |
| 3レベル  | kainjow | 317 |
| 2レベル  | jinja2cpp | 791 |
| 3レベル  | jinja2cpp | 569 |

| パターン | 方式 | CPU time (ns) |
| -------- | ---- | ------------: |
| @index   | jinja2cpp | 7242 |
| @last    | jinja2cpp | 7212 |
| if/else  | jinja2cpp | 8885 |

### 6. bench_format — NTTP vs std::format vs fmt (FMT_COMPILE)（同マシン初回記録）

> 旧 `docs/bench_format_2026-08-3*.md` は別マシン（Ryzen 7 1700）の値のため直接比較不可。本計測が現マシンでの基準値。

| ケース | NTTP (ns) | BC (ns) | std::format (ns) | fmt (ns) | std/NTTP | fmt/NTTP | BC/NTTP |
| ------ | --------: | ------: | ---------------: | -------: | -------: | -------: | ------: |
| 1 var string | 14.9 | 29.3 | 28.7 | 19.7 | 1.93x | 1.32x | 1.97x |
| 1 var int | 6.7 | 33.4 | 32.8 | 6.2 | 4.87x | 0.92x | 4.97x |
| 1 var double | 20.7 | 43.2 | 93.5 | 32.1 | 4.53x | 1.55x | 2.09x |
| 2 vars str/int | 18.1 | 51.0 | 79.2 | 27.9 | 4.38x | 1.54x | 2.82x |
| 2 vars str/dbl | 28.7 | 64.5 | 118.6 | 38.0 | 4.14x | 1.33x | 2.25x |
| 2 vars int/dbl | 24.1 | 66.5 | 133.7 | 39.7 | 5.54x | 1.65x | 2.76x |
| 3 vars | 42.0 | 85.1 | 159.9 | 49.3 | 3.81x | 1.17x | 2.03x |
| 10 vars | 108.4 | 233.8 | 438.3 | 97.4 | 4.04x | 0.90x | 2.16x |

NTTP は `std::format` に対し 1.9–5.5倍（1 var string を除き 3.8倍以上）。
`fmt::format(FMT_COMPILE)` とは互角（int・10 vars では fmt が約1割速い、他は NTTP が 1.2–1.6倍速い）。
BC（VM）は NTTP の約2–3倍、std::format に対しては 1 var string/int を除き高速。

## まとめ

- injamm Codegen：HTML/CSV/JSON で **12–13%**、large で **16%**、3レベルパスで **40%** 改善。BC 比 2.0–2.6倍（パス系は 3–5倍）。
- injamm BC：HTML/CSV/JSON で +5–9% の小幅回帰（noexcept/OOM 伝搬リファクタの影響か）。実用上は誤差範囲だが要注視。
- injamm NTTP：ほぼ横ばい。
- glaze 8.3.0 の stencil への影響なし（±5% 以内）。inja/mstch/kainjow/jinja2cpp/fmt はバージョン不変で傾向不変。
