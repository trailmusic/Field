#include "AttachmentManager.h"
#include "../../Core/PluginEditor.h"
#include "../../Core/PluginProcessor.h"
#include "../../Core/FieldTheme.h"

AttachmentManager::AttachmentManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
}

void AttachmentManager::attachSliderParameter(const juce::String& parameterID, juce::Slider& slider)
{
    if (isParameterValid(parameterID))
    {
        createSliderAttachment(parameterID, slider);
    }
}

void AttachmentManager::attachButtonParameter(const juce::String& parameterID, juce::Button& button)
{
    if (isParameterValid(parameterID))
    {
        createButtonAttachment(parameterID, button);
    }
}

void AttachmentManager::attachComboBoxParameter(const juce::String& parameterID, juce::ComboBox& comboBox)
{
    if (isParameterValid(parameterID))
    {
        createComboBoxAttachment(parameterID, comboBox);
    }
}

void AttachmentManager::attachAllParameters()
{
    attachMainControlsParameters();
    attachImagingParameters();
    attachEQParameters();
    attachDelayParameters();
    attachBypassParameters();
}

void AttachmentManager::attachImagingParameters()
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    attachParameterSafely(ParameterIDs::widthLo, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::widthLo, editor.widthLo));
    });
    
    attachParameterSafely(ParameterIDs::widthMid, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::widthMid, editor.widthMid));
    });
    
    attachParameterSafely(ParameterIDs::widthHi, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::widthHi, editor.widthHi));
    });
    
    attachParameterSafely(ParameterIDs::xoverLoHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::xoverLoHz, editor.xoverLoHz));
    });
    
    attachParameterSafely(ParameterIDs::xoverHiHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::xoverHiHz, editor.xoverHiHz));
    });
    
    attachParameterSafely(ParameterIDs::rotationDeg, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::rotationDeg, editor.rotationDeg));
    });
    
    attachParameterSafely(ParameterIDs::asymmetry, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::asymmetry, editor.asymmetry));
    });
}

void AttachmentManager::attachMainControlsParameters()
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // Main controls
    attachParameterSafely(ParameterIDs::gain, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::gain, editor.gain));
    });
    
    attachParameterSafely(ParameterIDs::inputGain, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::inputGain, editor.inputSlider));
    });
    
    attachParameterSafely(ParameterIDs::outputGain, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::outputGain, editor.outputSlider));
    });
    
    attachParameterSafely(ParameterIDs::mix, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::mix, editor.mixSlider));
    });
    
    attachParameterSafely(ParameterIDs::width, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::width, editor.width));
    });
    
    attachParameterSafely(ParameterIDs::tilt, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::tilt, editor.tilt));
    });
    
    attachParameterSafely(ParameterIDs::monoHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::monoHz, editor.monoHz));
    });
    
    attachParameterSafely(ParameterIDs::hpHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::hpHz, editor.hpHz));
    });
    
    attachParameterSafely(ParameterIDs::lpHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::lpHz, editor.lpHz));
    });
    
    attachParameterSafely(ParameterIDs::satDriveDb, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::satDriveDb, editor.satDrive));
    });
    
    attachParameterSafely(ParameterIDs::satMix, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::satMix, editor.satMix));
    });
    
    attachParameterSafely(ParameterIDs::airDb, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::airDb, editor.air));
    });
    
    attachParameterSafely(ParameterIDs::bassDb, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::bassDb, editor.bass));
    });
    
    attachParameterSafely(ParameterIDs::scoop, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::scoop, editor.scoop));
    });
    
    // Reverb parameters (old system removed)
    // Ducking and space parameters were part of the old reverb system and have been removed
    
    // Panning
    attachParameterSafely(ParameterIDs::pan, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::pan, editor.panKnob));
    });
    
    attachParameterSafely(ParameterIDs::panL, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::panL, editor.panKnobLeft));
    });
    
    attachParameterSafely(ParameterIDs::panR, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::panR, editor.panKnobRight));
    });
    
    // Space parameter (old system removed)
    // The space parameter was part of the old reverb system and has been removed
}

void AttachmentManager::attachEQParameters()
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    // EQ sliders
    attachParameterSafely(ParameterIDs::tiltFreq, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::tiltFreq, editor.tiltFreqSlider));
    });
    
    attachParameterSafely(ParameterIDs::scoopFreq, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::scoopFreq, editor.scoopFreqSlider));
    });
    
    attachParameterSafely(ParameterIDs::bassFreq, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::bassFreq, editor.bassFreqSlider));
    });
    
    attachParameterSafely(ParameterIDs::airFreq, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::airFreq, editor.airFreqSlider));
    });
    
    attachParameterSafely(ParameterIDs::eqShelfShape, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::eqShelfShape, editor.shelfShapeS));
    });
    
    attachParameterSafely(ParameterIDs::eqFilterQ, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::eqFilterQ, editor.filterQ));
    });
    
    attachParameterSafely(ParameterIDs::hpQ, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::hpQ, editor.hpQSlider));
    });
    
    attachParameterSafely(ParameterIDs::lpQ, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::lpQ, editor.lpQSlider));
    });
    
    // EQ buttons
    attachParameterSafely(ParameterIDs::tiltLinkS, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::tiltLinkS, editor.tiltLinkSButton));
    });
    
    attachParameterSafely(ParameterIDs::eqQLink, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::eqQLink, editor.qLinkButton));
    });
}

