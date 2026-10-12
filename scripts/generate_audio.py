import os
import wave
import struct
import math
import random

SAMPLE_RATE = 44100

def write_wav(path, samples, sample_rate=SAMPLE_RATE, channels=1):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, 'w') as w:
        w.setnchannels(channels)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        packed = bytearray()
        for s in samples:
            val = int(max(-1.0, min(1.0, s)) * 32767.0)
            packed.extend(struct.pack('<h', val))
        w.writeframes(packed)

def karplus_strong(freq, duration, decay=0.985, initial_noise=1.0):
    n_samples = int(SAMPLE_RATE * duration)
    delay = int(SAMPLE_RATE / freq)
    buf = [random.uniform(-initial_noise, initial_noise) for _ in range(delay)]
    out = []
    idx = 0
    for _ in range(n_samples):
        val = buf[idx]
        next_idx = (idx + 1) % delay
        avg = 0.5 * (val + buf[next_idx]) * decay
        buf[idx] = avg
        idx = next_idx
        out.append(val)
    return out

def wooden_clack(pitch=220.0):
    dur = 0.18
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-35.0 * t)
        noise = random.uniform(-0.3, 0.3) * math.exp(-80.0 * t)
        body = math.sin(2.0 * math.pi * pitch * t) * math.exp(-25.0 * t)
        click = math.sin(2.0 * math.pi * (pitch * 4.2) * t) * math.exp(-60.0 * t) * 0.5
        s = (body + click + noise) * env * 1.5
        samples.append(s)
    return samples

def wooden_capture():
    dur = 0.35
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-12.0 * t)
        tone1 = math.sin(2.0 * math.pi * 320.0 * t) * math.exp(-15.0 * t)
        tone2 = math.sin(2.0 * math.pi * 480.0 * t) * math.exp(-18.0 * t)
        body = math.sin(2.0 * math.pi * 180.0 * t) * math.exp(-10.0 * t)
        s = (tone1 * 0.6 + tone2 * 0.4 + body * 0.5) * env
        samples.append(s)
    return samples

def wooden_error():
    dur = 0.2
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-20.0 * t)
        s = math.sin(2.0 * math.pi * 110.0 * t) * env * 0.8
        samples.append(s)
    return samples

def wooden_pass():
    dur = 0.35
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.sin(math.pi * min(1.0, t / 0.05)) * math.exp(-8.0 * t)
        s = math.sin(2.0 * math.pi * 392.0 * t) * env * 0.6
        samples.append(s)
    return samples

def glass_clink(pitch=1200.0):
    dur = 0.25
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-18.0 * t)
        s1 = math.sin(2.0 * math.pi * pitch * t) * math.exp(-15.0 * t)
        s2 = math.sin(2.0 * math.pi * (pitch * 2.76) * t) * math.exp(-25.0 * t) * 0.4
        s3 = math.sin(2.0 * math.pi * (pitch * 5.4) * t) * math.exp(-40.0 * t) * 0.2
        samples.append((s1 + s2 + s3) * env * 0.9)
    return samples

def glass_capture():
    dur = 0.45
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-8.0 * t)
        c1 = math.sin(2.0 * math.pi * 880.0 * t) * 0.4
        c2 = math.sin(2.0 * math.pi * 1320.0 * t) * 0.3
        c3 = math.sin(2.0 * math.pi * 1760.0 * t) * 0.3
        samples.append((c1 + c2 + c3) * env * 0.8)
    return samples

def glass_error():
    dur = 0.2
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-15.0 * t)
        s = math.sin(2.0 * math.pi * 330.0 * t) * env * 0.6
        samples.append(s)
    return samples

def glass_pass():
    dur = 0.4
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = math.exp(-7.0 * t)
        s = (math.sin(2.0 * math.pi * 659.25 * t) + math.sin(2.0 * math.pi * 987.77 * t) * 0.5) * env * 0.5
        samples.append(s)
    return samples

def retro_blip(freq=600.0):
    dur = 0.08
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        env = 1.0 - (t / dur)
        phase = (t * freq) % 1.0
        val = 0.5 if phase < 0.5 else -0.5
        samples.append(val * env * 0.6)
    return samples

def retro_capture():
    dur = 0.22
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / dur
        freq = 300.0 + 900.0 * t
        time_s = i / SAMPLE_RATE
        env = 1.0 - t
        phase = (time_s * freq) % 1.0
        val = 0.5 if phase < 0.5 else -0.5
        samples.append(val * env * 0.5)
    return samples

