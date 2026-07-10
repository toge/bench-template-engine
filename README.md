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
- インストールバージョン: glaze 7.8.4, injamm 2026-07-10
- injamm BC はランタイムでパース→バイトコードコンパイルを行い、その後のレンダリングのみ計測
- injamm NTTP はテンプレートをコンパイル時にパースするため、実行時のパースオーバーヘッドがゼロ

## ベンチマーク結果

以下は 2026-07-10 時点の計測値。単位は ns/op（実測 CPU time）、値が小さいほど高速。

### テンプレートレンダリング（6種）

| テンプレート |  inja | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----: | -----------: | --------: | ----------: |
| HTML         | 10659 |         1971 |  **1060** |        1071 |
| CSV          | 10218 |         1449 |     **858** |         871 |
| URL          |  3876 |          458 |     **215** |         217 |
| JSON         | 12926 |         1811 |       1313 |    **1283** |
| Config       |  7194 |          997 |        477 |     **460** |
| Markdown     |  3188 |          400 |     **233** |         236 |

injamm は全カテゴリで最速。BC（バイトコードVM）が HTML/CSV/URL/Markdown でリードし、NTTP（コンパイル時パース）が JSON/Config で BC を上回る。テンプレートの特性に応じた使い分けでより高い性能を引き出せる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------: |
| 2レベル                   | BC   |           108 |
| 3レベル                   | BC   |          50.7 |
| 2レベル (runtime compile) | BC   |           724 |
| 2レベル                   | NTTP |       **96.3** |
| 3レベル                   | NTTP |      **48.6** |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式    | CPU time (ns) |
| ---------------------------- | ------- | ------------: |
| @index                       | BC       |       **367** |
| @index                       | NTTP    |           375 |
| @index                       | runtime |           975 |
| @last section                | BC       |           404 |
| @last section                | NTTP    |       **389** |
| @vars if/else                | BC       |       **663** |
| @vars if/else                | NTTP    |           685 |
| Large data 1000users         | BC       |      **58271** |
| Large data 1000users         | NTTP    |         57536 |
| Compile cost 1000x           | BC       |        980046 |
| Long template 50placeholders | BC       |         9788 |
