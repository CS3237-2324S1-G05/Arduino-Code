# import paho.mqtt.client as mqtt

# def on_connect(client, userdata, flags, rc):
#     print("Connected with result code: " + str(rc))
#     client.subscribe("weather/temp")

# def on_message(client, userdata, message):
#     temperature = float(message.payload.decode("utf-8"))
#     print("Received:" + str(temperature))
#     client.publish("class/weather", "hot")

# Server receives on status/gantry/entrance for "Entrance alive"
# Server receives on event/gantry/entrance for "Object Detected"
# Server sends on status/human-presence for "TRUE" or "FALSE"
# Server sends on event/gantry/entrance for "LOT: 004"

server = "192.168.10.111"
# server = "192.168.86.80"
# server = "192.168.86.181"

import random
import paho.mqtt.client as mqtt
import time
import threading

exitAliveTopic = "status/gantry/exit"
exitEventTopic = "event/gantry/exit"
exitCarplateTopic = "status/exit/carplate"
exitParkingFeeTopic = "status/exit/parking-fee"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("event/#")
    client.subscribe("status/#")
    client.subscribe("result/#")

def delayed_publish(client):
    time.sleep(8)
    client.publish(exitCarplateTopic, "SGX1234A")
    time.sleep(2)
    client.publish(exitParkingFeeTopic, "None")

def on_message(client, userdata, message):
    topic = message.topic
    msg = message.payload.decode("utf-8")
    print("Received message '{}' from topic '{}'".format(msg, topic))
    
    if topic == exitEventTopic and msg == "Object Detected":
        # Start a new thread to handle the delayed message publishing
        threading.Thread(target=delayed_publish, args=(client,)).start()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(server, 1883, 60)
client.loop_forever()
