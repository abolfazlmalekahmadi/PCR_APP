import threading
import serial
import time
import os
import logging
from datetime import datetime

dt = str(datetime.now()).split('.')[0]
#ts = datetime.timestamp(dt)
logging.basicConfig(filename=f'{dt}.log', filemode = 'w')
logging.warning('Logging the PCR output')

arduino = serial.Serial(port='/dev/ttyACM0', baudrate=9600, timeout=1)
def write_read():
    arduino.flushInput()
    data = arduino.readline()
    return data

def split_input(data):
    data = data.decode('utf-8')
    data = data.split('\t')
    print(repr(data[-1]))
    return data[-1], data[-2]

arduino.flush()
print(arduino.isOpen())
arduino.write(bytes("1", 'utf-8'))
cycle_number = 0
flag_1 = 1
flag_2 = 1
flag_3 = 1
while True:
    reading = write_read()
    print(reading)
    logging.warning(reading)
    try:
        status, cycle = split_input(reading)
    except:
        continue
    # ~ if status == "#1\n" and flag_1:
        # ~ cycle_number+=1
        # ~ print("Triggering!")
        # ~ flag_1 = 0
        # ~ flag_3 = 1
        # ~ os.system(f"libcamera-still -r -o 8-mordad/cycle-{cycle_number}-15s-15cm-#1.jpeg --shutter 15000000 --gain 1 --awbgains 1,1 --immediate")
    # ~ if status == "#2\n" and flag_2:
        # ~ print("Triggering!")
        # ~ flag_2 = 0
        # ~ flag_1 = 1
        # ~ os.system(f"libcamera-still -r -o 8-mordad/cycle-{cycle_number}-15s-15cm-#2.jpeg --shutter 15000000 --gain 1 --awbgains 1,1 --immediate")
    # ~ if status == "#3\n" and flag_3:
        # ~ print("Triggering!")
        # ~ flag_3 = 0
        # ~ flag_2 = 1
        # ~ os.system(f"libcamera-still -r -o 8-mordad/cycle-{cycle_number}-15s-15cm-#3.jpeg --shutter 15000000 --gain 1 --awbgains 1,1 --immediate")





