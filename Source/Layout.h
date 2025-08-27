#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace Layout
{
	// Density-independent pixels
	inline int dp (float px, float scale) { return juce::roundToInt (px * scale); }

	// Rhythm
	constexpr int PAD      = 12; // outer padding
	constexpr int PAD_S    = 8;
	constexpr int PAD_L    = 16;
	constexpr int GAP      = 10; // primary gap
	constexpr int GAP_S    = 6;
	constexpr int GAP_L    = 14;

	// Corners and lines
	constexpr int RADIUS_S = 3;
	constexpr int RADIUS   = 6;
	constexpr int RADIUS_L = 8;
	constexpr int DIVIDER_THICK = 1;

	// Breakpoints
	constexpr int BP_WIDE   = 1200;
	constexpr int BP_NARROW = 880;

	// Knob sizes
	enum class Knob { S, M, L, XL };
	inline int knobPx (Knob k)
	{
		switch (k)
		{
			case Knob::S:  return 60;
			case Knob::M:  return 70;  // standard
			case Knob::L:  return 100; // space
			case Knob::XL: return 150; // pan
		}
		return 70;
	}

	// Icon buttons (ThemedIconButton, AB, arrows)
	constexpr int ICON_BTN   = 40;
	constexpr int ICON_BTN_S = 32;
	constexpr int ICON_PAD   = 4;

	// Header
	constexpr int HEADER_H         = 50;
	constexpr int HEADER_ROW_H     = 24;
	constexpr int HEADER_PRESET_W  = 400;
	constexpr int HEADER_SPLIT_W   = 120;

	// XY / meters
	constexpr int XY_MIN_H     = 300;
	constexpr int METERS_W     = 160;
	constexpr int CORR_METER_H = 140;

	// Row spacers and label bands
	constexpr int ROW_SPACER_H   = 28;
	constexpr int LABEL_BAND_TOP = 70; // space reserved inside knob rows for labels
	constexpr int LABEL_BAND_EXTRA = 20; // additional bottom band for EQ/image rows

	// Micro sliders (sub knob controls)
	constexpr int MICRO_W = 60;
	constexpr int MICRO_H = 20;

	// Special controls
	constexpr int ALGO_SWITCH_W = 56;

	// Combo & menus
	namespace Combo {
		constexpr int H        = 24;
		constexpr int W_MIN    = 160;
		constexpr int PRESET_W = HEADER_PRESET_W;
	}

	namespace Menu {
		constexpr int ITEM_H   = 24;
		constexpr int SEP_PAD  = 6;
		constexpr int CORNER_R = RADIUS;
	}

	// Sections for readability (optional use)
	enum class Section { Header, Main, VolumeRow, EqRow, ImagingRow };

	// Common sizing helpers
	inline void sizeSquare (juce::Component& c, int side) { c.setSize (side, side); }
	inline void sizeIconButton (juce::Component& c, float s) { c.setSize (dp ((float) ICON_BTN, s), dp ((float) HEADER_ROW_H, s)); }
	inline void sizeKnob (juce::Component& c, Knob k, float s)
	{
		const int px = dp ((float) knobPx (k), s);
		c.setBounds (0, 0, px, px);
	}
	inline void sizeMicro (juce::Component& c, float s)
	{
		c.setSize (dp ((float) MICRO_W, s), dp ((float) MICRO_H, s));
	}
}


