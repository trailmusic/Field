#pragma once

// Show dev HUD if Debug OR explicitly enabled via FIELD_DEV_HUD
#if !defined(FIELD_DEV_HUD_ON)
# if defined(JUCE_DEBUG) || defined(FIELD_DEV_HUD)
#  define FIELD_DEV_HUD_ON 1
# else
#  define FIELD_DEV_HUD_ON 0
# endif
#endif


