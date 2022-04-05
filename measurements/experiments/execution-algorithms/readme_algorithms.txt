-Os, 80MHz, 100Hz tick
#define EXECUTION_TIME_ALGORITHMS          1

$ idf.py flash
$ idf.py size
$ idf.py -p /dev/ttyUSB0 monitor | tee execution_time.txt
$ python tests/config_tool.py

-> connect
-> config
[imu/1/config/response]:
        {"sensor": {"fs": 8, "range": "2g", "n": 8, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -100.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": false, "subsamp": 1}}

        
// DFT log, Algo 1(x6), 1-axis, 10 opakovaní
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

// DFT log, Algo 2(x6)
{"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dft"}, "peak": {"strategy": "zero_crossing"}}

// DFT log, Algo 3(x6)
{"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dft"}, "peak": {"strategy": "hill_walker"}}

// DCT log, Algo 1(x6)
{"sensor": {"n": 32, "fs": 16}, "transform": {"f": "dct"}, "peak": {"strategy": "neighbours"}}
