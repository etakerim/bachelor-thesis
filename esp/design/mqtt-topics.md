### Špecifikácia MQTT topics pre analyzátor vibrácií z akcelerácie

MQTT broker: mosquitto (https://mosquitto.org/), MQTT Broker (https://play.google.com/store/apps/details?id=in.naveens.mqttbroker)

MQTT klient: MQTT.fx(https://mqttfx.jensd.de/), paho-mqtt python

Formát dát: MessagePack (okrem Syslog, kde bude prenášaný syslog)
Topics z pohľadu mqtt

- **PUBLISH imu/\<uuid>/syslog**
- **SUBSCRIBE imu/\<uuid>/config/set**
- **PUBLISH imu/\<uuid>/config/get**
- **PUBLISH imu/\<uuid>/event/frequency**
- **PUBLISH imu/\<uuid>/stream/samples**
- **PUBLISH imu/\<uuid>/stream/frequency**
- **PUBLISH imu/\<uuid>/stream/statistics**

#### Konfigurácia JSON  (MessagePack)
```
{
	"samples": {
        "freq": uint16,
        "n": uint16,
        "range": "2G" / "4G" / "8G" / "16G", (enum)
        "axis": {
        	"x": bool,
        	"y": bool,
        	"z": bool
    	},
        "above": {
        	"x": float,
        	"y": float,
        	"z": float
    	},
    	"zerog": bool
        "unit": "SI" / "G" / "raw"
    },
    "smoothT": {
        "on": bool,
        "window": "boxcar" / "bartlett" / "hann" / "hamming" / "blackman", (enum)
        "k": uint16,
        "repeat":  uint8
    },
    "stats": {
        "n": uint16,
        "min": bool,
        "max": bool,
        "rms": bool,
        "mean": bool,
        "var": bool,
        "std": bool,
        "skew": bool,
        "kurt": bool,
        "corrXY": bool,
        "corrXZ": bool,
        "corrYZ": bool,
        "median": bool,
        "mad": bool,
        "aad": bool
    },
    "transformF": {
        "on": bool
        "window": "boxcar" / "bartlett" / "hann" / "hamming" / "blackman", (enum)
        "k": uint16,
        "overlap": float,
        "conv": "fft" / "dctii", (enum) 
        "log": "none" / "power" / "dB" (enum)
    },
    "smoothF": {
        "on": bool,
        "window": "boxcar" / "bartlett" / "hann" / "hamming" / "blackman" (enum)
        "k": uint16
        "repeat": uint8 
    },
    "welchAvg": {
        "on": bool,
        "n": uint8
    },
    "eventsF": {
        "on": bool,
        "strategy": "threshold" / "neighbours" / "0crossing" / "hillWalker" (enum)
        "threshold": {
            "warn": float
            "alert": float
        },
        "neighbours": {
            "k": uint16,
            "e": float,
            "hRel": float,
            "h": float
        },
        "0crossing": {
            "k": uint16,
            "slope": float
        },
    	"hillWalker": {
            "tolerance": float,
            "hole": uint16,
            "prominence": float,
            "isolation": uint16
        }
    },
    "logger": {
        samples: bool,
        stats: bool,
        eventsF: bool,
        transformF: bool
    },
    "mqtt": {
        samples: bool,
        stats: bool,
        eventsF: bool,
        transformF: bool
    }
}
```





