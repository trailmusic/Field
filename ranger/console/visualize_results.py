#!/usr/bin/env python3
"""
Min-Phase FIR Toolchain - Visualization Script
Plots impulse response, step response, and magnitude response
for linear vs minimum-phase FIR filters.
"""

import numpy as np
import matplotlib.pyplot as plt
import sys
import os
from pathlib import Path

def load_csv(filename):
    """Load taps from CSV file"""
    try:
        return np.loadtxt(filename, delimiter=',')
    except:
        return np.loadtxt(filename)

def plot_comparison(linear_file, minphase_file, title="FIR Comparison"):
    """Plot linear vs minimum-phase comparison"""
    
    # Load data
    linear = load_csv(linear_file)
    minphase = load_csv(minphase_file)
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    fig.suptitle(title, fontsize=16)
    
    # 1. Impulse Response
    ax1 = axes[0, 0]
    n_linear = np.arange(len(linear))
    n_min = np.arange(len(minphase))
    
    ax1.plot(n_linear, linear, 'b-', label='Linear-phase', linewidth=2)
    ax1.plot(n_min, minphase, 'r-', label='Minimum-phase', linewidth=2)
    ax1.set_title('Impulse Response')
    ax1.set_xlabel('Sample')
    ax1.set_ylabel('Amplitude')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Step Response
    ax2 = axes[0, 1]
    step_linear = np.cumsum(linear)
    step_min = np.cumsum(minphase)
    
    ax2.plot(n_linear, step_linear, 'b-', label='Linear-phase', linewidth=2)
    ax2.plot(n_min, step_min, 'r-', label='Minimum-phase', linewidth=2)
    ax2.set_title('Step Response')
    ax2.set_xlabel('Sample')
    ax2.set_ylabel('Amplitude')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # 3. Magnitude Response
    ax3 = axes[1, 0]
    N = max(len(linear), len(minphase))
    freqs = np.fft.fftfreq(N, 1.0)[:N//2]
    
    # Zero-pad to same length for comparison
    linear_padded = np.zeros(N)
    minphase_padded = np.zeros(N)
    linear_padded[:len(linear)] = linear
    minphase_padded[:len(minphase)] = minphase
    
    H_linear = np.fft.fft(linear_padded)
    H_min = np.fft.fft(minphase_padded)
    
    ax3.semilogx(freqs[1:], 20*np.log10(np.abs(H_linear[1:N//2])), 'b-', label='Linear-phase', linewidth=2)
    ax3.semilogx(freqs[1:], 20*np.log10(np.abs(H_min[1:N//2])), 'r-', label='Minimum-phase', linewidth=2)
    ax3.set_title('Magnitude Response')
    ax3.set_xlabel('Normalized Frequency')
    ax3.set_ylabel('Magnitude (dB)')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.set_ylim([-100, 5])
    
    # 4. Phase Response
    ax4 = axes[1, 1]
    phase_linear = np.angle(H_linear[1:N//2])
    phase_min = np.angle(H_min[1:N//2])
    
    ax4.semilogx(freqs[1:], np.unwrap(phase_linear), 'b-', label='Linear-phase', linewidth=2)
    ax4.semilogx(freqs[1:], np.unwrap(phase_min), 'r-', label='Minimum-phase', linewidth=2)
    ax4.set_title('Phase Response')
    ax4.set_xlabel('Normalized Frequency')
    ax4.set_ylabel('Phase (radians)')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    
    plt.tight_layout()
    return fig

def main():
    """Main visualization function"""
    
    if len(sys.argv) < 3:
        print("Usage: python3 visualize_results.py <linear.csv> <minphase.csv> [output.png]")
        print("Example: python3 visualize_results.py HB63_linear.csv HB63_min.csv")
        sys.exit(1)
    
    linear_file = sys.argv[1]
    minphase_file = sys.argv[2]
    output_file = sys.argv[3] if len(sys.argv) > 3 else "fir_comparison.png"
    
    # Check if files exist
    if not os.path.exists(linear_file):
        print(f"Error: {linear_file} not found")
        sys.exit(1)
    
    if not os.path.exists(minphase_file):
        print(f"Error: {minphase_file} not found")
        sys.exit(1)
    
    print(f"📊 Visualizing {linear_file} vs {minphase_file}")
    
    # Create comparison plot
    fig = plot_comparison(linear_file, minphase_file, f"Linear vs Minimum Phase: {Path(linear_file).stem}")
    
    # Save plot
    fig.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✅ Saved visualization to {output_file}")
    
    # Show plot if running interactively
    try:
        plt.show()
    except:
        pass

if __name__ == "__main__":
    main()
