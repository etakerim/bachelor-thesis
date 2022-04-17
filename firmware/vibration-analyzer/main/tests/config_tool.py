#!/usr/bin/env python
import cmd
import json
import time
import sys
from threading import Barrier
import paho.mqtt.client as mqtt
import msgpack


def mqtt_connect(client, userdata, flags, rc):
    print('Connected with result code: ', rc)
    userdata.connected = True
    userdata.done.wait()


def mqtt_disconnect(client, userdata, rc):
    print('Disconnected with result code: ', rc)
    userdata.connected = False
    userdata.done.wait()


def mqtt_message(client, userdata, msg):

    if msg.topic != f'{userdata.prefix}/syslog':
        s = json.dumps(msgpack.unpackb(msg.payload))
    else:
        s = msg.payload.decode('utf-8')
    print(f'[{msg.topic}]:\n\t{s}')

    if msg.topic == f'{userdata.prefix}/syslog':
        if not userdata.recv and msg.payload == b'config received':
            userdata.recv = True

        if userdata.recv and msg.payload in (
                b'imu started', b'config malformed'):
            client.unsubscribe(f'{userdata.prefix}/syslog')
            userdata.recv = False
            userdata.done.wait()

        if msg.payload in (b'login saved', b'login malformed'):
            client.unsubscribe(f'{userdata.prefix}/syslog')
            userdata.done.wait()

    elif msg.topic == f'{userdata.prefix}/config/response':
        client.unsubscribe(f'{userdata.prefix}/config/response')
        userdata.done.wait()

    elif msg.topic == f'{userdata.prefix}/login/response':
        client.unsubscribe(f'{userdata.prefix}/login/response')
        userdata.done.wait()


class DeviceShell(cmd.Cmd):
    intro = (
        'Interactive communication with Inertial Measurement Unit. '
        'Type help or ? to list commands.'
    )
    prompt = '-> '

    def __init__(self):
        self.client = mqtt.Client(userdata=self)
        self.client.on_connect = mqtt_connect
        self.client.on_disconnect = mqtt_disconnect
        self.client.on_message = mqtt_message
        self.prefix = ''

        self.connected = False
        self.prefix = False
        self.recv = False

        self.done = Barrier(2)

        super().__init__()

    def do_connect(self, arg):
        if self.connected:
            print('Error: Client is already connected. Disconnect first.')
            return

        dev = input('Device ID [1]: ')
        ip = input('Broker IP [192.168.1.103]: ')
        port = input('Broker Port [1883]: ')

        dev = dev if dev else '1'
        ip = ip if ip else '192.168.1.103'
        port = int(port) if port else 1883

        self.prefix = f'imu/{dev}'
        self.client.connect(ip, port, 60)
        self.client.loop_start()
        self.done.wait()

    def do_disconnect(self, arg):
        if not self.connected:
            print('Error: Client is not connected.')
            return
        self.client.disconnect()
        self.done.wait()
        self.client.loop_stop()

    def do_set(self, arg):
        """ Upload changes in configuration to the device."""
        if not self.connected:
            print('Error: Client is not connected to MQTT Broker.')
            return

        try:
            config = json.loads(input('config> '))
        except json.decoder.JSONDecodeError:
            print('Error: JSON format of config expected')
            return

        config = msgpack.packb(config, use_bin_type=True)
        self.client.subscribe(f'{self.prefix}/syslog')
        self.client.publish(f'{self.prefix}/config/set', payload=config, qos=1, retain=False)
        self.done.wait()

    def do_config(self, arg):
        """ Show current device configuration """
        if not self.connected:
            print('Error: Client is not connected to MQTT Broker.')
            return

        self.client.subscribe(f'{self.prefix}/config/response')
        self.client.publish(f'{self.prefix}/config/request')
        self.done.wait()

    def do_login(self, arg):
        """ Upload changes in network credentials to the device."""
        if not self.connected:
            print('Error: Client is not connected to MQTT Broker.')
            return

        try:
            config = json.loads(input('login> '))
        except json.decoder.JSONDecodeError:
            print('Error: JSON format of config expected')
            return

        config = msgpack.packb(config, use_bin_type=True)
        self.client.subscribe(f'{self.prefix}/syslog')
        self.client.publish(f'{self.prefix}/login/set', payload=config, qos=1, retain=False)
        self.done.wait()

    def do_credentials(self, arg):
        """ Show current network credentials """
        if not self.connected:
            print('Error: Client is not connected to MQTT Broker.')
            return

        self.client.subscribe(f'{self.prefix}/login/response')
        self.client.publish(f'{self.prefix}/login/request')
        self.done.wait()

    def do_topic(self, arg):
        """ Listen to messages on topic """
        if not self.connected:
            print('Error: Client is not connected to MQTT Broker.')
            return

        topic = input('topic> ')
        if not topic:
            print('Error: No topic to listen to')
            return

        wait = input('Listen time on topic in sec [10]: ')
        try:
            wait = int(wait) if wait else 10
        except ValueError:
            print('Error: wait time must be integer')
            return

        self.client.subscribe(f'{self.prefix}/{topic}')
        try:
            time.sleep(wait)
        finally:
            self.client.unsubscribe(f'{self.prefix}/{topic}')

    def do_end(self, arg):
        """ Exit shell """
        print('Exiting device shell ...')
        if self.connected:
            self.client.disconnect()
            self.done.wait()
        sys.exit()

    def cmdloop(self, intro=None):
        print(self.intro)
        while True:
            try:
                super().cmdloop(intro="")
                break
            except KeyboardInterrupt:
                print("^C")


if __name__ == '__main__':
    DeviceShell().cmdloop()
