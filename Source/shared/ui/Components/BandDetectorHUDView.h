#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct DetectorHUDState {
	int bandIndex{};
	juce::String source;   // "pre"|"post"|"ext1"|"ext2"
	float hpHz{60.f};
	float lpHz{8000.f};
	bool adaptive{false};
	float grPreviewDb{0.f};
	float scOverDb{0.f};
};

class BandDetectorHUDView : public juce::Component,
                            public juce::SettableTooltipClient,
                            private juce::Timer
{
public:
	std::function<void(int newSource)> onChangeSource;   // 0..3
	std::function<void(float)>         onChangeHP;
	std::function<void(float)>         onChangeLP;
	std::function<void(bool)>          onToggleAdaptive;

	BandDetectorHUDView();

	void setState(const DetectorHUDState& s);
    void setGR(float grDb) { st_.grPreviewDb = grDb; repaint(); }
	void setPinned(bool on) { pinned_ = on; repaint(); }
	void setMaxWidth(int px) { maxWidth_ = px; resized(); }
	int  getCollapsedHeight() const { return 28; }
	int  getExpandedHeight()  const { return expanded_ ? expandedH_ : getCollapsedHeight(); }
	void show(bool on) { setVisible(on); }

	bool keyPressed(const juce::KeyPress& kp) override;

	void paint(juce::Graphics& g) override;
	void resized() override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;

private:
	void timerCallback() override { if (hover_ || pinned_) repaint(); }
	void layoutRows();
	void showSourceMenu();
	void launchHpPopover();
	void launchLpPopover();

	// UI state
	DetectorHUDState st_{};
	bool pinned_{false};
	bool expanded_{false};
	bool hover_{false};
	int  maxWidth_{200};
	int  expandedH_{64};

	// cached layout rects
	juce::Rectangle<int> rPill_, rAdaptive_, rHp_, rLp_, rGr_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandDetectorHUDView)
};
