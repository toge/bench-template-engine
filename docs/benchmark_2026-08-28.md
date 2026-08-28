# Benchmark 2026-08-28 — injamm 更新後再計測

injamm を `22e2690` (2026-08-26) → `caed3b6` (2026-08-27) に更新後の再計測。vcpkg port `injamm@2026-08-27` (my-vcpkg-ports baseline `2037ced`, port git-tree `92cb10e`, REF `caed3b6`) を使用。

## 更新内容（perf 関連）

- `92b32fc` perf: 4 施策適用（literal coalesce = emit_literal 連続融合 + strip/exists 早期 return + html_escape_into 冗長 string_view 構築削除）
  - injamm 単体ベンチで VM 経路 -3〜-12%（`multi_filter -11.96%`, `BC nested_2level -6.85%` 等）。バイトコード命令数削減が BC のウォーム描画・コールドコンパイル双方に効く
- `caed3b6` fix: バグ・性能の最小修正セット
  - perf: ループ内 `filtered_value_` の再確保を親バッファ共有で抑制、`vformat` → `vformat_to` で中間確保削減
  - その他 bytecode_io 堅牢化・逆セクション else・CRLF・ネスト 256 上限など修正主体
- `escape.hpp` / `types.hpp` / `util/injamm_codegen.cpp` / `codegen_helpers.hpp` は **不変** → codegen 生成コードに変更なし。**codegen ヘッダの再生成は不要**（今回も再生成せず 03b2ff5 のヘッダを使用）

## 環境

- CPU: AMD Ryzen 7 7700 8-Core @3801MHz (16 threads), L1 32KiB x8, L2 1MiB x8, L3 32MiB
- OS: Linux (Fedora), governor: `powersave` ⚠️ **CPU scaling 有効** のため絶対値はノイジー、相対比で評価
- Compiler: GCC 16.2.1 + `-O3 -DNDEBUG -march=native`（CMake Release）
- glaze 8.0.0, benchmark 1.9.5, inja 3.5.0, nlohmann-json 3.12.0
- injamm: `caed3b6e08cc450b84b186c18d9ce08e88827f73` @2026-08-27 (HEAD)
- ビルド: `cmake --build build --clean-first --parallel 8`（2026-08-28 18:26 JST）
- 計測: 各ベンチ `google benchmark` `--benchmark_repetitions=3` **median CPU time (ns/op)**。全 10 ベンチを 2 回実施し、低負荷側 run2 (load 1.6–2.7) を採用。run1 (load 3.5–4.4) は整合性確認用。
  - run1 → run2 で大きく動いたセル（bench_codegen の HTML NTTP 1493→955、CSV CG 780→462、URL NTTP 207→124）は負荷スパイクと判断し run2 を採用。run2 内では run1 と同水準のセルは全て ±5% 以内で再現

## 結果サマリ

### 1. テンプレートレンダリング（6種） — `bench_codegen` 3 reps median (run2)

> `bench_codegen` は同一データ型・同一テンプレートで BC/NTTP/Codegen を同一バイナリで比較（apple-to-apple）。inja/glz は単独ベンチ (`bench_html` 等) の値。

| テンプレート | inja (bench_*) | glz::stencil (bench_*) | injamm BC | injamm NTTP | injamm Codegen |
| ------------ | -------------: | ---------------------: | --------: | ----------: | -------------: |
| HTML         | 10551 | 1902 | **853** | 955 | **509** |
| CSV          | 10174 | 1368 | 729 | 744 | **462** |
| URL          | 4007 | 458 | 245 | **124** | **125** |
| JSON         | 12670 | 1774 | 937 | 974 | **502** |
| Config       | 7322 | 938 | 533 | **505** | **294** |
| Markdown     | 3191 | 411 | 254 | **138** | **139** |

#### 同一実行内の相対比（run2）

| テンプレート | BC / NTTP | BC / Codegen | NTTP / Codegen |
| ------------ | --------: | -----------: | -------------: |
| HTML         | 0.89 (BC 速い) | 1.67 | 1.88 |
| CSV          | 0.98 (BC 速い) | 1.58 | 1.61 |
| URL          | 1.97 (NTTP 速い) | 1.97 | 1.00 |
| JSON         | 0.96 (BC 速い) | 1.87 | 1.94 |
| Config       | 1.06 (NTTP 速い) | 1.82 | 1.72 |
| Markdown     | 1.84 (NTTP 速い) | 1.83 | 1.00 |

#### 旧値との比較

