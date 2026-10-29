## OpenRenderBoxCompatibilityTests

Run the callback and Storage suites against native OpenRenderBox:

```sh
OPENRENDERBOX_USE_LOCAL_DEPS=1 OPENRENDERBOX_RENDERBOX=0 \
OPENRENDERBOX_LIBRARY_EVOLUTION=0 swift test --filter 'PathCallbacksTests|PathStorageTests|PathRegressionTests'
```

Storage fixtures allocate separate aligned buffers. A primitive's `storage`
pointer is not a buffer that can be passed to Storage initialization APIs.
