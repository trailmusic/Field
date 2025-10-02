#pragma once

#include <JuceHeader.h>

class RangerDocsPane : public juce::Component
{
public:
    RangerDocsPane();
    ~RangerDocsPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Navigation
    juce::TextButton homeButton;
    juce::TextButton backButton;
    juce::TextButton forwardButton;
    
    // Search
    juce::TextEditor searchBox;
    juce::TextButton searchButton;
    juce::TextButton clearSearchButton;
    
    // Content
    juce::TextEditor contentView;
    juce::TextButton printButton;
    juce::TextButton exportButton;
    
    // Status
    juce::Label statusLabel;
    juce::Label breadcrumbLabel;
    
    // Navigation state
    juce::Array<juce::String> navigationHistory;
    int currentHistoryIndex = -1;
    juce::String currentPath;
    
    void navigateTo(const juce::String& path);
    void addToHistory(const juce::String& path);
    void performSearch(const juce::String& query);
    void clearSearch();
    
    // Content generation
    juce::String generateHomePage();
    juce::String generateQuickStart();
    juce::String generateUserGuide();
    juce::String generateTechnicalReference();
    juce::String generateAPIReference();
    juce::String generateTroubleshooting();
    juce::String generateFAQ();
    juce::String generateChangelog();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerDocsPane)
};