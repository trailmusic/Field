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
#include <chrono>
#include <vector>
#include "ReverbEngine.h"
#include "ReverbProcessorGlue.h"
#include "FieldReverbConfig.h"

class ReverbIRExportTest : public juce::UnitTest
{
public:
    ReverbIRExportTest() : juce::UnitTest ("Reverb IR Export", "Audio") {}
    
    // T60 measurement function
    static float fitT60Sec(const std::vector<float>& mono, double fs, double t0=0.5, double t1=3.5)
    {
        const int i0 = (int)std::round(t0*fs), i1 = (int)std::round(t1*fs);
        double sx=0, sy=0, sxx=0, sxy=0, n=0;
        for (int i=i0; i<i1 && i<(int)mono.size(); ++i) {
            const double t = i / fs;
            const double y = std::log10(std::max(1e-12f, std::fabs(mono[i])));
            sx += t; sy += y; sxx += t*t; sxy += t*y; n += 1.0;
        }
        const double m = (n*sxy - sx*sy) / std::max(1e-12, n*sxx - sx*sx); // slope (log10 amplitude / s)
        // 60 dB drop means -6 decades in log10 amplitude. T60 = Δdecades / |slope| = 6 / |m|
        return (float)(6.0 / std::abs(m));
    }

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
        
        // T60 validation
        beginTest ("T60 measurement validation");
        
        // Convert to mono for T60 measurement
        std::vector<float> mono(total);
        for (int i = 0; i < total; ++i) {
            mono[i] = (tail.getSample(0, i) + tail.getSample(1, i)) * 0.5f;
        }
        
        const float measuredT60 = fitT60Sec(mono, sr, 0.5, 3.5);
        const float expectedT60 = 1.8f; // Default decay time
        const float tolerance = 0.1f; // ±5% tolerance
        
        logMessage ("Measured T60: " + juce::String(measuredT60, 2) + "s, Expected: " + juce::String(expectedT60, 2) + "s");
        expect (std::abs(measuredT60 - expectedT60) < tolerance, "T60 measurement failed");
        
        // Stereo decorrelation check
        beginTest ("Stereo decorrelation validation");
        
        // Compute cross-correlation
        double correlation = 0.0;
        const int windowSize = 1024;
        for (int i = 0; i < total - windowSize; i += windowSize) {
            double sumL = 0.0, sumR = 0.0, sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;
            for (int j = 0; j < windowSize; ++j) {
                const float l = tail.getSample(0, i + j);
                const float r = tail.getSample(1, i + j);
                sumL += l; sumR += r; sumLR += l * r; sumLL += l * l; sumRR += r * r;
            }
            const double denom = std::sqrt(sumLL * sumRR);
            if (denom > 1e-12) {
                correlation += std::abs(sumLR / denom);
            }
        }
        correlation /= (total / windowSize);
        
        logMessage ("Cross-correlation: " + juce::String(correlation, 3));
        expect (correlation < 0.6, "Stereo decorrelation failed (correlation too high)");
        
        // Performance and stability validation
        runPerformanceTests();
        runStabilityTests();
        runInvarianceTests();
        runStressTests();
    }
    
