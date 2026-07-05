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

以下は 2026-07-05 時点の計測値。単位は ns/op（実測 CPU time）、値が小さいほど高速。

### テンプレートレンダリング（6種）

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----: | -----------: | --------: | ----------: |
| HTML         | 11458 |         2128 |  **1153** |        1196 |
| CSV          | 10422 |         1658 |   **996** |        1038 |
| URL          |  4335 |          477 |        256 |     **246** |
| JSON         | 14367 |         1857 |       1421 |    **1333** |
| Config       |  7923 |         1052 |        526 |     **515** |
| Markdown     |  3626 |          446 |   **298** |         310 |

injamm は全カテゴリで最速。NTTP（コンパイル時パース）が URL/JSON/Config で BC を上回り、BC は HTML/CSV/Markdown でリードする。テンプレートの特性に応じた使い分けでより高い性能を引き出せる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------: |
| 2レベル                   | BC   |           116 |
| 3レベル                   | BC   |       **57** |
| 2レベル (runtime compile) | BC   |           705 |
| 2レベル                   | NTTP |           110 |
| 3レベル                   | NTTP |      **54** |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式    | CPU time (ns) |
| ---------------------------- | ------- | ------------: |
| @index                       | BC       |       **356** |
| @index                       | NTTP    |           420 |
| @index                       | runtime |          1087 |
| @last section                | BC       |       **376** |
| @last section                | NTTP    |           376 |
| @vars if/else                | BC       |       **607** |
| @vars if/else                | NTTP    |           626 |
| Large data 1000users         | BC       |      **53734** |
| Large data 1000users         | NTTP    |         54704 |
| Compile cost 1000x           | BC       |       1018790 |
| Long template 50placeholders | BC       |         10617 |
