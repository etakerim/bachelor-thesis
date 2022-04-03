#define MEMORY_MEASUREMENT          1

$ idf.py flash
$ idf.py -p /dev/ttyUSB0 monitor | tee data_stuctures.txt
$ python tests/config_tool.py

-> connect
-> config
[imu/1/config/response]:
        {"sensor": {"fs": 2, "range": "2g", "n": 8, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": true}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "samples": true, "stats": true, "spectra": true, "events": true, "subsamp": 1}}

-> set
 {"sensor": {"n": 8, "fs": 2}}
-> set
 {"sensor": {"n": 16, "fs": 4}}
-> set
 {"sensor": {"n": 32, "fs": 8}}
-> set
 {"sensor": {"n": 64, "fs": 16}}
-> set
 {"sensor": {"n": 128, "fs": 32}}
-> set
 {"sensor": {"n": 256, "fs": 64}}
-> set
 {"sensor": {"n": 512, "fs": 128}}
-> set
 {"sensor": {"n": 1024, "fs": 256}}

 
{"logger": {"local": false, "samples": false, "stats": true, "spectra": true, "events": true, "subsamp": 1}}

-- ERROR
 {"sensor": {"fs": 256, "range": "2g", "n": 1024, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": true}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "samples": false, "stats": true, "spectra": true, "events": true, "subsamp": 1}}
