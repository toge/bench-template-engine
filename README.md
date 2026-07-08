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
- インストールバージョン: glaze 7.8.4, injamm 2026-07-07
- injamm BC はランタイムでパース→バイトコードコンパイルを行い、その後のレンダリングのみ計測
- injamm NTTP はテンプレートをコンパイル時にパースするため、実行時のパースオーバーヘッドがゼロ

## ベンチマーク結果

以下は 2026-07-08 時点の計測値。単位は ns/op（実測 CPU time）、値が小さいほど高速。

### テンプレートレンダリング（6種）

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----: | -----------: | --------: | ----------: |
| HTML         | 11131 |         1960 |  **1005** |        1056 |
| CSV          | 10973 |         1410 |        857 |     **804** |
| URL          |  3997 |          461 |     **231** |         232 |
| JSON         | 13734 |         2710 |       1257 |    **1214** |
| Config       |  7625 |         1015 |    **485** |         490 |
| Markdown     |  3325 |          441 |        263 |     **261** |

injamm は全カテゴリで最速。NTTP（コンパイル時パース）が CSV/JSON/Markdown で BC を上回り、BC は HTML/URL/Config でリードする。テンプレートの特性に応じた使い分けでより高い性能を引き出せる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------: |
| 2レベル                   | BC   |           108 |
| 3レベル                   | BC   |          51.9 |
| 2レベル (runtime compile) | BC   |           675 |
| 2レベル                   | NTTP |       **106** |
| 3レベル                   | NTTP |      **51.5** |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式    | CPU time (ns) |
| ---------------------------- | ------- | ------------: |
| @index                       | BC       |           398 |
| @index                       | NTTP    |       **389** |
| @index                       | runtime |          1020 |
| @last section                | BC       |       **370** |
| @last section                | NTTP    |           388 |
| @vars if/else                | BC       |           641 |
| @vars if/else                | NTTP    |       **639** |
| Large data 1000users         | BC       |      **55660** |
| Large data 1000users         | NTTP    |         56156 |
| Compile cost 1000x           | BC       |        953568 |
| Long template 50placeholders | BC       |         10280 |