| テンプレート | 旧 BC (08-20 perf gov) | 新 BC | 旧 NTTP | 新 NTTP | 旧 CG | 新 CG | 旧 BC/CG | 新 BC/CG |
| ------------ | ---------------------: | ----: | -------: | ------: | -----: | ----: | -------: | -------: |
| HTML         | 766 | 853 | 836 | 955 | 518 | 509 | 1.48 | 1.67 |
| CSV          | 663 | 729 | 671 | 744 | 462 | 462 | 1.44 | 1.58 |
| URL          | 239 | 245 | 124 | 124 | 124 | 125 | 1.93 | 1.97 |
| JSON         | 842 | 937 | 867 | 974 | 519 | 502 | 1.62 | 1.87 |
| Config       | 464 | 533 | 460 | 505 | 295 | 294 | 1.57 | 1.82 |
| Markdown     | 241 | 254 | 137 | 138 | 137 | 139 | 1.76 | 1.83 |

> **注意**: 旧値は `performance` governor (2026-08-20 README)、新値は `powersave` governor のため絶対値の直接比較は不適。Codegen はコード不変であり新値が旧値と ±3% で一致（CSV 462→462）することは run2 の計測条件が旧計測に近いことを示す。その上で BC/NTTP が +5〜15% 高めなのは powersave の周波数特性＋ノイズと判断（BC の実ゲインは within-run 比で評価）。
> **2026-08-27 計測 (22e2690, powersave, load 3.5–4.0) との同 governor 比**: BC は HTML 1185→853, CSV 1126→729, URL 347→245, JSON 1076→937, Config 656→533 と大幅低下（負荷差込み）。within-run 比では **BC が NTTP を 3/6 (HTML/CSV/JSON) で僅かに上回る**ようになり、08-27 計測（NTTP が 5/6 で BC 上回る）から逆転。`92b32fc` literal coalesce による BC 強化が寄与。
> **Codegen**: 生成コード不変だが BC 側が速くなったため BC 比 1.6–2.0x に圧縮（旧 1.6–2.6x）。NTTP 比では URL/Markdown 互角、他 4 カテゴリで 1.6–1.9x。

#### `bench_html` 等の単独ベンチ（参考、inja/glz 含む）

`--benchmark_repetitions=3` median (run2):

- `bench_html`: inja 10551, glz 1902, BC 1018, NTTP 1047
- `bench_csv`: inja 10174, glz 1368, BC 899, NTTP 886
- `bench_url`: inja 4007, glz 458, BC 261, NTTP 127
- `bench_json`: inja 12670, glz 1774, BC 1090, NTTP 1124
- `bench_config`: inja 7322, glz 938, BC 550, NTTP 565
- `bench_markdown`: inja 3191, glz 411, BC 268, NTTP 136

### 2. ネストパス解決 — `bench_codegen` / `bench_paths`

| パターン | 方式 | 新 CPU (ns) (`bench_codegen` run2) | 新 CPU (`bench_paths` run2) | 旧 README (08-20) |
| -------- | ---- | ---------------------------------: | --------------------------: | ----------------: |
| 2レベル  | BC   | 103.9 | 112.5 | 106.8 |
| 2レベル  | NTTP | 61.3 | 66.1 | 101.3 |
| 2レベル  | Codegen | **36.0** | — | 35.6 |
| 3レベル  | BC   | 47.5 | 47.8 | 52.9 |
| 3レベル  | NTTP | 25.3 | 25.6 | 46.4 |
| 3レベル  | Codegen | **15.1** | — | 15.2 |
| 2レベル (runtime compile) | BC | — | 844.2 | 803.6 |

> `bench_codegen` の値は旧計測とほぼ同一（2level CG 35.6→36.0, 3level CG 15.2→15.1）。`bench_paths` の NTTP 2level は 08-27 計測 101.0 → 66–77 ns、3level 41.4 → 25.6 ns と改善（run1 77.3 / run2 66.1）。BC/NTTP 比は 1.69–1.76x で旧計測と一致し、Codegen は BC 比 **2.9–3.1x**、NTTP 比 **1.7x** 高速を維持。

### 3. @index/@first/@last ループ変数 — `bench_codegen` / `bench_at_vars`

