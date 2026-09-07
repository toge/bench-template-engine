# Template Benchmark

C++26 のテンプレート/文字列生成手法を比較するベンチマーク。

## 比較対象

| ライブラリ        | テンプレート解析       | レンダリング | 用途                                               |
| ----------------- | ---------------------- | ------------ | -------------------------------------------------- |
| inja              | ランタイム             | ランタイム   | Mustache 風テンプレート                            |
| glz::stencil      | なし（リフレクション） | ランタイム   | 構造体フィールドの文字列補間                       |
| injamm BC         | ランタイム             | ランタイム   | バイトコードVM + glazeリフレクション               |
| injamm NTTP       | コンパイル時           | ランタイム   | コンパイル時パース + glazeリフレクション           |
| injamm Codegen    | コンパイル時           | コンパイル時  | バイトコード→C++ソース変換、glaze非依存・直接アクセス |
| mstch             | ランタイム             | ランタイム   | Mustache (no1msd/mstch 1.0.2) — vcpkg           |
| kainjow-mustache  | ランタイム             | ランタイム   | Mustache (kainjow/Mustache 2025-06-15) — my-vcpkg-ports |
| jinja2cpp         | ランタイム             | ランタイム   | Jinja2 (jinja2cpp/Jinja2Cpp 2026-03-10) — my-vcpkg-ports |

