import paho.mqtt.client as mqtt
from pynput import keyboard

server = "192.168.43.156"
gantryCameraTopic = "trigger/camera/gantry/entrance"
entranceNumLotsTopic = "status/entrance/number-of-available-lots"
# gantryCameraTopic = "camera/take_photo"
exitNumLotsTopic = "status/exit/number-of-available-lots"
topic = "trigger/camera/lot/1"


def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("#")

def on_message(client, userdata, message):
    print("OMG MESSAGE")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(server, 1883, 60)
client.loop_start()

def on_press(key):
    try:
        if key.char == 'x':  # If 'x' is pressed
            print("Sending '1' to topic")
            client.publish(gantryCameraTopic, "1")
        if key.char == 'b':  # If 'x' is pressed
            print("Sending '99' to topic")
            client.publish(exitNumLotsTopic, "99", retain=True)
            
    except AttributeError:
        pass  # Handle special keys here if needed

# Collect events until released
with keyboard.Listener(on_press=on_press) as listener:
    listener.join()

client.loop_stop()
client.disconnect()
