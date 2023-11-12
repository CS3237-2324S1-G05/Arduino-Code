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
# Server sends on status/entrance/human-presence for "TRUE" or "FALSE"
# Server sends on status/entrance/carplate for "SGX1234A"
# Server sends on status/entrance/nearest-lot for "LOT: 004"

server = "192.168.10.111"
# server = "192.168.86.181"
# server = "192.168.86.80"

import random
import paho.mqtt.client as mqtt
import time
import threading

entranceAliveTopic = "status/gantry/entrance"
entranceEventTopic = "event/gantry/entrance"
entranceHumanStatusTopic = "status/entrance/human-presence"
entranceCarplateTopic = "status/entrance/carplate"
entranceNearestLotTopic = "status/entrance/nearest-lot"
entranceNumLotsTopic = "status/number-of-available-lots"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("event/#")
    client.subscribe("status/#")
    client.subscribe("result/#")

def delayed_publish(client):
    client.publish(entranceNearestLotTopic, "400")
    time.sleep(5)
    client.publish(entranceCarplateTopic, "SGX1234A")
    time.sleep(8)
    if (random.randint(0, 1) == 0):
        client.publish(entranceHumanStatusTopic, "0")
    else:
        client.publish(entranceHumanStatusTopic, "1")


def on_message(client, userdata, message):
    topic = message.topic
    msg = message.payload.decode("utf-8")
    print("Received message '{}' from topic '{}'".format(msg, topic))
    
    if topic == entranceEventTopic and msg == "Object Detected":
        # Start a new thread to handle the delayed message publishing
        threading.Thread(target=delayed_publish, args=(client,)).start()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(server, 1883, 60)
client.loop_forever()
