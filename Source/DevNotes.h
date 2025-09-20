#pragma once

// DevNotes.h — Inline developer notes (zero-cost in release builds)
// Usage examples:
//   DEV_NOTE("PHASE", "core", "Zero/Natural use LR4 non-resonant filters.");
//   DEV_PERF("ui", "Avoid allocations in paint(); cache paths/images.");
//   DEV_SAFETY("dsp", "Clamp LP to 0.45*fs to avoid edge sensitivity.");
// Notes compile to no-ops; they are grep-friendly and carry file:line metadata.

#ifndef NDEBUG
  #define DEV_NOTES_ENABLED 1
#else
  #define DEV_NOTES_ENABLED 1 // Keep enabled in release; expands to sizeof on string literal (no codegen)
#endif

#define DEV_STR1(x) #x
#define DEV_STR(x)  DEV_STR1(x)

#if DEV_NOTES_ENABLED
  #define DEV_NOTE(TAG, OWNER, MSG) do { (void)sizeof("[DEV][" TAG "][" OWNER "][" __FILE__ ":" DEV_STR(__LINE__) "] " MSG); } while(0)
  #define DEV_PERF(OWNER, MSG)   DEV_NOTE("PERF",   OWNER, MSG)
  #define DEV_SAFETY(OWNER, MSG) DEV_NOTE("SAFETY", OWNER, MSG)
  #define DEV_AUDIO(OWNER, MSG)  DEV_NOTE("AUDIO",  OWNER, MSG)
  #define DEV_PHASE(OWNER, MSG)  DEV_NOTE("PHASE",  OWNER, MSG)
  #define DEV_TODO(OWNER, MSG)   DEV_NOTE("TODO",   OWNER, MSG)
#else
  #define DEV_NOTE(TAG, OWNER, MSG) do { } while(0)
  #define DEV_PERF(OWNER, MSG)      do { } while(0)
  #define DEV_SAFETY(OWNER, MSG)    do { } while(0)
  #define DEV_AUDIO(OWNER, MSG)     do { } while(0)
  #define DEV_PHASE(OWNER, MSG)     do { } while(0)
  #define DEV_TODO(OWNER, MSG)      do { } while(0)
#endif
