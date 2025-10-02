/*
====================================================================================================
 ReverbIRExportTest — Offline IR generator (UnitTest you can run from JUCE)
----------------------------------------------------------------------------------------------------
 Purpose
    - Renders an impulse through the current engine path and writes WAV to disk.
    - Helps validate decay time, coloration, and ducking behavior offline.

 Usage
    - Call runUnitTests() in a debug menu, or let JUCE's UnitTestRunner run it.
    - Output path printed to Logger; adjust as needed.

 Notes
    - Uses Phase 1 path unless FIELD_REVERB_PHASE2 is enabled.
====================================================================================================
*/
#include <JuceHeader.h>
#include "ReverbEngine.h"
#include "ReverbProcessorGlue.h"
#include "FieldReverbConfig.h"

class ReverbIRExportTest : public juce::UnitTest
{
public:
    ReverbIRExportTest() : juce::UnitTest ("Reverb IR Export", "Audio") {}

    void runTest() override
    {
        beginTest ("Render 10 s IR @48k stereo");

        const double sr = 48000.0;
        const int block = 256;
        const int secs  = FIELD_REVERB_DEFAULT_IR_SECONDS;
        const int total = (int) (sr * secs);

        // Create a minimal processor for APVTS
        struct DummyProcessor : public juce::AudioProcessor
        {
            const juce::String getName() const override { return "Dummy"; }
            void prepareToPlay(double, int) override {}
            void releaseResources() override {}
            void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
            double getTailLengthSeconds() const override { return 0.0; }
            bool acceptsMidi() const override { return false; }
            bool producesMidi() const override { return false; }
            juce::AudioProcessorEditor* createEditor() override { return nullptr; }
            bool hasEditor() const override { return false; }
            int getNumPrograms() override { return 1; }
            int getCurrentProgram() override { return 0; }
            void setCurrentProgram(int) override {}
            const juce::String getProgramName(int) override { return "Program"; }
            void changeProgramName(int, const juce::String&) override {}
            void getStateInformation(juce::MemoryBlock&) override {}
            void setStateInformation(const void*, int) override {}
        };

        // Fake APVTS with params you use (or skip glue and set ReverbParams manually)
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add (std::make_unique<juce::AudioParameterFloat>("decay_sec", "Decay", 0.2f, 20.0f, 2.4f));
        layout.add (std::make_unique<juce::AudioParameterFloat>("er_level_db","ER Lvl", -60.f, 0.f, -10.f));
        layout.add (std::make_unique<juce::AudioParameterBool> ("duck_on", "Duck", false));
        DummyProcessor dummy;
        juce::AudioProcessorValueTreeState apvts (dummy, nullptr, "ReverbTest", std::move (layout));

        ReverbEngine engine;
        ReverbProcessorGlue glue (apvts, engine);
        glue.prepareToPlay (sr, block, 2);

        juce::AudioBuffer<float> tail (2, total);
        tail.clear();

        // impulse input buffer
        juce::AudioBuffer<float> io (2, block);
        juce::MidiBuffer midi;

        int rendered = 0;
        while (rendered < total)
        {
            const int n = juce::jmin (block, total - rendered);
            io.clear();
            if (rendered == 0) // fire impulse at t=0
                for (int c=0;c<io.getNumChannels();++c)
                    io.setSample (c, 0, 1.0f);

            io.setSize (2, n, true, true, true);
            glue.processBlock (io, midi);

            for (int c=0;c<2;++c)
                tail.copyFrom (c, rendered, io.getReadPointer (c), n);

            rendered += n;
        }

        // write wav
        juce::File out = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                         .getChildFile ("FIELD_Reverb_IR.wav");
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::FileOutputStream> fos (out.createOutputStream());
        expect (fos != nullptr);

        if (fos != nullptr)
        {
            std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (fos.get(), sr, 2, 24, {}, 0));
            if (w)
            {
                fos.release();
                w->writeFromAudioSampleBuffer (tail, 0, tail.getNumSamples());
                logMessage ("Wrote: " + out.getFullPathName());
            }
        }
    }
};

static ReverbIRExportTest s_reverbIRExportTest;
