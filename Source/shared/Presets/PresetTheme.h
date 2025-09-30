#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/FieldTheme.h"

struct PresetTheme {
	// Use FieldTheme colors for consistency
	juce::Colour bg        { 0xff101214 };  // Will be overridden by FieldTheme
	juce::Colour panel     { 0xff16191d };  // Will be overridden by FieldTheme
	juce::Colour text      { 0xffe8eaed };  // Will be overridden by FieldTheme
	juce::Colour subtext   { 0xffaeb4ba };  // Will be overridden by FieldTheme
	juce::Colour accent    { 0xff5bc7ff };  // Will be overridden by FieldTheme
	juce::Colour highlight { 0xff2b86ff };  // Will be overridden by FieldTheme
	juce::Colour chipBg    { 0x14ffffff };  // Will be overridden by FieldTheme
	juce::Colour chipText  { 0xffd7dde2 };  // Will be overridden by FieldTheme
	juce::Colour starOn    { 0xffffd166 };  // Will be overridden by FieldTheme
	juce::Colour starOff   { 0xff5a626a };  // Will be overridden by FieldTheme
	float radius = 8.f;

	// Create PresetTheme from FieldTheme for consistency
	static PresetTheme fromFieldTheme(const FieldTheme& fieldTheme) {
		PresetTheme t;
		t.bg = fieldTheme.sh;           // Use shadow color for background
		t.panel = fieldTheme.panel;     // Use panel color
		t.text = fieldTheme.text;      // Use text color
		t.subtext = fieldTheme.textMuted; // Use muted text color
		t.accent = fieldTheme.accent;   // Use accent color
		t.highlight = fieldTheme.hl;   // Use highlight color
		t.chipBg = fieldTheme.hl.withAlpha(0.08f); // Subtle highlight background
		t.chipText = fieldTheme.text;   // Use text color for chips
		t.starOn = fieldTheme.accent;   // Use accent for star on
		t.starOff = fieldTheme.textMuted; // Use muted text for star off
		t.radius = 8.f;                 // Keep consistent radius
		return t;
	}

	// Legacy method for backward compatibility
	static PresetTheme fromLookAndFeel (const juce::LookAndFeel& lnf) {
		if (auto* fieldLnf = dynamic_cast<const FieldLNF*>(&lnf)) {
			return fromFieldTheme(fieldLnf->theme);
		}
		PresetTheme t; return t; // Fallback to defaults
	}
};


