import json
import numpy as np
from typing import Tuple

TimeSeries = Tuple[np.array, np.array]


def sine_generate(f: int, t: float, fs: int) -> TimeSeries:
    N = fs * t                                         # Počet vzoriek
    Ts = 1 / fs                                        # Interval vzorkovania
    t_series = Ts * np.arange(N)
    return t_series, np.sin(2 * np.pi * f * t_series)  # y = A * sin(2*pi*t) 


def s_offsets(signal, rules):
    lower = int(signal['t0'] * rules['f_sampling'])
    upper = int((rules['duration'] - signal['tn']) * rules['f_sampling'])
    return lower, upper


def s_duration(signal, rules):
    return int(np.ceil((signal['tn'] - signal['t0']) * rules['f_sampling']))


def signal_rules(filename: str) -> dict:
    with open(filename, 'r') as fsource:
        return json.load(fsource)


def signal_generate(rules: dict) -> TimeSeries:
    N = rules['f_sampling'] * rules['duration']                                         
    Ts = 1 / rules['f_sampling']
    t_series = Ts * np.arange(N)
    signal = np.zeros(N)
    
    for sine in rules.get('sines', []):
        duration_samples = s_duration(sine, rules)
        t_part = Ts * np.arange(duration_samples)
        s = sine['amp'] * np.sin(2 * np.pi * sine['freq'] * t_part)
        
        signal += np.pad(s, pad_width=s_offsets(sine, rules), constant_values=0)[:len(t_series)]

    for noise in rules.get('noises', []):
        duration_samples = s_duration(noise, rules)
        s = np.random.normal(loc=0, scale=noise['amp'], size=duration_samples)
        
        signal += np.pad(s, pad_width=s_offsets(noise, rules), constant_values=0)[:len(t_series)]
        
    return t_series, signal


def signal_threshold(y: np.array, level: float) -> np.array:
    return np.where(abs(y) < level, 0, y)


def ground_truth(rules: dict, windows: list, Wn: int, resolution: float):
    labels = []
    for start in windows:
        freqs = []
        for sine in rules.get('sines', []):
            if (sine['t0'] <= start <= sine['tn'] or 
                    sine['t0'] <= start + Wn <= sine['tn']):
                freqs.append(int(sine['freq'] / resolution))
        labels.append(freqs)
    return labels