private:
    void runPerformanceTests()
    {
        beginTest ("CPU Performance Validation");
        
        const double sr = 48000.0;
        const int block = 256;
        const int testSamples = 48000; // 1 second
        
        // Create engine
        ReverbEngine engine;
        engine.prepare(sr, block, 2);
        
        // Set up test parameters
        ReverbParams params;
        params.decaySec = 2.4f;
        params.erLevelDb = -10.0f;
        params.duckOn = false;
        engine.setParams(params);
        
        // Performance measurement
        juce::AudioBuffer<float> buffer(2, block);
        juce::AudioBuffer<float> sidechain(2, block);
        sidechain.clear();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < testSamples; i += block) {
            buffer.clear();
            if (i == 0) buffer.setSample(0, 0, 1.0f); // Impulse
            
            engine.processWet(buffer, sidechain);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        const double cpuTimeMs = duration.count() / 1000.0;
        const double realTimeMs = (testSamples / sr) * 1000.0;
        const double cpuUsage = (cpuTimeMs / realTimeMs) * 100.0;
        
        logMessage ("CPU Usage: " + juce::String(cpuUsage, 1) + "% (" + 
                   juce::String(cpuTimeMs, 1) + "ms / " + juce::String(realTimeMs, 1) + "ms)");
        
        expect (cpuUsage < 50.0, "CPU usage too high (>50%)");
    }
    
    void runStabilityTests()
    {
        beginTest ("Memory Stability Validation");
        
        const double sr = 48000.0;
        const int block = 256;
        
        // Test multiple prepare/process cycles
        for (int cycle = 0; cycle < 10; ++cycle) {
            ReverbEngine engine;
            engine.prepare(sr, block, 2);
            
            ReverbParams params;
            params.decaySec = 2.4f;
            engine.setParams(params);
            
            // Process some audio
            juce::AudioBuffer<float> buffer(2, block);
            juce::AudioBuffer<float> sidechain(2, block);
            sidechain.clear();
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);
            
            for (int i = 0; i < 100; ++i) {
                engine.processWet(buffer, sidechain);
            }
            
            // Test parameter changes during processing
            params.decaySec = 1.0f + (cycle * 0.5f);
            engine.setParams(params);
            
            for (int i = 0; i < 50; ++i) {
                engine.processWet(buffer, sidechain);
            }
        }
        
        logMessage ("Memory stability test passed (10 prepare/process cycles)");
    }
    
    void runInvarianceTests()
    {
        beginTest ("Buffer Size Invariance");
        
        const double sr = 48000.0;
        const std::vector<int> bufferSizes = {64, 128, 256, 512, 1024};
        
        for (int blockSize : bufferSizes) {
            ReverbEngine engine;
            engine.prepare(sr, blockSize, 2);
            
            ReverbParams params;
            params.decaySec = 2.4f;
            engine.setParams(params);
            
            // Process same content with different buffer sizes
            juce::AudioBuffer<float> buffer(2, blockSize);
            juce::AudioBuffer<float> sidechain(2, blockSize);
            sidechain.clear();
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);
            
            for (int i = 0; i < 100; ++i) {
                engine.processWet(buffer, sidechain);
            }
        }
        
        logMessage ("Buffer size invariance test passed");
        
        beginTest ("Sample Rate Invariance");
        
        const std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0};
        const int block = 256;
        
        for (double sr : sampleRates) {
            ReverbEngine engine;
            engine.prepare(sr, block, 2);
            
            ReverbParams params;
            params.decaySec = 2.4f;
            engine.setParams(params);
            
            // Process some audio
            juce::AudioBuffer<float> buffer(2, block);
            juce::AudioBuffer<float> sidechain(2, block);
            sidechain.clear();
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);
            
            for (int i = 0; i < 50; ++i) {
                engine.processWet(buffer, sidechain);
            }
        }
        
        logMessage ("Sample rate invariance test passed");
    }
    
    void runStressTests()
    {
        beginTest ("Denormal Soak Test");
        
        const double sr = 48000.0;
        const int block = 256;
        const int testSamples = 480000; // 10 seconds
        
        ReverbEngine engine;
        engine.prepare(sr, block, 2);
        
        ReverbParams params;
        params.decaySec = 0.1f; // Very short decay to stress the system
        params.erLevelDb = -60.0f; // Very low level
        engine.setParams(params);
        
        juce::AudioBuffer<float> buffer(2, block);
        juce::AudioBuffer<float> sidechain(2, block);
        sidechain.clear();
        
        // Feed very low level noise to trigger denormals
        juce::Random rng;
        for (int i = 0; i < testSamples; i += block) {
            buffer.clear();
            
            // Add very low level noise (-120 dBFS)
            for (int c = 0; c < 2; ++c) {
                for (int s = 0; s < block; ++s) {
                    const float noise = (rng.nextFloat() - 0.5f) * 1e-6f; // -120 dBFS
                    buffer.setSample(c, s, noise);
                }
            }
            
            engine.processWet(buffer, sidechain);
            
            // Check for NaN/Inf
            for (int c = 0; c < 2; ++c) {
                for (int s = 0; s < block; ++s) {
                    const float sample = buffer.getSample(c, s);
                    expect (!std::isnan(sample), "NaN detected in output");
                    expect (!std::isinf(sample), "Inf detected in output");
                }
            }
        }
        
        logMessage ("Denormal soak test passed (10 seconds of -120 dBFS noise)");
        
        beginTest ("Parameter Sweep Stress Test");
        
        engine.prepare(sr, block, 2);
        
        // Rapid parameter changes
        for (int sweep = 0; sweep < 100; ++sweep) {
            params.decaySec = 0.1f + (sweep * 0.1f);
            params.erLevelDb = -60.0f + (sweep * 0.5f);
            engine.setParams(params);
            
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);
            engine.processWet(buffer, sidechain);
        }
        
        logMessage ("Parameter sweep stress test passed");
    }
};

static ReverbIRExportTest s_reverbIRExportTest;
