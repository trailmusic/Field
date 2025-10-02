#include "RangerAuditionHarness.h"

// Placeholder implementations for CustomOversampler and TPSafe
class CustomOversampler
{
public:
    juce::AudioBuffer<float> processUp(const juce::AudioBuffer<float>& input)
    {
        // Placeholder: 2x upsampling
        juce::AudioBuffer<float> output(1, input.getNumSamples() * 2);
        output.clear();
        
        // Simple zero-stuffing for now
        for (int i = 0; i < input.getNumSamples(); ++i)
        {
            output.setSample(0, i * 2, input.getSample(0, i));
        }
        
        return output;
    }
    
    void processDown(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output)
    {
        // Placeholder: 2x downsampling
        output.clear();
        
        // Simple decimation for now
        for (int i = 0; i < output.getNumSamples(); ++i)
        {
            if (i * 2 < input.getNumSamples())
            {
                output.setSample(0, i, input.getSample(0, i * 2));
            }
        }
    }
};

class TPSafe
{
public:
    void process(juce::AudioBuffer<float>& buffer)
    {
        // Placeholder: simple peak limiting
        float peak = 0.0f;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            peak = std::max(peak, std::abs(buffer.getSample(0, i)));
        }
        
        if (peak > 0.95f)
        {
            float gain = 0.95f / peak;
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(0, i, buffer.getSample(0, i) * gain);
            }
        }
    }
};

void AuditionHarness::prepare(double sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;
    
    const int bufferSize = 8192;
    impulse.setSize(1, bufferSize);
    step.setSize(1, bufferSize);
    irOut.setSize(1, bufferSize);
    stepOut.setSize(1, bufferSize);
    
    // Generate impulse (delta function)
    impulse.clear();
    impulse.setSample(0, 0, 1.0f);
    
    // Generate step function
    step.clear();
    for (int n = 0; n < step.getNumSamples(); ++n)
    {
        step.setSample(0, n, 1.0f);
    }
}

void AuditionHarness::runThroughPath(const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out)
{
    jassert(os != nullptr && tpSafe != nullptr);
    out.setSize(1, in.getNumSamples());
    out.clear();
    
    // Process in blocks to match real engine
    int offset = 0;
    while (offset < in.getNumSamples())
    {
        const int n = std::min(blockSize, in.getNumSamples() - offset);
        
        // Create input block
        juce::AudioBuffer<float> inBlock(1, n);
        for (int i = 0; i < n; ++i)
        {
            inBlock.setSample(0, i, in.getSample(0, offset + i));
        }
        
        // Upsample
        auto upBlock = os->processUp(inBlock);
        
        // (Optional DSP at high rate) - for filter-only audition, skip
        
        // Downsample
        juce::AudioBuffer<float> downBlock(1, n);
        os->processDown(upBlock, downBlock);
        
        // TP-safe on the block
        tpSafe->process(downBlock);
        
        // Copy to output
        for (int i = 0; i < n; ++i)
        {
            out.setSample(0, offset + i, downBlock.getSample(0, i));
        }
        
        offset += n;
    }
}

void AuditionHarness::computeTimeDomain()
{
    runThroughPath(impulse, irOut);
    runThroughPath(step, stepOut);
}

MagPlot AuditionHarness::computeMagnitude(const float* ir, int n, double sr, bool smooth1_24)
{
    // Zero-pad to next power of 2
    int N = 1;
    while (N < n) N <<= 1;
    
    std::vector<float> re(N, 0.0f);
    for (int i = 0; i < n; ++i)
    {
        re[i] = ir[i];
    }
    
    // FFT (using simple implementation for now)
    std::vector<std::complex<float>> spec(N / 2 + 1);
    FFTUtils::fft_real_forward(re.data(), N, spec.data());
    
    MagPlot p;
    const int bins = N / 2 + 1;
    p.freqHz.resize(bins);
    p.magDb.resize(bins);
    
    for (int k = 0; k < bins; ++k)
    {
        const float f = (float)k * (float)sr / (float)N;
        const float m = std::max(1e-12f, std::abs(spec[k]));
        p.freqHz[k] = f;
        p.magDb[k] = 20.0f * std::log10(m);
    }
    
    if (smooth1_24)
    {
        fractionalOctaveSmooth(p.freqHz, p.magDb, 24.0f);
    }
    
    return p;
}

void AuditionHarness::normalizeIR(juce::AudioBuffer<float>& ir, NormMode mode)
{
    auto* x = ir.getWritePointer(0);
    const int N = ir.getNumSamples();
    
    if (mode == NormMode::Peak)
    {
        float peak = 0.0f;
        for (int i = 0; i < N; ++i)
        {
            peak = std::max(peak, std::abs(x[i]));
        }
        if (peak > 0.0f)
        {
            for (int i = 0; i < N; ++i)
            {
                x[i] /= peak;
            }
        }
    }
    else if (mode == NormMode::Energy)
    {
        double e = 0.0;
        for (int i = 0; i < N; ++i)
        {
            e += (double)x[i] * x[i];
        }
        if (e > 0.0)
        {
            float s = (float)std::sqrt(e);
            for (int i = 0; i < N; ++i)
            {
                x[i] /= s;
            }
        }
    }
    else // DCUnity
    {
        // Sum of taps ~ DC gain; scale to unity
        double sum = 0.0;
        for (int i = 0; i < N; ++i)
        {
            sum += x[i];
        }
        if (std::abs(sum) > 1e-12)
        {
            float s = (float)sum;
            for (int i = 0; i < N; ++i)
            {
                x[i] /= s;
            }
        }
    }
}

