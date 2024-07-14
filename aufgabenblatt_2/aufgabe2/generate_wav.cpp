#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

const int SAMPLE_RATE = 44100;
const int BITS_PER_SAMPLE = 16;

struct FrequencyInfo {
    double frequency;
    double amplitude;
};

void writeWavHeader(std::ofstream &file, int dataSize) {
    // RIFF header
    file.write("RIFF", 4);
    int32_t chunkSize = 36 + dataSize;
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    // fmt subchunk
    file.write("fmt ", 4);
    int32_t subchunk1Size = 16;
    int16_t audioFormat = 1; // PCM
    int16_t numChannels = 1;
    int32_t sampleRate = SAMPLE_RATE;
    int32_t byteRate = SAMPLE_RATE * numChannels * BITS_PER_SAMPLE / 8;
    int16_t blockAlign = numChannels * BITS_PER_SAMPLE / 8;
    int16_t bitsPerSample = BITS_PER_SAMPLE;

    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // data subchunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
}

int16_t generateSample(double time, const std::vector<FrequencyInfo>& frequencies) {
    double sample = 0;
    for (const auto& freqInfo : frequencies) {
        sample += freqInfo.amplitude * sin(2 * M_PI * freqInfo.frequency * time);
    }
    return static_cast<int16_t>(sample * 32767); // Convert to 16-bit PCM
}

void generateWavFile(const std::string& fileName, int duration, const std::vector<FrequencyInfo>& frequencies) {
    int totalSamples = SAMPLE_RATE * duration;
    int dataSize = totalSamples * sizeof(int16_t);

    std::ofstream file(fileName, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << fileName << std::endl;
        return;
    }

    // Write the WAV file header
    writeWavHeader(file, dataSize);

    // Generate the audio samples
    for (int i = 0; i < totalSamples; ++i) {
        double time = static_cast<double>(i) / SAMPLE_RATE;
        int16_t sample = generateSample(time, frequencies);
        file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
    }

    file.close();
}

int main() {
    int duration;
    std::cout << "Enter duration in seconds: ";
    std::cin >> duration;

    int numFrequencies;
    std::cout << "Enter number of frequencies: ";
    std::cin >> numFrequencies;

    std::vector<FrequencyInfo> frequencies(numFrequencies);
    for (int i = 0; i < numFrequencies; ++i) {
        std::cout << "Enter frequency " << i + 1 << " in Hz: ";
        std::cin >> frequencies[i].frequency;
        std::cout << "Enter amplitude for frequency " << i + 1 << ": ";
        std::cin >> frequencies[i].amplitude;
    }

    std::string fileName;
    std::cout << "Enter output file name: ";
    std::cin >> fileName;

    generateWavFile(fileName, duration, frequencies);

    std::cout << "WAV file generated successfully: " << fileName << std::endl;

    return 0;
}
