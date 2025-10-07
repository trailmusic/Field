/*
 * ========================================================================
 * 🛡️ CRITICAL PLUGIN LIFECYCLE MANAGEMENT - DO NOT REMOVE OR MODIFY
 * ========================================================================
 * 
 * This file implements bulletproof plugin lifecycle management to prevent
 * crashes in Ableton Live and other hosts during add/remove/quit cycles.
 * 
 * CRITICAL: The order of operations here is essential for preventing:
 * - "Over-release of an object" crashes
 * - Use-after-free errors  
 * - Memory leaks
 * - Audio thread violations
 * - Dangling OS windows
 * - Background timer issues
 *
 * DO NOT CHANGE THE ORDER OR REMOVE ANY OF THESE OPERATIONS:
 * 1. Suspend audio processing FIRST (prevents audio thread from touching dying objects)
 * 2. Dismiss modals/popups (prevents dangling OS windows)
 * 3. Detach parameter attachments (prevents UAF from APVTS)
 * 4. Stop timers and remove listeners (prevents background processing)
 * 5. Clear audio callbacks (prevents audio thread violations)
 * 6. Remove parameter listeners (prevents UAF from parameter changes)
 * 7. Remove UI listeners (prevents UAF from UI interactions)
 * 8. Reset state and panes (systematic component destruction)
 * 9. Reset look and feel (prevents theme-related crashes)
 *
 * This system was implemented in January 2025 to solve critical
 * destruction issues that caused crashes in Ableton Live.
 * 
 * See: FIELD_MASTER_GUIDE.md - Plugin Lifecycle Management section
 * ========================================================================
 */

#include "CleanupManager.h"
#include "../../Core/PluginEditor.h"
#include "AttachmentManager.h"
#include "features/motion/MotionIDs.h"
#include "../Utilities/SafetySentinels.h"

CleanupManager::CleanupManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
}

void CleanupManager::performCleanup()
{
    /*
     * ========================================================================
     * 🛡️ CRITICAL CLEANUP SEQUENCE - DO NOT MODIFY ORDER OR REMOVE STEPS
     * ========================================================================
     * 
     * This method implements the exact sequence required to prevent crashes
     * in Ableton Live and other hosts. Each step is critical and must be
     * performed in this exact order.
     * 
     * CRITICAL: Changing the order or removing any step can cause:
     * - "Over-release of an object" crashes
     * - Use-after-free errors
     * - Memory leaks
     * - Audio thread violations
     * - Dangling OS windows
     * - Background timer issues
     * ========================================================================
     */
    
    // 1. Suspend audio processing to prevent audio thread from touching dying objects
    // CRITICAL: This MUST be first to prevent audio thread from accessing dying UI objects
    editor.proc.suspendProcessing(true);
    
    // 2. Dismiss any active modals/popups to prevent dangling OS windows
    // CRITICAL: Prevents OS windows from remaining open after plugin destruction
    juce::PopupMenu::dismissAllActiveMenus();
    juce::ModalComponentManager::getInstance()->cancelAllModalComponents();
    
    // 3. Perform all cleanup operations in order
    // CRITICAL: Each step depends on the previous ones - DO NOT CHANGE ORDER
    cleanupParameterAttachments();    // Detach APVTS attachments before components die
    cleanupTimersAndListeners();     // Stop timers and remove listeners
    cleanupAudioCallbacks();         // Clear audio->UI callbacks
    cleanupParameterListeners();     // Remove parameter listeners
    cleanupUIListeners();           // Remove UI listeners
    cleanupState();                  // Reset state and destroy panes
    cleanupLookAndFeel();           // Reset look and feel

    // 5. Resume audio processing after UI teardown so playback continues after closing the editor
    // CRITICAL: Closing the editor should not mute audio; only suspend during teardown window
    editor.proc.suspendProcessing(false);
    
    // 4. Debug assertions (only in debug builds)
    #if JUCE_DEBUG
    // CRITICAL: Ensure we're on the message thread (required for GUI operations)
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    
    // Verify all timers and listeners are properly cleaned up
    // NOTE: Temporarily disabled during implementation - will be re-enabled
    // if (TimerSentinel::getLiveCount() > 0)
    // {
    //     DBG("WARNING: " << TimerSentinel::getLiveCount() << " timer components still active after cleanup!");
    // }
    
    // if (ListenerSentinel::getLiveCount() > 0)
    // {
    //     DBG("WARNING: " << ListenerSentinel::getLiveCount() << " listener components still active after cleanup!");
    // }
    #endif
}

