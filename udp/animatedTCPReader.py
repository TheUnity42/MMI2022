import random
import time
import socket
import re
import numpy as np
import sys

from matplotlib import pyplot as plt

bufferSize  = 1024

def create_close_handler(sensors):
    def handle_close_event(evt):
        # shutdown
        for sensor in sensors:
            sensor.close()
        sys.exit(0)
    return handle_close_event

class Sensor(object):    
    def __init__(self, name, addr, file_handle=None):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(addr)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 500)
        self.file_handle = file_handle
        self.name = name
    def close(self):
        self.sock.close()
    def __call__(self):
        data = self.sock.recv(bufferSize).decode('utf-8')
        x = []
        y0 = []
        y1 = []
        y2 = []
        button = []
        for line in re.findall(r"\((.*)\)", data):
            splt = line.split(',')
            x.append(float(splt[1]))
            y0.append(float(splt[2]))
            y1.append(float(splt[3]))
            y2.append(float(splt[4]))
            button.append(float(splt[5]))
            if self.file_handle:
                self.file_handle.write(f"{self.name},{x[-1]},{y0[-1]},{y1[-1]},{y2[-1]},{button[-1]}\n")

        return x, y0, y1, y2
    

class SensorPlot:
    def __init__(self, ax, keep):
        self.x = np.linspace(-keep, 0, keep)
        self.y0 = [0]*keep
        self.y1 = [0]*keep
        self.y2 = [0]*keep
        self.keep = keep
        self.ax = ax
        
        self.plot0 = ax[0].plot(self.x, self.y0, color='r', label='Sensor 0')
        self.plot1 = ax[0].plot(self.x, self.y1, color='g', label='Sensor 1')
        self.plot2 = ax[0].plot(self.x, self.y2, color='b', label='Sensor 2')
    
    def update_from_sensor(self, n_x, n_y0, n_y1, n_y2):
        #new_x = x + new_x
        n_y0 = self.y0 + n_y0
        n_y1 = self.y1 + n_y1
        n_y2 = y2 + n_y2

        #x = new_x[-keep:]
        y0 = n_y0[-self.keep:]
        y1 = n_y1[-self.keep:]
        y2 = n_y2[-self.keep:]


        self.plot0[0].set_xdata(self.x)
        self.plot0[0].set_ydata(y0)
        self.plot1[0].set_xdata(self.x)
        self.plot1[0].set_ydata(y1)
        self.plot2[0].set_xdata(self.x)
        self.plot2[0].set_ydata(y2)

        # redraw the plots
        self.ax[0].draw_artist(self.plot0[0])
        self.ax[0].draw_artist(self.plot1[0])
        self.ax[0].draw_artist(self.plot2[0])
    


def main(ip, port, keep, file_name):
    addr0 = (str(ip), int(port))
    addr1 = (str(ip), int(port)+1)


    keep = int(keep) # number of points to keep

    file = open(file_name, "w+")

    sensor0 = Sensor("Sensor0", addr0, file)
    sensor1 = Sensor("Sensor1", addr1, file)

    fig, ax = plt.subplots(1, 2, figsize=(12, 6))
    fig.tight_layout(pad=3)

    s_plot0 = SensorPlot(ax, keep)
    s_plot1 = SensorPlot(ax, keep)

    fig.canvas.mpl_connect('close_event', create_close_handler([sensor0]))

    ax[0].set_title(f"Sensor Data from {addr0[0]}:{addr0[1]}-{addr1[0]}")
    ax[0].legend()
    for axi in ax:
        axi.set_ylim(0,3.5)
        axi.set_xlabel('Sample #')
        axi.set_ylabel('Voltage (V)')

    # cache background
    background = fig.canvas.copy_from_bbox(ax[0].bbox)

    plt.ion()
    plt.show()
    plt.pause(0.1)

    while True:
        # n_x, n_y0, n_y1, n_y2 = sensor0()
        # s1_x, s1_y0, s1_y1, s1_y2 = sensor1()

        # restore background
        fig.canvas.restore_region(background)
        # redraw the plots
        s_plot0.update_from_sensor(sensor0())
        s_plot1.update_from_sensor(sensor1())

        # fill in the axes rectangle
        fig.canvas.blit(ax[0].bbox)

        plt.pause(0.000001)

if __name__ == '__main__':
    if len(sys.argv) == 5:
        main(sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4])
    else:
        print("Usage: python3 plot.py <ip> <port> <graph_window> <file>")
        # main('192.168.86.51', 4242, 100)
    sys.exit(0)