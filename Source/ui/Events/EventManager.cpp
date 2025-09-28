#include "EventManager.h"
#include "../../Core/PluginEditor.h"

EventManager::EventManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
}

void EventManager::handleMouseDown(const juce::MouseEvent& e)
{
    isDragging = false;
    dragStartPosition = e.getPosition();
    dragTarget = e.eventComponent;
    
    // Handle resize start - check for resize grip
    const int grip = 16;
    if (e.position.x > editor.getWidth() - grip && e.position.y > editor.getHeight() - grip)
    {
        isResizing = true;
        resizeStartBounds = editor.getBounds();
        handleResizeStarted();
    }
}

void EventManager::handleMouseDrag(const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        isDragging = true;
        
        // Handle resize drag
        if (isResizing)
        {
            auto d = e.getPosition() - dragStartPosition;
            
            // Calculate new size
            int newWidth = resizeStartBounds.getWidth() + d.x;
            int newHeight = resizeStartBounds.getHeight() + d.y;
            
            // Apply minimum size constraints
            newWidth = juce::jmax(newWidth, editor.minWidth);
            newHeight = juce::jmax(newHeight, editor.minHeight);
            
            // Apply maximum size constraints
            newWidth = juce::jmin(newWidth, editor.maxWidth);
            newHeight = juce::jmin(newHeight, editor.maxHeight);
            
            // Do not maintain aspect ratio by default; hold Shift to lock aspect
            const bool maintainAspectRatio = e.mods.isShiftDown();
            
            if (maintainAspectRatio)
            {
                const float aspectRatio = (float)editor.baseWidth / (float)editor.baseHeight;
                if (std::abs(d.x) > std::abs(d.y))
                {
                    // Width changed more, adjust height to maintain ratio
                    newHeight = (int)(newWidth / aspectRatio);
                }
                else
                {
                    // Height changed more, adjust width to maintain ratio
                    newWidth = (int)(newHeight * aspectRatio);
                }
                
                // Re-apply constraints after aspect ratio adjustment
                newWidth = juce::jlimit(editor.minWidth, editor.maxWidth, newWidth);
                newHeight = juce::jlimit(editor.minHeight, editor.maxHeight, newHeight);
                
                // If constraints broke the aspect ratio, adjust the other dimension
                const float currentRatio = (float)newWidth / (float)newHeight;
                const float targetRatio = aspectRatio;
                const float ratioError = std::abs(currentRatio - targetRatio);
                
                if (ratioError > 0.01f) // Allow small tolerance
                {
                    if (currentRatio > targetRatio)
                    {
                        // Too wide, reduce width
                        newWidth = (int)(newHeight * aspectRatio);
                        newWidth = juce::jlimit(editor.minWidth, editor.maxWidth, newWidth);
                    }
                    else
                    {
                        // Too tall, reduce height  
                        newHeight = (int)(newWidth / aspectRatio);
                        newHeight = juce::jlimit(editor.minHeight, editor.maxHeight, newHeight);
                    }
                }
            }
            
            editor.setBounds(resizeStartBounds.withSize(newWidth, newHeight));
        }
    }
}

void EventManager::handleMouseUp(const juce::MouseEvent& e)
{
    if (isDragging)
    {
        isDragging = false;
        handleResizeEnded();
    }
    
    isResizing = false;
    dragTarget = nullptr;
}

