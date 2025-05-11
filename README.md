# OpenBox

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2FOpenSwiftUIProject%2FOpenBox%2Fbadge%3Ftype%3Dswift-versions)](https://swiftpackageindex.com/OpenSwiftUIProject/OpenBox)

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2FOpenSwiftUIProject%2FOpenBox%2Fbadge%3Ftype%3Dplatforms)](https://swiftpackageindex.com/OpenSwiftUIProject/OpenBox)

[![codecov](https://codecov.io/github/OpenSwiftUIProject/OpenBox/graph/badge.svg?token=1MC561FQUR)](https://codecov.io/github/OpenSwiftUIProject/OpenBox)

OpenBox is an open source implementation of Apple's Private framework - RenderBox.

RenderBox is a high performance rendering engine written in C++.

And it powers the underlying rendering of SwiftUI.

The project is for the following purposes:
- Add RB support for non-Apple platform (eg. Linux, WASI and Windows)
- Diagnose and debug RB issues on Apple platform

Currently, this project is in early development.

## Build

The current suggested toolchain to build the project is Swift 6.1 / Xcode 16.3.

## Supported platforms

| **CI Status** |
|---|
|[![Compatibility tests](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/compatibility_tests.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/compatibility_tests.yml)|
|[![macOS](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/macos.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/macos.yml)|
|[![iOS](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/ios.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/ios.yml)|
|[![Ubuntu](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenBox/actions/workflows/ubuntu.yml)|

## License

See LICENSE file - MIT