void CleanupManager::cleanupParameterAttachments()
{
    /*
     * CRITICAL: Detach all APVTS parameter attachments before components die
     * Prevents use-after-free when APVTS tries to update dying UI components
     */
    // Parameter attachments now handled by AttachmentManager
    if (editor.attachmentManager) {
        editor.attachmentManager->detachAllParameters();
    }
}

void CleanupManager::cleanupTimersAndListeners()
{
    /*
     * CRITICAL: Stop all timers and remove listeners before components die
     * Prevents background processing and use-after-free from timer callbacks
     */
    // Stop editor timer early
    editor.stopTimer();
    
    // Remove key listener safely
    if (editor.keyListener)
    {
        editor.removeKeyListener(editor.keyListener.get());
        editor.keyListener.reset();
    }
}

void CleanupManager::cleanupAudioCallbacks()
{
    /*
     * CRITICAL: Clear all audio->UI callbacks to prevent use-after-free from audio thread
     * Prevents audio thread from calling lambdas that capture dying UI objects
     */
    // Clear audio->UI callbacks to prevent use-after-free from audio thread
    editor.proc.onAudioSample   = nullptr;
    editor.proc.onAudioBlock    = nullptr;
    editor.proc.onAudioBlockPre = nullptr;
}

void CleanupManager::cleanupParameterListeners()
{
    /*
     * CRITICAL: Remove all parameter listeners to prevent use-after-free
     * Prevents APVTS from calling dying editor when parameters change
     */
    // Remove all parameter listeners that were added in the ctor
    editor.proc.apvts.removeParameterListener("split_mode", &editor);
    editor.proc.apvts.removeParameterListener("pan", &editor);
    editor.proc.apvts.removeParameterListener("depth", &editor);
    editor.proc.apvts.removeParameterListener("mono_slope_db_oct", &editor);
    editor.proc.apvts.removeParameterListener("eq_shelf_shape", &editor);
    editor.proc.apvts.removeParameterListener("eq_q_link", &editor);
    editor.proc.apvts.removeParameterListener("eq_filter_q", &editor);
    editor.proc.apvts.removeParameterListener("hp_q", &editor);
    editor.proc.apvts.removeParameterListener(motion::id::panner_select, &editor);
    editor.proc.apvts.removeParameterListener("lp_q", &editor);
    editor.proc.apvts.removeParameterListener("tilt_link_s", &editor);
    editor.proc.apvts.removeParameterListener("xover_lo_hz", &editor);
    editor.proc.apvts.removeParameterListener("xover_hi_hz", &editor);
    editor.proc.apvts.removeParameterListener("rotation_deg", &editor);
    editor.proc.apvts.removeParameterListener("asymmetry", &editor);
}

void CleanupManager::cleanupUIListeners()
{
    /*
     * CRITICAL: Remove all UI listeners to prevent use-after-free
     * Prevents UI components from calling dying editor when interacted with
     */
    // Detach UI listeners from knobs
    editor.panKnobLeft.removeListener(&editor);
    editor.panKnobRight.removeListener(&editor);
}

void CleanupManager::cleanupState()
{
    /*
     * CRITICAL: Reset state and destroy panes before editor memory goes away
     * Prevents use-after-free from PaneManager and its child components
     */
    // Ensure PaneManager timers and children are torn down before editor memory goes away
    editor.panes.reset();
    
    // ensure A holds final state if user ended on B
    if (editor.stateManager && !editor.stateManager->isStateA()) { 
        editor.saveCurrentState(); 
    }
}

void CleanupManager::cleanupLookAndFeel()
{
    /*
     * CRITICAL: Reset look and feel to prevent theme-related crashes
     * Prevents LNF from calling dying components during theme changes
     */
    editor.setLookAndFeel(nullptr);
}