void EventManager::handleMouseMove(const juce::MouseEvent& e)
{
    if (!editor.tooltipAssistantOn_) 
    { 
        editor.tooltipBubble.setVisible(false); 
        return; 
    }
    
    auto withinHeader = editor.getLocalBounds().removeFromTop(static_cast<int>(60 * editor.scaleFactor));
    if (!withinHeader.contains(e.position.toInt())) 
    { 
        editor.tooltipBubble.setVisible(false); 
        editor.lastTooltipTarget = nullptr; 
        return; 
    }

    // Consider a few primary header controls
    juce::Component* targets[] = { &editor.colorModeButton, &editor.tooltipsButton, &editor.fullScreenButton };
    juce::String tip; 
    juce::Component* hit = nullptr;
    
    for (auto* c : targets)
    {
        auto rel = e.getEventRelativeTo(c);
        if (c->isVisible() && c->getBounds().contains(rel.getPosition().toInt())) 
        { 
            hit = c; 
            break; 
        }
    }
    
    if (hit == nullptr) 
    { 
        editor.tooltipBubble.setVisible(false); 
        editor.lastTooltipTarget = nullptr; 
        return; 
    }

    if (hit != editor.lastTooltipTarget)
    {
        editor.lastTooltipTarget = hit;
        if (hit == &editor.colorModeButton) 
            tip = "Cycle colour theme";
        else if (hit == &editor.tooltipsButton) 
            tip = "Tooltip Assistant – compact, contextual hints";
        else if (hit == &editor.fullScreenButton) 
            tip = "Toggle fullscreen";

        editor.tooltipBubble.setText(tip);
        auto anchor = hit->getBounds();
        anchor.setPosition(hit->getParentComponent()->getBounds().getPosition() + anchor.getPosition());
        editor.tooltipBubble.setAnchor(anchor);
        editor.tooltipBubble.setVisible(true);
        editor.tooltipBubble.toFront(false);
    }
}

void EventManager::handleButtonClicked(juce::Button* button)
{
    if (!button) return;
    
    // Route button events to appropriate handlers
    routeButtonEvent(button, [this, button]() {
        // Handle specific button logic
        // This will be implemented based on specific button types
    });
}

void EventManager::handleButtonStateChanged(juce::Button* button)
{
    if (!button) return;
    
    // Handle button state changes
    // This will be implemented based on specific button types
}

void EventManager::handleSliderValueChanged(juce::Slider* slider)
{
    if (!slider) return;
    
    auto set = [](juce::Label& l, const juce::String& t){ l.setText(t, juce::dontSendNotification); };
    auto Hz  = [](double v){ return juce::String(v, 1) + " Hz"; };
    auto dB  = [](double v){ return juce::String(v, 1) + " dB"; };
    auto pct = [](double v){ return juce::String(v, 0) + "%"; };

    if (slider == &editor.gain) {
        set(editor.gainValue, dB(editor.gain.getValue()));
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setGainValue((float)editor.gain.getValue());
        }
    }
    else if (slider == &editor.width) {
        const double pctVal = juce::jlimit(0.0, 1000.0, editor.width.getValue() * 100.0);
        set(editor.widthValue, juce::String(pctVal, 0) + "%");
    }
    else if (slider == &editor.tilt)   set(editor.tiltValue, juce::String(editor.tilt.getValue(), 1));
    else if (slider == &editor.monoHz) set(editor.monoValue, Hz(editor.monoHz.getValue()));
    else if (slider == &editor.hpHz)   set(editor.hpValue,   Hz(editor.hpHz.getValue()));
    else if (slider == &editor.lpHz)   set(editor.lpValue,   Hz(editor.lpHz.getValue()));
    else if (slider == &editor.satDrive) set(editor.satDriveValue, dB(editor.satDrive.getValue()));
    else if (slider == &editor.satMix)   set(editor.satMixValue, pct(juce::jmap(editor.satMix.getValue(), editor.satMix.getMinimum(), editor.satMix.getMaximum(), 0.0, 100.0)));
    else if (slider == &editor.air)    set(editor.airValue, dB(editor.air.getValue()));
    else if (slider == &editor.bass)   set(editor.bassValue, dB(editor.bass.getValue()));
    else if (slider == &editor.scoop)  {
        set(editor.scoopValue, juce::String(editor.scoop.getValue(), 1));
        editor.scoop.setName(editor.scoop.getValue() > 0.0 ? "BOOST" : "SCOOP");
    }
    else if (slider == &editor.panKnob){ 
        set(editor.panValue, juce::String(editor.panKnob.getValue(), 2)); 
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setPanValue((float)editor.panKnob.getValue());
        }
    }
    else if (slider == &editor.panKnobLeft || slider == &editor.panKnobRight)
    {
        set(editor.panValueLeft,  juce::String(editor.panKnobLeft.getValue(), 2));
        set(editor.panValueRight, juce::String(editor.panKnobRight.getValue(), 2));
        editor.panKnob.setSplitPercentage((float)juce::jmap(editor.panKnobLeft.getValue(),  -1.0, 1.0, 0.0, 100.0),
                                          (float)juce::jmap(editor.panKnobRight.getValue(), -1.0, 1.0, 0.0, 100.0));
        editor.panKnob.repaint();
    }
    else if (slider == &editor.spaceKnob)   { 
        set(editor.spaceValue, juce::String(editor.spaceKnob.getValue(), 2)); 
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setSpaceValue((float)editor.spaceKnob.getValue());
        }
    }
    else if (slider == &editor.duckingKnob) set(editor.duckingValue, juce::String(editor.duckingKnob.getValue(), 1));
    else if (slider == &editor.duckAttack)  set(editor.duckAttackValue, juce::String(editor.duckAttack.getValue(), 1));
    else if (slider == &editor.duckRelease) set(editor.duckReleaseValue, juce::String(editor.duckRelease.getValue(), 1));
    else if (slider == &editor.duckThreshold) set(editor.duckThresholdValue, juce::String(editor.duckThreshold.getValue(), 1));
    else if (slider == &editor.duckRatio)  set(editor.duckRatioValue, juce::String(editor.duckRatio.getValue(), 1));
    // duckKnee doesn't exist - removed from old reverb system
    else if (slider == &editor.widthLo)     set(editor.widthLoValue, juce::String(editor.widthLo.getValue(), 1));
    else if (slider == &editor.widthMid)    set(editor.widthMidValue, juce::String(editor.widthMid.getValue(), 1));
    else if (slider == &editor.widthHi)     set(editor.widthHiValue, juce::String(editor.widthHi.getValue(), 1));
    else if (slider == &editor.xoverLoHz)   set(editor.xoverLoValue, Hz(editor.xoverLoHz.getValue()));
    else if (slider == &editor.xoverHiHz)   set(editor.xoverHiValue, Hz(editor.xoverHiHz.getValue()));
    else if (slider == &editor.rotationDeg) set(editor.rotationValue, juce::String(editor.rotationDeg.getValue(), 1));
    else if (slider == &editor.asymmetry)   set(editor.asymValue, juce::String(editor.asymmetry.getValue(), 2));
}

