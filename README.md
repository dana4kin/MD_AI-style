# MD_AI-style

MD_AI-style is a small offline desktop viewer for Markdown files. It renders GitHub Flavored
Markdown in the same clean, readable style used for AI chat answers: clear heading hierarchy,
comfortable text width, styled tables, task lists, quotes, inline code and code blocks.

The application is written in C++20 with Qt 6 and the MD4C parser. Ready-to-run packages are
published for Windows, Linux and macOS.

## What it does

- Opens `.md` and `.markdown` files from the menu, command line or drag and drop.
- Automatically refreshes the document after it changes on disk.
- Supports tables, task lists, strikethrough and automatic links.
- Resolves local images relative to the Markdown document.
- Provides system, light and dark themes, zoom, search and PDF export.
- Works offline and blocks remote resources embedded in a document.
- Opens clicked web links in the system browser.

Raw HTML is displayed as text instead of being executed. JavaScript is disabled in the document
view. This protects the local viewer from active content in untrusted Markdown files.

## Downloads

Open the [latest release](https://github.com/dana4kin/MD_AI-style/releases/latest) and download the
archive for your operating system:

- `MD_AI-style-Windows-x64.zip`
- `MD_AI-style-Linux-x64.tar.gz`
- `MD_AI-style-macOS-universal.dmg`

No separate Qt installation is required.

## Use

1. Start `MD_AI-style`.
2. Press **Ctrl+O** and select a Markdown file, or drag the file into the window.
3. Use **View → Theme** to select the appearance.
4. Press **Ctrl+F** to search or **Ctrl+P** to export the rendered document to PDF.

You can also pass a file on the command line:

```text
MD_AI-style README.md
```

## Build from source

Requirements:

- CMake 3.28 or newer
- C++20 compiler
- Qt 6.5 or newer with `Qt WebEngine`
- Git, used by CMake to fetch the pinned MD4C source

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix out/MD_AI-style
```

## Rendering boundary

The renderer follows GitHub Flavored Markdown and reproduces the visual conventions of Markdown
shown in AI chat. The exact private UI renderer of any particular online service is not distributed,
so pixel-for-pixel identity with a future service update cannot be guaranteed. The bundled stylesheet
is versioned with this application, which makes offline documents stable and reproducible.

## License

MD_AI-style is licensed under the MIT License. Qt is dynamically distributed under LGPLv3; see
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
