import requests
import time
import numpy as np
from playsound import playsound  # pip install playsound==1.2.2
import sys

# ESP32 live ECG URL
ESP32_URL = "http://192.163.157.31/ecg"  # make sure double slashes removed

# Sampling setup
DURATION = 30  # seconds of ECG data collection
SAMPLE_RATE = 10  # samples per second
TOTAL_SAMPLES = DURATION * SAMPLE_RATE

# Paths to music files
MUSIC_PATHS = {
    "Wake": r"C:\Users\hp\Music\wake.mp3",
    "REM Sleep": r"C:\Users\hp\Music\rem.mp3",
    "NREM Sleep": r"C:\Users\hp\Music\nrem.mp3"
}

def display_progress(current, total, bar_length=30):
    percent = float(current) / total
    arrow = '#' * int(round(percent * bar_length))
    spaces = '-' * (bar_length - len(arrow))
    sys.stdout.write(f"\r⏳ Collecting ECG [{arrow + spaces}] {int(percent * 100)}%")
    sys.stdout.flush()

# Infinite loop
while True:
    ecg_data = []
    print("\n📡 Starting 30-second ECG data collection...")

    for i in range(TOTAL_SAMPLES):
        try:
            response = requests.get(ESP32_URL, timeout=1)
            val = float(response.text.strip())
            ecg_data.append(val)
        except Exception as e:
            print("\n❌ Error:", e)
            ecg_data.append(0.0)
        display_progress(i + 1, TOTAL_SAMPLES)
        time.sleep(1.0 / SAMPLE_RATE)

    print("\n✅ ECG Data collection complete!")

    # Convert to NumPy array
    ecg_array = np.array(ecg_data)

    # Analyze signal
    amplitude = np.max(ecg_array) - np.min(ecg_array)
    variance = np.var(ecg_array)
    print(f"📊 Amplitude: {amplitude:.4f}, Variance: {variance:.6f}")

    # Decide sleep stage (brute force)
    if amplitude > 0.6 and variance > 0.02:
        stage = "Wake"
    elif amplitude > 0.3 and variance > 0.005:
        stage = "REM Sleep"
    else:
        stage = "NREM Sleep"

    music_file = MUSIC_PATHS[stage]
    print(f"🛏️ Detected Sleep Stage: {stage}")
    print(f"🎵 Playing: {music_file}")

    # Play music in current thread (wait until complete)
    playsound(music_file)

    # Loop repeats after music ends
