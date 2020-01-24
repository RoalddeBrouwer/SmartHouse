import paho.mqtt.client as mqtt
from bitstring import ConstBitStream
import struct
import paho.mqtt.client as paho  # mqtt library
from d7a.alp.parser import Parser as AlpParser
import json, ast
import threading
from localization_final import localize as localize

gatewayList = ["4337313400210032", "433731340023003d", "42373434002a0049", "463230390032003e"]
rx_values = [0, 0, 0, 0]
receivedList = []
count = 0

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    #print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/d7/4836383700270048/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
        # print(msg.topic + " " + str(msg.payload))
        global count, temp, desi
        count += 1
        # print count
        if count == 1:
            timer = threading.Timer(1.0, process_data_after_msg_received)
            timer.start()
            data = bytearray(msg.payload.decode("hex"))
            result = AlpParser().parse(ConstBitStream(data), len(data))
            # get the data from the message
            getData = result.actions.__getitem__(0)
            getData = getData.operand.data
            # print(getData)

            # turn data into floats and remove brackets
            des = getData[0] - 48;
            des2 = getData[1] - 48;
            desi = int(str(des) + str(des2))
            aa = bytearray(getData[2:6])
            temp = struct.unpack('<f', aa)
            temp = str(temp).replace('(', "")
            temp = temp.replace(',', '')
            temp = temp.replace(')', '')

        hexstring = str(msg.payload.decode("utf-8"))
        data = bytearray(hexstring.decode("hex"))
        parsedCommando = AlpParser().parse(ConstBitStream(data), len(data))  # from the AlpParser example
        # print parsedCommando  # this is the parsed data that is received
        if parsedCommando.actions[0].operand.offset.id == 64:
            global rx_values
            rxLevel = parsedCommando.interface_status.operand.interface_status.rx_level
            # print 'rx_Level: ', rxLevel

            # Extract the gateway_id & device_id from msg
            empty, d7, device_id, gateway_id = msg.topic.split("/")
            # print 'Gateway_id: ', gateway_id
            # print 'Device_id: ', device_id

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
            # print("data published to thingsboard \n")
            pass

def process_data_after_msg_received():
            # print('received values', rx_values)
            global location, count, desi, temp
            if len(receivedList) == 4:
                # print " received 4"
                count = 0
            elif len(receivedList) == 3:
                # print " received 3"
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;
            elif len(receivedList) == 2:
                # print " received 2"
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;
            elif len(receivedList) == 1:
                # print " received 1"
                for i in range(len(rx_values)):
                    if rx_values[i] == 0:
                        rx_values[i] = 200
                count = 0;

            location = localize(rx_values, 7)  # get location based on fingerprinting (rx_values, k-nearest neighbors)
            for i in range(len(rx_values)):
                rx_values[i] = 0
            # print('Location is approximately x:' + str(location['x']) + ' y:' + str(location['y']))
            loadJson()
            receivedList[:] = []

def loadJson():
            mapCoordinates = processCoordinates(location)
            tempCoor = {
                'xPos': mapCoordinates['x'],
                'yPos': mapCoordinates['y'],
                'Gateways': len(receivedList),
                'Temperature': temp,
                'Desired': desi}
            with open("telemetry.json", "w") as feedsjson:
                json.dump(tempCoor, feedsjson)
            with open('telemetry.json') as json_file:
                global payload
                payload = json.load(json_file)
                payload = ast.literal_eval(json.dumps(payload))
            client1.publish("v1/devices/me/telemetry", str(payload))  # topic-v1/devices/me/telemetry
            # print("Please check LATEST TELEMETRY field of your device")

def processCoordinates(coords):
            mapX = coords['x'] * 0.06475 + 0.155
            mapY = coords['y'] * 0.059875 + 0.151
            # print mapX,mapY
            return {'x': mapX, 'y': mapY}

ACCESS_TOKEN = 'iobV2qaZeBKAZ6TdLgMk'  # Token of your device
broker = "thingsboard.idlab.uantwerpen.be"  # host name

port = 1883  # data listening port
broker_address = "student-04.idlab.uantwerpen.be"
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker_address, port, 60)

client1 = paho.Client("control1")  # create client object
client1.on_publish = on_publish  # assign function to callback
client1.username_pw_set(ACCESS_TOKEN)  # access token from thingsboard device
client1.connect(broker, port, keepalive=60)  # establish connection
# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()


