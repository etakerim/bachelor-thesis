### Experiment: Časová efektivita vyhladzovacieho filtra

Dôležité prepínače kompilátora cez `idf.py menuconfig`:

- Optimalizácia na veľkosť: -Os (`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`
- Taktovacia frekvencia: 160 MHz   (`CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ=160`)

- Tick operačného systému: 100Hz tick  (`CONFIG_FREERTOS_HZ=100`)

V `main.c` je potrebné sa presvedčiť, že z direktív pre meranie je odkomentovaná len nasledovná: 

```c
#define EXECUTION_TIME_PIPELINE          1
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
    {"sensor": {"fs": 16, "range": "2g", "n": 32, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": false, "max": false, "rms": false, "avg": false, "var": false, "std": false, "skew": false, "kurtosis": false, "med": false, "mad": false, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": true, "n": 4, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "neighbours", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -100.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": true, "subsamp": 1}}
```

Po každom riadku nastavením novej konfigurácie je potrebné čakať na 10 meraní časov: `idf.py monitor | tee execution_time.txt`

### Tabuľka A (9x) - 1 os

```
-> set
{"sensor": {"fs": 16, "n": 32, "axis": [false, false, true]}, "transform": {"f": "dft", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 16, "n": 32, "axis": [false, false, true]}, "transform": {"f": "dct", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}
```

### Tabuľka B (9x) - 3 osi

```
{"sensor": {"fs": 16, "n": 32, "axis": [true, true, true]}, "transform": {"f": "dft", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 16, "n": 32, "axis": [true, true, true]}, "transform": {"f": "dct", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}
```

### Tabuľka C (stats)

```
{"sensor": {"fs": 16, "n": 32, "axis": [false, false, true]},  "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": true},  "logger": {"samples": "f", "stats": true}, "transform": {"f": "dft", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}
```

### Tabuľka D

```
{"sensor": {"fs": 16, "n": 32, "axis": [true, true, true]}, "transform": {"f": "dft", "log": true}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 128, "n": 256}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}

{"sensor": {"fs": 512, "n": 1024}, "peak": {"strategy": "neighbours"}}
{"peak": {"strategy": "zero_crossing"}}
{"peak": {"strategy": "hill_walker"}}
```

```bash
sed -rn '/^(.*)(main:.*)$/p' pipeline_time-A.txt > pipeline_time-A_filter.csv
sed -rn '/^(.*)(main:.*)$/p' pipeline_time-B.txt > pipeline_time-B_filter.csv
sed -rn '/^(.*)(main:.*)$/p' pipeline_time-C.txt > pipeline_time-C_filter.csv
sed -rn '/^(.*)(main:.*)$/p' pipeline_time-D.txt > pipeline_time-D_filter.csv
```

