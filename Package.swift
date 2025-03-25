// swift-tools-version: 6.0

import Foundation
import PackageDescription

func envEnable(_ key: String, default defaultValue: Bool = false) -> Bool {
    guard let value = Context.environment[key] else {
        return defaultValue
    }
    if value == "1" {
        return true
    } else if value == "0" {
        return false
    } else {
        return defaultValue
    }
}

// MARK: - Env and Config

let isXcodeEnv = Context.environment["__CFBundleIdentifier"] == "com.apple.dt.Xcode"
let development = envEnable("OPENBOX_DEVELOPMENT")

let releaseVersion = Context.environment["OPENBOX_TARGET_RELEASE"].flatMap { Int($0) } ?? 2024

let swiftBinPath = Context.environment["_"] ?? "/usr/bin/swift"
let swiftBinURL = URL(fileURLWithPath: swiftBinPath)
let SDKPath = swiftBinURL.deletingLastPathComponent().deletingLastPathComponent().deletingLastPathComponent().path
let includePath = SDKPath.appending("/usr/lib/swift")

let sharedCSettings: [CSetting] = [
    .unsafeFlags(["-I", includePath], .when(platforms: .nonDarwinPlatforms)),
    .define("__COREFOUNDATION_FORSWIFTFOUNDATIONONLY__", to: "1", .when(platforms: .nonDarwinPlatforms)),
]
let sharedSwiftSettings: [SwiftSetting] = [
    .swiftLanguageMode(.v5),
]

// MARK: - Targets

let openBoxTarget = Target.target(
    name: "OpenBox",
    cSettings: sharedCSettings
)
let openBoxShimsTarget = Target.target(
    name: "OpenBoxShims",
    swiftSettings: sharedSwiftSettings
)
let openBoxTestTarget = Target.testTarget(
    name: "OpenBoxTests",
    dependencies: [
        "OpenBox",
    ],
    exclude: ["README.md"],
    swiftSettings: sharedSwiftSettings
)
let openBoxCompatibilityTestTarget = Target.testTarget(
    name: "OpenBoxCompatibilityTests",
    dependencies: [
        .product(name: "RealModule", package: "swift-numerics"),
    ],
    exclude: ["README.md"],
    swiftSettings: sharedSwiftSettings
)

// MARK: - Package

let package = Package(
    name: "OpenBox",
    products: [
        .library(name: "OpenBox", type: .dynamic, targets: ["OpenBox"]),
        .library(name: "OpenBoxShims", targets: ["OpenBoxShims"]),
    ],
    dependencies: [
        .package(url: "https://github.com/apple/swift-numerics", from: "1.0.2"),
    ],
    targets: [
        openBoxTarget,
        openBoxShimsTarget,
        
        openBoxTestTarget,
        openBoxCompatibilityTestTarget,
    ],
    cxxLanguageStandard: .cxx20
)


#if os(macOS)
let renderBoxCondtion = envEnable("OPENBOX_RENDERBOX", default: true)
#else
let renderBoxCondtion = envEnable("OPENBOX_RENDERBOX")
#endif
let useLocalDeps = envEnable("OPENBOX_USE_LOCAL_DEPS")

if renderBoxCondtion {
    let privateFrameworkRepo: Package.Dependency
    if useLocalDeps {
        privateFrameworkRepo = Package.Dependency.package(path: "../DarwinPrivateFrameworks")
    } else {
        privateFrameworkRepo = Package.Dependency.package(url: "https://github.com/OpenSwiftUIProject/DarwinPrivateFrameworks.git", branch: "main")
    }
    package.dependencies.append(privateFrameworkRepo)
    var swiftSettings: [SwiftSetting] = (openBoxShimsTarget.swiftSettings ?? [])
    swiftSettings.append(.define("OPENBOX_RENDERBOX"))
    openBoxShimsTarget.swiftSettings = swiftSettings
    openBoxShimsTarget.dependencies.append(
        .product(name: "RenderBox", package: "DarwinPrivateFrameworks")
    )
    
    let rbVersion = Context.environment["DARWIN_PRIVATE_FRAMEWORKS_TARGET_RELEASE"].flatMap { Int($0) } ?? 2024
    package.platforms = switch rbVersion {
        case 2024: [.iOS(.v18), .macOS(.v15), .macCatalyst(.v18), .tvOS(.v18), .watchOS(.v10), .visionOS(.v2)]
        case 2021: [.iOS(.v15), .macOS(.v12), .macCatalyst(.v15), .tvOS(.v15), .watchOS(.v7)]
        default: nil
    }
} else {
    openBoxShimsTarget.dependencies.append("OpenBox")
}

let compatibilityTestCondition = envEnable("OPENBOX_COMPATIBILITY_TEST")
if compatibilityTestCondition && renderBoxCondtion {
    openBoxCompatibilityTestTarget.dependencies.append(
        .product(name: "RenderBox", package: "DarwinPrivateFrameworks")
    )
    
    var swiftSettings: [SwiftSetting] = (openBoxCompatibilityTestTarget.swiftSettings ?? [])
    swiftSettings.append(.define("OPENBOX_COMPATIBILITY_TEST"))
    openBoxCompatibilityTestTarget.swiftSettings = swiftSettings
} else {
    openBoxCompatibilityTestTarget.dependencies.append("OpenBox")
}

extension [Platform] {
    static var nonDarwinPlatforms: [Platform] {
        [.linux, .android, .wasi, .openbsd, .windows]
    }
}
