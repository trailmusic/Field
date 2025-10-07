#include "BandDetectorHUDView.h"
using namespace juce;

static String sourceToLabel(const String& s) {
	if (s == "post") return "Post";
	if (s == "ext1") return "Ext1";
	if (s == "ext2") return "Ext2";
	return "Pre";
}

BandDetectorHUDView::BandDetectorHUDView() {
	setInterceptsMouseClicks(true, true);
	setWantsKeyboardFocus(true);
	setTooltip("Detector HUD");
	startTimerHz(60);
	addMouseListener(this, true);
}

void BandDetectorHUDView::setState(const DetectorHUDState& s) {
	st_ = s;
	layoutRows();
	repaint();
}

bool BandDetectorHUDView::keyPressed(const KeyPress& kp) {
	if      (kp.getTextCharacter() == '1' && onChangeSource) onChangeSource(0);
	else if (kp.getTextCharacter() == '2' && onChangeSource) onChangeSource(1);
	else if (kp.getTextCharacter() == '3' && onChangeSource) onChangeSource(2);
	else if (kp.getTextCharacter() == '4' && onChangeSource) onChangeSource(3);
	else if (kp == KeyPress::leftKey && onChangeHP)  onChangeHP (st_.hpHz * 0.98f);
	else if (kp == KeyPress::rightKey && onChangeHP) onChangeHP (st_.hpHz * 1.02f);
	return true;
}

void BandDetectorHUDView::layoutRows() {
    auto r = getLocalBounds().reduced(12);
    const int rowH = 28;

	// Row 1: pill + adaptive
	auto row1 = r.removeFromTop(rowH);
    rPill_     = row1.removeFromLeft(jmin(160, maxWidth_ - 96)).reduced(0);
    rAdaptive_ = row1.removeFromLeft(96).reduced(0);

	// Row 2: HP | LP chips
	auto row2 = r.removeFromTop(rowH);
    rHp_ = row2.removeFromLeft(jmin(140, maxWidth_/2 - 4)).reduced(0);
    rLp_ = row2.removeFromLeft(jmin(140, maxWidth_/2 - 4)).reduced(0);

	// Row 3: GR micro meter (optional)
	rGr_ = r.removeFromTop(rowH).reduced(2);

    expandedH_ = 12 + rowH * 3 + 12;
}

void BandDetectorHUDView::resized() { layoutRows(); }

void BandDetectorHUDView::paint(Graphics& g) {
    auto b = getLocalBounds().toFloat();
    // theme container
    auto* lf = getLF();
    auto bg   = lf ? lf->theme.drawerBg     : Colours::black.withAlpha(0.90f);
    auto bord = lf ? lf->theme.drawerBorder : Colours::white.withAlpha(0.20f);
    auto chip = lf ? lf->theme.chipBg       : Colours::darkgrey;
    auto txtP = lf ? lf->theme.textPrimary  : Colours::white.withAlpha(0.95f);
    g.setColour(bg); g.fillRoundedRectangle(b, 8.f);
    g.setColour(bord); g.drawRoundedRectangle(b, 8.f, 1.0f);

	// pill
    auto drawChip = [&](Rectangle<int> r, const String& text, Colour bg, bool on) {
        g.setColour(on ? bg : bg.darker(0.6f)); g.fillRoundedRectangle(r.toFloat(), 8.f);
        g.setColour(txtP.withAlpha(on ? 0.95f : 0.6f));
        g.setFont(Font(12.0f));
        g.drawText(text, r.reduced(10, 0), Justification::centredLeft);
	};

	const auto pillText = sourceToLabel(st_.source);
    Colour cPre = lf ? lf->theme.srcPre : Colours::grey;
    Colour cPost = lf ? lf->theme.srcPost : Colours::orange;
    Colour cExt = lf ? lf->theme.srcExt : Colours::cornflowerblue;
	Colour pillCol = cPre;
	if (st_.source == "post") pillCol = cPost;
	else if (st_.source.startsWithIgnoreCase("ext")) pillCol = cExt;
    // Asterisk when external selected but inactive
    const bool showExtInactive = st_.source.startsWithIgnoreCase("ext") && !st_.extActive;
    if (showExtInactive && lf) pillCol = lf->theme.srcExtInactive;
    drawChip(rPill_, pillText + (showExtInactive ? "*" : ""), pillCol, true);

	// adaptive toggle
    drawChip(rAdaptive_, st_.adaptive ? "Adaptive •" : "Adaptive ○", chip, st_.adaptive);

	// HP/LP chips
    drawChip(rHp_,  String("HP ") + (st_.hpHz >= 1000.0f ? String(st_.hpHz/1000.0, 1) + " kHz" : String(roundToInt(st_.hpHz)) + " Hz"), chip, true);
    drawChip(rLp_,  String("LP ") + (st_.lpHz >= 1000.0f ? String(st_.lpHz/1000.0, 1) + " kHz" : String(roundToInt(st_.lpHz)) + " Hz"), chip, true);

	// GR mini meter (3 bars)
    auto gr = juce::jlimit(0.f, 18.f, -st_.grPreviewDb); // 0..18 dB reduction
    int lit = (int) juce::jmap(gr, 0.f, 18.f, 0.f, 3.f);
	auto rr = rGr_; rr.reduce(8, 2);
	int w = rr.getWidth()/3 - 3;
	for (int i=0;i<3;++i) {
		auto cell = rr.removeFromLeft(w);
        g.setColour(lf ? lf->theme.grOff : Colours::dimgrey);
        g.fillRoundedRectangle(cell.toFloat(), 3.f);
        // Dim bars when external source inactive
        Colour bar = (i < lit ? (lf ? lf->theme.grOn : Colours::red) : (lf ? lf->theme.grOff : Colours::darkred));
        if (showExtInactive) bar = bar.withAlpha(0.45f);
        g.setColour(bar);
        g.fillRoundedRectangle(cell.reduced(1).toFloat(), 3.f);
		rr.removeFromLeft(4);
	}

	setTooltip("Source / Adaptive / Sidechain HP-LP");
}

