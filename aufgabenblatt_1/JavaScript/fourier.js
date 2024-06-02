const fs = require('fs');
const wav = require('node-wav');
const fft = require('fft-js').fft;
const fftUtil = require('fft-js').util;

// Funktion zum Einlesen der WAV-Datei
function readWavFile(filename) {
  const buffer = fs.readFileSync(filename);
  const result = wav.decode(buffer);
  return {
    sampleRate: result.sampleRate,
    data: result.channelData[0] // Verwenden des linken Kanals, falls stereo
  };
}

// Funktion zum Generieren überlappender Blöcke
function generateOverlappingBlocks(data, blockSize, maxSamples) {
  const numBlocks = Math.min(data.length - blockSize + 1, maxSamples);
  const blocks = [];
  for (let i = 0; i < numBlocks; i++) {
    blocks.push(data.slice(i, i + blockSize));
  }
  return blocks;
}

// Funktion zur Durchführung der FFT
function performFFT(block, sampleRate) {
  const phasors = fft(block);
  const frequencies = fftUtil.fftFreq(phasors, sampleRate);
  const magnitudes = fftUtil.fftMag(phasors);
  return {
    freqs: frequencies.slice(0, block.length / 2),
    amplitudes: magnitudes.slice(0, block.length / 2)
  };
}

// Hauptfunktion
function main(filename, blockSize, maxSamples, thresholdRatio = 0.1) {
  const wavData = readWavFile(filename);
  const sampleRate = wavData.sampleRate;
  const data = wavData.data;

  console.log(`Sample Rate: ${sampleRate} Hz`);
  console.log("Number of Channels: 1");  // Wir verwenden nur den linken Kanal
  console.log(`Duration: ${data.length / sampleRate} seconds`);

  const blocks = generateOverlappingBlocks(data, blockSize, maxSamples);

  const freqMap = new Map();

  blocks.forEach((block, i) => {
    const { freqs, amplitudes } = performFFT(block, sampleRate);

    // Hauptfrequenzen und deren Amplituden finden
    const maxAmplitude = Math.max(...amplitudes);
    const thresholdAmplitude = thresholdRatio * maxAmplitude;

    for (let j = 0; j < amplitudes.length; j++) {
      if (amplitudes[j] >= thresholdAmplitude) {
        const frequency = freqs[j];
        if (freqMap.has(frequency)) {
          freqMap.get(frequency).push(amplitudes[j]);
        } else {
          freqMap.set(frequency, [amplitudes[j]]);
        }
      }
    }
  });
  /*
  // Durchschnittliche Amplituden berechnen
  const summary = [];
  freqMap.forEach((amplitudes, frequency) => {
    const averageAmplitude = amplitudes.reduce((sum, amp) => sum + amp, 0) / amplitudes.length;
    summary.push({ frequency, averageAmplitude });
  });

  // Ergebnisse sortieren und anzeigen
  summary.sort((a, b) => a.frequency - b.frequency);
  summary.forEach(({ frequency, averageAmplitude }) => {
    console.log(`Frequency: ${frequency.toFixed(2)} Hz, Average Amplitude: ${averageAmplitude.toFixed(2)}`);
  });
  */
}

// Ausführung der Hauptfunktion
const filename = '../Python/nicht_zu_laut_abspielen.wav';  // Pfad zur WAV-Datei anpassen
const blockSize = 1024;  // Blockgröße anpassen
const maxSamples = 500000;  // Maximale Anzahl der Blöcke

main(filename, blockSize, maxSamples);
