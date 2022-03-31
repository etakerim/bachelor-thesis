import paho.mqtt.client as mqtt
import msgpack
import json
import time

conn = False
recv = False
done = False
prefix = ''


def on_connect(client, userdata, flags, rc):
    global conn
    print('Connected with result code: ', rc)
    conn = True


def on_message(client, userdata, msg):
    global recv, done, prefix
    CONFIG_SYSLOG = (b'config malformed', b'config applied')

    if msg.topic == f'{prefix}/syslog':
        if not recv and msg.payload in CONFIG_SYSLOG:
            recv = True
        elif msg.payload == b'imu started':
            client.unsubscribe(f'{prefix}/syslog')
            recv = False
            done = True

    elif msg.topic == f'{prefix}/config/response':
        client.unsubscribe(f'{prefix}/config/response')
        done = True

    if msg.topic != f'{prefix}/syslog':
        s = msgpack.unpackb(msg.payload)
    else:
        s = msg.payload.decode('utf-8')
    print(f'[{msg.topic}]: {s}')


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    dev = input('Device ID [1]: ')
    ip = input('Broker IP [192.168.1.103]: ')
    port = input('Broker Port [1883]: ')

    dev = dev if dev else '1'
    ip = ip if ip else '192.168.1.103'
    port = int(port) if port else 1883
    prefix = f'imu/{dev}'

    client.connect(ip, port, 60)
    client.loop_start()
    while not conn:
        time.sleep(1)

    while True:
        try:
            cmd = input('> ')

            if cmd == 'set':
                try:
                    config = json.loads(input('config> '))
                except json.decoder.JSONDecodeError as err:
                    print('Error: JSON format of config expected')
                    continue

                config = msgpack.packb(config, use_bin_type=True)
                client.subscribe(f'{prefix}/syslog')
                client.publish(
                    f'{prefix}/config/set',
                    payload=config,
                    qos=1, retain=False
                )
                while not done:
                    time.sleep(1)
                done = False

            elif cmd == 'config':
                client.subscribe(f'{prefix}/syslog')
                client.subscribe(f'{prefix}/config/response')
                client.publish(f'{prefix}/config/request')
                recv = True

                while not done:
                    time.sleep(1)
                done = False

            elif cmd == 'topic':
                topic = input('topic> ')
                if not topic:
                    print('Error: No topic to listen to')
                    continue
                wait = input('Listen time on topic in sec [10]: ')
                try:
                    wait = int(wait) if wait else 10
                except ValueError:
                    print('Error: wait time must be integer')
                    continue

                client.subscribe(f'{prefix}/{topic}')
                try:
                    time.sleep(wait)
                finally:
                    client.unsubscribe(f'{prefix}/{topic}')

            elif cmd == 'end':
                break

        except KeyboardInterrupt:
            print('Help: To exit enter command: "end"')

    print('Exiting intractive command prompt ...')