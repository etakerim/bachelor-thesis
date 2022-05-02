### Experiment: Časová efektivita algoritmov na spracovanie signálu

Dôležité prepínače kompilátora cez `idf.py menuconfig`:
    - Optimalizácia na veľkosť: -Os (`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`)
    - Taktovacia frekvencia: 160 MHz   (`CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ=160`)
    - Tick operačného systému: 100Hz tick  (`CONFIG_FREERTOS_HZ=100`)

V main.c je potrebné sa presvedčiť, že z direktív pre meranie je odkomentovaná len nasledovná: 
```c
#define MEMORY_MEASUREMENT          1
```

Nastav počiatočnú konfiguráciu nástrojom `python config_tool.py`:
```
-> connect
-> set
    {"sensor": {"fs": 8, "range": "2g", "n": 8, "overlap": 0.5, "axis": [true, true, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": false, "subsamp": 1}}
```

Po každom riadku nastavením novej konfigurácie je potrebné čakať na 10 meraní časov z `idf.py monitor | tee memory_usage.txt`
```
-> set
    {"sensor": {"n": 8}, "tsmooth": {"n": 8}, "fsmooth": {"n": 8}}
-> set
    {"sensor": {"n": 16}, "tsmooth": {"n": 16}, "fsmooth": {"n": 16}}
-> set
    {"sensor": {"n": 32}, "tsmooth": {"n": 32}, "fsmooth": {"n": 32}}
-> set
    {"sensor": {"n": 64}, "tsmooth": {"n": 64}, "fsmooth": {"n": 64}}
-> set
    {"sensor": {"n": 128}, "tsmooth": {"n": 128}, "fsmooth": {"n": 128}}
-> set
    {"sensor": {"n": 256}, "tsmooth": {"n": 256}, "fsmooth": {"n": 256}}
-> set
    {"sensor": {"n": 512}, "tsmooth": {"n": 512}, "fsmooth": {"n": 512}}
-> set
    {"sensor": {"n": 1024}, "tsmooth": {"n": 1024}, "fsmooth": {"n": 1024}}
```

Následné fitrovanie relevantných riadkov:
```bash
$ sed -rn 's/^(.*)(MEM.*)$/\2/p' memory_usage.txt > memory_usage_filter.csv
```
