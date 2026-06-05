# Template Benchmark

C++26 のテンプレート/文字列生成手法を比較するベンチマーク。

## 比較対象

| ライブラリ | 性質 | 用途 |
|------------|------|------|
| inja | ランタイム | Mustache 風テンプレート |
| glz::stencil | ランタイム | 構造体フィールドの文字列補間 |
| frozenchars | コンパイル時 | `_fs` リテラルによる定数文字列 |

## ビルド

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_OVERLAY_PORTS=<custom-ports-dir>/ports
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
```

## 結果の見方

- `BM_frozenchars_*`: コンパイル時文字列の `sv()` 呼び出しコスト（ほぼゼロ）
- `BM_inja_*`: テンプレートパース + レンダリングコスト
- `BM_glz_stencil_*`: stencil 文字列補間コスト

## 注意

- frozenchars はコンパイル時専用のため、ランタイムテンプレートと同じ条件では比較できない
- inja はパースを `State::SetUp` で1回のみ行い、レンダリングのみ計測
- glz::stencil は構造体のリフレクションを使用するため、データ構造の定義が必要
