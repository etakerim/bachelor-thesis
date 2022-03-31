import numpy as np
import scipy.fft


def find_peaks_threshold(y: np.array, t: float):
    """Detekcia špičiek prahovou úrovňou

    :param y: Postupnosť vzoriek
    :param t: Prahová úroveň. Rozsah: [0, max{y}]
    """
    return np.where(y >= t)


def find_peaks_neighbours(y: np.array, k: int, e: float, h_rel: float, h: float) -> list:
    """Význačnosť vrchola spomedzi susedov
    
    :param y: Postupnosť vzoriek
    :param k: Počet najbližších uvažovaných susedov na každú zo strán od vrchola [i-k; i+k]. Rozsah: [1, length(y) / 2]
    :param e: Relatívna tolerancia vyššieho bodu v susedstve od vrchola. Rozsah: [0, max{y}]
    :param h_rel: Relatívna výška vrcholu v susedstve. Rozsah: [0, max{y}]
    :param h: Absolútna amplitúda. Rozsah: [min{y}, max{y}]
    """
    peaks = []
    for i in range(len(y)):
        if y[i] >= h:
            possible_peak = True
            a = max(i - k, 0)
            b = min(i + k, len(y))
            valley = y[a]

            for j in range(a, b):
                if i != j:
                    if y[j] - y[i] > e:
                        possible_peak = False
                    if y[j] < valley:
                        valley = y[j]

            if possible_peak is True and y[i] - valley >= h_rel:
                peaks.append(i)
    return peaks


def find_peaks_zero_crossing(y: np.array, k: int, slope: float) -> list:
    """Algoritmus prechodu nulou do záporu
    
    :param y: Postupnosť vzoriek
    :param k: dlžka sečnice [i-k; i+k]. Rozsah:[1, length(y) / 2]
    :param slope: prahová strmosť kopca. slope >= 0
    """
    peaks = []

    for i in range(k, len(y) - k):
        if ((y[i + k] - y[i]) < 0 and (y[i] - y[i - k]) > 0 and
                abs(y[i + k] - y[i]) >= slope and abs(y[i] - y[i - k]) >= slope):
            peaks.append(i)

    return peaks


def find_peaks_hill_walker(y: np.array, tolerance: float, hole: float, prominence: float, isolation: float) -> list:
    """Algoritmus horského turistu
    
    :param y: Postupnosť vzoriek
    :param tolerance: Prahová úroveň vo vertikálnej osi. Rozsah: [0, max{y}]
    :param hole: Prahová úroveň v horizontálnej osi. Rozsah: [0, length(y)]
    :param prominence: Relatívna výška oproti predošlej navštívenej doline. Rozsah: [0, max{y}]
    :param isolation: Vzdialenosť k najbližšiemu skoršiemu vrcholu. Rozsah: [0, length(y)]
    """
    peaks = []
    i_change = 0
    y_valley = 0
    possible_change = False
    uphill = bool((y[1] - y[0]) >= 0)
    
    for i in range(1, len(y)):
        y_step = y[i] - y[i - 1]
        slope = bool(y_step >= 0)

        if possible_change is False and uphill != slope:
            possible_change = True
            i_change = i - 1
        elif possible_change is True and uphill == slope:
            possible_change = False
        
        if (possible_change is True and
                uphill != slope and
                abs(i - i_change) > hole and
                abs(y[i] - y[i_change]) > tolerance):
            
            possible_change = False
            prev_uphill = uphill
            uphill = slope

            if prev_uphill is False and uphill is True:
                y_valley = y[i_change]

            elif (prev_uphill is True and 
                      uphill is False and 
                      abs(y[i - hole] - y_valley) > prominence):
                if len(peaks) < 1 or (len(peaks) >= 1 and i_change - peaks[-1] > isolation):
                    y_peak = y[i_change]
                    peaks.append(i_change)
                  
    return peaks


def event_detector(bins: np.array, magnitudes: np.array, peaks: np.array, min_duration=1, time_proximity=0) -> list:
    """Generátor udalostí
    Spektrogram nemáme v online prevádzke dostupný celý naraz, takže detekcia čiar v 2D obrázku neprichádza do úvahy

    :param minimal_duration: Po koľkých oknách je vyhlásená špička za udalosť. Udáva oneskorenie vyhlásenia štartovacieho eventu
    :param time proximity`: Aká veľká môže byť medzera v čase (okná vzad) medzi nájdenými vrcholmi. Udáva oneskorenie vyhlásenia záverečného eventu
    """

    events = []
    for b in range(len(bins)):
        events.append({
            'action': 'NONE',
            'start': 0,
            'last_seen': -1,
            'duration': 0,
            'amplitude': 0 
        }) 
    results = []
    
    for t, window in enumerate(peaks):
        y = np.full(len(bins), False)  # maska nájdených vrcholov podľa indexov
        y[window] = True

        for i in range(len(y)):
            if events[i]['action'] == 'END':
                events[i]['start'] = 0,
                events[i]['last_seen'] = -1
                events[i]['duration'] = 0
                events[i]['amplitude'] = 0
    
            events[i]['action'] = 'NONE'
        
            if y[i]:
                events[i]['last_seen'] += 1
                fastforward = max(1, events[i]['last_seen']);

                if (events[i]['duration'] < min_duration and
                        events[i]['duration'] + fastforward >= min_duration):

                    events[i]['action'] = 'START'
                    events[i]['start'] = t - events[i]['duration'] - fastforward + 1;
                
                    results.append({
                        'action': 'START', 
                        'start':  events[i]['start'],
                        'frequency': bins[i],
                        'amplitude': events[i]['amplitude']
                    })

                events[i]['duration'] += fastforward;
                events[i]['amplitude'] += (magnitudes[t][i] - events[i]['amplitude']) / events[i]['duration']
                events[i]['last_seen'] = 0

            elif events[i]['last_seen'] >= 0:
                events[i]['last_seen'] += 1
                
                if events[i]['last_seen'] > time_proximity: 
                    if events[i]['duration'] >= min_duration:
                        events[i]['action'] = 'END'
                        
                        results.append({
                            'action': 'END', 
                            'start':  events[i]['start'],
                            'duration': events[i]['duration'],
                            'frequency': bins[i],
                            'amplitude': events[i]['amplitude']
                        })
                    else:
                        events[i]['start'] = 0
                        events[i]['last_seen'] = -1
                        events[i]['duration'] = 0
                        events[i]['amplitude'] = 0
                
    # Odošli neukončené udalosti stále vo vyrovnávacej pamäti - v online verzii netreba
    for i in range(len(events)):
        
        if events[i]['action'] == 'END':
            events[i]['start'] = 0,
            events[i]['last_seen'] = -1
            events[i]['duration'] = 0
            events[i]['amplitude'] = 0
        
        if events[i]['duration'] >= min_duration:
            results.append({
                'action': 'END', 
                'start':  events[i]['start'],
                'duration': events[i]['duration'],
                'frequency': bins[i],
                'amplitude': events[i]['amplitude']
            })
                
    return results


