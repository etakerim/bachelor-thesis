import paho.mqtt.client as mqtt
import msgpack
import base64


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # client.subscribe("$SYS/#")

    # client.publish("imu/1/config/request")
    client.subscribe("imu/1/config/response")
    # client.publish("imu/1/config/load")
    # client.subscribe("imu/1/syslog")

    client.subscribe("imu/1/stream/samples")
    client.subscribe("imu/1/stream/statistics/x")
    client.subscribe("imu/1/stream/frequency/x")
    client.subscribe("imu/1/event/frequency/x")


def on_message(client, userdata, msg):
    topics = ("imu/1/stream/samples", "imu/1/stream/statistics/x", "imu/1/stream/frequency/x", "imu/1/event/frequency/x")

    #if msg.topic in topics:
    d = msgpack.unpackb(msg.payload)
    print(f'[{msg.topic}]: {d}')
    #else:
    #    print(f'[{msg.topic}]: {msg.payload}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("192.168.100.198", 1883, 60)
    client.loop_forever()