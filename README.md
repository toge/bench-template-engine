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

| テンプレート | inja  | frozenchars | glz::stencil | injamm BC | injamm NTTP |
| ------------ | ----- | ----------- | ------------ | --------- | ----------- |
| HTML         | 15719 | 12548       | 2846         | 1256      | **1113**    |
| CSV          | 14623 | 11798       | 2146         | 1027      | **990**     |
| URL          | 6213  | 1462        | 664          | **234**   | 540         |
| JSON         | 18068 | 14634       | 2727         | **1329**  | 1389        |
| Config       | 11372 | 8224        | 1422         | 693       | **631**     |
| Markdown     | 4750  | 1024        | 619          | 356       | **352**     |

injamm は全カテゴリで最速。単純なテンプレートでは NTTP（コンパイル時パース）が BC をやや上回り、if/else を含む複雑なテンプレートでは BC が NTTP と同等かやや優れる。

### ネストパス解決（bench_paths）

| パターン                  | 方式 | CPU time (ns) |
| ------------------------- | ---- | ------------- |
| 2レベル                   | BC   | 129           |
| 3レベル                   | BC   | **63**        |
| 2レベル (runtime compile) | BC   | 291           |
| 2レベル                   | NTTP | 146           |
| 3レベル                   | NTTP | **63**        |

### @index/@first/@last ループ変数（bench_at_vars）

| パターン                     | 方式 | CPU time (ns) |
| ---------------------------- | ---- | ------------- |
| @index                       | BC   | 552           |
| @index                       | NTTP | 488           |
| @last section                | BC   | 447           |
| @last section                | NTTP | 639           |
| @vars if/else                | BC   | 639           |
| @vars if/else                | NTTP | 774           |
| Large data 1000users         | BC   | 76049         |
| Large data 1000users         | NTTP | 73932         |
| Compile cost 1000x           | BC   | 711157        |
| Long template 50placeholders | BC   | 14247         |
