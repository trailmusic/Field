#include "AttachmentManager.h"
#include "../../Core/PluginEditor.h"
#include "../../Core/PluginProcessor.h"
#include "../../Core/FieldTheme.h"
#include <algorithm>

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
    attachBypassParameters();
}

void AttachmentManager::attachImagingParameters()
{
    attachSliderParameterSafely(ParameterIDs::widthLo, editor.widthLo);
    attachSliderParameterSafely(ParameterIDs::widthMid, editor.widthMid);
    attachSliderParameterSafely(ParameterIDs::widthHi, editor.widthHi);
    attachSliderParameterSafely(ParameterIDs::xoverLoHz, editor.xoverLoHz);
    attachSliderParameterSafely(ParameterIDs::xoverHiHz, editor.xoverHiHz);
    attachSliderParameterSafely(ParameterIDs::rotationDeg, editor.rotationDeg);
    attachSliderParameterSafely(ParameterIDs::asymmetry, editor.asymmetry);
}

void AttachmentManager::attachMainControlsParameters()
{
    // Main controls
    attachSliderParameterSafely(ParameterIDs::gain, editor.gain);
    attachSliderParameterSafely(ParameterIDs::inputGain, editor.sliderManager->getInputSlider());
    attachSliderParameterSafely(ParameterIDs::outputGain, editor.sliderManager->getOutputSlider());
    attachSliderParameterSafely(ParameterIDs::mix, editor.sliderManager->getMixSlider());
    attachSliderParameterSafely(ParameterIDs::width, editor.width);
    attachSliderParameterSafely(ParameterIDs::tilt, editor.tilt);
    attachSliderParameterSafely(ParameterIDs::monoHz, editor.monoHz);
    attachSliderParameterSafely(ParameterIDs::hpHz, editor.hpHz);
    attachSliderParameterSafely(ParameterIDs::lpHz, editor.lpHz);
    attachSliderParameterSafely(ParameterIDs::satDriveDb, editor.satDrive);
    attachSliderParameterSafely(ParameterIDs::satMix, editor.satMix);
    attachSliderParameterSafely(ParameterIDs::airDb, editor.air);
    attachSliderParameterSafely(ParameterIDs::bassDb, editor.bass);
    attachSliderParameterSafely(ParameterIDs::scoop, editor.scoop);
    
    // Panning
    attachSliderParameterSafely(ParameterIDs::pan, editor.panKnob);
    attachSliderParameterSafely(ParameterIDs::panL, editor.panKnobLeft);
    attachSliderParameterSafely(ParameterIDs::panR, editor.panKnobRight);
}

void AttachmentManager::attachEQParameters()
{
    // EQ sliders
    attachSliderParameterSafely(ParameterIDs::tiltFreq, editor.tiltFreqSlider);
    attachSliderParameterSafely(ParameterIDs::scoopFreq, editor.scoopFreqSlider);
    attachSliderParameterSafely(ParameterIDs::bassFreq, editor.bassFreqSlider);
    attachSliderParameterSafely(ParameterIDs::airFreq, editor.airFreqSlider);
    attachSliderParameterSafely(ParameterIDs::eqShelfShape, editor.shelfShapeS);
    attachSliderParameterSafely(ParameterIDs::eqFilterQ, editor.filterQ);
    attachSliderParameterSafely(ParameterIDs::hpQ, editor.hpQSlider);
    attachSliderParameterSafely(ParameterIDs::lpQ, editor.lpQSlider);
    
    // EQ buttons
    attachButtonParameterSafely(ParameterIDs::tiltLinkS, editor.tiltLinkSButton);
    attachButtonParameterSafely(ParameterIDs::eqQLink, editor.qLinkButton);
}


void AttachmentManager::attachBypassParameters()
{
    // Bypass button
    attachButtonParameterSafely(ParameterIDs::bypass, editor.bypassButton);
    
    // OS mode
    attachComboBoxParameterSafely(ParameterIDs::osMode, editor.osSelect);
    
    // Mono maker
    attachComboBoxParameterSafely(ParameterIDs::monoSlopeDbOct, editor.monoSlopeChoice);
    attachButtonParameterSafely(ParameterIDs::monoAudition, editor.monoAuditionButton);
}

