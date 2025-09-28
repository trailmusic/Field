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
    
    // Handle specific button types
    if (button == &editor.bypassButton)
    {
        // Bypass button - attach to parameter
        if (auto* p = editor.proc.apvts.getParameter("bypass"))
            p->setValueNotifyingHost(button->getToggleState() ? 1.0f : 0.0f);
    }
    else if (button == &editor.colorModeButton)
    {
        // Color mode cycle (Ocean → Green → Pink → Yellow → Grey)
        using TV = ThemeVariant;
        static ThemeVariant order[] = { ThemeVariant::Ocean, ThemeVariant::Green, ThemeVariant::Pink, ThemeVariant::Yellow, ThemeVariant::Grey };
        auto currentAccent = editor.lnf.theme.accent.getARGB();
        int idx = 0;
        if (currentAccent == juce::Colour(0xFF5AA9E6).getARGB()) idx = 0; // Ocean
        else if (currentAccent == juce::Colour(0xFF5AA95A).getARGB()) idx = 1; // Green
        else if (currentAccent == juce::Colour(0xFFE91E63).getARGB()) idx = 2; // Pink
        else if (currentAccent == juce::Colour(0xFFFFC107).getARGB()) idx = 3; // Yellow
        else if (currentAccent == juce::Colour(0xFF9EA3AA).getARGB()) idx = 4; // Grey
        idx = (idx + 1) % 5;
        editor.lnf.setTheme(order[idx]);
        button->setTooltip(ThemeManager::getThemeName(order[idx]));
        // Propagate to components that cache green flag
        const bool greenNow = (order[idx] == ThemeVariant::Green);
        editor.spaceKnob.setGreenMode(greenNow);
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setGreenMode(greenNow);
        }
        editor.repaint();
    }
    else if (button == &editor.tooltipsButton)
    {
        // Tooltip assistant toggle
        editor.tooltipAssistantOn_ = button->getToggleState();
        button->repaint();
        editor.tooltipBubble.setVisible(false);
    }
    else if (button == &editor.fullScreenButton)
    {
        // Full screen toggle
        const bool on = button->getToggleState();
        if (auto* tlw = editor.getTopLevelComponent())
        {
            if (auto* rw = dynamic_cast<juce::ResizableWindow*>(tlw))
            {
                if (on)
                {
                    // Save current window bounds to restore later
                    editor.savedBounds = rw->getBounds();
                    rw->setFullScreen(true);
                }
                else
                {
                    rw->setFullScreen(false);
                    if (!editor.savedBounds.isEmpty())
                        rw->setBounds(editor.savedBounds);
                }
                return;
            }
        }
        // Fallback: if no top-level resizable window is accessible, do nothing to avoid bad states
        // Reset the toggle to off if we couldn't enter fullscreen safely
        if (on)
            button->setToggleState(false, juce::dontSendNotification);
    }
    else if (button == &editor.linkButton)
    {
        // Link button toggle
        button->setToggleState(!button->getToggleState(), juce::dontSendNotification);
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setLinked(button->getToggleState());
        }
    }
    else if (button == &editor.snapButton)
    {
        // Snap button toggle
        const bool on = !button->getToggleState();
        button->setToggleState(on, juce::dontSendNotification);
        if (auto* xyTab = editor.panes->getXYTab()) {
            xyTab->setSnapEnabled(on);
        }
    }
    else if (button == &editor.presetField)
    {
        // Preset command palette
        static PresetRegistry presetRegistry; // lifetime across openings
        presetRegistry.reloadAll();
        PresetCommandPalette::show(
            presetRegistry, *button,
            // Apply
            [this](const PresetEntry& e){
                // Convert NamedValueSet to APVTS via PresetManager
                LibraryPreset tmp; tmp.meta.id = e.id; tmp.meta.name = e.name; tmp.params = e.params;
                editor.presetManager.applyPresetAtomic(tmp);
                editor.presetNameLabel.setText(e.name, juce::dontSendNotification);
            },
            // Load to slot
            [this](const PresetEntry& e, bool toA){ LibraryPreset tmp; tmp.params = e.params; editor.presetManager.loadToSlot(tmp, toA); },
            // Star (persist favorite)
            [reg=&presetRegistry](const PresetEntry& e, bool fav){ reg->setFavorite(e.id, fav); },
            // Save As
            [this](juce::String name, juce::StringArray tags, juce::String cat){ auto pr = editor.presetManager.currentAsPreset(name, cat, tags, "User preset", "", "You"); editor.presetStore.saveUserPreset(pr); editor.presetStore.scan(); },
            button->getButtonText()
        );
    }
    else if (button == &editor.abButtonA)
    {
        // A/B button A
        if (!button->getToggleState()) editor.toggleABState();
    }
    else if (button == &editor.abButtonB)
    {
        // A/B button B
        if (!button->getToggleState()) editor.toggleABState();
    }
    else if (button == &editor.copyButton)
    {
        // Copy button - show popup menu
        juce::PopupMenu m; m.addItem(1, "Copy A to B"); m.addItem(2, "Copy B to A");
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int r)
        {
            if (r == 1) { editor.copyState(true);  editor.pasteState(false); }
            if (r == 2) { editor.copyState(false); editor.pasteState(true);  }
        });
    }
    else if (button == &editor.helpButton)
    {
        // Help button - show FAQ dialog
        struct HelpFAQComponent : public juce::Component
        {
            HelpFAQComponent(FieldLNF& l) : lnf(l)
            {
                setSize(400, 300);
                addAndMakeVisible(closeButton);
                closeButton.setButtonText("Close");
                closeButton.onClick = [this] { getParentComponent()->exitModalState(0); };
            }
            void paint(juce::Graphics& g) override
            {
                g.fillAll(lnf.theme.base);
                g.setColour(lnf.theme.text);
                g.setFont(16.0f);
                g.drawText("Field Plugin - Help & FAQ", getLocalBounds().removeFromTop(40), juce::Justification::centred);
                g.setFont(14.0f);
                g.drawText("This is the Field plugin help system.\n\nFor detailed documentation, please visit the project repository.", 
                          getLocalBounds().reduced(20).removeFromTop(200), juce::Justification::topLeft);
            }
            void resized() override
            {
                closeButton.setBounds(getLocalBounds().removeFromBottom(40).reduced(20));
            }
            FieldLNF& lnf;
            juce::TextButton closeButton;
        };
        auto* helpDialog = new HelpFAQComponent(editor.lnf);
        juce::DialogWindow::showDialog("Help & FAQ", helpDialog, nullptr, juce::Colours::transparentBlack, true);
    }
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
    
    // Handle specific combo box types
    if (comboBox == &editor.osSelect)
    {
        // OS mode selection - apply options tint
        auto applyOptionsTint = [this]() {
            if (auto* p = editor.proc.apvts.getParameter("os_mode"))
            {
                if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p))
                {
                    const int idx = cp->getIndex();
                    const bool isOn = (idx > 0);
                    editor.optionsButton.setToggleState(isOn, juce::dontSendNotification);
                    editor.optionsButton.repaint();
                }
            }
        };
        applyOptionsTint();
    }
    else if (comboBox == &editor.monoSlopeChoice)
    {
        // Mono slope choice - update mono slope switch
        if (editor.monoSlopeSwitch)
        {
            const int idx = comboBox->getSelectedItemIndex();
            editor.monoSlopeSwitch->setIndex(juce::jlimit(0, 2, idx));
        }
    }
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
