#include "MeterManager.h"
#include "../../Core/PluginEditor.h"

//==============================================================================
// MeterManager Implementation
//==============================================================================

MeterManager::MeterManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor), 
      lrMeters(editor.proc, editor.lnf),
      ioMeters(editor.proc, editor.lnf),
      corrMeter(editor.proc, editor.lnf)
{
    // Initialize meters container
    metersContainer.setTitle("");
    metersContainer.setShowBorder(false);
}

void MeterManager::initializeMeters()
{
    // Add meters to their container
    metersContainer.addAndMakeVisible(ioMeters);
    metersContainer.addAndMakeVisible(lrMeters);
    metersContainer.addAndMakeVisible(corrMeter);
}

void MeterManager::layoutMeters(juce::Rectangle<int> metersArea, float s, float sv)
{
    if (metersArea.isEmpty()) return;

    // Use the full container width - no more artificial width limits
    auto containerBounds = metersContainer.getBounds();
    
    // Calculate meter widths for 5 equal sections (IO-L, IO-R, LR-L, LR-R, Corr)
    const int totalWidth = containerBounds.getWidth();
    const int sectionWidth = totalWidth / 5; // Each section gets 1/5 of container
    const int interGap   = 0; // No gap between meters
    const int outerPadX  = 0; // No outer padding
    const int outerPadY  = 0; // No outer padding

    // Calculate areas for each meter (2 sections each for IO and LR, 1 for Corr)
    const int ioWidth = sectionWidth * 2;   // IO gets 2 sections
    const int lrWidth = sectionWidth * 2;   // LR gets 2 sections  
    const int corrWidth = sectionWidth;     // Corr gets 1 section

    // Center-align the meters in the container
    const int startX = (totalWidth - (ioWidth + lrWidth + corrWidth)) / 2;
    
    auto ioArea   = juce::Rectangle<int>(startX, 0, ioWidth, containerBounds.getHeight()).reduced(outerPadX, outerPadY);
    auto lrArea   = juce::Rectangle<int>(startX + ioWidth, 0, lrWidth, containerBounds.getHeight()).reduced(outerPadX, outerPadY);
    auto corrArea = juce::Rectangle<int>(startX + ioWidth + lrWidth, 0, corrWidth, containerBounds.getHeight()).reduced(outerPadX, outerPadY);

    ioMeters.setBounds(ioArea);
    lrMeters.setBounds(lrArea);
    corrMeter.setBounds(corrArea);
}

void MeterManager::setMetersContainerBounds(juce::Rectangle<int> bounds)
{
    metersContainer.setBounds(bounds);
}

juce::Rectangle<int> MeterManager::getMetersContainerBounds() const
{
    return metersContainer.getBounds();
}

void MeterManager::cleanupMeters()
{
    // Cleanup handled by destructor
}
