-Os, 80MHz, 100Hz tick
#define EXECUTION_TIME_SMOOTHING          1

-> config
[imu/1/config/response]:
        {"sensor": {"fs": 8, "range": "2g", "n": 8, "overlap": 0.5, "axis": [false, false, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": false, "mqtt": true, "samples": "", "stats": false, "events": false, "subsamp": 1}}

-> set
 {"sensor": {"n": 512, "fs": 256}}
 
-> set
 {"tsmooth": {"on": true, "n": 4, "repeat": 1}}
 {"tsmooth": {"on": true, "n": 16, "repeat": 1}}
 {"tsmooth": {"on": true, "n": 64, "repeat": 1}}
 
 {"tsmooth": {"on": true, "n": 4, "repeat": 4}}
 {"tsmooth": {"on": true, "n": 16, "repeat": 4}}
 {"tsmooth": {"on": true, "n": 64, "repeat": 4}}
 
 {"tsmooth": {"on": true, "n": 4, "repeat": 8}}
 {"tsmooth": {"on": true, "n": 16, "repeat": 8}}
 {"tsmooth": {"on": true, "n": 64, "repeat": 8}}
 

