import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np


def plot_sinusoid(x, y, p, ax):
    ax.plot(x, y)
    if len(p) > 0: 
        ax.plot(x[p], y[p], 'o', color='r')
    ax.grid()


def plot_spectra_slices(bins, magnitudes, peaks=[]):
    fig, axes = plt.subplots(len(magnitudes), 1, figsize=(20, 10))
    for i, amp in enumerate(magnitudes):
        p = peaks[i] if len(peaks) > i else []
        plot_sinusoid(bins, amp, p, axes[i])


def plot_spectrogram(spectrum, ax, ylog, yrange):
    if ylog is True:
        ax.set_yscale('log', base=2)
        ax.yaxis.set_major_formatter(ticker.ScalarFormatter())
        plt.ticklabel_format(axis='y', style='plain')
        
    if yrange is not None:
        ax.set_ylim(*yrange)

    im = ax.pcolormesh(spectrum['t_windows'], spectrum['bins'], spectrum['magnitudes'].T, shading='gouraud')
    ax.set_xlabel('Čas [s]')
    ax.set_ylabel('Frekvencia [Hz]')
    ax.grid(True)
    return im

            
def draw_peaks(ax, spectrum, peaks):
    for i, p in enumerate(peaks):
        t = spectrum['t_windows'][i]
        f = spectrum['bins'][p]
        for pos in f:
            ax.plot(t, pos, 'o', color='w', alpha=0.5)

            
def draw_events(ax, spectrum, events):
    for event in events:
        if event['action'] == 'END':
            t_start = spectrum['t_windows'][event['start']]
            t_end = spectrum['t_windows'][event['start'] + event['duration'] - 1]
            f = event['frequency'] 
            ax.plot((t_start, t_end), (f, f), color='r')

            
def plot_spectra_heatmap(spectrum, peaks=[], ylog=False, yrange=None, figsize=(20, 6)):
    fig, ax = plt.subplots(1, 1, figsize=figsize)
    im = plot_spectrogram(spectrum, ax, ylog, yrange)
    fig.colorbar(im, ax=ax, pad=0.01)
    draw_peaks(ax, spectrum, peaks)


def plot_spectra_events(spectrum, events, ylog=False, yrange=None, figsize=(20, 6)):
    fig, ax = plt.subplots(1, 1, figsize=figsize)
    im = plot_spectrogram(spectrum, ax, ylog, yrange)
    draw_events(ax, spectrum, events)
    fig.colorbar(im, ax=ax, pad=0.01)

            
def plot_spectra_event_vs_peaks(spectrum, events, peaks=[], ylog=False, yrange=None, figsize=(20, 6), ax=None):
    fig, ax = plt.subplots(1, 1, figsize=figsize)
    im = plot_spectrogram(spectrum, ax, ylog, yrange)
    fig.colorbar(im, ax=ax, pad=0.01)
    draw_peaks(ax, spectrum, peaks)
    draw_events(ax, spectrum, events)