## ビルド

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
cmake --build build -j
```

## 実行

```bash
cd build
./bench_html
./bench_csv
./bench_url
./bench_json
./bench_config
./bench_markdown
./bench_at_vars
./bench_paths
./bench_codegen
./bench_oneshot
./bench_format   # injamm NTTP/BC vs std::format vs fmt::format(FMT_COMPILE) + manual
```

## 結果の見方

- `BM_inja_*`: inja のテンプレートパース + レンダリングコスト
- `BM_glz_stencil_*`: glz::stencil のレンダリングコスト（テンプレート解析なし、リフレクション）
- `BM_injamm_*_bc`: injamm バイトコードVM のレンダリングコスト
- `BM_injamm_*_nttp`: injamm コンパイル時パース + レンダリングコスト
- `BM_injamm_*_codegen`: injamm codegen（コンパイル時生成C++）のレンダリングコスト
- `BM_codegen_*`: bench_codegen 内の比較用 BC/NTTP/Codegen 一式
- `BM_std_format_*` / `BM_fmt_*`: `std::format` / `fmt::format(FMT_COMPILE)` のフォーマット文字列レンダリングコスト
- `BM_injamm_*` (bench_format): 変数置換のみの単純テンプレートでの injamm NTTP/BC。`std::format`/`fmt` と同一データ・同一出力で比較（`{{var}}` は HTML エスケープあり、`{{{var}}}` raw が `std::format` と等価）。詳細は `docs/bench_format_2026-08-31.md`（別マシン値）、現マシンでの基準値は `docs/benchmark_2026-09-07.md` §6

## 注意

- inja はパースを `State::SetUp` で1回のみ行い、レンダリングのみ計測
- glz::stencil は構造体のリフレクションを使用するため、テンプレートパースが不要
- インストールバージョン: glaze 8.3.0, injamm 2026-09-05 (`e12b19f`), mstch 1.0.2#5, kainjow-mustache 2025-06-15, jinja2cpp 2026-03-10, benchmark 1.9.5
- mstch / kainjow-mustache は Mustache 準拠のため `loop.index` / `loop.is_last` 等を持たず、bench_at_vars はスキップ。bench_json は `is_last` フラグを data 側で付与して計測。URL/Markdown/Config では mstch のみ `{{var}}` が `/` を HTML エスケープするため `{{{var}}}` で計測（kainjow は `/` をエスケープしないため `{{var}}` でも問題なしだが統一して `{{{var}}}`）
- jinja2cpp は Jinja2 準拠のため `{% for user in users %}` 記法を使用。`loop.index` / `loop.last` (jinja2cpp は `loop.is_last` 非対応) 等が利用可能で bench_at_vars も計測。テンプレートは `Load` で1回パース（ウォーム）、kainjow も `mustache` オブジェクトをループ外で構築（ウォーム）、mstch のみ毎回パース（コールド）
- injamm BC はランタイムでパース→バイトコードコンパイルを行い、その後のレンダリングのみ計測
- injamm NTTP はテンプレートをコンパイル時にパースするため、実行時のパースオーバーヘッドがゼロ
  - **2026-08-05 修正 (2026-08-10 リリースに含む)**: 8aaeae0 での BC 回帰（SSE ブロック + visit_by_index fold 展開）を解消。Config/CSV は回帰前の水準に回復、JSON は大幅改善。
- **既存の BC ベンチは BC に有利な条件**: `BM_injamm_*_bc` はエンジンをループ外（SetUp 相当）で1回だけ構築する「ウォーム」計測のため、BC の実行時コンパイルコストが相殺されている。そのため純描画速度では NTTP と互角になり、CT 版の優位性が見えない。多数の異なるテンプレートを1回ずつ描画する「ワンショット」実用シナリオでは BC が不利になる（後述 `bench_oneshot`）。

## ベンチマーク結果

以下は 2026-09-07 時点の計測値（各ベンチマーク3回実行の中央値を2 run 取得し平均、glaze 8.3.0 / injamm 2026-09-05）。単位は ns/op（実測 CPU time）、値が小さいほど高速。governor が `powersave` のため同一計測内の相対比で評価（詳細は `docs/benchmark_2026-09-07.md`）。

### テンプレートレンダリング（6種）

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP | injamm Codegen |
| ------------ | ----: | -----------: | --------: | ----------: | -------------: |
| HTML         | 10648 |         1853 |       930 |         954 |        **444** |
| CSV          | 10314 |         1371 |       773 |         786 |        **405** |
| URL          |  4002 |          471 |       248 |     **119** |     **122** |
| JSON         | 13068 |         1717 |       984 |         999 |        **439** |
| Config       |  7732 |          990 |       518 |         530 |        **290** |
| Markdown     |  3256 |          420 |       245 |     **133** |     **133** |

injamm は全カテゴリで最速。Codegen（コンパイル時コード生成）は BC を全6カテゴリで上回り（**1.9–2.3倍**高速）。NTTP に対しては URL/Markdown でほぼ互角、他 4 カテゴリで **1.8–2.3倍** 高速。08-28 比で Codegen は HTML/CSV/JSON で 12–13% 改善、BC は同カテゴリで 5–9% の小幅回帰、NTTP はほぼ横ばい。

#### mstch / kainjow / jinja2cpp 追加計測（2026-09-07, 参考値）

mstch 1.0.2#5, kainjow-mustache 2025-06-15, jinja2cpp 2026-03-10（いずれもバージョン不変）を 2026-09-07 に再計測（3回中央値×2 runs の平均、他セルと同一条件）。kainjow はテンプレートをループ外で1回構築（ウォーム）、mstch は毎回 `render` でパース（コールド）、jinja2cpp も `Load` をループ外で1回（ウォーム）。

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP | injamm Codegen |  mstch | kainjow | jinja2cpp |
| ------------ | ----: | -----------: | --------: | ----------: | -------------: | -----: | ------: | --------: |
| HTML         | 10648 |         1853 |       930 |         954 |            444 |  10541 |    1737 |      8957 |
| CSV          | 10314 |         1371 |       773 |         786 |            405 |   9988 |    1695 |      8764 |
| URL          |  4002 |          471 |       248 |         119 |            122 |   3045 |     631 |       974 |
| JSON         | 13068 |         1717 |       984 |         999 |            439 |  13650 |    2060 |     10842 |
| Config       |  7732 |          990 |       518 |         530 |            290 |   6417 |    1162 |      7061 |
| Markdown     |  3256 |          420 |       245 |         133 |            133 |   2651 |     740 |       869 |

- mstch は HTML/CSV/JSON で inja と同等、jinja2cpp は inja よりやや速いが kainjow より遅い。kainjow は inja/mstch/jinja2cpp に対して 3–6倍 速く、glaze stencil には迫るが injamm BC/NTTP には依然 1.5–2倍 遅い。kainjow が高速な理由はテンプレートを再利用（ウォーム）しているためであり、毎回パースする mstch との差はパースコストを含む。
- jinja2cpp は HTML/CSV/JSON で inja と同等かやや速い、URL/Markdown では inja より大幅に速いが kainjow には及ばず。Jinja2 記法 `{% for %}` のパースコストが含まれるが `Load` はウォームのため比較的高速。
- Mustache 仕様上 `loop.is_last` 等がないため JSON では mstch/kainjow は `is_last` フラグを自前付与、jinja2cpp は `loop.last` ( `loop.is_last` 非対応) で計測。URL 等では mstch のみ `{{{var}}}` で `/` エスケープを回避（kainjow は `/` をエスケープしない）。

| パターン      | 方式      | CPU time (ns) |
| ------------- | --------- | ------------: |
| 2レベル       | mstch     |          1304 |
| 3レベル       | mstch     |           745 |
| 2レベル       | kainjow   |           594 |
| 3レベル       | kainjow   |           317 |
| 2レベル       | jinja2cpp |           791 |
| 3レベル       | jinja2cpp |           569 |

ネストパスでは mstch/kainjow/jinja2cpp いずれも injamm BC 比で 5–10倍 遅い。kainjow が最も速く、jinja2cpp がそれに次ぐ。injamm は glaze リフレクション + VM/直接アクセスのため桁違いに速い。

| パターン | 方式      | CPU time (ns) |
| -------- | --------- | ------------: |
| @index   | jinja2cpp |          7242 |
| @last    | jinja2cpp |          7212 |
| if/else  | jinja2cpp |          8885 |

jinja2cpp の `loop.index` / `loop.last` は inja と同等（7242 vs 6736, 7212 vs 6712, 8885 vs 8241）。Mustache 系（mstch/kainjow）は `loop` 変数を持たないため bench_at_vars はスキップ。

### ネストパス解決

| パターン                  | 方式            | CPU time (ns) |
| ------------------------- | --------------- | ------------: |
| 2レベル                   | BC              |         102.6 |
| 2レベル                   | NTTP            |          59.8 |
| 2レベル                   | **Codegen**     |       **35.3** |
| 3レベル                   | BC              |          46.7 |
| 3レベル                   | NTTP            |          24.2 |
| 3レベル                   | **Codegen**     |        **9.1** |
| 2レベル (runtime compile) | BC              |         875.2 |

Codegen は直接フィールドアクセスにより BC 比 **約2.9–5.1倍**、NTTP 比 **約1.7–2.7倍** 高速。VM 解釈オーバーヘッド・リフレクションコストがゼロのため、ネストが深いほど差が拡大する（3レベル Codegen は 08-28 比 -40%）。

### @index/@first/@last ループ変数

| パターン             | 方式        | CPU time (ns) |
| -------------------- | ----------- | ------------: |
| @index               | BC          |           388 |
| @index               | NTTP        |           390 |
| @index               | **Codegen** |       **209** |
| @last section        | BC          |           408 |
| @last section        | NTTP        |           397 |
| @last section        | **Codegen** |       **158** |
| @vars if/else        | BC          |           473 |
| @vars if/else        | NTTP        |           458 |
| @vars if/else        | **Codegen** |       **209** |
| Large data 1000users | BC          |         62357 |
| Large data 1000users | NTTP        |         58862 |
| Large data 1000users | **Codegen** |     **24346** |
| Long template 50pl   | BC          |          9892 |

Codegen はループ変数アクセスでも一貫して最速。@last（約2.5–2.6倍）、if/else の分岐最適化（約2.2倍）、大規模データ（約2.4–2.6倍、08-28 比で Codegen -16%）で特に顕著。

### ワンショット描画（bench_oneshot）

既存ベンチが BC に有利な「ウォーム」条件（エンジンを1回構築、同じテンプレートを繰り返し描画）であることを裏付けるための計測。N 個の**異なる**テンプレートをそれぞれ1回ずつ描画するシナリオで、BC は毎回コンパイル（コールド）し、NTTP はビルド時コンパイル済み（実行時コストゼロ）で比較する。

| 異なるテンプレート数 N | BC コールド（毎回コンパイル） | NTTP（事前コンパイル済み） | BC ウォーム（既存ベンチと同条件） |
| ---------------------: | ----------------------------: | -------------------------: | --------------------------------: |
|                      1 |                          1514 |                        602 |                               639 |
|                      4 |                          6129 |                       2420 |                              2513 |
|                     16 |                         24422 |                       9756 |                             10137 |

- NTTP の純描画コストは BC ウォームとほぼ互角（N=16 で 9756 vs 10137）。BC コールドは NTTP の **約2.5倍** 遅く（異テンプレート1つあたり約1500ns = BC の実行時コンパイルコスト）。このコストは既存ベンチでは相殺されている。
- まとめ: ホットループで1テンプレートを繰り返し描画するなら BC 一択だが、多数の異テンプレートを1回ずつ描画する実用ケースでは CT(NTTP) 版が明確に優位。CT 版の存在意義は速度というより、この実行時コンパイル不要性・コンパイル時構文検証・constexpr 描画にある。

実行: `cd build && ./bench_oneshot`

### Codegen（C++ソースコード生成）

`injamm_codegen` はバイトコード (`.bc`) から glaze 非依存の C++ ヘッダを生成する。

```
Template string → [injamm_bc] → Bytecode (.bc) → [injamm_codegen] → C++ header (.hpp)
```

生成されたコードは `data.users[i].name` のように構造体フィールドに直接アクセスするため、VM 解釈・リフレクションのオーバーヘッドがゼロ。コンパイラの最適化（インライン展開、定数畳み込み、ルーチン特化）がフルに働く。

- glaze 非依存: ランタイムに glaze ライブラリをリンクする必要がない
- 生成コードは `template<typename T>` のジェネリック関数で、フィールド名が一致すれば任意の型で利用可能
- ビルド時に `codegen_helpers.hpp` をインクルードし、HTMLエスケープ・フィルター処理などに必要な関数を提供

計測は `cd build && ./bench_codegen`

実行時パース/コンパイル不要のため、ウォームアップバイアスがなく、どのようなワークロードでも安定して最高性能を発揮する。出力が BC と完全一致することも確認済み。

#### Codegen特有の注意点

- コード生成はビルド時に行われるため、テンプレート変更時に再ビルドが必要
- テンプレートが動的に決まるケース（ユーザー入力テンプレートなど）には使用不可（その場合は BC を使用）
- 生成されるコード量はテンプレートサイズに比例するが、コンパイル時間への影響は軽微