void AuditionHarness::fractionalOctaveSmooth(std::vector<float>& freqHz, std::vector<float>& magDb, float bandsPerOctave)
{
    // Simple moving average on log-frequency grid
    const float octaveWidth = 1.0f / bandsPerOctave;
    const int n = (int)freqHz.size();
    
    std::vector<float> smoothed(n);
    
    for (int i = 0; i < n; ++i)
    {
        const float centerFreq = freqHz[i];
        const float logCenter = std::log2(centerFreq);
        const float halfWidth = octaveWidth * 0.5f;
        
        float sum = 0.0f;
        float weight = 0.0f;
        
        for (int j = 0; j < n; ++j)
        {
            const float logFreq = std::log2(freqHz[j]);
            const float distance = std::abs(logFreq - logCenter);
            
            if (distance <= halfWidth)
            {
                const float w = 1.0f - (distance / halfWidth);
                sum += magDb[j] * w;
                weight += w;
            }
        }
        
        smoothed[i] = (weight > 0.0f) ? (sum / weight) : magDb[i];
    }
    
    magDb = smoothed;
}

void AuditionHarness::computeBaseline()
{
    // Set to linear phase bank and compute
    computeTimeDomain();
    baselineMag = computeMagnitude(irOut.getReadPointer(0), irOut.getNumSamples(), sampleRate, true);
}

void AuditionHarness::computeCandidate()
{
    // Set to minimum phase bank and compute
    computeTimeDomain();
    candidateMag = computeMagnitude(irOut.getReadPointer(0), irOut.getNumSamples(), sampleRate, true);
}

void AuditionHarness::computeDelta()
{
    // Compute difference between candidate and baseline
    const int n = std::min(baselineMag.magDb.size(), candidateMag.magDb.size());
    deltaMag.freqHz.resize(n);
    deltaMag.magDb.resize(n);
    
    for (int i = 0; i < n; ++i)
    {
        deltaMag.freqHz[i] = baselineMag.freqHz[i];
        deltaMag.magDb[i] = candidateMag.magDb[i] - baselineMag.magDb[i];
    }
}

bool AuditionHarness::verifyTPSafe()
{
    // Generate steep HF burst at 19-21 kHz @ -0.1 dBFS
    const int testLength = 1024;
    juce::AudioBuffer<float> testSignal(1, testLength);
    testSignal.clear();
    
    const float testFreq = 20000.0f; // 20 kHz
    const float amplitude = 0.9f; // -0.1 dBFS
    
    for (int i = 0; i < testLength; ++i)
    {
        const float t = (float)i / (float)sampleRate;
        testSignal.setSample(0, i, amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * testFreq * t));
    }
    
    // Process through path
    juce::AudioBuffer<float> output(1, testLength);
    runThroughPath(testSignal, output);
    
    // Check for peaks above ceiling
    const float ceiling = 0.95f;
    for (int i = 0; i < testLength; ++i)
    {
        if (std::abs(output.getSample(0, i)) > ceiling)
        {
            return false;
        }
    }
    
    return true;
}

void AuditionHarness::runTPSafeTest()
{
    // This would run the TP-Safe verification test
    // Implementation depends on specific TP-Safe algorithm
}

juce::String AuditionHarness::exportCSV(const juce::String& filename)
{
    juce::String csv;
    csv += "Frequency (Hz),Baseline (dB),Candidate (dB),Delta (dB)\n";
    
    const int n = std::min({baselineMag.freqHz.size(), baselineMag.magDb.size(), 
                           candidateMag.magDb.size(), deltaMag.magDb.size()});
    
    for (int i = 0; i < n; ++i)
    {
        csv += juce::String(baselineMag.freqHz[i]) + ",";
        csv += juce::String(baselineMag.magDb[i]) + ",";
        csv += juce::String(candidateMag.magDb[i]) + ",";
        csv += juce::String(deltaMag.magDb[i]) + "\n";
    }
    
    return csv;
}

juce::String AuditionHarness::copyTapArray()
{
    juce::String taps;
    taps += "const float filterTaps[] = {\n";
    
    const int n = irOut.getNumSamples();
    for (int i = 0; i < n; ++i)
    {
        taps += juce::String(irOut.getSample(0, i), 8);
        if (i < n - 1) taps += ",";
        if (i % 8 == 7) taps += "\n";
        else taps += " ";
    }
    
    taps += "\n};\n";
    return taps;
}

// FFT implementation (simplified)
namespace FFTUtils
{
    void fft_real_forward(float* data, int n, std::complex<float>* output)
    {
        // Simplified FFT implementation
        // In production, use a proper FFT library like KissFFT
        for (int k = 0; k < n / 2 + 1; ++k)
        {
            std::complex<float> sum(0.0f, 0.0f);
            for (int i = 0; i < n; ++i)
            {
                float angle = -2.0f * juce::MathConstants<float>::pi * k * i / n;
                sum += data[i] * std::complex<float>(std::cos(angle), std::sin(angle));
            }
            output[k] = sum;
        }
    }
    
    void fractionalOctaveSmooth(std::vector<float>& freqHz, std::vector<float>& magDb, float bandsPerOctave)
    {
        // Implementation moved to AuditionHarness class
    }
}
