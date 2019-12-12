import paho.mqtt.client as mqtt
import pymongo
import time
import json

from bitstring import ConstBitStream

from d7a.alp.parser import Parser as AlpParser

from pymongo import MongoClient
from localization_final import localize as localize


gatewayList = ["4337313400210032","433731340023003d","42373434002a0049","463230390032003e"]
rx_values=[0,0,0,0]
receivedList = []


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("/d7/483638370045002a/#")
    client.subscribe("/d7/4836383700440038/#")
    print("Subscribed to topic", "/d7/4836383700440038/#")


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

    hexstring = str(msg.payload.decode("utf-8"))
    data = bytearray(hexstring.decode("hex"))
    parsedCommando = AlpParser().parse(ConstBitStream(data), len(data))  # from the AlpParser example
    print parsedCommando  # this is the parsed data that is received

    if parsedCommando.actions[0].operand.offset.id == 64:
        rxLevel = parsedCommando.interface_status.operand.interface_status.rx_level
        print 'rx_Level: ', rxLevel

        # Extract the gateway_id & device_id from msg
        empty, d7, device_id, gateway_id = msg.topic.split("/")
        print 'Gateway_id: ', gateway_id
        print 'Device_id: ', device_id

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

        if len(receivedList) == 4:
            print 'YOU REACHED THE POINT WHERE YOU START THE LOCALIZATION'
            print rx_values
            process_data_after_msg_received(rx_values)
            # this is the moment you call the localization method and do the other stuff
            receivedList[:] = []

            for i in range(len(rx_values)):
                rx_values[i] = 0
            print rx_values

                # push into the rx_value array


########################################

def process_data_after_msg_received(rx_values):
    print('received values', rx_values)

    location = localize(rx_values , 7)  # get location based on fingerprinting (rx_values, k-nearest neighbors)
    print('Location is approximately x:' + str(location['x']) + ' y:' + str(location['y']))



client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("student-04.idlab.uantwerpen.be", 1883, 60)
client.loop_forever()