### Experiment: Časová efektivita vyhladzovacieho filtra

Dôležité prepínače kompilátora cez `idf.py menuconfig`:

- Optimalizácia na veľkosť: -Os (`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`
- Taktovacia frekvencia: 160 MHz   (`CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ=160`)

- Tick operačného systému: 100Hz tick  (`CONFIG_FREERTOS_HZ=100`)

V `main.c` je potrebné sa presvedčiť, že z direktív pre meranie je odkomentovaná len nasledovná: 
```c
#define EXECUTION_TIME_SMOOTHING          1
```
Kompilovanie a nahratie na ESP32:
```bash
$ idf.py build
$ idf.py flash
```

Nastav počiatočnú konfiguráciu nástrojom `python config_tool.py`:
```
-> connect
-> set
    {"sensor": {"fs": 256, "range": "2g", "n": 512, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": false, "subsamp": 1}}
```

Po každom riadku nastavením novej konfigurácie je potrebné čakať na 10 meraní časov: `idf.py monitor | tee execution_time.txt`
```
-> set
    {"tsmooth": {"on": true, "n": 4, "repeat": 1}}
-> set
    {"tsmooth": {"on": true, "n": 16, "repeat": 1}}
-> set
    {"tsmooth": {"on": true, "n": 64, "repeat": 1}}
-> set
    {"tsmooth": {"on": true, "n": 4, "repeat": 4}}
-> set
    {"tsmooth": {"on": true, "n": 16, "repeat": 4}}
-> set
    {"tsmooth": {"on": true, "n": 64, "repeat": 4}}
-> set
    {"tsmooth": {"on": true, "n": 4, "repeat": 8}}
-> set
    {"tsmooth": {"on": true, "n": 16, "repeat": 8}}
-> set
    {"tsmooth": {"on": true, "n": 64, "repeat": 8}}
```


