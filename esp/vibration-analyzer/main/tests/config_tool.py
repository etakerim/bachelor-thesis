import paho.mqtt.client as mqtt
import msgpack
import json


recv = False
config = b''

# Login
# Prompt

def on_connect(client, userdata, flags, rc):
    global config
    print('Connected with result code: ', rc)
    client.subscribe('imu/1/syslog')
    client.subscribe('imu/1/config/response')
    client.publish('imu/1/config/set', payload=config, qos=1, retain=False)


def on_message(client, userdata, msg):
    global recv
    CONFIG_SYSLOG = (b'config malformed', b'config applied')

    if msg.topic == 'imu/1/syslog':
        if not recv and msg.payload in CONFIG_SYSLOG:
            recv = True
        elif msg.payload == b'imu started':
            client.publish('imu/1/config/request')

    if msg.topic != 'imu/1/syslog':
        s = msgpack.unpackb(msg.payload)
    else:
        s = msg.payload
    print(f'[{msg.topic}]: {s}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    config = json.loads(input("# "))
    config = msgpack.packb(config, use_bin_type=True)

    client.connect('192.168.1.103', 1883, 60)
    client.loop_start()

    #while True:
    #      prompt = json.loads(input("# "))
    #      prompt = msgpack.packb(config, use_bin_type=True)