void EventManager::handleSliderDragStarted(juce::Slider* slider)
{
    if (!slider) return;
    
    // Handle slider drag start
}

void EventManager::handleSliderDragEnded(juce::Slider* slider)
{
    if (!slider) return;
    
    // Handle slider drag end
}

void EventManager::handleComboBoxChanged(juce::ComboBox* comboBox)
{
    if (!comboBox) return;
    
    // Route combo box events to appropriate handlers
    routeComboBoxEvent(comboBox, [this, comboBox]() {
        // Handle combo box changes
        // This will be implemented based on specific combo box types
    });
}

void EventManager::handleKeyPressed(const juce::KeyPress& key)
{
    // Handle keyboard shortcuts
    // This will be implemented based on specific key combinations
}

void EventManager::handleKeyReleased(const juce::KeyPress& key)
{
    // Handle key release events
}

void EventManager::handleParameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle parameter changes
    // This will be implemented based on specific parameter types
}

void EventManager::handleTimerCallback()
{
    if (!editor.isShowing()) return;
    
    // Adaptive timer: burst to 60 Hz for ~150 ms after any user interaction, then 30 Hz
    const auto now = juce::Time::getMillisecondCounter();
    const bool burst = (now - editor.lastUserInteractionMs) <= 150;
    const int targetHz = burst ? 60 : 30;
    
    if (targetHz != editor.uiTimerHzCurrent)
    {
        editor.uiTimerHzCurrent = targetHz;
        editor.startTimerHz(editor.uiTimerHzCurrent);
    }
    
    // Throttle heavy UI work to reduce message-thread contention (combobox/popup lag)
    static int uiTick = 0; 
    ++uiTick;
    const bool doHeavyUi = (uiTick % 3) == 0; // ~6-7 Hz when timer is 20 Hz
    
    // If a modal component (PopupMenu/ComboBox list) is open, skip most UI work to keep interaction snappy
    if (juce::Component::getCurrentlyModalComponent() != nullptr)
    {
        return;
    }
    
    // Delegate to PaneManager for audio sample processing
    if (editor.panes)
    {
        // Get audio values from the processor's visualization buses
        // The visPost bus contains the latest audio samples for visualization
        double L = 0.0, R = 0.0;
        if (editor.proc.visPost.fifo.getNumReady() > 0)
        {
            // Read the latest samples from the visualization bus
            auto readLock = editor.proc.visPost.fifo.read(1);
            if (readLock.blockSize1 > 0)
            {
                L = editor.proc.visPost.buf.getSample(0, readLock.startIndex1);
                R = editor.proc.visPost.buf.getSample(1, readLock.startIndex1);
            }
        }
        editor.panes->onAudioSample(L, R);
    }
    
    // Update XY pad if needed
    if (doHeavyUi && editor.panes && editor.panes->getActiveID() == PaneID::XY)
    {
        if (auto* xyTab = editor.panes->getXYTab())
        {
            if (auto* xyPad = xyTab->getXYPad())
            {
                xyPad->repaint();
            }
        }
    }
}

