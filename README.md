# OpenRenderBox

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2FOpenSwiftUIProject%2FOpenRenderBox%2Fbadge%3Ftype%3Dswift-versions)](https://swiftpackageindex.com/OpenSwiftUIProject/OpenRenderBox)

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2FOpenSwiftUIProject%2FOpenRenderBox%2Fbadge%3Ftype%3Dplatforms)](https://swiftpackageindex.com/OpenSwiftUIProject/OpenRenderBox)

[![codecov](https://codecov.io/github/OpenSwiftUIProject/OpenRenderBox/graph/badge.svg?token=1MC561FQUR)](https://codecov.io/github/OpenSwiftUIProject/OpenRenderBox)

OpenRenderBox is an open source implementation of Apple's Private framework - RenderBox.

RenderBox is a high performance rendering engine written in C++.

And it powers the underlying rendering of SwiftUI.

The project is for the following purposes:
- Add RB support for non-Apple platform (eg. Linux, WASI and Windows)
- Diagnose and debug RB issues on Apple platform

Currently, this project is in early development.

## Build

The current suggested toolchain to build the project is Swift 6.3.3 / Xcode 26.6.

## Shims

The `OpenRenderBoxShims` product provides Swift aliases and a C header for path APIs.
Swift clients use:

```swift
import OpenRenderBoxShims
```

C, C++, and Objective-C clients use:

```c
#include <OpenRenderBoxShims/OpenRenderBoxShims.h>
```

The C module is `OpenRenderBoxShimsCxx`; its public header uses the product name.

The package selects the public header directory for the configured backend.
Clients use `ORB` names with either backend and do not need to define
`OPENRENDERBOX_RENDERBOX` in their preprocessor settings.

## Supported platforms

| **CI Status** |
|---|
|[![Compatibility tests](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/compatibility_tests.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/compatibility_tests.yml)|
|[![macOS](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/macos.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/macos.yml)|
|[![iOS](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/ios.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/ios.yml)|
|[![Ubuntu](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/OpenSwiftUIProject/OpenRenderBox/actions/workflows/ubuntu.yml)|

## License

See LICENSE file - MIT