static bool inside(const juce::Point<int>& p, const juce::Rectangle<int>& r){return r.contains(p);} 
void BandDetectorHUDView::showSourceMenu() {
	juce::PopupMenu m;
	m.addItem(1, "Pre (XY)");
	m.addItem(2, "Post (XY)");
	m.addItem(3, "External 1");
	m.addItem(4, "External 2");
    auto opts = juce::PopupMenu::Options()
                  .withTargetComponent(this)
                  .withMinimumWidth(240)
                  .withStandardItemHeight(28)
                  .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);
    m.showMenuAsync(opts, [this](int id){ if (onChangeSource && id>=1) onChangeSource(id-1); });
}

void BandDetectorHUDView::launchHpPopover() {
	struct LogHzSliderPopover : public juce::Component {
		juce::Slider s{juce::Slider::LinearHorizontal, juce::Slider::NoTextBox};
		std::function<void(float)> onChange;
		LogHzSliderPopover(float minHz, float maxHz, float startHz){
			s.setRange(minHz, maxHz, 0.01); s.setSkewFactorFromMidPoint(1000.0);
			s.setValue(startHz, juce::dontSendNotification);
			s.onValueChange = [this]{ if (onChange) onChange((float)s.getValue()); };
			addAndMakeVisible(s);
		}
		void resized() override { s.setBounds(getLocalBounds().reduced(8)); }
	};
    auto pop = std::make_unique<LogHzSliderPopover>(20.f, 2000.f, st_.hpHz);
	pop->onChange = [this](float v){ if (onChangeHP) onChangeHP(v); };
    auto& cob = juce::CallOutBox::launchAsynchronously(std::move(pop), rHp_, nullptr);
    cob.setDismissalMouseClicksAreAlwaysConsumed(true);
}

void BandDetectorHUDView::launchLpPopover() {
	struct LogHzSliderPopover : public juce::Component {
		juce::Slider s{juce::Slider::LinearHorizontal, juce::Slider::NoTextBox};
		std::function<void(float)> onChange;
		LogHzSliderPopover(float minHz, float maxHz, float startHz){
			s.setRange(minHz, maxHz, 0.01); s.setSkewFactorFromMidPoint(4000.0);
			s.setValue(startHz, juce::dontSendNotification);
			s.onValueChange = [this]{ if (onChange) onChange((float)s.getValue()); };
			addAndMakeVisible(s);
		}
		void resized() override { s.setBounds(getLocalBounds().reduced(8)); }
	};
    auto pop = std::make_unique<LogHzSliderPopover>(2000.f, 20000.f, st_.lpHz);
	pop->onChange = [this](float v){ if (onChangeLP) onChangeLP(v); };
    auto& cob = juce::CallOutBox::launchAsynchronously(std::move(pop), rLp_, nullptr);
    cob.setDismissalMouseClicksAreAlwaysConsumed(true);
}

void BandDetectorHUDView::mouseEnter (const juce::MouseEvent&) { hover_ = true; expanded_ = true; repaint(); }
void BandDetectorHUDView::mouseExit  (const juce::MouseEvent&) { hover_ = false; if (!pinned_) expanded_ = false; repaint(); }

bool BandDetectorHUDView::hitTest (int x, int y)
{
    // Only receive mouse for the pill and HP/LP/adaptive chips; pass through elsewhere
    auto p = juce::Point<int>(x,y);
    return rPill_.contains(p) || rAdaptive_.contains(p) || rHp_.contains(p) || rLp_.contains(p);
}

void BandDetectorHUDView::mouseDown (const juce::MouseEvent& e)
{
    if (rPill_.contains(e.getPosition())) { showSourceMenu(); return; }
    if (rAdaptive_.contains(e.getPosition())) { if (onToggleAdaptive) onToggleAdaptive(!st_.adaptive); return; }
    if (rHp_.contains(e.getPosition())) { launchHpPopover(); return; }
    if (rLp_.contains(e.getPosition())) { launchLpPopover(); return; }
}
