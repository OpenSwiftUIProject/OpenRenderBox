## OpenRenderBoxCompatibilityTests

Run the Path suites against native OpenRenderBox:

```sh
OPENRENDERBOX_USE_LOCAL_DEPS=1 OPENRENDERBOX_RENDERBOX=0 \
OPENRENDERBOX_LIBRARY_EVOLUTION=0 swift test --filter Path
```

Run the same public API suites against the host RenderBox:

```sh
OPENRENDERBOX_USE_LOCAL_DEPS=1 OPENRENDERBOX_RENDERBOX=1 \
OPENRENDERBOX_COMPATIBILITY_TEST=1 OPENRENDERBOX_LIBRARY_EVOLUTION=0 \
swift test --filter 'OpenRenderBoxCompatibilityTests.*Path'
```

The local dependency options require sibling OpenCoreGraphics and
DarwinPrivateFrameworks checkouts. Native Path suites run without enabling
other unfinished compatibility suites.

The exact curve-trimming fixtures
use RenderBox from iOS 18.5 (22F77).
They run against native OpenRenderBox and Apple RenderBox on iOS 18.5.
Other Apple releases skip those fixtures because their curve subdivision
can differ. The suites also cover callback cancellation, Storage lifetime,
primitive queries, arcs, and wrapper geometry.
To test the reference runtime, open `Package.swift` in Xcode with the
Apple compatibility environment above, select the
`OpenRenderBoxCompatibilityTests` package scheme, and run the Path suites on
an iOS 18.5 Simulator.

Storage fixtures allocate separate aligned buffers. A primitive's `storage`
pointer is not a buffer that can be passed to Storage initialization APIs.
