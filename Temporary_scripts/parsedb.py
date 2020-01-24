import json
from pymongo import MongoClient

databaseClient = MongoClient();
database = databaseClient["FingerprintDB"]
collection = database["RSS"]
gatewayList = ["4337313400210032","433731340023003d","42373434002a0049","463230390032003e"]
rx_values=[0,0,0,0]

receivedList = [0,0,0,0]
rxList = [0,0,0,0]
inputX = 0
inputY = 0
#collection.drop()
with open('FinalDatabase.json') as json_file:
    data = json.load(json_file)
    for x in data:
        receivedList[0] = '4337313400210032'
        receivedList[1] = '433731340023003d'
        receivedList[2] = '42373434002a0049'
        receivedList[3] = '463230390032003e'
        rxList[0] = x['GatewayInfo']['4337313400210032']
        rxList[1] = x['GatewayInfo']['433731340023003d']
        rxList[2] = x['GatewayInfo']['42373434002a0049']
        rxList[3] = x['GatewayInfo']['463230390032003e']
        inputX = x['Location']['X']
        inputY = x['Location']['Y']



        dataToDB = { #Alternative data format in testDB file
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

        print dataToDB

        collection.insert_one(dataToDB)

