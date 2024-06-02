import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from matplotlib.ticker import ScalarFormatter


def read_wav_file(filename):
    sample_rate, data = wavfile.read(filename)
    return sample_rate, data


def generate_overlapping_blocks(data, block_size, max_samples):
    num_blocks = len(data) - block_size + 1
    if num_blocks > max_samples:
        num_blocks = max_samples
    blocks = np.array([data[i:i + block_size] for i in range(num_blocks)])
    return blocks


def perform_fft(block, sample_rate):
    freqs = np.fft.rfftfreq(len(block), 1 / sample_rate)
    fft_values = np.fft.rfft(block)
    amplitudes = np.abs(fft_values)
    return freqs, amplitudes

def main(filename, block_size, max_samples):
    sample_rate, data = read_wav_file(filename)
    print(f'Sample Rate: {sample_rate} Hz')
    print(f'Number of Channels: {data.shape[1] if data.ndim > 1 else 1}')
    print(f'Duration: {len(data) / sample_rate} seconds')

    # Use the first channel if the audio is stereo
    if data.ndim > 1:
        data = data[:, 0]

    blocks = generate_overlapping_blocks(data, block_size, max_samples)
    all_freqs = []
    all_amplitudes = []

    for block in blocks:
        freqs, amplitudes = perform_fft(block, sample_rate)
        all_freqs.append(freqs)
        all_amplitudes.append(amplitudes)

    all_freqs = np.array(all_freqs)
    all_amplitudes = np.array(all_amplitudes)

    # Normalize amplitudes within each block
    all_amplitudes = all_amplitudes / np.max(all_amplitudes, axis=1, keepdims=True)

    plt.figure(figsize=(12, 6))
    plt.imshow(all_amplitudes.T, aspect='auto', origin='lower',
               extent=[0, len(blocks), 20, sample_rate / 2], cmap='viridis')
    plt.colorbar(label='Normalized Amplitude')
    plt.xlabel('Block Number')
    plt.ylabel('Frequency (Hz)')
    plt.yscale('log')
    plt.ylim([20, 20000])

    # Customize y-axis labels to show decimal numbers instead of scientific notation
    ax = plt.gca()
    ax.yaxis.set_major_formatter(ScalarFormatter())
    ax.yaxis.set_minor_formatter(ScalarFormatter())

    plt.title('Frequency Spectrum over Blocks')
    plt.show()



if __name__ == "__main__":
    filename = "nicht_zu_laut_abspielen.wav"  # Replace with your .wav file path
    block_size = 1024  # Choose your block size
    max_samples = 500000
    main(filename, block_size, max_samples)
