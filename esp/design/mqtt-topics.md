### Špecifikácia MQTT topics pre analyzátor vibrácií z akcelerácie

MQTT broker: mosquitto (https://mosquitto.org/), MQTT Broker (https://play.google.com/store/apps/details?id=in.naveens.mqttbroker)

MQTT klient: MQTT.fx(https://mqttfx.jensd.de/), paho-mqtt python

Formát dát: MessagePack (okrem Syslog, kde bude prenášaný syslog)



#### imu/<uuid>/status

#### imu/<uuid>/syslog

#### imu/<uuid>/config/set

#### imu/<uuid>/config/get

#### imu/<uuid>/event/frequency

#### imu/<uuid>/event/statistics/+

#### imu/<uuid>/stream/samples

#### imu/<uuid>/stream/frequency

#### imu/<uuid>/stream/statistics/+