void AttachmentManager::detachAllParameters()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void AttachmentManager::detachParameter(const juce::String& parameterID)
{
    // Remove slider attachments for this parameter
    sliderAttachments.erase(
        std::remove_if(sliderAttachments.begin(), sliderAttachments.end(),
            [&parameterID](const SliderAttachmentInfo& info) {
                return info.parameterID == parameterID;
            }),
        sliderAttachments.end());
    
    // Remove button attachments for this parameter
    buttonAttachments.erase(
        std::remove_if(buttonAttachments.begin(), buttonAttachments.end(),
            [&parameterID](const ButtonAttachmentInfo& info) {
                return info.parameterID == parameterID;
            }),
        buttonAttachments.end());
    
    // Remove combo box attachments for this parameter
    comboBoxAttachments.erase(
        std::remove_if(comboBoxAttachments.begin(), comboBoxAttachments.end(),
            [&parameterID](const ComboBoxAttachmentInfo& info) {
                return info.parameterID == parameterID;
            }),
        comboBoxAttachments.end());
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

void AttachmentManager::attachSliderParameterSafely(const juce::String& parameterID, juce::Slider& slider)
{
    if (isParameterValid(parameterID))
    {
        SliderAttachmentInfo info;
        info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            editor.proc.apvts, parameterID, slider);
        info.parameterID = parameterID;
        sliderAttachments.push_back(std::move(info));
    }
}

void AttachmentManager::attachButtonParameterSafely(const juce::String& parameterID, juce::Button& button)
{
    if (isParameterValid(parameterID))
    {
        ButtonAttachmentInfo info;
        info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            editor.proc.apvts, parameterID, button);
        info.parameterID = parameterID;
        buttonAttachments.push_back(std::move(info));
    }
}

void AttachmentManager::attachComboBoxParameterSafely(const juce::String& parameterID, juce::ComboBox& comboBox)
{
    if (isParameterValid(parameterID))
    {
        ComboBoxAttachmentInfo info;
        info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            editor.proc.apvts, parameterID, comboBox);
        info.parameterID = parameterID;
        comboBoxAttachments.push_back(std::move(info));
    }
}

void AttachmentManager::createSliderAttachment(const juce::String& parameterID, juce::Slider& slider)
{
    SliderAttachmentInfo info;
    info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        editor.proc.apvts, parameterID, slider);
    info.parameterID = parameterID;
    sliderAttachments.push_back(std::move(info));
}

void AttachmentManager::createButtonAttachment(const juce::String& parameterID, juce::Button& button)
{
    ButtonAttachmentInfo info;
    info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        editor.proc.apvts, parameterID, button);
    info.parameterID = parameterID;
    buttonAttachments.push_back(std::move(info));
}

void AttachmentManager::createComboBoxAttachment(const juce::String& parameterID, juce::ComboBox& comboBox)
{
    ComboBoxAttachmentInfo info;
    info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        editor.proc.apvts, parameterID, comboBox);
    info.parameterID = parameterID;
    comboBoxAttachments.push_back(std::move(info));
}

bool AttachmentManager::isParameterAttached(const juce::String& parameterID) const
{
    // Check if parameter is attached to any component
    auto sliderIt = std::find_if(sliderAttachments.begin(), sliderAttachments.end(),
        [&parameterID](const SliderAttachmentInfo& info) {
            return info.parameterID == parameterID;
        });
    
    auto buttonIt = std::find_if(buttonAttachments.begin(), buttonAttachments.end(),
        [&parameterID](const ButtonAttachmentInfo& info) {
            return info.parameterID == parameterID;
        });
    
    auto comboIt = std::find_if(comboBoxAttachments.begin(), comboBoxAttachments.end(),
        [&parameterID](const ComboBoxAttachmentInfo& info) {
            return info.parameterID == parameterID;
        });
    
    return sliderIt != sliderAttachments.end() || 
           buttonIt != buttonAttachments.end() || 
           comboIt != comboBoxAttachments.end();
}

int AttachmentManager::getAttachmentCount() const
{
    return static_cast<int>(sliderAttachments.size() + buttonAttachments.size() + comboBoxAttachments.size());
}

void AttachmentManager::logAttachmentStatus() const
{
    DBG("AttachmentManager Status:");
    DBG("  Slider attachments: " << sliderAttachments.size());
    DBG("  Button attachments: " << buttonAttachments.size());
    DBG("  ComboBox attachments: " << comboBoxAttachments.size());
    DBG("  Total attachments: " << getAttachmentCount());
    
    // Log parameter IDs for debugging
    for (const auto& info : sliderAttachments) {
        DBG("  Slider: " << info.parameterID);
    }
    for (const auto& info : buttonAttachments) {
        DBG("  Button: " << info.parameterID);
    }
    for (const auto& info : comboBoxAttachments) {
        DBG("  ComboBox: " << info.parameterID);
    }
}

void AttachmentManager::attachMotionParameters()
{
    // Motion parameters are handled by MotionControlsPane through direct attachment
    // This method is a placeholder for future centralized motion parameter management
    // Currently, MotionControlsPane manages its own attachments to avoid conflicts
    DBG("Motion parameters are managed by MotionControlsPane directly");
}
