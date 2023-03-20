import random
import time
import socket
import re
import numpy as np
import sys

from matplotlib import pyplot as plt
from matplotlib import animation

 

localIP     = "0.0.0.0"

localPort   = 6001

bufferSize  = 1024

def handle_close_event(evt):
    sys.exit(0)

class Sensor(object):
    
    def __init__(self, addr, testing=False):
        self.testing = testing
        if not self.testing:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind((localIP, localPort))
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 500)
            #self.file = open("TestData.csv")
        else:
            self.x = [0]
            self.y = [0]
    def __call__(self):
        if not self.testing:
            # print("Starting read")
            data = self.sock.recv(bufferSize).decode('utf-8')
            # print("read bytes:", data)
            x = []
            y = []
            for line in re.findall(r"\((.*)\)", data):
                splt = line.split(',')
                # print(splt[1], splt[2])
                x.append(float(splt[1]))
                y.append(float(splt[2]))

            #print("finishing read")
            #print(x, y)
            return x, y
        else:
            # delay for a bit
            time.sleep(0.1)
            y = []
            for i in range(25):
                # self.x.append(self.x[-1] + 1)
                y.append(3.3*random.random())
            return self.x, y

def main(ip, port, keep):
    addr = (str(ip), int(port))

    keep = int(keep) # number of points to keep

    sensor = Sensor(addr, False)

    # 3 element gaussian kernel
    kernel = np.array([0.25, 0.5, 0.25])

    x = np.linspace(-keep,0,keep)
    y = [0]*keep
    convy = [0]*keep

    fig, ax = plt.subplots(1, 2, figsize=(12, 6))
    fig.tight_layout(pad=3)

    fig.canvas.mpl_connect('close_event', handle_close_event)

    plot = ax[0].plot(x, y, color='b')
    conv = ax[1].plot(x, convy, color='r')

    ax[0].set_title(f"Sensor Data from {addr[0]}:{addr[1]}")
    ax[1].set_title(f"Gaussian Convolution")
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
        new_x, new_y = sensor()
        #new_x = x + new_x
        new_y = y + new_y
        #x = new_x[-keep:]
        y = new_y[-keep:]
        convy = np.convolve(y, kernel, mode='same')[-keep:]

        plot[0].set_xdata(x)
        plot[0].set_ydata(y)

        conv[0].set_xdata(x)
        conv[0].set_ydata(convy)

        # restore background
        fig.canvas.restore_region(background)
        # redraw the plots
        ax[0].draw_artist(plot[0])
        ax[1].draw_artist(conv[0])
        # fill in the axes rectangle
        fig.canvas.blit(ax[0].bbox)

        plt.pause(0.000001)

if __name__ == '__main__':
    if len(sys.argv) == 4:
        main(sys.argv[1], int(sys.argv[2]), sys.argv[3])
    else:
        main('192.168.86.51', 4242, 100)