void AttachmentManager::attachDelayParameters()
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Delay safety attachment helper
    auto safeDelayAttachment = [&](const char* paramId, std::function<void()> attachmentFunction) {
        if (editor.proc.apvts.getParameter(paramId) != nullptr) {
            attachmentFunction();
        }
    };
    
    // Delay parameters with safety checks
    safeDelayAttachment(ParameterIDs::delayEnabled, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delayEnabled, editor.delayEnabled));
    });
    
    safeDelayAttachment(ParameterIDs::delayMode, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::delayMode, editor.delayMode));
    });
    
    safeDelayAttachment(ParameterIDs::delaySync, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delaySync, editor.delaySync));
    });
    
    safeDelayAttachment(ParameterIDs::delayGridFlavor, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::delayGridFlavor, editor.delayGridFlavor));
    });
    
    safeDelayAttachment(ParameterIDs::delayTimeMs, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayTimeMs, editor.delayTime));
    });
    
    safeDelayAttachment(ParameterIDs::delayTimeDiv, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::delayTimeDiv, editor.delayTimeDiv));
    });
    
    safeDelayAttachment(ParameterIDs::delayFeedbackPct, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayFeedbackPct, editor.delayFeedback));
    });
    
    safeDelayAttachment(ParameterIDs::delayWet, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayWet, editor.delayWet));
    });
    
    safeDelayAttachment(ParameterIDs::delayKillDry, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delayKillDry, editor.delayKillDry));
    });
    
    safeDelayAttachment(ParameterIDs::delayFreeze, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delayFreeze, editor.delayFreeze));
    });
    
    safeDelayAttachment(ParameterIDs::delayPingpong, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delayPingpong, editor.delayPingpong));
    });
    
    safeDelayAttachment(ParameterIDs::delayCrossfeedPct, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayCrossfeedPct, editor.delaySpread));
    });
    
    safeDelayAttachment(ParameterIDs::delayStereoSpreadPct, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayStereoSpreadPct, editor.delaySpread));
    });
    
    safeDelayAttachment(ParameterIDs::delayWidth, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayWidth, editor.delayWidth));
    });
    
    safeDelayAttachment(ParameterIDs::delayModRateHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayModRateHz, editor.delayModRate));
    });
    
    safeDelayAttachment(ParameterIDs::delayModDepthMs, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayModDepthMs, editor.delayModDepth));
    });
    
    safeDelayAttachment(ParameterIDs::delayWowflutter, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayWowflutter, editor.delayWowflutter));
    });
    
    safeDelayAttachment(ParameterIDs::delayJitterPct, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayJitterPct, editor.delayJitter));
    });
    
    safeDelayAttachment(ParameterIDs::delayHpHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayHpHz, editor.delayHp));
    });
    
    safeDelayAttachment(ParameterIDs::delayLpHz, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayLpHz, editor.delayLp));
    });
    
    safeDelayAttachment(ParameterIDs::delayTiltDb, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayTiltDb, editor.delayTilt));
    });
    
    safeDelayAttachment(ParameterIDs::delaySat, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delaySat, editor.delaySat));
    });
    
    safeDelayAttachment(ParameterIDs::delayDiffusion, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayDiffusion, editor.delayDiffusion));
    });
    
    safeDelayAttachment(ParameterIDs::delayDiffuseSizeMs, [&]() {
        sliderAttachments.push_back(std::make_unique<SA>(editor.proc.apvts, ParameterIDs::delayDiffuseSizeMs, editor.delayDiffuseSize));
    });
    
    safeDelayAttachment(ParameterIDs::delayDuckSource, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::delayDuckSource, editor.delayDuckSource));
    });
    
    safeDelayAttachment(ParameterIDs::delayDuckPost, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::delayDuckPost, editor.delayDuckPost));
    });
    
    safeDelayAttachment(ParameterIDs::delayFilterType, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::delayFilterType, editor.delayFilterType));
    });
}

void AttachmentManager::attachBypassParameters()
{
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Bypass button
    attachParameterSafely(ParameterIDs::bypass, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::bypass, editor.bypassButton));
    });
    
    // OS mode
    attachParameterSafely(ParameterIDs::osMode, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::osMode, editor.osSelect));
    });
    
    // Mono maker
    attachParameterSafely(ParameterIDs::monoSlopeDbOct, [&]() {
        comboBoxAttachments.push_back(std::make_unique<CA>(editor.proc.apvts, ParameterIDs::monoSlopeDbOct, editor.monoSlopeChoice));
    });
    
    attachParameterSafely(ParameterIDs::monoAudition, [&]() {
        buttonAttachments.push_back(std::make_unique<BA>(editor.proc.apvts, ParameterIDs::monoAudition, editor.monoAuditionButton));
    });
}

void AttachmentManager::detachAllParameters()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void AttachmentManager::detachParameter(const juce::String& parameterID)
{
    // Note: JUCE attachment classes don't expose parameter ID, so we can't selectively detach
    // For now, we'll just clear all attachments when detaching a specific parameter
    // This is a limitation of the current JUCE API
    detachAllParameters();
}

bool AttachmentManager::isParameterValid(const juce::String& parameterID)
{
    return editor.proc.apvts.getParameter(parameterID) != nullptr;
}

void AttachmentManager::attachParameterSafely(const juce::String& parameterID, std::function<void()> attachmentFunction)
{
    if (isParameterValid(parameterID))
    {
        attachmentFunction();
    }
}

void AttachmentManager::createSliderAttachment(const juce::String& parameterID, juce::Slider& slider)
{
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        editor.proc.apvts, parameterID, slider));
}

void AttachmentManager::createButtonAttachment(const juce::String& parameterID, juce::Button& button)
{
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        editor.proc.apvts, parameterID, button));
}

void AttachmentManager::createComboBoxAttachment(const juce::String& parameterID, juce::ComboBox& comboBox)
{
    comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        editor.proc.apvts, parameterID, comboBox));
}
