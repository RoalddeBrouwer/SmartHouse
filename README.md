# SmartHouse

What is the application:
(Satish, Roald, Stevie, Sam)

The goal of the project is to create a smarthouse which tracks the person inside the house and will maximize energy efficiency. 

To achieve this goal we have an end device which should be on the user at all times. This end device will trigger an interrupt everytime the user moves a certain period of time. This interrupt then wakes up the device, which then tries to send a dash7 message (containg desired temperature and current temperature). This data is processed by the back-end which will extract these temperatures and send them to a visualisation platform. The back-end will also localise the device and send this data to the visualisation platform. In case the dash7 messages fail, our system will opt for LoraWan. It will send a lora packet (without receiving an ack), this happens when the person is outside of his house (far from dash7 gateways). 

The implementation is done on different levels: Embedded software(1), back-end(2), localisation(3) and visualisation(4). Confirmed afterwards using energie profiling(5). 

Neccesity:

– Nucleos; 1 or more mobile
– Sensors: temperature, acceleratoreter
- localisation: Dash 7.

HARDWARE: 
- STML4xx
- OCTA Connect: 
Temperatuur: SHT31
Accelerometer: LSM303AGR
Lightsensor: TCS34725
- SHIELDS:
Lora: Murata
Dash7: Murata

# Embedded software(low powering):
by satish

Most of these steps are documenten and are in the "documenten_embedded" folder. 
To summarize the steps taken:
- Enable stopmode
- problem encountered, watchdog freeze in stopmode
- Configure accelerometer to work in low power mode and with double click recognition to mimick the walking.
- Configuring dash7, and lora. Figuring out the way to make payloads.
- Setting interrupts for the buttons (on the octa) and writing code for ease of localisation 
- NVIC priorities for buttons
- Setting up the BLE-chip (research+stack) and enabling uart to communicate with the chip.
- Correctly seting up the BLE-stack (uart-lines) and writing code to achieve communication between BLE chip and device.
Future improvements:
- Using standby mode instead of stopmode
- Disabling all the unused hardware
- lowering clock frequency
- Improving flow for lora communication. 

# Localisation:
by sam
The indoor localisation for the project is realised by using a localisation method based on signal strength. 
From D7 messages the signal strengths are extracted, which are received by the gateways which are setup in the area. 
Fingerprinting is used to achieve the ability to locate the active device in the area. 
The two fases of fingerprinting consist of:
## 1. Offline fase
 In the offline fase the database is build, which is used in the second fase. 
 Below an overview is presented of all the points of intrest (purple points) where measurements are conducted.
 IMAGE TOEVOEGEN 
 
 To perform the measurements the databasebuilder.py script is used. An flowchart of the databasebuilder program can be seen     at the end of the localisation section. 
 For each point of interest the user just needs to give the coordinates of the point and the DB will be build automatically.
 
## 2. Online fase
 In the online fase a measurements is conducted. This sample is compared against the training database.
 The euclidean distance is calculated againts all samples of the DB. 
 kNN is used to find the most appropriate locations. 
 The total flow of the active localisation proces can be seen in the flowgraph below. 
 The code that is used in the active fase of the localisation proces can be found in localisation_final.py

## Database builder
![databasebuilder](databasebuilderv4.jpg)

## Localisation
![localisation](localisationv4.jpg)

# Back-end:
(Stevie)

# Thingsboard: 
(Roald)

# Energy profiling:
(satish)

For energy profiling the steps taken were mostly with "HARDWARE".

This is all taken up in the "energy" folder. 


 

 


