# Field Reverb — Integration Pack (Models + Presets + Wiring)

**Files:**
- `ModelMacros.h`
- `Presets_Reverb.json`

## Wiring (quick)
```cpp
#include "ModelMacros.h"
// ... build params (from APVTS/UI) ...
applyModel(params, currentModelName); // "Plate" | "Hall" | "Chamber" | "Room"
engine.setParams(params);
```

## Preset loader snippet (message thread)
See chat for full snippet; load JSON → populate UI list → on select: applyModel(), map params, DR-EQ, ToneEQ, Ducking, then refreshReportedLatency().
