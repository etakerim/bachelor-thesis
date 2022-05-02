### Experiment: Časová efektivita algoritmov na spracovanie signálu

Dôležité prepínače kompilátora cez `idf.py menuconfig`:
    - Optimalizácia na veľkosť: -Os (`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`)
    - Taktovacia frekvencia: 160 MHz   (`CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ=160`)
    - Tick operačného systému: 100Hz tick  (`CONFIG_FREERTOS_HZ=100`)

V main.c je potrebné sa presvedčiť, že z direktív pre meranie je odkomentovaná len nasledovná: 
```c
#define EXECUTION_TIME_ALGORITHMS          1
```

Nastav počiatočnú konfiguráciu nástrojom `python config_tool.py`:
```
-> connect
-> set
    {"sensor": {"fs": 8, "range": "2g", "n": 8, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -100.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": false, "subsamp": 1}}
```

Po každom riadku nastavením novej konfigurácie je potrebné čakať na 10 meraní časov z `idf.py monitor | tee execution_algorithms.txt`
```
-> set
    {"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dft"}, "peak": {"strategy": "neighbours"}}
-> set
    {"sensor": {"n": 32, "fs": 16}}
-> set
    {"sensor": {"n": 64, "fs": 32}}
-> set
    {"sensor": {"n": 128, "fs": 64}}
-> set
    {"sensor": {"n": 256, "fs": 128}}
-> set
    {"sensor": {"n": 512, "fs": 256}}
-> set
    {"sensor": {"n": 1024, "fs": 512}}
    
-> set
    {"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dft"}, "peak": {"strategy": "zero_crossing"}}
-> set
    {"sensor": {"n": 32, "fs": 16}}
-> set
    {"sensor": {"n": 64, "fs": 32}}
-> set
    {"sensor": {"n": 128, "fs": 64}}
-> set
    {"sensor": {"n": 256, "fs": 128}}
-> set
    {"sensor": {"n": 512, "fs": 256}}
-> set
    {"sensor": {"n": 1024, "fs": 512}}
    
-> set
    {"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dft"}, "peak": {"strategy": "hill_walker"}}
-> set
    {"sensor": {"n": 32, "fs": 16}}
-> set
    {"sensor": {"n": 64, "fs": 32}}
-> set
    {"sensor": {"n": 128, "fs": 64}}
-> set
    {"sensor": {"n": 256, "fs": 128}}
-> set
    {"sensor": {"n": 512, "fs": 256}}
-> set
    {"sensor": {"n": 1024, "fs": 512}}
    

-> set
    {"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dct"}, "peak": {"strategy": "neighbours"}}
-> set
    {"sensor": {"n": 32, "fs": 16}}
-> set
    {"sensor": {"n": 64, "fs": 32}}
-> set
    {"sensor": {"n": 128, "fs": 64}}
-> set
    {"sensor": {"n": 256, "fs": 128}}
-> set
    {"sensor": {"n": 512, "fs": 256}}
-> set
    {"sensor": {"n": 1024, "fs": 512}}
```



