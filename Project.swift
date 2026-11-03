import Foundation
import ProjectDescription

let sourceRoot = "Sources/OpenRenderBoxCxx"
let root = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
let sourceModuleMap = try String(
    contentsOf: root.appendingPathComponent("\(sourceRoot)/include/module.modulemap"),
    encoding: .utf8
)
let moduleHeaders = sourceModuleMap.components(separatedBy: .newlines).compactMap { line -> String? in
    guard line.contains("header \"") else { return nil }
    return line.components(separatedBy: "\"")[1]
}

// Use the same module declarations for SwiftPM and the framework.
let frameworkModuleMap = "framework " + sourceModuleMap
    .replacingOccurrences(of: "module OpenRenderBoxCxx", with: "module OpenRenderBox")
    .replacingOccurrences(of: "header \"OpenRenderBox/", with: "header \"")
    .replacingOccurrences(of: "header \"../", with: "header \"")
let moduleMapDirectory = root.appendingPathComponent("Derived/ModuleMaps")
try FileManager.default.createDirectory(at: moduleMapDirectory, withIntermediateDirectories: true)
try frameworkModuleMap.write(
    to: moduleMapDirectory.appendingPathComponent("OpenRenderBox.modulemap"),
    atomically: true,
    encoding: .utf8
)

// Preserve feature paths for the C++ and Objective-C submodules.
let internalHeaders = moduleHeaders.filter { $0.hasPrefix("../") }.map { String($0.dropFirst(3)) }
let headerCopyCommands = internalHeaders.map { header in
    let directory = (header as NSString).deletingLastPathComponent
    return """
    mkdir -p "${TARGET_BUILD_DIR}/${PUBLIC_HEADERS_FOLDER_PATH}/\(directory)"
    cp "${SRCROOT}/\(sourceRoot)/\(header)" "${TARGET_BUILD_DIR}/${PUBLIC_HEADERS_FOLDER_PATH}/\(header)"
    """
}.joined(separator: "\n")
let exportModuleHeaders = TargetScript.pre(
    script: "set -eu\n" + headerCopyCommands,
    name: "Export module headers",
    inputPaths: internalHeaders.map { .glob(.path("\(sourceRoot)/\($0)")) },
    outputPaths: internalHeaders.map { .path("$(TARGET_BUILD_DIR)/$(PUBLIC_HEADERS_FOLDER_PATH)/\($0)") },
    basedOnDependencyAnalysis: true
)

let destinations: Destinations = [.mac, .iPhone, .iPad, .macCatalyst, .appleTv, .appleWatch, .appleVision]
let deploymentTargets = DeploymentTargets.multiplatform(
    iOS: "18.0", macOS: "15.0", watchOS: "10.0", tvOS: "18.0", visionOS: "2.0"
)

let project = Project(
    name: "OpenRenderBox",
    settings: .settings(base: [
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "GCC_PREPROCESSOR_DEFINITIONS": ["$(inherited)", "OPENRENDERBOX_CF_CGTYPES"],
        "SWIFT_ACTIVE_COMPILATION_CONDITIONS": ["$(inherited)", "OPENRENDERBOX_CF_CGTYPES"],
        "SWIFT_VERSION": "5.0",
    ]),
    targets: [
        .target(
            name: "OpenRenderBoxCxx",
            destinations: destinations,
            product: .staticLibrary,
            bundleId: "org.openswiftui.OpenRenderBoxCxx",
            deploymentTargets: deploymentTargets,
            infoPlist: nil,
            buildableFolders: [
                .folder(.path(sourceRoot), exceptions: [.exception(excluded: ["include/module.modulemap", "Vector/vector.tpp"])])
            ],
            settings: .settings(base: [
                "HEADER_SEARCH_PATHS": ["$(SRCROOT)/\(sourceRoot)", "$(SRCROOT)/\(sourceRoot)/include"],
                // Match SwiftPM while path operations still have incomplete implementations.
                "GCC_WARN_ABOUT_RETURN_TYPE": "YES",
                "SKIP_INSTALL": "YES",
            ])
        ),
        .target(
            name: "OpenRenderBox",
            destinations: destinations,
            product: .framework,
            bundleId: "org.openswiftui.OpenRenderBox",
            deploymentTargets: deploymentTargets,
            buildableFolders: ["Sources/OpenRenderBox"],
            headers: .headers(public: .list([.glob(.path("\(sourceRoot)/include/OpenRenderBox/*.h"))])),
            scripts: [exportModuleHeaders],
            dependencies: [
                .target(name: "OpenRenderBoxCxx"),
                .sdk(name: "Foundation", type: .framework),
                .sdk(name: "CoreGraphics", type: .framework),
                .sdk(name: "QuartzCore", type: .framework),
                .sdk(name: "Metal", type: .framework),
                .sdk(name: "c++", type: .library),
            ],
            settings: .settings(base: [
                "BUILD_LIBRARY_FOR_DISTRIBUTION": "YES",
                // Keep Xcode responsible for installing the framework module map.
                "MODULEMAP_FILE[sdk=*]": "$(SRCROOT)/Derived/ModuleMaps/OpenRenderBox.modulemap",
                // The Swift target does not reference every exported C or Objective-C symbol.
                "OTHER_LDFLAGS": ["$(inherited)", "-force_load", "$(BUILT_PRODUCTS_DIR)/libOpenRenderBoxCxx.a"],
                "OTHER_SWIFT_FLAGS": ["$(inherited)", "-package-name", "OpenRenderBox", "-enable-upcoming-feature", "InternalImportsByDefault"],
                "SKIP_INSTALL": "NO",
                // ORBVersion.c defines the framework version symbols.
                "VERSIONING_SYSTEM": "",
            ])
        ),
    ]
)
