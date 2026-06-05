# Template Benchmark

C++26 のテンプレート/文字列生成手法を比較するベンチマーク。

## 比較対象

| ライブラリ | テンプレート解析 | レンダリング | 用途 |
|------------|-----------------|--------------|------|
| inja | ランタイム | ランタイム | Mustache 風テンプレート |
| frozenchars::inja | コンパイル時 | ランタイム | Mustache 風テンプレート |
| glz::stencil | なし（リフレクション） | ランタイム | 構造体フィールドの文字列補間 |

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
```

## 結果の見方

- `BM_inja_*`: inja のテンプレートパース + レンダリングコスト
- `BM_frozenchars_*`: frozenchars::inja のレンダリングコスト（テンプレート解析はコンパイル時）
- `BM_glz_stencil_*`: glz::stencil のレンダリングコスト（テンプレート解析なし、リフレクション）

## 注意

- inja はパースを `State::SetUp` で1回のみ行い、レンダリングのみ計測
- frozenchars::inja はテンプレートをコンパイル時に解析し、実行時にコンテキストの値を解決
- glz::stencil は構造体のリフレクションを使用するため、テンプレートパースが不要
