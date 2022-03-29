import paho.mqtt.client as mqtt
import msgpack

"""
{
    'sensor': {'fs': 50, 'range': '2g', 'n': 128, 'overlap': 0.5, 'axis': [True, True, True]}, 
    'tsmooth': {'on': False, 'n': 3, 'repeat': 1}, 
    'stats': {'min': True, 'max': True, 'rms': True, 'avg': True, 'var': True, 'std': True, 'skew': True, 'kurtosis': True, 'med': True, 'mad': True, 'corr': False}, 
    'transform': {'w': 'hann', 'f': 'dft', 'log': False}, 
    'fsmooth': {'on': False, 'n': 4, 'repeat': 1},
    'peak': {'tmin': 1, 'tprox': 0, 'strategy': 'threshold', 'threshold': {'t': -15.0}, 'neighbours': {'k': 8, 'e': 0.0, 'h': -50.0, 'h_rel': 10.0}, 'zero_crossing': {'k': 4, 'slope': 3.0}, 'hill_walker': {'t': 0.0, 'h': 0, 'p': 10.0, 'i': 3.0}}, 
    'logger': {'local': False, 'samples': False, 'stats': True, 'spectra': False, 'events': False, 'subsamp': 1}
}
"""


def listen_to_topics(client):
    # client.subscribe("$SYS/#")
    client.subscribe("imu/1/syslog")
    client.subscribe("imu/1/stream/samples")
    client.subscribe("imu/1/stream/statistics/+")
    client.subscribe("imu/1/stream/frequency/+")
    client.subscribe("imu/1/event/frequency/+")


def send_config(client, conf):
    msg = msgpack.packb(conf, use_bin_type=True)
    client.publish("imu/1/config/set", payload=msg, qos=1, retain=False)


def get_config(client):
    client.publish("imu/1/config/request")


def on_connect(client, userdata, flags, rc):
    print("Connected with result code: ", rc)
    # listen_to_topics(client)

    client.subscribe("imu/1/syslog")
    client.subscribe("imu/1/config/response")
    send_config(client, {"sensor": {"n": 128}})


def on_message(client, userdata, msg):
    if msg.topic != "imu/1/syslog":
        s = msgpack.unpackb(msg.payload)
    else:
        s = msg.payload

    print(f'[{msg.topic}]: {s}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("192.168.1.103", 1883, 60)
    client.loop_forever()