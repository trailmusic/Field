#include "MachineTab.h"
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"
#include "../../Core/PluginProcessor.h"

MachineTab::MachineTab (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf)
    : proc (p), vt (state), engine (p, p.apvts), toneCard(*dynamic_cast<FieldLNF*>(lnf)), spaceCard(*dynamic_cast<FieldLNF*>(lnf)), clarityCard(*dynamic_cast<FieldLNF*>(lnf))
{
    setOpaque (false);
    
    setLookAndFeel(lnf);

    // wrapped in cells later
    // quality/capture removed
    addAndMakeVisible (strength);
    addAndMakeVisible (proposalsContent);
    // Three cards (initially simple placeholders; wired later)
    toneCard.title = "Tone & Balance"; spaceCard.title = "Reverb, Delay, Motion"; clarityCard.title = "Clarity & Movement";
    addAndMakeVisible (toneCard); addAndMakeVisible (spaceCard); addAndMakeVisible (clarityCard);
    listenBtn.setClickingTogglesState (true);
    // Button text handled by cell captions
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        listenBtn.setLookAndFeel(lf);
    }

    // Add placeholder items as first options (these will show as "labels")
    genreBox.addItem ("Genre", 1);
    genreBox.addItem ("EDM/House", 2); genreBox.addItem ("Hip-Hop/Trap", 3); genreBox.addItem ("Pop", 4); genreBox.addItem ("Rock/Indie", 5); genreBox.addItem ("Acoustic/Jazz", 6); genreBox.addItem ("Voice/Podcast", 7);
    
    venueBox.addItem ("Venue", 1);
    venueBox.addItem ("Streaming", 2); venueBox.addItem ("Club/PA", 3); venueBox.addItem ("Theater/Cinema", 4); venueBox.addItem ("Mobile", 5);
    
    trackTypeBox.addItem ("Track Type", 1);
    trackTypeBox.addItem ("Vocal", 2);
    trackTypeBox.addItem ("Drums", 3);
    trackTypeBox.addItem ("Bass", 4);
    trackTypeBox.addItem ("Guitar/Synth", 5);
    trackTypeBox.addItem ("Keys/Piano", 6);
    trackTypeBox.addItem ("FX/Ambience", 7);
    trackTypeBox.addItem ("Mix\nBus", 8);
    trackTypeBox.addItem ("Master", 9);
    
    // Set to show placeholder items initially
    genreBox.setSelectedId (1, juce::dontSendNotification);
    venueBox.setSelectedId (1, juce::dontSendNotification);
    trackTypeBox.setSelectedId (1, juce::dontSendNotification);
    // Ensure these use tintedSelected rendering (hide default label, use accent text)
    for (juce::ComboBox* cb : { &genreBox, &venueBox, &trackTypeBox })
    {
        cb->getProperties().set ("tintedSelected", true);
        cb->getProperties().set ("disableTextWrapping", true); // Override site-wide two-word wrapping
    }
    // no quality/capture combos
    strength.setRange (0.0, 1.0, 0.01);
    strength.setNumDecimalPlacesToDisplay (0);
    strength.setTextValueSuffix (" %");
    strength.setValue (1.0, juce::dontSendNotification);
    strength.textFromValueFunction = [] (double v)
    {
        return juce::String ((int) std::round (v * 100.0)) + " %";
    };
    strength.valueFromTextFunction = [] (const juce::String& t)
    {
        auto s = t.trim();
        auto withoutPct = s.upToFirstOccurrenceOf ("%", false, false).trim();
        double pct = withoutPct.getDoubleValue();
        return juce::jlimit (0.0, 1.0, pct * 0.01);
    };
    strength.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);

    startTimerHz (30);

    // Enhanced styling: AB button-style with borders, shadowing, and raised effects
    auto styleCell = [&](juce::Button& b)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        if (lf)
        {
            // Use theme-based colors with enhanced styling
            b.setColour (juce::TextButton::buttonColourId, lf->theme.panel);
            b.setColour (juce::TextButton::buttonOnColourId, lf->theme.accent);
            b.setColour (juce::TextButton::textColourOffId, lf->theme.text.withAlpha (0.92f));
            b.setColour (juce::TextButton::textColourOnId, lf->theme.text);
            
            // Enhanced styling will be handled through properties and FieldRendering
            b.setColour (juce::TextButton::textColourOffId, lf->theme.text.withAlpha (0.92f));
            b.setColour (juce::TextButton::textColourOnId, lf->theme.text);
        }
        else
        {
            // Fallback to hardcoded colors if theme not available
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF3A3E45));
            b.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF5AA9E6));
            b.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.92f));
            b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
            // Enhanced styling will be handled through properties and FieldRendering
        }
        b.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
        
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            b.setLookAndFeel(lf);
        }
    };
    styleCell (analyzeBtn);
    styleCell (stopBtn);
    styleCell (showPreBtn);
    styleCell (listenBtn);
    
    // Style dropdown controls with theme
    auto styleDropdown = [&](juce::ComboBox& cb)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        if (lf)
        {
            cb.setColour (juce::ComboBox::backgroundColourId, lf->theme.panel);
            cb.setColour (juce::ComboBox::textColourId, lf->theme.text);
            cb.setColour (juce::ComboBox::arrowColourId, lf->theme.textMuted);
            cb.setColour (juce::ComboBox::buttonColourId, lf->theme.panel);
            cb.setColour (juce::ComboBox::outlineColourId, lf->theme.sh);
        }
        cb.setLookAndFeel(&getLookAndFeel());
    };
    styleDropdown (genreBox);
    styleDropdown (venueBox);
    styleDropdown (trackTypeBox);
    
    // Apply metallic styling to dropdowns
    setAreaMetallicForCell (genreBox, MetallicKind::Band);
    setAreaMetallicForCell (venueBox, MetallicKind::Band);
    setAreaMetallicForCell (trackTypeBox, MetallicKind::Band);
    
    // Add enhanced styling properties for dropdowns
    genreBox.getProperties().set("enhancedStyling", true);
    venueBox.getProperties().set("enhancedStyling", true);
    trackTypeBox.getProperties().set("enhancedStyling", true);
    
    // Add cursor changes for dropdowns
    genreBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    venueBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    trackTypeBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    // Style strength slider with theme
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    if (lf)
    {
        strength.setColour (juce::Slider::backgroundColourId, lf->theme.panel);
        strength.setColour (juce::Slider::thumbColourId, lf->theme.accent);
        strength.setColour (juce::Slider::trackColourId, lf->theme.hl);
        strength.setColour (juce::Slider::textBoxTextColourId, lf->theme.text);
        strength.setColour (juce::Slider::textBoxBackgroundColourId, lf->theme.panel);
        strength.setColour (juce::Slider::textBoxOutlineColourId, lf->theme.sh);
    }
    
    // Enhanced metallic rendering with AB button-style effects and icons
    setButtonStyling(analyzeBtn, MetallicKind::Band, IconSystem::Learn);
    setButtonStyling(stopBtn, MetallicKind::Band, IconSystem::Stop);
    setButtonStyling(showPreBtn, MetallicKind::Band, IconSystem::LeftArrow);
    setButtonStyling(listenBtn, MetallicKind::Band, IconSystem::Speaker);
    
    // Add enhanced styling properties for raised effect and borders
    analyzeBtn.getProperties().set("enhancedStyling", true);
    stopBtn.getProperties().set("enhancedStyling", true);
    showPreBtn.getProperties().set("enhancedStyling", true);
    listenBtn.getProperties().set("enhancedStyling", true);
    
    analyzeBtn.setTriggeredOnMouseDown (false);
    analyzeBtn.setClickingTogglesState (true); // Make Learn button toggleable
    stopBtn.setTriggeredOnMouseDown (false);
    showPreBtn.setTriggeredOnMouseDown (false);
    listenBtn.setTriggeredOnMouseDown (false);
    
    // Add cursor changes for better UX
    analyzeBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    stopBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    showPreBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    listenBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    // Set LookAndFeel for Pre and Listen buttons
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        showPreBtn.setLookAndFeel(lf);
        listenBtn.setLookAndFeel(lf);
    }
    
    // Button text will be handled by cell captions

    // Wrap Learn/Stop into small cells to match delay-style cells
    learnCell = std::make_unique<SmallSwitchCell> (analyzeBtn);
    stopCell  = std::make_unique<SmallSwitchCell> (stopBtn);
    
    // Set captions to show button text
    // learnCell->setCaption("Learn"); // Removed - Learn button uses icon only
    stopCell->setCaption("Stop");
    
    addAndMakeVisible (*learnCell);
    addAndMakeVisible (*stopCell);

    showPreBtn.setToggleState (false, juce::dontSendNotification); // default to Post
    showPreBtn.setClickingTogglesState (true);
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        showPreBtn.setLookAndFeel(lf);
    }
    preCell = std::make_unique<SmallSwitchCell> (showPreBtn);
    preCell->setCaption("Pre");
    addAndMakeVisible (*preCell);

    // Wrap remaining buttons in cells

    listenCell = std::make_unique<SmallSwitchCell> (listenBtn);
    listenCell->setCaption("Listen");
    addAndMakeVisible (*listenCell);

    // Wrap dropdowns in cells (no captions - text is inside dropdowns)
    genreCell = std::make_unique<SmallSwitchCell> (genreBox);
    addAndMakeVisible (*genreCell);

    venueCell = std::make_unique<SmallSwitchCell> (venueBox);
    addAndMakeVisible (*venueCell);

    trackTypeCell = std::make_unique<SmallSwitchCell> (trackTypeBox);
    addAndMakeVisible (*trackTypeCell);

    // Handlers
    analyzeBtn.onClick = [this]
    {
        // Push context to engine
        engine.setContext (juce::jmax(0, genreBox.getSelectedId()-1), juce::jmax(0, venueBox.getSelectedId()-1), juce::jmax(0, trackTypeBox.getSelectedId()-1));
        startLearn();
    };
    stopBtn.onClick = [this]
    {
        stopLearn (false);
    };
    showPreBtn.onClick = [this]
    {
        engine.setUsePre (showPreBtn.getToggleState());
    };
    listenBtn.onClick = [this]
    {
        if (listenBtn.getToggleState()) beginPreview(); else endPreview();
    };
    strength.onValueChange = [this]
    {
        if (listenBtn.getToggleState())
            beginPreview();
    };
    // target drop-down removed
    genreBox.onChange = [this]
    {
        vt.setProperty ("ui_machine_genre", genreBox.getSelectedId() - 1, nullptr);
    };
    venueBox.onChange = [this]
    {
        vt.setProperty ("ui_machine_venue", venueBox.getSelectedId() - 1, nullptr);
    };
    trackTypeBox.onChange = [this]
    {
        vt.setProperty ("ui_machine_tracktype", trackTypeBox.getSelectedId() - 1, nullptr);
    };
    // captureSec fixed at 60s
}

