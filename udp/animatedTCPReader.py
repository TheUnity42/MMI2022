import random
import time
import socket
import re
import numpy as np
import sys

from matplotlib import pyplot as plt
from matplotlib import animation

 

# localIP     = "0.0.0.0"

# localPort   = 6001

bufferSize  = 1024

def handle_close_event(evt):
    sys.exit(0)

class Sensor(object):
    
    def __init__(self, addr, testing=False):
        self.testing = testing
        if not self.testing:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind(addr)
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 500)
            self.file = open("TestData.csv", "w+")
        else:
            self.x = [0]
            self.y = [0]
    def __call__(self):
        if not self.testing:
            # print("Starting read")
            data = self.sock.recv(bufferSize).decode('utf-8')
            # print("read bytes:", data)
            x = []
            y0 = []
            y1 = []
            y2 = []
            button = []
            for line in re.findall(r"\((.*)\)", data):
                splt = line.split(',')
                # print(splt)
                x.append(float(splt[1]))
                y0.append(float(splt[2]))
                y1.append(float(splt[3]))
                y2.append(float(splt[4]))
                button.append(float(splt[5]))
                self.file.write(f"{x[-1]},{y0[-1]},{y1[-1]},{y2[-1]},{button[-1]}\n")

            #print("finishing read")
            #print(x, y)
            return x, y0, y1, y2
        else:
            # delay for a bit
            time.sleep(0.1)
            y0 = []
            for i in range(25):
                # self.x.append(self.x[-1] + 1)
                y0.append(3.3*random.random())
            return self.x, y0

def main(ip, port, keep):
    addr = (str(ip), int(port))

    keep = int(keep) # number of points to keep

    sensor0 = Sensor(addr, False)

    # 3 element gaussian kernel
    kernel = np.array([0.25, 0.5, 0.25])
    # kernel = np.array([-1,1,-1])

    x = np.linspace(-keep,0,keep)
    y0 = [0]*keep
    conv0 = [0]*keep
    y1 = [0]*keep
    conv1 = [0]*keep
    y2 = [0]*keep
    conv2 = [0]*keep

    fig, ax = plt.subplots(1, 2, figsize=(12, 6))
    fig.tight_layout(pad=3)

    fig.canvas.mpl_connect('close_event', handle_close_event)

    plot0 = ax[0].plot(x, y0, color='r', label='Sensor 0')
    plot1 = ax[0].plot(x, y1, color='g', label='Sensor 1')
    plot2 = ax[0].plot(x, y2, color='b', label='Sensor 2')

    conv_plot0 = ax[1].plot(x, conv0, color='r', label='Sensor 0')
    conv_plot1 = ax[1].plot(x, conv1, color='g', label='Sensor 1')
    conv_plot2 = ax[1].plot(x, conv2, color='b', label='Sensor 2')

    ax[0].set_title(f"Sensor Data from {addr[0]}:{addr[1]}")
    ax[0].legend()
    ax[1].set_title(f"Gaussian Convolution")
    ax[1].legend()
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
        new_x, new_y0, new_y1, new_y2 = sensor0()
        #new_x = x + new_x
        new_y0 = y0 + new_y0
        new_y1 = y1 + new_y1
        new_y2 = y2 + new_y2

        #x = new_x[-keep:]
        y0 = new_y0[-keep:]
        y1 = new_y1[-keep:]
        y2 = new_y2[-keep:]

        conv0 = np.convolve(y0, kernel, mode='same')[-keep:]
        conv1 = np.convolve(y1, kernel, mode='same')[-keep:]
        conv2 = np.convolve(y2, kernel, mode='same')[-keep:]

        plot0[0].set_xdata(x)
        plot0[0].set_ydata(y0)
        plot1[0].set_xdata(x)
        plot1[0].set_ydata(y1)
        plot2[0].set_xdata(x)
        plot2[0].set_ydata(y2)

        conv_plot0[0].set_xdata(x)
        conv_plot0[0].set_ydata(conv0)
        conv_plot1[0].set_xdata(x)
        conv_plot1[0].set_ydata(conv1)        
        conv_plot2[0].set_xdata(x)
        conv_plot2[0].set_ydata(conv2)

        # restore background
        fig.canvas.restore_region(background)
        # redraw the plots
        ax[0].draw_artist(plot0[0])
        ax[0].draw_artist(plot1[0])
        ax[0].draw_artist(plot2[0])
        ax[1].draw_artist(conv_plot0[0])
        ax[1].draw_artist(conv_plot1[0])
        ax[1].draw_artist(conv_plot2[0])
        # fill in the axes rectangle
        fig.canvas.blit(ax[0].bbox)

        plt.pause(0.000001)

if __name__ == '__main__':
    if len(sys.argv) == 4:
        main(sys.argv[1], int(sys.argv[2]), sys.argv[3])
    else:
        main('192.168.86.51', 4242, 100)