def retro_error():
    dur = 0.15
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        freq = 150.0 if (i // 1500) % 2 == 0 else 110.0
        phase = (t * freq) % 1.0
        val = 0.5 if phase < 0.5 else -0.5
        env = 1.0 - (i / n)
        samples.append(val * env * 0.6)
    return samples

def retro_pass():
    dur = 0.16
    n = int(SAMPLE_RATE * dur)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        freq = 440.0 + 220.0 * (i / n)
        phase = (t * freq) % 1.0
        val = 0.5 if phase < 0.5 else -0.5
        env = 1.0 - (i / n)
        samples.append(val * env * 0.5)
    return samples

def generate_zen_garden(duration=16.0):
    n = int(SAMPLE_RATE * duration)
    out = [0.0] * n
    scale = [261.63, 293.66, 329.63, 392.00, 440.00, 523.25, 587.33, 659.25]
    bpm = 54
    beat_dur = 60.0 / bpm
    total_beats = int(duration / beat_dur)
    
    rng = random.Random(42)
    for beat in range(total_beats):
        if rng.random() < 0.65:
            note_idx = rng.choice([0, 2, 3, 4, 5, 7])
            freq = scale[note_idx % len(scale)]
            delay = int(beat * beat_dur * SAMPLE_RATE)
            decay = rng.uniform(0.994, 0.997)
            plucked = karplus_strong(freq, 2.5, decay, 0.8)
            vol = rng.uniform(0.15, 0.35)
            for j, s in enumerate(plucked):
                if delay + j < n:
                    out[delay + j] += s * vol
    for i in range(n):
        t = i / SAMPLE_RATE
        pad = math.sin(2.0 * math.pi * 130.81 * t) * 0.04 + math.sin(2.0 * math.pi * 196.00 * t) * 0.03
        out[i] += pad
    fade = int(SAMPLE_RATE * 0.8)
    for i in range(fade):
        factor = i / fade
        out[i] *= factor
        out[n - 1 - i] *= factor
    return out

def generate_night_rain(duration=16.0):
    n = int(SAMPLE_RATE * duration)
    out = [0.0] * n
    rng = random.Random(101)
    rain_filter = 0.0
    for i in range(n):
        white = rng.uniform(-0.04, 0.04)
        rain_filter = 0.92 * rain_filter + 0.08 * white
        out[i] = rain_filter * 0.3
    chords = [
        [220.0, 261.63, 329.63, 392.0],
        [174.61, 220.0, 261.63, 329.63],
        [196.0, 246.94, 293.66, 392.0],
        [164.81, 196.0, 246.94, 293.66],
    ]
    chord_dur = duration / len(chords)
    for ci, chord in enumerate(chords):
        t_start = ci * chord_dur
        for note in chord:
            for i in range(int(chord_dur * SAMPLE_RATE)):
                idx = int(t_start * SAMPLE_RATE) + i
                if idx < n:
                    t = i / SAMPLE_RATE
                    env = math.sin(math.pi * (i / (chord_dur * SAMPLE_RATE)))
                    tone = math.sin(2.0 * math.pi * note * t) * 0.05
                    out[idx] += tone * env
    fade = int(SAMPLE_RATE * 0.8)
    for i in range(fade):
        factor = i / fade
        out[i] *= factor
        out[n - 1 - i] *= factor
    return out

def generate_mountain_stream(duration=16.0):
    n = int(SAMPLE_RATE * duration)
    out = [0.0] * n
    rng = random.Random(202)
    flt = 0.0
    for i in range(n):
        w = rng.uniform(-0.03, 0.03)
        flt = 0.95 * flt + 0.05 * w
        out[i] = flt * 0.25
    scale = [392.0, 440.0, 493.88, 587.33, 659.25, 783.99]
    step = int(SAMPLE_RATE * 0.45)
    idx = 0
    while idx < n:
        if rng.random() < 0.6:
            freq = rng.choice(scale)
            t_len = min(int(SAMPLE_RATE * 1.2), n - idx)
            for j in range(t_len):
                t = j / SAMPLE_RATE
                env = math.exp(-4.5 * t)
                s = math.sin(2.0 * math.pi * freq * t) * 0.08 * env
                out[idx + j] += s
        idx += step
    fade = int(SAMPLE_RATE * 0.8)
    for i in range(fade):
        factor = i / fade
        out[i] *= factor
        out[n - 1 - i] *= factor
    return out

def main():
    base = os.path.join(os.path.dirname(__file__), '..', 'assets', 'audio')
    
    write_wav(os.path.join(base, 'themes', 'wooden', 'place.wav'), wooden_clack(240.0))
    write_wav(os.path.join(base, 'themes', 'wooden', 'capture.wav'), wooden_capture())
    write_wav(os.path.join(base, 'themes', 'wooden', 'error.wav'), wooden_error())
    write_wav(os.path.join(base, 'themes', 'wooden', 'pass.wav'), wooden_pass())
    
    write_wav(os.path.join(base, 'themes', 'glass', 'place.wav'), glass_clink(1250.0))
    write_wav(os.path.join(base, 'themes', 'glass', 'capture.wav'), glass_capture())
    write_wav(os.path.join(base, 'themes', 'glass', 'error.wav'), glass_error())
    write_wav(os.path.join(base, 'themes', 'glass', 'pass.wav'), glass_pass())
    
    write_wav(os.path.join(base, 'themes', 'retro', 'place.wav'), retro_blip(587.0))
    write_wav(os.path.join(base, 'themes', 'retro', 'capture.wav'), retro_capture())
    write_wav(os.path.join(base, 'themes', 'retro', 'error.wav'), retro_error())
    write_wav(os.path.join(base, 'themes', 'retro', 'pass.wav'), retro_pass())
    
    write_wav(os.path.join(base, 'music', 'zen_garden.wav'), generate_zen_garden(16.0))
    write_wav(os.path.join(base, 'music', 'night_rain.wav'), generate_night_rain(16.0))
    write_wav(os.path.join(base, 'music', 'mountain_stream.wav'), generate_mountain_stream(16.0))
    print("Audio assets generated successfully.")

if __name__ == '__main__':
    main()