def fft_transform(v):
    elements = np.fft.fft(v)
    return np.absolute(elements)[:len(elements)//2]


def fft_log_transform(v):
    yf = fft_transform(v)
    return 20 * np.log10(yf / yf.max())


def rfft_transform(v):
    elements = np.fft.rfft(v)
    return np.absolute(elements)[:-1]


def rfft_log_transform(v):
    yf = rfft_transform(v)
    return 20 * np.log10(yf / yf.max())


def dctii_transform(v):
    elements = scipy.fft.dct(np.array(v), type=2)
    return np.absolute(elements)[:len(elements)//2]


def dctii_log_transform(v):
    yf = dctii_transform(v)
    return 20 * np.log10(yf / yf.max())


def dctiv_transform(v):
    elements = scipy.fft.dct(np.array(v), type=4)
    return np.absolute(elements)[:len(elements)//2]


def dctiv_log_transform(v):
    yf = dctiv_transform(v)
    return 20 * np.log10(yf / yf.max())


def frequency_spectrum(
        t: np.array, 
        y: np.array, 
        fs: int, 
        window,
        Wn: int,
        overlap: float,
        trans='fft', 
        mode='linear'
    ):
    """ Oknovanie (1) a transformácia do frekvenčnej domény (2)

    :param y: Postupnosť vzoriek
    :param t: Časové značky v sekundách pre jednotlivé vzorky
    :param window: Oknová funkcia. Rozsah: boxcar, bartlett, hann, hamming, blackman
    :param Wn: Dĺžka posuvného okna v počte vzoriek. Rozsah: $[1, \mathrm{length}(y)]$
    :param overlap: Pomer prelínania posuvných okien. Rozsah: $[0, 0.99]$
    :param fs: Vzorkovacia frekvencia v Hz
    :param mode: Spôsob transformácie magnitúdového spektra (linear vs. log)
    """
    
    step = Wn - int(Wn * overlap)
    Wi = np.arange(0, len(y) - Wn, step)
    
    if trans == 'fft':
        transform = fft_transform
        if mode == 'log':
            transform = fft_log_transform
    elif trans == 'rfft':
        transform = rfft_transform
        if mode == 'log':
            transform = rfft_log_transform
    elif trans == 'dct-ii':
        transform = dctii_transform
        if mode == 'log':
            transform = dctii_log_transform
    elif trans == 'dct-iv':
        transform = dctiv_transform
        if mode == 'log':
            transform = dctiv_log_transform
    
    win = window(Wn)
    x = t[Wi]
    f = np.fft.fftfreq(Wn, 1/fs)[:Wn//2]
    f_mag = np.asarray([
        transform(y[i:i+Wn] * win) for i in Wi
    ])
     
    return {
        'fs': fs,
        'Wn': Wn,
        'overlap': overlap,
        'resolution': fs / Wn,
        'bins': f,
        't_windows': x,
        'magnitudes': f_mag
    }


def peaks_indices_windowed(peak_find, magnitudes):
    return [peak_find(window) for window in magnitudes]


def peak_freqs_windowed(i_peaks, bins, resolution):
    return [list(zip(bins[p], bins[p] + resolution)) for p in i_peaks] 


def smooth_signal(t, y, window, k: int):
    """Vyhladzovanie signálu"""
    kernel = window(k) / k
    return t[k - 1:], np.convolve(y, kernel, 'valid')


def smooth_spectra(spectrum, window, k: int):
    """Vyhladzovanie oknovaného spektra"""
    kernel = window(k) / k
    spectrum['bins'] = spectrum['bins'][k - 1:]
    spectrum['magnitudes'] = np.array([
        np.convolve(block, kernel, 'valid') 
        for block in spectrum['magnitudes']
    ])

def welch_method(spectrum: dict, n=1):
    spectrum['t_windows'] = spectrum['t_windows'][::n]
    averages = []
    
    for i in range(0, len(spectrum['magnitudes']), n):  
        block_sum = np.zeros(len(spectrum['bins']))
        if i+n > len(spectrum['magnitudes']):
            spectrum['t_windows'] = spectrum['t_windows'][:-1]
            break

        for j in range(i, i+n):
            block_sum += spectrum['magnitudes'][j]
        block_sum /= n
        averages.append(block_sum)
        
    spectrum['magnitudes'] = np.array(averages)
