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
	auto r = getLocalBounds().reduced(6);
	const int rowH = 20;

	// Row 1: pill + adaptive
	auto row1 = r.removeFromTop(rowH);
	rPill_     = row1.removeFromLeft(jmin(100, maxWidth_ - 56)).reduced(2);
	rAdaptive_ = row1.removeFromLeft(48).reduced(2);

	// Row 2: HP | LP chips
	auto row2 = r.removeFromTop(rowH);
	rHp_ = row2.removeFromLeft(jmin(86, maxWidth_/2 - 4)).reduced(2);
	rLp_ = row2.removeFromLeft(jmin(86, maxWidth_/2 - 4)).reduced(2);

	// Row 3: GR micro meter (optional)
	rGr_ = r.removeFromTop(rowH).reduced(2);

	expandedH_ = 6 + rowH * 3 + 6;
}

void BandDetectorHUDView::resized() { layoutRows(); }

void BandDetectorHUDView::paint(Graphics& g) {
	auto b = getLocalBounds().toFloat();
	// container
	g.setColour(Colours::black.withAlpha(0.35f)); g.fillRoundedRectangle(b, 6.f);
	g.setColour(Colours::white.withAlpha(0.08f)); g.drawRoundedRectangle(b, 6.f, 1.0f);

	// pill
	auto drawChip = [&](Rectangle<int> r, const String& text, Colour bg, bool on) {
		g.setColour((on ? bg : bg.darker(0.6f))); g.fillRoundedRectangle(r.toFloat(), 10.f);
		g.setColour(Colours::white.withAlpha(on ? 0.95f : 0.5f)); g.drawText(text, r, Justification::centred);
	};

	const auto pillText = sourceToLabel(st_.source);
	Colour cPre = Colours::grey, cPost = Colours::orange, cExt = Colours::cornflowerblue;
	Colour pillCol = cPre;
	if (st_.source == "post") pillCol = cPost;
	else if (st_.source.startsWithIgnoreCase("ext")) pillCol = cExt;
	drawChip(rPill_, pillText, pillCol, true);

	// adaptive toggle
	drawChip(rAdaptive_, "Adapt", Colours::darkslategrey, st_.adaptive);

	// HP/LP chips
	drawChip(rHp_,  "HP " + String(roundToInt(st_.hpHz)) + " Hz", Colours::darkgrey, true);
	drawChip(rLp_,  "LP " + String(st_.lpHz/1000.0f, 1) + " kHz", Colours::darkgrey, true);

	// GR mini meter (3 bars)
	auto gr = juce::jlimit(0.f, 18.f, -st_.grPreviewDb); // 0..18 dB reduction
	int lit = (int) juce::jmap(gr, 0.f, 18.f, 0.f, 3.f);
	auto rr = rGr_; rr.reduce(8, 2);
	int w = rr.getWidth()/3 - 3;
	for (int i=0;i<3;++i) {
		auto cell = rr.removeFromLeft(w);
		g.setColour(Colours::dimgrey); g.fillRoundedRectangle(cell.toFloat(), 3.f);
		g.setColour(i < lit ? Colours::red : Colours::darkred); g.fillRoundedRectangle(cell.reduced(1).toFloat(), 3.f);
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
	m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(*this).withTargetScreenArea(rPill_),
				  [this](int id){ if (onChangeSource && id>=1) onChangeSource(id-1); });
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
	juce::CallOutBox::launchAsynchronously(std::move(pop), rHp_, nullptr);
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
	juce::CallOutBox::launchAsynchronously(std::move(pop), rLp_, nullptr);
}

void BandDetectorHUDView::mouseEnter (const juce::MouseEvent&) { hover_ = true; expanded_ = true; repaint(); }
void BandDetectorHUDView::mouseExit  (const juce::MouseEvent&) { hover_ = false; if (!pinned_) expanded_ = false; repaint(); }