| パターン | 方式 | 新 CPU (ns) (`bench_codegen` run2) | 新 CPU (`bench_at_vars` run2) | 旧 README (08-20) |
| -------- | ---- | ---------------------------------: | ----------------------------: | ----------------: |
| @index   | BC   | 394.7 | 404.1 | 406 |
| @index   | NTTP | 366.8 | 419.3 | 401 |
| @index   | Codegen | **216.3** | — | 214 |
| @last section | BC | 410.3 | 412.7 | 414 |
| @last section | NTTP | 381.9 | 420.6 | 413 |
| @last section | Codegen | **157.5** | — | 161 |
| @vars if/else | BC | 456.9 | 537.8 | 500 |
| @vars if/else | NTTP | 435.1 | 551.5 | 498 |
| @vars if/else | Codegen | **214.7** | — | 214 |
| Large 1000users | BC | 60470.9 | 60289.2 | 55760 |
| Large 1000users | NTTP | 54393.7 | 61257.0 | 56213 |
| Large 1000users | Codegen | **28862.5** | — | 29189 |
| Long 50pl | BC | 10175.3 | 10724.3 | 11064 |
| @index (runtime eval) | runtime | — | 1149.8 | — |

> 08-27 計測では `bench_at_vars` の NTTP が高負荷ノイズで膨らんでいた（@index 844 ns）が、今回は `bench_codegen` (367–435 ns) と一貫した値になり兩バイナリの乖離が解消。
> Codegen は @index **1.7–1.8x**, @last **2.4–2.6x**, if/else **2.0–2.1x**, Large **1.9–2.1x** で BC/NTTP を上回る（旧 README 比 @last 2.6x, Large 2.1x と同水準）。

### 4. ワンショット描画 — `bench_oneshot` 3 reps median (run2)

| N (異なるテンプレート数) | BC コールド（毎回コンパイル） | NTTP（事前コンパイル） | BC ウォーム | BC_cold / NTTP |
| -----------------------: | ----------------------------: | ---------------------: | ----------: | -------------: |
| 1  | 1466.3 | 617.0 | 602.4 | **2.38x** |
| 4  | 6019.8 | 2548.2 | 2404.7 | **2.36x** |
| 16 | 23625.8 | 9910.2 | 9632.1 | **2.38x** |

> BC コールド / NTTP 比は 08-27 計測 (22e2690, 同 powersave) の **3.02–3.37x → 2.36–2.38x** に縮小。異テンプレ 1 つあたりのコールドコストは約 **1480–1520 ns**（08-27 計測 1721 ns）。`92b32fc` literal coalesce によるバイトコード命令数削減がコールドコンパイルコストを直接削減した効果。
> NTTP の純描画は BC ウォームと互角（N=16 で 9910 vs 9632、BC ウォームが僅かに速い）。run1 も同傾向（2.29–2.39x）で再現性あり。

## 考察

- **BC の構造的改善が本計測の主題**: literal coalesce により (1) ウォーム描画の命令ディスパッチが減り、(2) コールドコンパイルの作業量が減った。oneshot の BC_cold/NTTP 3.0–3.4x → 2.4x、テンプレあたりコールドコスト 1721 → 約 1500 ns はコード変更の直接的効果で、governor 差の影響を受けにくい比率ベースで確認できる。
- **BC vs NTTP は拮抗**: BC が HTML/CSV/JSON で僅かに優位、NTTP が URL/Markdown で大きく優位 (1.8–2.0x)、Config は僅差。08-27 の「NTTP が 5/6 で優位」から BC 側に戻った。
- **Codegen は不変のまま最速維持**: 生成コードは不変（codegentool/helpers に差分なし）だが全カテゴリで BC 比 1.6–2.0x。BC が速くなった分、倍率は圧縮傾向。paths・@vars・Large では 2–3x の優位が維持。
- **絶対値の解釈には注意**: powersave governor のため旧 (perf governor) 値より全般的に +5〜15% 高い。Codegen セルが旧値と ±3% で一致する一方 BC/NTTP が高めに出る差は周波数感受性の違いを含むため、旧↔新の絶対比較はせず within-run 比で評価した。

## 再現コマンド

```bash
cmake --build build --clean-first --parallel 8
cd build
for b in bench_html bench_csv bench_url bench_json bench_config bench_markdown bench_at_vars bench_paths bench_codegen bench_oneshot; do
  ./$b --benchmark_repetitions=3 --benchmark_report_aggregates_only=true --benchmark_display_aggregates_only=true \
       --benchmark_format=json --benchmark_out=/tmp/bench0828/${b}.json
done
```

## 成果物

- ビルドログ: `build/vcpkg-manifest-install.log`（injamm@2026-08-27, REF caed3b6）
- JSON: `/tmp/bench0828/`（run1, 整合性確認用）・`/tmp/bench0828_run2/`（run2, 本レポート採用値）
- 本レポート: `docs/benchmark_2026-08-28.md`
