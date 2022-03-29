import paho.mqtt.client as mqtt
import msgpack

'''
{
    'sensor': {'fs': 50, 'range': '2g', 'n': 128, 'overlap': 0.5, 'axis': [True, True, True]}, 
    'tsmooth': {'on': False, 'n': 3, 'repeat': 1}, 
    'stats': {'min': True, 'max': True, 'rms': True, 'avg': True, 'var': True, 'std': True, 'skew': True, 'kurtosis': True, 'med': True, 'mad': True, 'corr': False}, 
    'transform': {'w': 'hann', 'f': 'dft', 'log': False}, 
    'fsmooth': {'on': False, 'n': 4, 'repeat': 1},
    'peak': {'tmin': 1, 'tprox': 0, 'strategy': 'threshold', 'threshold': {'t': -15.0}, 'neighbours': {'k': 8, 'e': 0.0, 'h': -50.0, 'h_rel': 10.0}, 'zero_crossing': {'k': 4, 'slope': 3.0}, 'hill_walker': {'t': 0.0, 'h': 0, 'p': 10.0, 'i': 3.0}}, 
    'logger': {'local': False, 'samples': False, 'stats': True, 'spectra': False, 'events': False, 'subsamp': 1}
}
'''


def listen_to_topics(client):
    # client.subscribe('$SYS/#')
    client.subscribe('imu/1/syslog')
    client.subscribe('imu/1/stream/samples')
    client.subscribe('imu/1/stream/statistics/+')
    client.subscribe('imu/1/stream/frequency/+')
    client.subscribe('imu/1/event/frequency/+')


def send_config(client, conf):
    msg = msgpack.packb(conf, use_bin_type=True)
    client.publish('imu/1/config/set', payload=msg, qos=1, retain=False)


recv = False
success = False
idx = 0
configs = [
    {'sensor': {'fs': 50}},
    {'sensor': {'range': '4g'}},
    {'sensor': {'n': 64}},
    {'sensor': {'overlap': 0.5}},
    {'sensor': {'axis': [False, True, False]}},
    {'tsmooth': {'on': True}},
    {'tsmooth': {'n': 5}},
    {'peak': {'neighbours': {'k': 6}}},
]


def compare_change(i, gotten, expected):
    # Potenciálne vylepšenie: rekurzívne overenie podľa kľúčov v 'expected'
    if i == 0:
        return gotten['sensor']['fs'] == expected['sensor']['fs']
    elif i == 1:
        return gotten['sensor']['range'] == expected['sensor']['range']
    elif i == 2:
        return gotten['sensor']['n'] == expected['sensor']['n']
    elif i == 3:
        return gotten['sensor']['overlap'] == expected['sensor']['overlap']
    elif i == 4:
        return gotten['sensor']['axis'] == expected['sensor']['axis']
    elif i == 5:
        return gotten['tsmooth']['on'] == expected['tsmooth']['on']
    elif i == 6:
        return gotten['tsmooth']['n'] == expected['tsmooth']['n']
    elif i == 7:
        return gotten['peak']['neighbours']['k'] == expected['peak']['neighbours']['k']


def config_test(msg):
    global idx, success, recv
    CONFIG_SYSLOG = (b'config malformed', b'config applied')

    if msg.topic == 'imu/1/syslog':
        if not recv:
            recv = True
            if msg.payload == b'config malformed':
                success = False
            elif msg.payload == b'config applied':
                success = True
        else:
            if msg.payload == b'imu started':
                client.publish('imu/1/config/request')
                print("Send")

    elif msg.topic == 'imu/1/config/response':
        conf = msgpack.unpackb(msg.payload)
        if recv:
            recv = False
            if success:
                success = False
                assert compare_change(idx, conf, configs[idx])
            idx += 1
            print("idx", idx)
            if idx == len(configs):
                print('Done')
            else:
                send_config(client, configs[idx])


def on_connect(client, userdata, flags, rc):
    print('Connected with result code: ', rc)
    listen_to_topics(client)

    client.subscribe('imu/1/syslog')
    client.subscribe('imu/1/config/response')
    #send_config(client, configs[idx])


def on_message(client, userdata, msg):
    # config_test(msg)

    if msg.topic != 'imu/1/syslog':
        s = msgpack.unpackb(msg.payload)
    else:
        s = msg.payload
    print(f'[{msg.topic}]: {s}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect('192.168.1.103', 1883, 60)
    client.loop_forever()