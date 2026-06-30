# Template Benchmark

C++26 のテンプレート/文字列生成手法を比較するベンチマーク。

## 比較対象

| ライブラリ        | テンプレート解析       | レンダリング | 用途                                    |
| ----------------- | ---------------------- | ------------ | --------------------------------------- |
| inja              | ランタイム             | ランタイム   | Mustache 風テンプレート                 |
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
- `BM_glz_stencil_*`: glz::stencil のレンダリングコスト（テンプレート解析なし、リフレクション）
- `BM_injamm_*_bc`: injamm バイトコードVM のレンダリングコスト
- `BM_injamm_*_nttp`: injamm コンパイル時パース + レンダリングコスト

## 注意

- inja はパースを `State::SetUp` で1回のみ行い、レンダリングのみ計測
- glz::stencil は構造体のリフレクションを使用するため、テンプレートパースが不要
- injamm BC はランタイムでパース→バイトコードコンパイルを行い、その後のレンダリングのみ計測
- injamm NTTP はテンプレートをコンパイル時にパースするため、実行時のパースオーバーヘッドがゼロ

## ベンチマーク結果

以下は 2026-06-29 時点の計測値。単位は ns/op（実測 CPU time）、値が小さいほど高速。

### テンプレートレンダリング（6種）

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----: | -----------: | --------: | ----------: |
| HTML         | 15497 |         2976 |  **1365** |        1477 |
| CSV          | 15205 |         2388 |  **1215** |        1408 |
| URL          |  5496 |          665 |   **298** |         459 |
| JSON         | 19486 |         2746 |  **1683** |        1793 |
| Config       | 10733 |         1439 |   **669** |         862 |
| Markdown     |  4747 |          617 |   **472** |         557 |

injamm は全カテゴリで最速。単純なテンプレートでは NTTP(コンパイル時パース)が BC(中間Bytecode) をやや上回り、if/else を含む複雑なテンプレートでは BC が NTTP と同等かやや優れる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------: |
| 2レベル                   | BC   |           148 |
| 3レベル                   | BC   |        **70** |
| 2レベル (runtime compile) | BC   |          1006 |
| 2レベル                   | NTTP |           225 |
| 3レベル                   | NTTP |         **87** |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式    | CPU time (ns) |
| ---------------------------- | ------- | ------------: |
| @index                       | BC      |       **539** |
| @index                       | NTTP    |           587 |
| @index                       | runtime |          1433 |
| @last section                | BC      |       **499** |
| @last section                | NTTP    |           749 |
| @vars if/else                | BC      |       **742** |
| @vars if/else                | NTTP    |           975 |
| Large data 1000users         | BC      |      **63003** |
| Large data 1000users         | NTTP    |         67902 |
| Compile cost 1000x           | BC      |        992910 |
| Long template 50placeholders | BC      |         12281 |
