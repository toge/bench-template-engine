# Template Benchmark

C++26 のテンプレート/文字列生成手法を比較するベンチマーク。

## 比較対象

| ライブラリ        | テンプレート解析       | レンダリング | 用途                                    |
| ----------------- | ---------------------- | ------------ | --------------------------------------- |
| inja              | ランタイム             | ランタイム   | Mustache 風テンプレート                 |
| frozenchars::inja | コンパイル時           | ランタイム   | Mustache 風テンプレート                 |
| glz::stencil      | なし（リフレクション） | ランタイム   | 構造体フィールドの文字列補間            |
| injamm BC         | ランタイム             | ランタイム   | バイトコードVM + glazeリフレクション    |
| injamm NTTP       | コンパイル時           | ランタイム   | コンパイル時パース + glazeリフレクション |

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
```

## 結果の見方

- `BM_inja_*`: inja のテンプレートパース + レンダリングコスト
- `BM_frozenchars_*`: frozenchars::inja のレンダリングコスト（テンプレート解析はコンパイル時）
- `BM_glz_stencil_*`: glz::stencil のレンダリングコスト（テンプレート解析なし、リフレクション）
- `BM_injamm_*_bc`: injamm バイトコードVM のレンダリングコスト
- `BM_injamm_*_nttp`: injamm コンパイル時パース + レンダリングコスト

## 注意

- inja はパースを `State::SetUp` で1回のみ行い、レンダリングのみ計測
- frozenchars::inja はテンプレートをコンパイル時に解析し、実行時にコンテキストの値を解決
- glz::stencil は構造体のリフレクションを使用するため、テンプレートパースが不要
- injamm BC はランタイムでパース→バイトコードコンパイルを行い、その後のレンダリングのみ計測
- injamm NTTP はテンプレートをコンパイル時にパースするため、実行時のパースオーバーヘッドがゼロ

## ベンチマーク結果

以下は 2026-06-08 時点の計測値。単位は ns/op（実測 CPU time）、値が小さいほど高速。

### テンプレートレンダリング（6種）

| テンプレート |  inja | frozenchars | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----: | ----------: | -----------: | --------: | ----------: |
| HTML         | 11493 |        8240 |         1972 |       869 |     **845** |
| CSV          | 10699 |        8125 |         1605 |   **656** |         699 |
| URL          |  4213 |         926 |          460 |   **178** |         456 |
| JSON         | 13219 |        9906 |         1770 |   **874** |         977 |
| Config       |  7626 |        5687 |          987 |       520 |     **461** |
| Markdown     |  3395 |         641 |          407 |   **243** |         320 |

injamm は全カテゴリで最速。単純なテンプレートでは NTTP(コンパイル時パース)が BC(中間Bytecode) をやや上回り、if/else を含む複雑なテンプレートでは BC が NTTP と同等かやや優れる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------: |
| 2レベル                   | BC   |            93 |
| 3レベル                   | BC   |        **47** |
| 2レベル (runtime compile) | BC   |           292 |
| 2レベル                   | NTTP |           109 |
| 3レベル                   | NTTP |        **45** |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式    | CPU time (ns) |
| ---------------------------- | ------- | ------------: |
| @index                       | BC      |           337 |
| @index                       | NTTP    |           376 |
| @index                       | runtime |           526 |
| @last section                | BC      |           282 |
| @last section                | NTTP    |           409 |
| @vars if/else                | BC      |           476 |
| @vars if/else                | NTTP    |           592 |
| Large data 1000users         | BC      |         58246 |
| Large data 1000users         | NTTP    |         51913 |
| Compile cost 1000x           | BC      |        500596 |
| Long template 50placeholders | BC      |         10503 |
