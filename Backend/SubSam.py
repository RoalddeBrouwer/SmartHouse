import paho.mqtt.client as mqtt
from bitstring import ConstBitStream
import struct
from d7a.alp.parser import Parser as AlpParser
import threading
from localization_final import localize as localize
import ttn
import paho.mqtt.client as paho
import json, ast
import base64


app_id = "iotsam"
access_key = "ttn-account-v2.gr0pgblPiPEd8A3yn0bn79x11BgBSs7D99JMwAtgyqE"
dev_id = "lorasam"
gatewayList = ["4337313400210032", "433731340023003d", "42373434002a0049", "463230390032003e"]
rx_values = [0, 0, 0, 0]
receivedList = []
count = 0
ACCESS_TOKEN = 'CrKsJXmAp2qsJfd0C9Pc'  # Token of your device on thingsboard
broker = "thingsboard.idlab.uantwerpen.be"  # host name thingsboard

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    client.subscribe("/d7/4836383700440038/#")
# The callback for when a dash7 message is received.
def on_message(client, userdata, msg):
        global count, temp , desi
        count += 1
        print "dash 7 "
        if count == 1:
            timer = threading.Timer(1.0, process_data_after_msg_received)
            timer.start()
            # get the payload from the message
            data = bytearray(msg.payload.decode("hex"))
            result = AlpParser().parse(ConstBitStream(data), len(data))
            # get the data from the payload
            getData = result.actions.__getitem__(0)
            getData = getData.operand.data

            # turn data into temperature and desired temperature and remove brackets
            des = getData[0] - 48;
            des2 = getData[1] - 48;
            desi = int(str(des) + str(des2))
            aa = bytearray(getData[2:6])
            temp = struct.unpack('<f', aa)
            temp = str(temp).replace('(', "")
            temp = temp.replace(',', '')
            temp = temp.replace(')', '')

        # get the rx values from the message
        hexstring = str(msg.payload.decode("utf-8"))
        data = bytearray(hexstring.decode("hex"))
        parsedCommando = AlpParser().parse(ConstBitStream(data), len(data))  # from the AlpParser example

        if parsedCommando.actions[0].operand.offset.id == 64:
            global rx_values
            rxLevel = parsedCommando.interface_status.operand.interface_status.rx_level
            # Extract the gateway_id & device_id from msg
            empty, d7, device_id, gateway_id = msg.topic.split("/")
            # Check which gateways we receive
            if gateway_id not in receivedList and gateway_id in gatewayList:
                receivedList.append(gateway_id)
                if gateway_id == '4337313400210032':
                    rx_values[0] = rxLevel
                if gateway_id == '433731340023003d':
                    rx_values[1] = rxLevel
                if gateway_id == '42373434002a0049':
                    rx_values[2] = rxLevel
                if gateway_id == '463230390032003e':
                    rx_values[3] = rxLevel

def on_publish(client1, userdata, result):  # create function for callback
            pass

def process_data_after_msg_received():
            # Fill the missing rx values with 200 for localisation
            global location, count, desi, temp
            if len(receivedList) == 4:
                count = 0
            elif len(receivedList) == 3:
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;
            elif len(receivedList) == 2:
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;
            elif len(receivedList) == 1:
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;

            # Get location based on fingerprinting (rx_values, k-nearest neighbors)
            location = localize(rx_values, 7)
            for i in range(len(rx_values)):
                rx_values[i] = 0
            # Fill the JSON file with information
            loadJson()
            receivedList[:] = []

def loadJson():
            # Transform the location to the corresponding values on the map
            mapCoordinates = processCoordinates(location)
            # Transform data into JSON format
            tempCoor = {
                'xPos': mapCoordinates['x'],
                'yPos': mapCoordinates['y'],
                'Gateways': len(receivedList),
                'Temperature': temp,
                'Mode' : 0,
                'Desired': desi}
            # Open JSON file
            with open("telemetry.json", "w") as feedsjson:
                json.dump(tempCoor, feedsjson)
            # Fill JSON file
            with open('telemetry.json') as json_file:
                global payload
                payload = json.load(json_file)
                payload = ast.literal_eval(json.dumps(payload))
            # Publish data to thingsboard
            client1.publish("v1/devices/me/telemetry", str(payload))  # topic-v1/devices/me/telemetry

def loadJsonLora():
    # Transform to JSON format
    tempCoor = {
        'Temperature': 'out',
        'Desired': tempLora,
        'xPos': 0.9,
        'yPos': 0.9,
        'Gateways': 0,
        'Mode': 1} # 1 = LoraWan
    # Open JSON file
    with open("telemetry.json", "w") as feedsjson:
        json.dump(tempCoor, feedsjson)
    # Fill JSON file
    with open('telemetry.json') as json_file:
        global payload
        payload = json.load(json_file)
        payload = ast.literal_eval(json.dumps(payload))
    # Publish data to thingsboard
    client1.publish("v1/devices/me/telemetry", str(payload))  # topic-v1/devices/me/telemetry

def processCoordinates(coords):
            # Transform the coordinates
            mapX = coords['x'] * 0.06475 + 0.155
            mapY = coords['y'] * 0.059875 + 0.151
            return {'x': mapX, 'y': mapY}

def uplink_callback(msg, client): # Function when we receive a Lora message
    global tempLora, desiredLora
    # Get the payload from the message
    desiredLora = list(base64.b64decode(msg.payload_raw).encode('hex'))
    # Transform the payload to desired temperature
    tempLora = str(desiredLora[9]+desiredLora[11])
    # Fill the JSON file with information
    loadJsonLora()

def connect_callback(res, client):
    pass

def connect_downlink(msg, client):
    pass

# mqtt client to receive DASH7 messages
port = 1883  # data listening port
broker_address = "student-04.idlab.uantwerpen.be"
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker_address, port, 60)

# ttn client to receive Lora messages
handler = ttn.HandlerClient(app_id, access_key)
client_ttn = handler.data()
client_ttn.set_connect_callback(connect_callback)
client_ttn.set_downlink_callback(connect_downlink)
client_ttn.set_uplink_callback(uplink_callback)
client_ttn.connect()

# Paho client to publish to the thingsboard
client1 = paho.Client("control1")
client1.on_publish = on_publish
client1.username_pw_set(ACCESS_TOKEN)
client1.connect(broker, port, keepalive=60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()

