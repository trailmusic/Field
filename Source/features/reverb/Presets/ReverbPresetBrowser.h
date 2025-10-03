#pragma once
#include <JuceHeader.h>
#include "Presets/ReverbPresetManager.h"

// ===================== ReverbPresetBrowser ===================================
/**
 * UI component for browsing and selecting reverb presets
 * Provides search, filtering, and preset application functionality
 */
class ReverbPresetBrowser : public juce::Component,
                            public juce::ListBoxModel,
                            public juce::TextEditor::Listener,
                            public juce::ComboBox::Listener
{
public:
    ReverbPresetBrowser();
    ~ReverbPresetBrowser() override = default;
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    
    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    
    // ComboBox::Listener
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
    // Preset management
    void setPresetManager(ReverbPresetManager* manager);
    void loadPresetPack(const juce::File& jsonFile);
    void refreshPresets();
    
    // Search and filtering
    void setSearchQuery(const juce::String& query);
    void setModelFilter(const juce::String& model);
    void setTagFilter(const juce::String& tag);
    void clearFilters();
    
    // Preset application
    std::function<void(int presetIndex)> onPresetSelected;
    std::function<void(const juce::String& presetName)> onPresetApplied;

private:
    // UI Components
    juce::ListBox presetList;
    juce::TextEditor searchBox;
    juce::ComboBox modelFilter;
    juce::ComboBox tagFilter;
    juce::Label searchLabel;
    juce::Label modelLabel;
    juce::Label tagLabel;
    juce::TextButton refreshButton;
    juce::TextButton clearButton;
    
    // Data
    ReverbPresetManager* presetManager = nullptr;
    juce::Array<int> filteredPresets;
    juce::String currentSearchQuery;
    juce::String currentModelFilter;
    juce::String currentTagFilter;
    
    // Helper methods
    void updateFilteredPresets();
    void populateModelFilter();
    void populateTagFilter();
    juce::String formatPresetInfo(int presetIndex) const;
    juce::Colour getPresetColor(int presetIndex) const;
    
    // Constants
    static constexpr int ROW_HEIGHT = 60;
    static constexpr int SEARCH_HEIGHT = 30;
    static constexpr int FILTER_HEIGHT = 30;
    static constexpr int MARGIN = 8;
};
