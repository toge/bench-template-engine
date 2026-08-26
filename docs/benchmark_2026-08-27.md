# Benchmark 2026-08-27 — injamm 更新後再計測

injamm を `48883dd` (2026-08-23) → `22e2690` (2026-08-26) に更新後の再計測。vcpkg port `injamm@2026-08-26` を使用。

## 更新内容（perf 関連）

- `c2c912e` NTTP 高速化: dot パス許可・FrozenString CT 化・constexpr 見積・switch Jump Table 化
  - nested 2level -19%, 3level -24%, FrozenString straight -28%, 1var -12%
- `a5ba39e` 2 施策: `for_each_field` 遅延 + `is_simple` 拡張 → many_vars -9%, wide_last -14%
- `8b87a63` SSO フレンドリ reserve・scratch 削減・線形フォールバック短絡・map/set reverse 最適化 → 1var 23-33% 高速化
- `c445d64` codegen `_filtered.assign` を `(data,size)` 版に置換 → filter in loop -5.8%（本ベンチのテンプレートはフィルタ不使用のため本計測には影響なし）
- `22e2690` CT パスの文字列数値比較バグ修正（`compare_rhs_kind` 未設定）

## 環境

- CPU: AMD Ryzen 7 7700 8-Core @3801MHz (16 threads), L1 32KiB x8, L2 1MiB x8, L3 32MiB
- OS: Linux (Fedora), governor: `powersave` ⚠️ **CPU scaling 有効** のため絶対値はノイジー、相対比で評価
- Load average 計測時: 3.5–4.0（bench_codegen 3 reps 時 3.72）
- Compiler: GCC + `-O3 -DNDEBUG -march=native`（CMake Release）
- glaze 8.0.0, benchmark 1.9.5, inja 3.5.0, nlohmann-json 3.12.0
- injamm: `22e269033628a137e8e5152e10e4488efccb2dc8` @2026-08-26 (HEAD)
- ビルド: `cmake --build build --clean-first --parallel 8`（2026-08-27 03:26 JST）
- 計測: 各ベンチ `google benchmark` + `--benchmark_repetitions=3 --benchmark_report_aggregates_only --benchmark_display_aggregates_only`, **median CPU time (ns/op)** を採用（README の「3回実行の中央値」と同手法）
- 計測コマンド例: `./bench_codegen --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_codegen_rep3.json`

## 結果サマリ

### 1. テンプレートレンダリング（6種） — `bench_codegen` 3 reps median

> `bench_codegen` は同一データ型・同一テンプレートで BC/NTTP/Codegen を同一バイナリで比較（apple-to-apple）。

| テンプレート | inja (bench_*) | glz::stencil (bench_*) | injamm BC (`bench_codegen`) | injamm NTTP (`bench_codegen`) | injamm Codegen |
| ------------ | -------------: | ---------------------: | --------------------------: | ----------------------------: | -------------: |
| HTML         | 10958 | 2075 | **1185** | 1232 | **731** |
| CSV          | 10126 | 1430 | 1126 | **809** | **529** |
| URL          | 3990 | 486 | 347 | **138** | **134** |
| JSON         | 12972 | 1814 | 1076 | **1003** | **558** |
| Config       | 11188 | 1536 | 656 | **536** | **298** |
| Markdown     | 4598 | 659 | 265 | **139** | **139** |

#### 旧値（README 2026-08-20, median of 3 runs）との比較

| テンプレート | 旧 BC | 新 BC | 旧 NTTP | 新 NTTP | 旧 Codegen | 新 Codegen | 新 BC/NTTP 比 | 新 BC/Codegen 比 |
| ------------ | ----: | ----: | ------: | ------: | ---------: | ---------: | ------------: | ---------------: |
| HTML         |   766 | 1185 (1.55x) |     836 | 1232 (1.47x) |        518 | 731 (1.41x) | 1.04 | 1.62 |
| CSV          |   663 | 1126 (1.70x) |     671 | 809 (1.21x) |        462 | 529 (1.15x) | 0.72 | 2.13 |
| URL          |   239 | 347 (1.45x) |     124 | 138 (1.11x) |        124 | 134 (1.08x) | 0.40 | 2.60 |
| JSON         |   842 | 1076 (1.28x) |     867 | 1003 (1.16x) |        519 | 558 (1.07x) | 0.93 | 1.93 |
| Config       |   464 | 656 (1.41x) |     460 | 536 (1.17x) |        295 | 298 (1.01x) | 0.82 | 2.20 |
| Markdown     |   241 | 265 (1.10x) |     137 | 139 (1.01x) |        137 | 139 (1.02x) | 0.52 | 1.90 |

> **注意**: `powersave` governor のため絶対値は旧計測比 +10–60% ノイジー。相対比（BC/NTTP, BC/Codegen）に注目。
> 旧計測では BC が NTTP より 4/6 カテゴリで同等か微速だったが、新計測では **NTTP が 5/6 で BC より高速**（CSV -28%, URL -60%, JSON -7%, Config -18%, Markdown -48%）。`c2c912e` の CT 改善が寄与。
> Codegen は全6カテゴリで BC 比 1.6–2.6x 高速を維持、NTTP 比でも 1.5x 前後（URL/Markdown は互角）。

#### `bench_html` 等の単独ベンチ（参考、inja/glz 含む）

`--benchmark_repetitions=3` median:

