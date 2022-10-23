from PyQt5 import QtWidgets, uic, QtCore, QtGui
import sys
from pyqtgraph import PlotWidget
import pyqtgraph
import numpy as np
import time
import serial
import os
import threading
import cv2

ser = serial.Serial('/dev/ttyACM0', 9600,timeout=3)

class Ui(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):    
        super(Ui, self).__init__(*args, **kwargs)
        uic.loadUi('../ui_files/mainwindow.ui', self)

        self.btn_save.clicked.connect(self.btn_save_func)
        self.btn_cancel.clicked.connect(app.instance().quit)
        
        # Plot Style
        self.graph_cycle.setLabel('left', 'Temperature', units='C')
        self.graph_cycle.setLabel('bottom', 'Time')
        self.graph_cycle.setTitle('Cycling Graph')
        self.graph_cycle.setBackground('w')
        self.graph_cycle.showGrid(x=True, y=True)

        # Plot Reference
        # self.temp = list(range(300))
        self.temp = [0]*600
        self.time = [0]*600
        self.dt = 1000 # ms
        pen = pyqtgraph.mkPen(color=(255, 0, 0))
        self.plot_ref = self.graph_cycle.plot(self.temp, self.time, pen=pen)

#        self.t2 = threading.Thread(target=self.capture_image, daemon=True)
#        self.t1 = threading.Thread(target=self.send_command)
        self.tab_main.setCurrentIndex(0)
        self.btn_start.clicked.connect(self.send_command)
        
        self.show()
    def aggregate_params(self):
        pred_time = self.spin_time_pred.value()
        pred_temp = self.spin_temp_pred.value()
        den_time = self.spin_time_den.value()
        den_temp = self.spin_temp_den.value()
        ext_temp = self.spin_temp_ext.value()
        ext_time = self.spin_time_ext.value()
        ann_time = self.spin_time_ann.value()
        ann_temp = self.spin_temp_ann.value()
        print(f"pred_time: {pred_time}")
        return pred_time, pred_temp, den_time, den_temp, ext_temp, ext_time, ann_temp, ann_time

#    def send_command_mother(self):
#        self.t1.start()

    def send_command(self):
        ser.reset_input_buffer()
        pred_time, pred_temp, den_time, den_temp, ext_temp, ext_time, ann_temp, ann_time = self.aggregate_params()
        # params_str = f"{pred_time},{pred_temp},{den_time},{den_temp},{ext_time},{ext_temp},{ann_time},{ann_temp}>"
        params_str = f"{pred_time},{pred_temp},{den_time}"
        print(params_str)
        ser.write(bytes(str(params_str), 'utf-8'))
        time.sleep(0.05)
        self.timer = QtCore.QTimer()
        self.timer.setInterval(self.dt)
        self.timer.timeout.connect(self.update_plot_data)

        # Read data thread
                # Capture Image thread

        self.timer.start()
        # self.read_serial()
        print(ser.readline() )


    def read_serial(self):
        while True:
            ser.flushInput()
            read = ser.readline().decode('utf-8').rstrip()  
            time.sleep(0.1)
            print(read)
    def capture_image(self):
        os.system("libcamera-jpeg -o test.jpeg --shutter 5000000")
        #os.system("sleep 5")
        #os.system("echo 'Threading'")

    def btn_save_func(self):
        self.lbl_test.setText(self.le_name.text())

    def read_data(self):
        # print("Reading data")
        # if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        # print(line)
        args = line.split("\t")
        # mode = args[0]
        #cycle_stage = args[0]
        current_temp = args[2]
        # print(type(current_temp))
        state = args[0]
        if state=="#":
            print("TRIGGER!")
            self.t2.start()
            #os.system("libcamera-jpeg -o test.jpeg --shutter 1000000")
        # self.thread.join()
#        print(f"current_temp: {current_temp}")
        # current_power = args[3]
        # self.lbl_test.setText(current_temp)
        # print(f"current temp ")
        # print(current_temp)
        return float(current_temp)
    
    def update_plot_data(self):
        self.time = self.time[1:]  # Remove the first y element.
        self.time.append(self.time[-1] +1)  # Add a new value 1 higher than the last.
        # print(self.time)
        self.temp = self.temp[1:]  # Remove the first
        try:
            # print("TRYING")
            # print(self.read_data())
            self.temp.append(self.read_data()) # Add a new random value.
            # print(self.temp)
        except Exception as e: 
            print(e)
            print("EXCEPT")
            pass
        # print(self.temp)
        self.plot_ref.setData(self.time, self.temp)  # Update the data.
        
if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = Ui()
    app.exec_()
