#include "CleanupManager.h"
#include "../../Core/PluginEditor.h"
#include "AttachmentManager.h"
#include "features/motion/MotionIDs.h"

CleanupManager::CleanupManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
}

void CleanupManager::performCleanup()
{
    // Perform all cleanup operations in order
    cleanupParameterAttachments();
    cleanupTimersAndListeners();
    cleanupAudioCallbacks();
    cleanupParameterListeners();
    cleanupUIListeners();
    cleanupState();
    cleanupLookAndFeel();
}

void CleanupManager::cleanupParameterAttachments()
{
    // Parameter attachments now handled by AttachmentManager
    if (editor.attachmentManager) {
        editor.attachmentManager->detachAllParameters();
    }
}

void CleanupManager::cleanupTimersAndListeners()
{
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
    // Clear audio->UI callbacks to prevent use-after-free from audio thread
    editor.proc.onAudioSample   = nullptr;
    editor.proc.onAudioBlock    = nullptr;
    editor.proc.onAudioBlockPre = nullptr;
}

void CleanupManager::cleanupParameterListeners()
{
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
    // Detach UI listeners from knobs
    editor.panKnobLeft.removeListener(&editor);
    editor.panKnobRight.removeListener(&editor);
}

void CleanupManager::cleanupState()
{
    // Ensure PaneManager timers and children are torn down before editor memory goes away
    editor.panes.reset();
    
    // ensure A holds final state if user ended on B
    if (editor.stateManager && !editor.stateManager->isStateA()) { 
        editor.saveCurrentState(); 
    }
}

void CleanupManager::cleanupLookAndFeel()
{
    editor.setLookAndFeel(nullptr);
}
