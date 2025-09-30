#pragma once
#include "ThemedIconButton.h"

//------------------------------------------------------------------------------
// Simple Icon Button Classes
// These are lightweight wrapper classes for common icon button configurations
//------------------------------------------------------------------------------

class OptionsButton : public ThemedIconButton 
{ 
public: 
    OptionsButton() : ThemedIconButton(Options{ IconSystem::CogWheel, false, ThemedIconButton::Style::SolidAccentWhenOn, 3.0f, 4.0f, false }) {} 
};

class LinkButton : public ThemedIconButton 
{ 
public: 
    LinkButton() : ThemedIconButton(Options{ IconSystem::Link, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true }) {} 
};

class SnapButton : public ThemedIconButton 
{ 
public: 
    SnapButton() : ThemedIconButton(Options{ IconSystem::Snap, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, false }) {} 
};

class CopyButton : public ThemedIconButton 
{ 
public: 
    CopyButton() : ThemedIconButton(Options{ IconSystem::Save, false, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {} 
};
