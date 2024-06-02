# Laden der notwendigen Pakete
library(tuneR)
library(seewave)

# Funktion zum Einlesen der WAV-Datei
read_wav_file <- function(filename) {
  wave <- readWave(filename)
  sample_rate <- wave@samp.rate
  data <- wave@left  # Verwenden des linken Kanals, falls stereo
  return(list(sample_rate = sample_rate, data = data))
}

# Funktion zum Generieren überlappender Blöcke
generate_overlapping_blocks <- function(data, block_size, max_samples) {
  num_blocks <- min(length(data) - block_size + 1, max_samples)
  blocks <- lapply(1:num_blocks, function(i) data[i:(i + block_size - 1)])
  return(blocks)
}

# Funktion zur Durchführung der FFT
perform_fft <- function(block, sample_rate) {
  fft_result <- fft(block)
  amplitudes <- Mod(fft_result)
  freqs <- (0:(length(block) / 2)) * sample_rate / length(block)
  return(list(freqs = freqs, amplitudes = amplitudes[1:(length(block) / 2 + 1)]))
}

# Hauptfunktion
main <- function(filename, block_size, max_samples, threshold_ratio = 0.8) {
  wav_data <- read_wav_file(filename)
  sample_rate <- wav_data$sample_rate
  data <- wav_data$data
  
  cat("Sample Rate:", sample_rate, "Hz\n")
  cat("Number of Channels: 1\n")  # Wir verwenden nur den linken Kanal
  cat("Duration:", length(data) / sample_rate, "seconds\n")
  
  blocks <- generate_overlapping_blocks(data, block_size, max_samples)
  results <- list()
  
  for (i in 1:length(blocks)) {
    fft_result <- perform_fft(blocks[[i]], sample_rate)
    freqs <- fft_result$freqs
    amplitudes <- fft_result$amplitudes
    
    # Hauptfrequenzen und deren Amplituden finden
    max_amplitude <- max(amplitudes)
    threshold_amplitude <- threshold_ratio * max_amplitude
    
    significant_indices <- which(amplitudes >= threshold_amplitude)
    significant_freqs <- freqs[significant_indices]
    significant_amplitudes <- amplitudes[significant_indices]
    
    results[[i]] <- data.frame(
      Block = i,
      Frequency = significant_freqs,
      Amplitude = significant_amplitudes
    )
  }
  
  # Ergebnisse anzeigen
  for (result in results) {
    cat(sprintf("Block: %d\n", result$Block[1]))
    for (j in 1:nrow(result)) {
      cat(sprintf("  Frequency: %.2f Hz, Amplitude: %.2f\n", result$Frequency[j], result$Amplitude[j]))
    }
  }
}

# Konfiguration
setwd("/home/fabian/programming/HardwarenaheSystemprogrammierung/aufgabenblatt_1")
filename <- "./Python/nicht_zu_laut_abspielen.wav"  # Pfad zur WAV-Datei anpassen
block_size <- 1024  # Blockgröße anpassen
max_samples <- 500000  # Maximale Anzahl der Blöcke
threshold_ratio <- 0.1  # Prozentsatz der höchsten Amplitude im Block

# Profiler starten
Rprof(filename="memory_profile.out", memory.profiling=TRUE)

# Ausführen der Hauptfunktion
main(filename, block_size, max_samples, threshold_ratio)

# Profiler stoppen
Rprof(NULL)

# Profiling-Daten analysieren
summaryRprof("memory_profile.out", memory = "both")

