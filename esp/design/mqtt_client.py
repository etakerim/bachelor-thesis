import paho.mqtt.client as mqtt
import msgpack


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # client.subscribe("$SYS/#")
    client.subscribe("imu/1/stream/statistics/x")
    client.subscribe("imu/1/stream/frequency/x")


def on_message(client, userdata, msg):
    topics = ("imu/1/stream/statistics/x", "imu/1/stream/frequency/x", "imu/1/event/frequency/x")
    if msg.topic in topics:
        print(f'[{msg.topic}]: {msgpack.unpackb(msg.payload)}')
    else:
        print(f'[{msg.topic}]: {msg.payload}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("192.168.1.103", 1883, 60)
    client.loop_forever()