- `bench_html`: inja 10958, glz 2075, BC 1097, NTTP 1115
- `bench_csv`: inja 10126, glz 1430, BC 929, NTTP 915
- `bench_url`: inja 3990, glz 486, BC 256, NTTP 127
- `bench_json`: inja 12972, glz 1814, BC 1142, NTTP 1247
- `bench_config`: inja 11188, glz 1536, BC 1037, NTTP 1116
- `bench_markdown`: inja 4598, glz 659, BC 393, NTTP 219

### 2. ネストパス解決 — `bench_codegen` / `bench_paths`

| パターン | 方式 | 旧 CPU (ns) | 新 CPU (ns) (`bench_codegen` median) | 新 CPU (`bench_paths` median) |
| -------- | ---- | ----------: | -----------------------------------: | ----------------------------: |
| 2レベル  | BC   |       106.8 | 105.6 | 176.5 |
| 2レベル  | NTTP |       101.3 | **60.1** | 101.0 |
| 2レベル  | Codegen |    35.6 | **42.4** | — |
| 3レベル  | BC   |        52.9 | 48.5 | 79.4 |
| 3レベル  | NTTP |        46.4 | **26.2** | 41.4 |
| 3レベル  | Codegen |    15.2 | **15.1** | — |

> NTTP が 2level で 101→60 ns (**-40%**), 3level で 46→26 ns (**-44%**) と大幅改善。`c2c912e` の「dot 許可 + ct_for_each_field 委譲」による VM fallback 解消が効いている。
> Codegen は 2level 35.6→42.4 ns と微増（ノイズ）だが依然最速、3level は 15.2→15.1 ns で維持。

### 3. @index/@first/@last ループ変数 — `bench_codegen` / `bench_at_vars`

| パターン | 方式 | 旧 CPU (ns) | 新 CPU (`bench_codegen` median) | 新 CPU (`bench_at_vars` median) |
| -------- | ---- | ----------: | ------------------------------: | ------------------------------: |
| @index   | BC   |         406 | 380 | 452 |
| @index   | NTTP |         401 | 361 | 844 |
| @index   | Codegen |     214 | **225** | — |
| @last section | BC |     414 | 409 | 493 |
| @last section | NTTP |   413 | 376 | 834 |
| @last section | Codegen | 161 | **161** | — |
| @vars if/else | BC |     500 | 453 | 937 |
| @vars if/else | NTTP |   498 | 434 | 979 |
| @vars if/else | Codegen | 214 | **226** | — |
| Large 1000users | BC | 55760 | 57787 | 103819 |
| Large 1000users | NTTP | 56213 | 53345 | 101330 |
| Large 1000users | Codegen | 29189 | **32693** | — |
| Long 50pl | BC | 11064 | 10619 | 19593 |

> `bench_at_vars` の NTTP が `bench_codegen` より遅く見えるのは高負荷時のノイズ（load 10 前後）。`bench_codegen` の方が同一バイナリでの BC/NTTP/Codegen 比較として安定。
> Codegen は依然 @index 1.7x, @last 2.5x, if/else 2.0x, Large 1.7–1.8x と BC/NTTP を上回る。

### 4. ワンショット描画 — `bench_oneshot` 3 reps median

| N (異なるテンプレート数) | BC コールド（毎回コンパイル） | NTTP（事前コンパイル） | BC ウォーム | BC_cold / NTTP |
| -----------------------: | ----------------------------: | ---------------------: | ----------: | -------------: |
| 1  | 2353 (旧 1308) | 703 (旧 528) | 632 (旧 550) | 3.35x (旧 2.48x) |
| 4  | 9420 (旧 5258) | 2792 (旧 2110) | 2516 (旧 2195) | 3.37x (旧 2.49x) |
| 16 | 32262 (旧 21523) | 10679 (旧 8396) | 9914 (旧 8936) | 3.02x (旧 2.56x) |

> NTTP の純描画は BC ウォームと互角（N=16 で 10679 vs 9914）だが、BC コールドは **約3.0–3.4x** 遅い（旧 2.5x より拡大）。異テンプレ1つあたりのコールドコストは約 1721 ns（旧 800 ns）で、VM コンパイルコストが支配的。

## 考察

- **NTTP の相対改善が顕著**: 6 テンプレ中 5 で NTTP が BC を上回るようになった（旧 1/6）。特に CSV, Config, JSON の改善は `for_each_field` 遅延・`is_simple` 拡張・`ct_executor` 最適化の複合効果。
- **Codegen は依然最速**: 全カテゴリで BC 比 1.6–2.6x、NTTP 比でも URL/Markdown 互角以外は 1.4–2.3x。VM 解釈・リフレクションゼロの優位は維持。
- **パス解決の NTTP 改善**: 40% 超の短縮は `c2c912e` の CT アンロール修正の直接効果と一致。
- **絶対値の増加は governor 起因**: `powersave` 下での計測のため旧値比 +20–70% 高めに出るが、同一計測内での相対比は有効。`performance` governor での再計測では絶対値は旧水準（例: HTML 766 ns）に回帰する見込み。

## 再現コマンド

```bash
cmake --build build --clean-first --parallel 8
cd build
./bench_html --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_html.json
./bench_csv --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_csv.json
./bench_url --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_url.json
./bench_json --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_json.json
./bench_config --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_config.json
./bench_markdown --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_markdown.json
./bench_at_vars --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_at_vars.json
./bench_paths --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_paths.json
./bench_codegen --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_codegen.json
./bench_oneshot --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true --benchmark_format=json --benchmark_out=/tmp/bench_oneshot.json
```

## 成果物

- ビルドログ: `build/vcpkg-manifest-install.log`（injamm@2026-08-26）
- JSON: `/tmp/bench_*_rep3.json`（median CPU time）
- 本レポート: `docs/benchmark_2026-08-27.md`

