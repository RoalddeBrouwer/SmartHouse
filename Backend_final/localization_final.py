import numpy as np
from pymongo import MongoClient

databaseClient = MongoClient();
database = databaseClient["FingerprintDB"]
collection = database["RSS"]
gatewayList = ["4337313400210032","433731340023003d","42373434002a0049","463230390032003e"]
rx_values=[0,0,0,0]


def localize(rx_values, k):
    temp = []
    for document in collection.find():
        diff = []
        for gateway_id in gatewayList:
            if gateway_id == '4337313400210032':
                diff.append(int(rx_values[0]) - int(document['GatewayInfo'][gateway_id]))
            if gateway_id == '433731340023003d':
                diff.append(int(rx_values[1]) - int(document['GatewayInfo'][gateway_id]))
            if gateway_id == '42373434002a0049':
                diff.append(int(rx_values[2]) - int(document['GatewayInfo'][gateway_id]))
            if gateway_id == '463230390032003e':
                diff.append(int(rx_values[3]) - int(document['GatewayInfo'][gateway_id]))
        rms = np.sqrt(np.mean(np.square(diff)))

        temp.append({'x': document['Location']['X'], 'y': document['Location']['Y'], 'rms': rms})

    # k-nearest neighbors
    ordered_locations = sorted(temp, key=lambda i: i['rms'])  # sort on RMS value
    nearest_neighbors = ordered_locations[:k]
    # print('knn: ' + str(nearest_neighbors))

    x_coordinate = 0
    x_values = []
    y_coordinate = 0
    y_values = []
    for fingerprint in nearest_neighbors:
        #print fingerprint['x']
        x_values.append(fingerprint['x'])
        #print fingerprint['y']
        y_values.append(fingerprint['y'])

    x_coordinate = np.average(x_values)
    y_coordinate = np.average(y_values)
    # print str(x_values) + 'the average coordinate of x is ' + str(x_coordinate)
    # print str(y_values) + 'the average coordinate of y is ' + str(y_coordinate)

    return {'x': x_coordinate, 'y': y_coordinate}