void EventManager::handleResizeStarted()
{
    isResizing = true;
    resizeStartBounds = editor.getBounds();
}

void EventManager::handleResizeEnded()
{
    isResizing = false;
}

void EventManager::handleTooltipShow(juce::Component* target, const juce::String& text)
{
    if (target && !text.isEmpty())
    {
        currentTooltipTarget = target;
        currentTooltipText = text;
        tooltipVisible = true;
        showTooltip(target, text);
    }
}

void EventManager::handleTooltipHide()
{
    hideTooltip();
}

void EventManager::handlePresetLoad(const juce::String& presetName)
{
    // Handle preset loading
    // This will be implemented based on preset system
}

void EventManager::handlePresetSave(const juce::String& presetName)
{
    // Handle preset saving
    // This will be implemented based on preset system
}

void EventManager::handleTabChanged(int newTabIndex)
{
    // Handle tab changes
    // This will be implemented based on tab system
}

void EventManager::handleXYPadDrag(const juce::Point<float>& position)
{
    // Handle XY pad drag events
    // This will be implemented based on XY pad functionality
}

void EventManager::handleXYPadClick(const juce::Point<float>& position)
{
    // Handle XY pad click events
    // This will be implemented based on XY pad functionality
}

void EventManager::updateTooltipPosition()
{
    if (tooltipVisible && currentTooltipTarget)
    {
        // Update tooltip position
        // This will be implemented based on tooltip system
    }
}

void EventManager::hideTooltip()
{
    tooltipVisible = false;
    currentTooltipTarget = nullptr;
    currentTooltipText = juce::String();
}

void EventManager::showTooltip(juce::Component* target, const juce::String& text)
{
    // Show tooltip
    // This will be implemented based on tooltip system
}

void EventManager::bindParameterToControl(juce::Component* control, const juce::String& parameterID)
{
    // Bind parameter to control
    // This will be implemented based on parameter binding system
}

void EventManager::unbindParameterFromControl(juce::Component* control)
{
    // Unbind parameter from control
    // This will be implemented based on parameter binding system
}

void EventManager::routeMouseEvent(const juce::MouseEvent& e, const std::function<void()>& handler)
{
    // Route mouse events
    if (handler) handler();
}

void EventManager::routeButtonEvent(juce::Button* button, const std::function<void()>& handler)
{
    // Route button events
    if (handler) handler();
}

void EventManager::routeSliderEvent(juce::Slider* slider, const std::function<void()>& handler)
{
    // Route slider events
    if (handler) handler();
}

void EventManager::routeComboBoxEvent(juce::ComboBox* comboBox, const std::function<void()>& handler)
{
    // Route combo box events
    if (handler) handler();
}
