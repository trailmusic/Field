#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "IconSystem.h"

struct Preset
{
    juce::String name;
    juce::String category;
    juce::String subcategory;
    juce::String description;
    juce::String author;
    bool isUserPreset;
    
    // Parameter values
    std::map<juce::String, float> parameters;
    
    Preset() : isUserPreset(false) {}
    Preset(const juce::String& n, const juce::String& c, const juce::String& sc, 
           const juce::String& desc, const juce::String& auth, bool user = false)
        : name(n), category(c), subcategory(sc), description(desc), author(auth), isUserPreset(user) {}
};

class PresetManager
{
public:
    PresetManager();
    
    void loadPresets();
    void savePreset(const juce::String& name, const juce::String& category, 
                   const juce::String& subcategory, const juce::String& description);
    void deletePreset(const juce::String& name);
    void applyPreset(const juce::String& name);
    
    juce::StringArray getCategories() const;
    juce::StringArray getSubcategories(const juce::String& category) const;
    juce::StringArray getPresetNames(const juce::String& category = "", const juce::String& subcategory = "") const;
    juce::StringArray searchPresets(const juce::String& searchTerm) const;
    
    Preset getPreset(const juce::String& name) const;
    void setParameterGetter(std::function<std::map<juce::String, float>()> getter);
    void setParameterSetter(std::function<void(const juce::String&, float)> setter);
    
private:
    std::vector<Preset> presets;
    std::function<std::map<juce::String, float>()> parameterGetter;
    std::function<void(const juce::String&, float)> parameterSetter;
    
    void createDefaultPresets();
    void savePresetsToFile();
    void loadPresetsFromFile();
};

class PresetComboBox : public juce::ComboBox, public juce::ComboBox::Listener, public juce::TextEditor::Listener
{
public:
    PresetComboBox();
    
    void setPresetManager(PresetManager* manager);
    void refreshPresets();
    void setCategory(const juce::String& category);
    void setSubcategory(const juce::String& subcategory);
    void searchPresets(const juce::String& searchTerm);
    void toggleFavorite(const juce::String& presetName);
    bool isFavorite(const juce::String& presetName) const;
    void setShowFavoritesOnly(bool showFavorites);
    void setTagFilter(const juce::String& tag);
    void clearFilters();
    
    // New search integration methods
    void enterSearchMode();
    void exitSearchMode();
    bool isInSearchMode() const { return inSearchMode; }
    void updateSearchResults(const juce::String& searchText);
    
    std::function<void(const juce::String&)> onPresetSelected;
    
    // ComboBox::Listener implementation
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
    // TextEditor::Listener implementation
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    
    // Override mouse and keyboard events for search mode
    void mouseDown(const juce::MouseEvent& e) override;
    
private:
    PresetManager* presetManager;
    juce::String currentCategory;
    juce::String currentSubcategory;
    juce::String currentSearch;
    juce::String currentTagFilter;
    bool showFavoritesOnly;
    std::set<juce::String> favoritePresets;
    
    // Search mode state
    bool inSearchMode;
    juce::String selectedPresetName;
    juce::String searchPlaceholder;
    
    void updatePresetList();
    void loadFavorites();
    void saveFavorites();
    bool presetMatchesFilters(const Preset& preset) const;
};

class SavePresetButton : public juce::Button
{
public:
    SavePresetButton();
    
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override;
    
    std::function<void()> onSavePreset;
    
private:
    void mouseDown(const juce::MouseEvent& e) override;
};

// Custom preset dropdown with submenu
class PresetDropdownWithSubmenu : public juce::ComboBox, public juce::ComboBox::Listener, public juce::TextEditor::Listener
{
public:
    PresetDropdownWithSubmenu();
    ~PresetDropdownWithSubmenu() override;
    
    void setPresetManager(PresetManager* manager);
    void refreshPresets();
    void setCategory(const juce::String& category);
    void setSubcategory(const juce::String& subcategory);
    void searchPresets(const juce::String& searchTerm);
    void toggleFavorite(const juce::String& presetName);
    bool isFavorite(const juce::String& presetName) const;
    void setShowFavoritesOnly(bool showFavorites);
    void setTagFilter(const juce::String& tag);
    void clearFilters();
    
    // New search integration methods
    void enterSearchMode();
    void exitSearchMode();
    bool isInSearchMode() const { return inSearchMode; }
    void updateSearchResults(const juce::String& searchText);
    
    std::function<void(const juce::String&)> onPresetSelected;
    
    // ComboBox::Listener implementation
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
    // TextEditor::Listener implementation
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    
    // Override mouse and keyboard events for search mode
    void mouseDown(const juce::MouseEvent& e) override;
    
    // Custom paint method for visual distinction
    void paint(juce::Graphics& g) override;
    
private:
    PresetManager* presetManager;
    juce::String currentCategory;
    juce::String currentSubcategory;
    juce::String currentSearch;
    juce::String currentTagFilter;
    bool showFavoritesOnly;
    std::set<juce::String> favoritePresets;
    
    // Search mode state
    bool inSearchMode;
    juce::String selectedPresetName;
    juce::String searchPlaceholder;
    
    // Submenu components
    std::unique_ptr<juce::Component> submenuComponent;
    bool submenuVisible;
    
    void updatePresetList();
    void loadFavorites();
    void saveFavorites();
    bool presetMatchesFilters(const Preset& preset) const;
    void showSubmenu(const Preset& preset);
    void hideSubmenu();
    void createSubmenuComponent(const Preset& preset);
}; 