void MachineTab::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    auto bar = b.removeFromTop (56);
    // paint full-width top bar background for responsive layout
    barArea = bar.reduced (8, 4);
    paintTopBarBackground (g, barArea);
    
    // Add consistent styling for main content area
    auto r = b.toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto panel = lf ? lf->theme.meters.panelDark : juce::Colour(0xFF2A2C30);
    auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
    
    // AB Button styling: Solid panel background with elevation shadow
    const float cr = 8.0f; // Match KnobCell corner radius
    
    // Elevation shadow first (AB button style)
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
    
    // Solid panel background (no aliasing)
    g.setColour(panel);
    g.fillRoundedRectangle(r, cr);
    
    // Border (AB button style)
    g.setColour(sh);
    g.drawRoundedRectangle(r, cr, 1.0f);
}

void MachineTab::paintOverChildren (juce::Graphics& g)
{
    if (! learning) return;
    auto bounds = getLocalBounds().toFloat();

    // Snapshot cards content (children only), blur, and draw back
    juce::Rectangle<int> cardsR = bounds.toNearestInt().withTrimmedTop (56).reduced (8, 8);
    if (cardsR.getWidth() > 0 && cardsR.getHeight() > 0)
    {
        juce::Image cardsSnap (juce::Image::ARGB, cardsR.getWidth(), cardsR.getHeight(), true);
        {
            juce::Graphics g2 (cardsSnap);
            g2.fillAll (juce::Colours::transparentBlack);
            auto drawChild = [&](juce::Component& c)
            {
                if (! c.isVisible()) return;
                auto local = c.getLocalBounds();
                if (local.isEmpty()) return;
                auto s = c.createComponentSnapshot (local, true);
                if (s.isValid())
                {
                    const int dx = c.getX() - cardsR.getX();
                    const int dy = c.getY() - cardsR.getY();
                    g2.drawImageAt (s, dx, dy);
                }
            };
            drawChild (toneCard);
            drawChild (spaceCard);
            drawChild (clarityCard);
            drawChild (proposalsContent);
        }
        if (blurredCards.isNull() || blurredCards.getWidth() != cardsSnap.getWidth() || blurredCards.getHeight() != cardsSnap.getHeight())
            blurredCards = juce::Image (juce::Image::ARGB, juce::jmax (1, cardsSnap.getWidth()), juce::jmax (1, cardsSnap.getHeight()), true);
        blurredCards.clear (blurredCards.getBounds(), juce::Colours::transparentBlack);
        juce::ImageEffects::applyGaussianBlurEffect (9.0f, cardsSnap, blurredCards);
        g.setOpacity (0.95f);
        g.drawImageAt (blurredCards, cardsR.getX(), cardsR.getY());
        g.setOpacity (1.0f);
    }
    // Dim blurred cards slightly
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRect (cardsR);

    // Single large animated bar region (letterless)
    const float pad = 16.0f;
    const float cellW = bounds.getWidth() - pad * 2.0f;
    const float cellH = bounds.getHeight() * 0.34f;
    const float baseY = bounds.getCentreY() - cellH * 0.5f;
    const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const juce::Colour accent (0xFF5AA9E6);

    juce::Rectangle<float> inner (pad, baseY, cellW, cellH);
    const int bars = 42;
    const float gap = 2.0f;
    const float bw = juce::jmax (1.0f, (inner.getWidth() - (bars-1)*gap) / (float) bars);
    for (int i = 0; i < bars; ++i)
    {
        float x = inner.getX() + i * (bw + gap);
        float ph = (float) i / (float) bars;
        // Base motion (sin)
        float baseAmp = 0.40f + 0.35f * (0.5f * (1.0f + std::sin ((float) (t*2.1 + ph * juce::MathConstants<float>::twoPi + i*0.16f))));
        // Audio-react and progress sweep highlight
        const float sweep = scanPos.getCurrentValue();
        const float xNorm = juce::jlimit (0.0f, 1.0f, (x + bw*0.5f - inner.getX()) / inner.getWidth());
        const float sigma = 0.12f;
        const float hl = std::exp (-0.5f * (xNorm - sweep)*(xNorm - sweep) / (sigma*sigma));
        baseAmp += overlayLevel01 * 0.25f + 0.20f * hl;
        // Taller bars
        float amp = juce::jlimit (0.0f, 1.0f, baseAmp * 3.0f);
        float h = inner.getHeight() * juce::jlimit (0.10f, 1.0f, amp);
        float y = inner.getY() + (inner.getHeight() - h) * 0.5f;
        float mix = juce::jlimit (0.1f, 1.0f, 0.25f + 0.55f * hl);
        g.setColour (juce::Colour (0xFF3A3E45).interpolatedWith (accent, mix).withAlpha (0.95f));
        g.fillRoundedRectangle ({ x, y, bw, h }, 2.0f);
    }

    // Countdown (right-bottom)
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
    g.drawFittedText (juce::String ((int) std::ceil (learnRemaining)) + "s", bounds.toNearestInt().removeFromBottom(28).removeFromRight(60), juce::Justification::centredRight, 1);

    // Centered status overlay with gradient panel
    juce::String stat = engine.getStatus();
    if (stat.isNotEmpty())
    {
        // Dynamic panel height to accommodate live messages (above underline)
        juce::StringArray msgs = engine.getMessages();
        const int msgLines = juce::jmin (4, msgs.size());
        const int msgLineH = 16;
        const int msgBlockH = msgLines > 0 ? (msgLines * msgLineH) : 0;
        const float panelH = 84.0f + (msgBlockH > 0 ? (float) (msgBlockH + 4) : 0.0f);
        auto panel = bounds.withSizeKeepingCentre (bounds.getWidth() * 0.64f, panelH).toFloat();
        juce::Colour cTop = juce::Colours::black.withAlpha (0.35f);
        juce::Colour cBot = juce::Colours::black.withAlpha (0.20f);
        juce::ColourGradient grad (cTop, panel.getCentreX(), panel.getY(), cBot, panel.getCentreX(), panel.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (panel, 10.0f);
        g.setColour (juce::Colours::white.withAlpha (0.20f));
        g.drawRoundedRectangle (panel, 10.0f, 1.0f);

        // Title line (technical status)
        g.setColour (juce::Colours::white.withAlpha (0.96f));
        g.setFont (juce::Font (juce::FontOptions (20.0f).withStyle ("Bold")));
        g.drawFittedText (stat, panel.toNearestInt().reduced (16, 10).removeFromTop (34), juce::Justification::centred, 1);

        // Phase dots animation (center band)
        {
            // Resolve accent colour
            juce::Colour accent = juce::Colour (0xFF5AA9E6);
            const juce::Component* c = this;
            while (c)
            {
                if (auto* lf = dynamic_cast<FieldLNF*>(&c->getLookAndFeel())) { accent = lf->theme.accent; break; }
                c = c->getParentComponent();
            }
            auto mid = panel.reduced (16.0f, 10.0f).toFloat();
            mid.removeFromTop (34.0f); // after title
            // Reserve space at bottom for info + messages + underline gap
            mid.removeFromBottom ((float) (28 + msgBlockH + (msgLines > 0 ? 4 : 0) + 6));
            const int N = 7;
            const float gap = 10.0f;
            const float dotR = 3.0f;
            const float totalW = N * (dotR*2.0f) + (N-1) * gap;
            float x0 = mid.getCentreX() - totalW * 0.5f;
            float y  = mid.getCentreY();
            const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
            for (int i = 0; i < N; ++i)
            {
                float x = x0 + i * ((dotR*2.0f) + gap);
                float ph = (float) i * 0.35f + (float) t * 2.2f;
                float a  = 0.35f + 0.65f * 0.5f * (1.0f + std::sin (ph));
                float s  = 1.0f  + 0.35f * 0.5f * (1.0f + std::sin (ph + 1.57f));
                g.setColour (accent.withAlpha (juce::jlimit (0.2f, 1.0f, a)));
                g.fillEllipse (x - dotR * s, y - dotR * s, (dotR * 2.0f) * s, (dotR * 2.0f) * s);
            }
        }

        // Subline with analysis info
        juce::String info = engine.getAnalysisInfo();
        g.setColour (juce::Colours::white.withAlpha (0.80f));
        g.setFont (juce::Font (juce::FontOptions (14.0f)));
        auto content = panel.toNearestInt().reduced (16, 10);
        content.removeFromTop (34); // below title
        // Area just above underline (leave 6px for underline itself)
        auto aboveUnderline = content.removeFromBottom (28 + msgBlockH + (msgLines > 0 ? 4 : 0) + 6);
        // Draw info line at the top of this block
        auto infoR = aboveUnderline.removeFromTop (28);
        g.drawFittedText (info, infoR, juce::Justification::centred, 1);
        // Messages occupy the space directly under the info line and above the underline
        if (msgLines > 0)
        {
            g.setColour (juce::Colours::white.withAlpha (0.90f));
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            auto msgsR = aboveUnderline.removeFromTop (msgBlockH);
            const int startIdx = juce::jmax (0, msgs.size() - msgLines);
            for (int i = 0; i < msgLines; ++i)
            {
                auto lineR = msgsR.removeFromTop (msgLineH);
                g.drawFittedText (msgs[startIdx + i], lineR, juce::Justification::centred, 1);
            }
        }

        // Progress underline (scan position)
        {
            float prog = juce::jlimit (0.0f, 1.0f, scanPos.getCurrentValue());
            auto underline = panel.withY (panel.getBottom() - 6.0f).withHeight (2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.fillRoundedRectangle (underline, 1.0f);
            g.setColour (juce::Colours::white.withAlpha (0.45f));
            auto filled = underline.withWidth (underline.getWidth() * prog);
            g.fillRoundedRectangle (filled, 1.0f);
        }
    }
}

void MachineTab::resized()
{
    auto r = getLocalBounds();
    // Recompute tight bar area (keep layout independent of paint)
    auto bar = r.removeFromTop (56);
    auto ba = bar.reduced (4, 2); // Reduced padding for more control height
    // add padding inside the container
    auto ia = ba.reduced (6, 3); // Reduced internal padding
    // Responsive layout - calculate available space
    const int totalWidth = ia.getWidth();
    const int minControlWidth = 80; // Minimum width for any control
    const int dropdownWidth = juce::jmax(minControlWidth, (totalWidth - 100) / 8); // 8 controls total
    
    // Calculate total width needed for all controls
    int totalControlsWidth = 0;
    int controlCount = 0;
    if (learnCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (stopCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (preCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (venueCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (genreCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (trackTypeCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    if (listenCell) { totalControlsWidth += dropdownWidth; controlCount++; }
    totalControlsWidth += dropdownWidth; // strength slider
    
    // Add spacing between controls (6px between each, 10px for larger gaps)
    int totalSpacing = (controlCount - 1) * 6 + 10 + 8 + 8 + 10; // spacing for gaps
    totalControlsWidth += totalSpacing;
    
    // Center the controls within the available space
    int startX = (ia.getWidth() - totalControlsWidth) / 2;
    auto centeredArea = ia.withX(ia.getX() + startX);
    
    auto place = [&](juce::Component& c, int w){ c.setBounds (centeredArea.removeFromLeft (w)); centeredArea.removeFromLeft (6); };
    // Responsive layout - all controls get equal space
    if (learnCell) place (*learnCell, dropdownWidth);
    if (stopCell)  place (*stopCell, dropdownWidth);
    if (preCell)   place (*preCell, dropdownWidth); // Moved Pre to where preview was
    centeredArea.removeFromLeft (10);
    
    // Dropdowns get responsive width
    if (venueCell) place (*venueCell, dropdownWidth);
    centeredArea.removeFromLeft (8);
    if (genreCell) place (*genreCell, dropdownWidth);
    centeredArea.removeFromLeft (8);
    if (trackTypeCell) place (*trackTypeCell, dropdownWidth);
    centeredArea.removeFromLeft (10);
    
    // Strength slider and Listen button
    place (strength, dropdownWidth);
    if (listenCell) place (*listenCell, dropdownWidth);
    // content occupies the rest without scroll; place cards
    // Add 10px top and bottom padding for content
    auto content = r.withTrimmedTop (2).reduced (8, 8).reduced (0, 10);
    const int cols = content.getWidth() > 1200 ? 3 : (content.getWidth() > 800 ? 2 : 1);
    const int gutter = 12;
    if (cols == 3)
    {
        int w = (content.getWidth() - 2*gutter) / 3;
        toneCard.setBounds   (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        spaceCard.setBounds  (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        clarityCard.setBounds(content.removeFromLeft (content.getWidth()).reduced (2));
    }
    else if (cols == 2)
    {
        int w = (content.getWidth() - gutter) / 2;
        toneCard.setBounds   (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        spaceCard.setBounds  (content.removeFromLeft (content.getWidth()).reduced (2));
        // next row
        content = r.withTrimmedTop (2).reduced (8, 8);
        content.removeFromTop (toneCard.getBottom() + gutter - r.getY());
        clarityCard.setBounds(content.removeFromLeft (content.getWidth()).reduced (2));
    }
    else
    {
        int h = content.getHeight() / 3;
        toneCard.setBounds   (content.removeFromTop (h).reduced (2)); content.removeFromTop (gutter);
        spaceCard.setBounds  (content.removeFromTop (h).reduced (2)); content.removeFromTop (gutter);
        clarityCard.setBounds(content.reduced (2));
    }
    
    // Position proposalsContent to cover the main content area
    proposalsContent.setBounds(content);
}

void MachineTab::timerCallback()
{
    // When engine becomes Ready, fetch proposals once and rebuild cards
    if (engine.getState() == MachineEngine::Ready)
    {
        if (pendingProposals.empty())
        {
            auto props = engine.getProposals();
            {
                const juce::ScopedLock sl (uiLock);
                pendingProposals = std::move (props);
            }
            learning = false;
            rebuildProposalCards();
        }
    }

    if (learning)
    {
        // pull latest samples for overlay level
        const int pulled = proc.visPost.pull (overlayBuf, 1024);
        const int n = overlayBuf.getNumSamples();
        if (n > 0)
        {
            double s=0.0; const float* L = overlayBuf.getReadPointer(0); const float* R = overlayBuf.getNumChannels()>1?overlayBuf.getReadPointer(1):nullptr;
            for (int i=0;i<n;++i){ const float l=L[i], r= R?R[i]:l; s += 0.5*(l*l + r*r); }
            const float rms = (float) std::sqrt (s / juce::jmax (1, n));
            const float db = juce::Decibels::gainToDecibels (juce::jlimit (1e-6f, 1.f, rms));
            const float lev = juce::jlimit (0.f, 1.f, 1.f - (float)((-db) / 60.f));
            overlayLevel01 = overlayLevel01*0.85f + lev*0.15f;
        }
        // countdown and progress from engine directly
        learnRemaining = engine.getRemainingSeconds();
        const double prog = juce::jlimit (0.0, 1.0, (60.0 - learnRemaining) / 60.0);
        scanPos.setTargetValue ((float) prog);
        repaint();
    }
    // Learn button UI state
    // UI animation: ensure Learn button (blink/countdown) repaints periodically
    analyzeBtn.getProperties().set ("countdown_secs", learnRemaining);
    analyzeBtn.getProperties().set ("learn_active", learning);
    analyzeBtn.repaint();
}

void MachineTab::startLearn()
{
    learning = true;
    pendingProposals.clear();
    learnRemaining = 60.0;
    lastTickMs = -1.0;
    engine.startLearn (showPreBtn.getToggleState(), 60.0);
}

void MachineTab::stopLearn (bool finalize)
{
    engine.stopLearn (finalize);
    learning = false;
    learnRemaining = 0.0;
}

void MachineTab::beginPreview()
{
    // If proposals are ready, apply composite preview based on strength and non-bypassed cards
    std::vector<Proposal> active;
    {
        const juce::ScopedLock sl (uiLock);
        // Use card bypass states to filter
        auto addIf = [&](const Proposal& pr, const MachineCard& card)
        {
            if (! card.bypassed) active.push_back (pr);
        };
        // map by id to existing cards
        for (const auto& pr : pendingProposals)
        {
            if (pr.id == "tone") addIf (pr, toneCard);
            else if (pr.id == "reverb") addIf (pr, spaceCard);
            else if (pr.id == "imaging") addIf (pr, clarityCard);
        }
    }
    if (! active.empty())
        engine.applyComposite (active, (float) strength.getValue(), /*previewOnly=*/true);
}

void MachineTab::endPreview()
{
    engine.revertPreview();
}

void MachineTab::rebuildProposalCards()
{
    // Map proposals by id for quick lookup
    auto findById = [this](const juce::String& id) -> const Proposal*
    {
        for (const auto& pr : pendingProposals) if (pr.id == id) return &pr;
        return nullptr;
    };

    if (const Proposal* tone = findById ("tone"))
    {
        toneCard.title = tone->title;
        toneCard.hint  = tone->summary;
        toneCard.setMetrics (tone->metrics);
        toneCard.setParams (tone->params);
        // Build display line from spectral slope metric
        {
            juce::Array<float> a; a.ensureStorageAllocated (64);
            const float slope = (float) tone->metrics.getWithDefault ("slope_db_per_oct", 0.0f);
            const float norm = juce::jlimit (0.0f, 1.0f, (slope + 6.0f) / 6.0f);
            for (int i = 0; i < 64; ++i)
            {
                float t = (float) i / 63.0f;
                float v = juce::jlimit (0.0f, 1.0f, 0.45f + (norm - 0.5f) * 0.6f + (t - 0.5f) * 0.06f);
                a.add (v);
            }
            toneCard.displayA = a;
            juce::Array<float> ticks;
            int added = 0;
            for (const auto& d : tone->params)
            {
                if (added >= 2) break;
                const float span = juce::jmax (1e-6f, d.hi - d.lo);
                ticks.add (juce::jlimit (0.0f, 1.0f, (d.target - d.lo) / span));
                ++added;
            }
            toneCard.displayB = ticks;
        }
        toneCard.onBypass = [this, tone](bool)
        {
            if (listenBtn.getToggleState())
                beginPreview();
        };
        toneCard.repaint();
    }
    if (const Proposal* space = findById ("reverb"))
    {
        spaceCard.title = space->title;
        spaceCard.hint  = space->summary;
        spaceCard.setMetrics (space->metrics);
        spaceCard.setParams (space->params);
        // Build display line from dryness/flux/crest
        {
            juce::Array<float> a; a.ensureStorageAllocated (64);
            const float dry  = (float) space->metrics.getWithDefault ("dryness_index", 0.5f);
            const float flux = (float) space->metrics.getWithDefault ("avg_flux", 0.5f);
            const float crest= (float) space->metrics.getWithDefault ("crest", 1.0f);
            const float base = juce::jlimit (0.0f, 1.0f, 0.5f * dry + 0.2f * (1.0f - flux) + 0.1f * juce::jlimit (0.0f, 1.0f, (crest - 1.0f) * 0.5f));
            for (int i = 0; i < 64; ++i)
            {
                float t = (float) i / 63.0f;
                float v = juce::jlimit (0.0f, 1.0f, base + 0.08f * std::sin (t * juce::MathConstants<float>::twoPi));
                a.add (v);
            }
            spaceCard.displayA = a;
            juce::Array<float> ticks;
            int added = 0;
            for (const auto& d : space->params)
            {
                if (added >= 3) break;
                const float span = juce::jmax (1e-6f, d.hi - d.lo);
                ticks.add (juce::jlimit (0.0f, 1.0f, (d.target - d.lo) / span));
                ++added;
            }
            spaceCard.displayB = ticks;
        }
        spaceCard.onBypass = [this, space](bool)
        {
            if (listenBtn.getToggleState())
                beginPreview();
        };
        spaceCard.repaint();
    }
    if (const Proposal* img = findById ("imaging"))
    {
        clarityCard.title = img->title;
        clarityCard.hint  = img->summary;
        clarityCard.setMetrics (img->metrics);
        clarityCard.setParams (img->params);
        // Build display line from low/mid/hi correlation
        {
            juce::Array<float> a; a.ensureStorageAllocated (64);
            const float cL = juce::jlimit (0.0f, 1.0f, (float) img->metrics.getWithDefault ("corr_low",  0.8f));
            const float cM = juce::jlimit (0.0f, 1.0f, (float) img->metrics.getWithDefault ("corr_mid",  0.8f));
            const float cH = juce::jlimit (0.0f, 1.0f, (float) img->metrics.getWithDefault ("corr_hi",   0.8f));
            for (int i = 0; i < 64; ++i)
            {
                float t = (float) i / 63.0f;
                float seg = (t < 0.33f ? cL : (t < 0.66f ? cM : cH));
                float v = juce::jlimit (0.0f, 1.0f, 0.1f + seg * 0.8f);
                a.add (v);
            }
            clarityCard.displayA = a;
            juce::Array<float> ticks;
            int added = 0;
            for (const auto& d : img->params)
            {
                if (added >= 3) break;
                const float span = juce::jmax (1e-6f, d.hi - d.lo);
                ticks.add (juce::jlimit (0.0f, 1.0f, (d.target - d.lo) / span));
                ++added;
            }
            clarityCard.displayB = ticks;
        }
        clarityCard.onBypass = [this, img](bool)
        {
            if (listenBtn.getToggleState())
                beginPreview();
        };
        clarityCard.repaint();
    }
}

void MachineTab::applyPatches (float)
{
    // MVP stub
}

void MachineTab::paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto r = area.toFloat();
    // Space-age texture: subtle starfield + gradient
    juce::ColourGradient grad (
        juce::Colour::fromHSV (0.60f, 0.25f, 0.10f, 0.90f), r.getX(), r.getY(),
        juce::Colour::fromHSV (0.62f, 0.20f, 0.16f, 0.90f), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 10.0f);
    g.saveState();
    juce::Random rnd (0xC0FFEE);
    for (int i = 0; i < 40; ++i)
    {
        float sx = r.getX() + rnd.nextFloat() * r.getWidth();
        float sy = r.getY() + rnd.nextFloat() * r.getHeight();
        float sz = 0.8f + rnd.nextFloat() * 1.6f;
        g.setColour (juce::Colours::white.withAlpha (0.06f + rnd.nextFloat()*0.06f));
        g.fillEllipse (sx, sy, sz, sz);
    }
    g.restoreState();
    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (r.reduced (1.0f), 9.0f, 1.2f);
}