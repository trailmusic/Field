#pragma once

// DevNotes.h — Inline developer notes & banners (zero-cost in release builds)
//
// Green banner standard (grep-friendly):
//   // ===== [SECTION] Short Title =====
//   // Why: one line purpose/intent
//   // Notes: optional 1–2 bullets
//
// Helper:
//   DEV_BANNER("SECTION", "Short Title", "Why one-liner");
//
#ifndef NDEBUG
  #define DEV_NOTES_ENABLED 1
#else
  #define DEV_NOTES_ENABLED 1
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
  #define DEV_BANNER(SECTION, TITLE, WHY) do { (void)sizeof("[BANNER][" SECTION "] " TITLE " — " WHY); } while(0)
#else
  #define DEV_NOTE(TAG, OWNER, MSG) do { } while(0)
  #define DEV_PERF(OWNER, MSG)      do { } while(0)
  #define DEV_SAFETY(OWNER, MSG)    do { } while(0)
  #define DEV_AUDIO(OWNER, MSG)     do { } while(0)
  #define DEV_PHASE(OWNER, MSG)     do { } while(0)
  #define DEV_TODO(OWNER, MSG)      do { } while(0)
  #define DEV_BANNER(SECTION, TITLE, WHY) do { } while(0)
#endif
