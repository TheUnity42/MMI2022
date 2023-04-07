import time
import socket
import re
import numpy as np
import sys
import pickle
import serial

from matplotlib import pyplot as plt

bufferSize  = 1024

def create_close_handler(boards, file, claw):
    def handle_close_event(evt):
        # shutdown
        for board in boards:
            board.close()
        if file:
            file.close()
        if claw:
            claw.close()
        sys.exit(0)
    return handle_close_event

class Boards(object):    
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
    def __init__(self, ax, board, keep):
        self.x = np.linspace(-keep, 0, keep)
        self.y0 = [0]*keep
        self.y1 = [0]*keep
        self.y2 = [0]*keep

        self.y0_mu = 0
        self.y1_mu = 0
        self.y2_mu = 0
        self.attenuation = 0.0

        self.keep = keep
        self.ax = ax
        self.board = board
        
        self.plot0 = ax[0].plot(self.x, self.y0, label=f'{board.name}: Sensor 0')
        self.plot1 = ax[0].plot(self.x, self.y1, label=f'{board.name}: Sensor 1')
        self.plot2 = ax[0].plot(self.x, self.y2, label=f'{board.name}: Sensor 2')
    
    def __call__(self):
        n_x, n_y0, n_y1, n_y2 = self.board()
        #new_x = x + new_x
        n_y0 = self.y0 + n_y0
        n_y1 = self.y1 + n_y1
        n_y2 = self.y2 + n_y2

        self.y0 = [ny - self.y0_mu for ny in n_y0[-self.keep:]]
        self.y1 = [ny - self.y1_mu for ny in n_y1[-self.keep:]]
        self.y2 = [ny - self.y2_mu for ny in n_y2[-self.keep:]]

        self.y1_mu = (1 - self.attenuation) * self.y1_mu + self.attenuation * np.mean(self.y1)
        self.y2_mu = (1 - self.attenuation) * self.y2_mu + self.attenuation * np.mean(self.y2)
        self.y0_mu = (1 - self.attenuation) * self.y0_mu + self.attenuation * np.mean(self.y0)

        self.plot0[0].set_xdata(self.x)
        self.plot0[0].set_ydata(self.y0)
        self.plot1[0].set_xdata(self.x)
        self.plot1[0].set_ydata(self.y1)
        self.plot2[0].set_xdata(self.x)
        self.plot2[0].set_ydata(self.y2)

        # redraw the plots
        self.ax[0].draw_artist(self.plot0[0])
        self.ax[0].draw_artist(self.plot1[0])
        self.ax[0].draw_artist(self.plot2[0])
    


def main(ip, port, file_name, predict, claw_port):
    keep = 150
    addr0 = (str(ip), int(port))
    addr1 = (str(ip), int(port)+1)

    predict = bool(predict)
    print("=== MMI Live Data UI ===")
    print(f"\tListening on Ports {addr0[1]}-{addr1[1]}")
    print(f"\tRunning Prediction: {predict}")
    print(f"\tLogging Data to {file_name}")
    print(f"\tClaw Port: {claw_port}")

    keep = int(keep) # number of points to keep

    file = open(file_name, "w+")

    # open serial port for claw
    claw = None
    if predict:
        claw = serial.Serial(claw_port, 9600, timeout=1)
        claw.flush()

    board0 = Boards("Board0", addr0, file)
    board1 = Boards("Board1", addr1, file)

    fig, ax = plt.subplots(1, 1, figsize=(12, 6))
    ax = [ax]
    fig.tight_layout(pad=3)

    activations = [0]*keep
    activations_x = np.linspace(-keep, 0, keep)
    activation_plot = ax[0].plot(activations_x, activations, label="Activation")

    s_plot0 = SensorPlot(ax, board0, keep)
    s_plot1 = SensorPlot(ax, board1, keep)

    fig.canvas.mpl_connect('close_event', create_close_handler([board0, board1], file, claw))

    ax[0].set_title(f"Sensor Data from {addr0[0]}:{addr0[1]}-{addr1[1]}")
    ax[0].legend()
    for axi in ax:
        axi.set_ylim(0,3.5)
        axi.set_xlabel('Sample #')
        axi.set_ylabel('Voltage (V)')

    # cache background
    background = fig.canvas.copy_from_bbox(ax[0].bbox)

    # load the model
    tfc = None
    with open('ring_model.pkl', 'rb') as f:
        tfc = pickle.load(f)

    plt.ion()
    plt.show()
    plt.pause(0.1)

    tts = time.time()
    run_model = False

    while True:
        # restore background
        fig.canvas.restore_region(background)
        # redraw the plots
        s_plot0()
        s_plot1()

        if run_model and predict:
            X = np.vstack([s_plot0.y0, s_plot0.y1, s_plot0.y2, s_plot1.y0, s_plot1.y1, s_plot1.y2]).T
            
            y = tfc.predict(X)
            # y = np.convolve(y, np.ones(int(keep/5))/(int(keep/5)), 'same')

            # pack last 8 activations into a single byte
            claw_byte = 0
            for i in range(8):
                if y[-i] > 0.5:
                    claw_byte |= 1 << i
            print(claw_byte)

            # send to claw
            claw.write(bytes([claw_byte]))

            activation_plot[0].set_ydata(3.3 * y)
            ax[0].draw_artist(activation_plot[0])
        else:
            run_model = time.time() - tts > 3

        # fill in the axes rectangle
        fig.canvas.blit(ax[0].bbox)

        plt.pause(0.000001)

if __name__ == '__main__':
    if len(sys.argv) == 5:
        main("0.0.0.0", sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4])
    else:
        print("Usage: python plot.py <port> <file> <predict> <claw port>")
    sys.exit(0)