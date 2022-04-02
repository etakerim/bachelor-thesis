ESP_LOGW("main", "MEM, %d, %u, %u, %u", i,
        heap_caps_get_total_size(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
);

-Os
#define MEMORY_MEASUREMENT          1

$ idf.py flash
$ idf.py size
$ idf.py -p /dev/ttyUSB0 monitor | tee memory_usage.txt
$ python tests/config_tool.py

-> connect
-> config
[imu/1/config/response]:
        {"sensor": {"fs": 8, "range": "2g", "n": 8, "overlap": 0.5, "axis": [true, true, true]}, "tsmooth": {"on": false, "n": 8, "repeat": 1}, "stats": {"min": true, "max": true, "rms": true, "avg": true, "var": true, "std": true, "skew": true, "kurtosis": true, "med": true, "mad": true, "corr": false}, "transform": {"w": "hann", "f": "dft", "log": true}, "fsmooth": {"on": false, "n": 8, "repeat": 1}, "peak": {"tmin": 4, "tprox": 5, "strategy": "threshold", "threshold": {"t": -15.0}, "neighbours": {"k": 9, "e": 0.0, "h": -50.0, "h_rel": 10.0}, "zero_crossing": {"k": 4, "slope": 3.0}, "hill_walker": {"t": 0.0, "h": 0, "p": 10.0, "i": 3.0}}, "logger": {"local": true, "samples": false, "stats": true, "spectra": false, "events": false, "subsamp": 1}}

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
 
$ sed -rn 's/^(.*)(MEM.*)$/\2/p' memory_usage.txt > memory_usage_filter.csv
Manuálne upravíme tabuľku
