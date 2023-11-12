# server = "192.168.10.111"
# server = "192.168.86.181"
# server = "192.168.86.80"
# server = "192.168.43.156"
server = "192.168.43.186"

import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    # client.subscribe("event/#")
    # client.subscribe("status/#")
    # client.subscribe("result/#")
    client.subscribe("#")

def on_message(client, userdata, message):
    topic = message.topic
    msg = message.payload.decode("utf-8")
    print("Received message '{}' from topic '{}'".format(msg, topic))
    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(server, 1883, 60)
client.loop_forever()
