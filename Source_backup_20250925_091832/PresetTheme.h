#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct PresetTheme {
	juce::Colour bg        { 0xff101214 };
	juce::Colour panel     { 0xff16191d };
	juce::Colour text      { 0xffe8eaed };
	juce::Colour subtext   { 0xffaeb4ba };
	juce::Colour accent    { 0xff5bc7ff };
	juce::Colour highlight { 0xff2b86ff };
	juce::Colour chipBg    { 0x14ffffff };
	juce::Colour chipText  { 0xffd7dde2 };
	juce::Colour starOn    { 0xffffd166 };
	juce::Colour starOff   { 0xff5a626a };
	float radius = 8.f;

	static PresetTheme fromLookAndFeel (const juce::LookAndFeel& /*lnf*/) {
		PresetTheme t; return t;
	}
};


