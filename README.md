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

Tuist builds `OpenRenderBoxCxx` as a static library and links it into the dynamic
`OpenRenderBox.framework`. This build uses the Darwin SDK's CoreGraphics types.
Use SwiftPM for the other backend configurations.

```sh
tuist generate run --no-open --cache-profile none
tuist xcodebuild build -workspace OpenRenderBox.xcworkspace -scheme OpenRenderBox \
  -configuration Release -destination 'generic/platform=macOS' \
  -derivedDataPath .build/tuist-framework CODE_SIGNING_ALLOWED=NO
Scripts/Tests/framework-test .build/tuist-framework/Build/Products/Release/OpenRenderBox.framework
```

The framework contains the public C headers and the C++, Objective-C, and Swift
modules. The client test uses a separate copy of the bundle with no source search
paths and compiles the Swift interface.

## Source Layout

`Sources/OpenRenderBoxCxx` groups headers and implementations by feature.
`include/OpenRenderBox` contains relative symbolic links to the public C and
Objective-C headers, so each declaration has one source file. Clients keep paths such as
`<OpenRenderBox/ORBPath.h>`. Headers that share C and Objective-C declarations
use `ORB_OBJC_FOUNDATION` to guard the Objective-C declarations.

The target uses `publicHeadersPath: "include"`. The module map lists the exported
headers and preserves the Swift submodule names and language requirements.
Swift sources remain in the separate `OpenRenderBox` target.

Framework builds and template exports copy the linked header contents. The
framework client test checks that exported headers are regular files.

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
