#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PresetRegistry.h"

class PresetCommandPalette : public juce::Component,
                             private juce::TextEditor::Listener,
                             private juce::ListBoxModel,
                             private juce::KeyListener
{
public:
    std::function<void(const PresetEntry&)> onApply;
    std::function<void(const PresetEntry&, bool toA)> onLoadToSlot;
    std::function<void(const PresetEntry&, bool fav)> onStar;
    std::function<void(const juce::String& name,
                       const juce::StringArray& tags,
                       const juce::String& category)> onSaveAs;

    explicit PresetCommandPalette (PresetRegistry& reg);

    static void show (PresetRegistry& reg, juce::Component& anchor,
                      std::function<void(const PresetEntry&)> onApply,
                      std::function<void(const PresetEntry&, bool)> onLoadToSlot,
                      std::function<void(const PresetEntry&, bool)> onStar,
                      std::function<void(const juce::String&, const juce::StringArray&, const juce::String&)> onSaveAs,
                      const juce::String& initialQuery = {});

    void resized() override;
    void paint (juce::Graphics&) override;

    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

private:
    PresetRegistry& reg;
    juce::TextEditor search;
    juce::ToggleButton andMode { "AND tags" };
    juce::ListBox list { "results", this };
    juce::Label desc, hint, meta;
    juce::TextButton apply{"Apply"}, loadA{"Load → A"}, loadB{"Load → B"},
                     star{"★"}, saveAs{"Save As…"};

    juce::Array<const PresetEntry*> results;
    int selectedRow = -1;

    struct Query { juce::String text; juce::StringArray tags; bool andMode = true; bool favOnly = false; };
    Query parseQuery (juce::String s) const;
    bool  matches (const PresetEntry&, const Query&) const;
    void  refresh();

    bool keyPressed (const juce::KeyPress& k, juce::Component*) override;
    void textEditorTextChanged (juce::TextEditor&) override { refresh(); }

    const PresetEntry* sel() const;
};


