###################################################
import paho.mqtt.client as mqtt
import pymongo
import time
import json

from bitstring import ConstBitStream

from d7a.alp.parser import Parser as AlpParser

from pymongo import MongoClient

import threading

###################################################

gatewayList = ["4337313400210032","433731340023003d","42373434002a0049","463230390032003e"]
receivedList = []
rxList = []
count = 0
countToDB = 0
level = ""
amountReceived = 0


###################################################
# User Input for database builder
inputX = 7
inputY = 1
###################################################

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("/d7/483638370045002a/#")
    client.subscribe("/d7/483638370045002a/#")
    print("Subscribed to topic", "/d7/483638370045002a/#")



def on_message(client, userdata, msg):
    #print(msg.topic+" "+str(msg.payload))

    hexstring = str(msg.payload.decode("utf-8"))
    data = bytearray(hexstring.decode("hex"))
    parsedCommando = AlpParser().parse(ConstBitStream(data), len(data)) #from the AlpParser example
    #print parsedCommando #this is the parsed data that is received

    global amountReceived
    if amountReceived == 0:
        timer = threading.Timer(1.0, process_data_after_msg_received)
        timer.start()
    amountReceived +=1;

    if parsedCommando.actions[0].operand.offset.id == 64:
        global count
        count += 1
        rxLevel = parsedCommando.interface_status.operand.interface_status.rx_level

        # Extract the gateway_id & device_id from msg
        empty, d7, device_id, gateway_id = msg.topic.split("/")

        if gateway_id  not in receivedList and gateway_id in gatewayList:
            # print 'GW added and rx_level added \n'
            receivedList.append(gateway_id)
            rxList.append(rxLevel)

def process_data_after_msg_received():
    global amountReceived;
    if len(receivedList) > 0:
        if amountReceived == len(receivedList):
            for x in gatewayList:
                #print(x)
                if x not in receivedList:
                    #print str(x) + ' is not a part of the received GW --> Adding it to the list '
                    receivedList.append(x)
                    rxList.append(200)
            #print '\nReceived < 4 GWs - assigned -200 for other GWs  \n'

    dataToDB = {
        'Location': {
            'X': inputX,
            'Y': inputY,
        },
        'GatewayInfo': { #link the GW to the rss
            str(receivedList[0]): rxList[0],
            str(receivedList[1]): rxList[1],
            str(receivedList[2]): rxList[2],
            str(receivedList[3]): rxList[3],
        }
    }

    receivedList[:] = []
    rxList[:] = []
    #print 'Lists are empty'

    global countToDB
    countToDB+=1
    print dataToDB
    #print 'Write to DB '
    collection.insert_one(dataToDB)

    count = 0
    amountReceived = 0




client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("student-04.idlab.uantwerpen.be", 1883, 60)


databaseClient = MongoClient();
database = databaseClient["FingerprintDB100"]
collection = database["RSS100"]

# Printing all the data inserted with location, gwinfo and ID
cursor = collection.find()
for record in cursor:
    print(record)

# collection.drop() # run this to clear the DB



# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
