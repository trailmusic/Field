#include "StateManager.h"
#include "../../Core/PluginEditor.h"

StateManager::StateManager(MyPluginAudioProcessorEditor& editor) : editor(editor) {}

void StateManager::saveCurrentState()
{
    std::map<juce::String, float> s;
    s["gain"]     = (float) editor.gain.getValue();
    s["width"]    = (float) editor.width.getValue();
    s["tilt"]     = (float) editor.tilt.getValue();
    s["mono"]     = (float) editor.monoHz.getValue();
    s["hp"]       = (float) editor.hpHz.getValue();
    s["lp"]       = (float) editor.lpHz.getValue();
    s["satDrive"] = (float) editor.satDrive.getValue();
    s["satMix"]   = (float) editor.satMix.getValue();
    s["air"]      = (float) editor.air.getValue();
    s["bass"]     = (float) editor.bass.getValue();
    s["scoop"]    = (float) editor.scoop.getValue();
    s["pan"]      = (float) editor.panKnob.getValue();
    
    if (isStateA_) stateA = s; else stateB = s;
}

void StateManager::loadState(bool loadStateA)
{
    auto& s = loadStateA ? stateA : stateB;
    if (auto it = s.find("gain");     it != s.end()) applyStateToSlider(editor.gain, it->second);
    if (auto it = s.find("width");     it != s.end()) applyStateToSlider(editor.width, it->second);
    if (auto it = s.find("tilt");      it != s.end()) applyStateToSlider(editor.tilt, it->second);
    if (auto it = s.find("mono");      it != s.end()) applyStateToSlider(editor.monoHz, it->second);
    if (auto it = s.find("hp");        it != s.end()) applyStateToSlider(editor.hpHz, it->second);
    if (auto it = s.find("lp");        it != s.end()) applyStateToSlider(editor.lpHz, it->second);
    if (auto it = s.find("satDrive");  it != s.end()) applyStateToSlider(editor.satDrive, it->second);
    if (auto it = s.find("satMix");    it != s.end()) applyStateToSlider(editor.satMix, it->second);
    if (auto it = s.find("air");       it != s.end()) applyStateToSlider(editor.air, it->second);
    if (auto it = s.find("bass");      it != s.end()) applyStateToSlider(editor.bass, it->second);
    if (auto it = s.find("scoop");     it != s.end()) applyStateToSlider(editor.scoop, it->second);
    if (auto it = s.find("pan");       it != s.end()) applyStateToSlider(editor.panKnob, it->second);
}

void StateManager::toggleABState()
{
    isStateA_ = !isStateA_;
    editor.abButtonA.setToggleState(isStateA_, juce::dontSendNotification);
    editor.abButtonB.setToggleState(!isStateA_, juce::dontSendNotification);
    loadState(isStateA_);
}

void StateManager::copyState(bool copyFromA)
{
    clipboardState = copyFromA ? stateA : stateB;
}

void StateManager::pasteState(bool pasteToA)
{
    if (pasteToA) stateA = clipboardState; else stateB = clipboardState;
    loadState(pasteToA);
}

void StateManager::updatePresetDisplay()
{
    // Hook to PresetManager - placeholder for now
}

void StateManager::applyStateToSlider(juce::Slider& s, float v)
{
    s.setValue(v, juce::sendNotificationSync);
}
