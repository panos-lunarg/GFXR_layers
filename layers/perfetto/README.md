# Capturing a perfetto trace on Android

- Push generated layer .so to phone and copy it to the application's folder in /data/app
- Push perfetto configuration file gfxr_events.cfg to the phone
- Start a gfxr capture as normal but enable the new layer as well. The perfetto layer must be loaded **before** the GFXR layer.
```
adb shell "setprop debug.vulkan.layers VK_LAYER_LUNARG_gfxreconstruct_perfetto_layer:VK_LAYER_LUNARG_gfxreconstruct"
adb shell "settings put global gpu_debug_layers VK_LAYER_LUNARG_gfxreconstruct_perfetto_layer:VK_LAYER_LUNARG_gfxreconstruct"
```

- Perfetto is enabled with:
```
adb shell 'cat storage/emulated/0/Download/gfxr_events.cfg | perfetto --txt -c - -o /data/misc/perfetto-traces/trace.perfetto-trace'
```

- Run application

- When done (`gfxr_events.cfg` defines capture duration 90 sec and this can be adjusted by changing the value in the file) perfetto will prompt its completion with something like this:
```
[531.990] perfetto_cmd.cc:916     Wrote 3390078 bytes into /data/misc/perfetto-traces/trace.perfetto-trace
```

- Pull file from phone and open it in the perfetto viewer
