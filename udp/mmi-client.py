import time
import socket
import re
import numpy as np
import sys
import pickle
import serial

from matplotlib import pyplot as plt
from matplotlib.widgets import Slider, Button

# buffer size to read in from socket
bufferSize  = 1024

def create_close_handler(boards, file, claw):
    # create a handler for the close event
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
        # create a new socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # bind the socket to the port
        self.sock.bind(addr)
        # set the socket receive buffer size
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 500)
        # set the socket timeout
        self.sock.settimeout(0.1)
        # set the file handle
        self.file_handle = file_handle
        self.name = name

    def close(self):
        self.sock.close()

    def __call__(self):
        try:
            # try to read data from the socket, decode it, and split it into lines
            data = self.sock.recv(bufferSize).decode('utf-8')
            x = []
            y0 = []
            y1 = []
            y2 = []
            button = []
            # use regex to read the data out of the incoming packets
            for line in re.findall(r"\((.*)\)", data):
                splt = line.split(',')
                x.append(float(splt[1]))
                y0.append(float(splt[2]))
                y1.append(float(splt[3]))
                y2.append(float(splt[4]))
                button.append(float(splt[5]))
                # if the file handle is valid, write the data to the file
                if self.file_handle:
                    # write the data to the file
                    self.file_handle.write(f"{self.name},{x[-1]},{y0[-1]},{y1[-1]},{y2[-1]},{button[-1]}\n")

            return x, y0, y1, y2
        except socket.timeout:
            # if there is a timeout, return empty lists
            return [], [], [], []
    

class SensorPlot:
    def __init__(self, ax, board, keep):
        # create the x and y data
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
        
        # create the plots and store them
        self.plot0 = ax[0].plot(self.x, self.y0, label=f'{board.name}: Sensor 0')
        self.plot1 = ax[0].plot(self.x, self.y1, label=f'{board.name}: Sensor 1')
        self.plot2 = ax[0].plot(self.x, self.y2, label=f'{board.name}: Sensor 2')
    
    def __call__(self):
        # get new data from the board
        n_x, n_y0, n_y1, n_y2 = self.board()
        # append the new data to the old data
        n_y0 = self.y0 + n_y0
        n_y1 = self.y1 + n_y1
        n_y2 = self.y2 + n_y2

        # attenuate the data
        self.y0 = [ny - self.y0_mu for ny in n_y0[-self.keep:]]
        self.y1 = [ny - self.y1_mu for ny in n_y1[-self.keep:]]
        self.y2 = [ny - self.y2_mu for ny in n_y2[-self.keep:]]

        self.y1_mu = (1 - self.attenuation) * self.y1_mu + self.attenuation * np.mean(self.y1)
        self.y2_mu = (1 - self.attenuation) * self.y2_mu + self.attenuation * np.mean(self.y2)
        self.y0_mu = (1 - self.attenuation) * self.y0_mu + self.attenuation * np.mean(self.y0)

        # set the new plotting data
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
    """
    Main function for the MMI Live Data UI

    Parameters
    ----------
    ip : str
        The IP address to listen on
    port : str
        The port to listen on
    file_name : str
        The name of the file to log the data to
    predict : str
        Whether or not to run the prediction
    claw_port : str
        The serial port to use for the claw
    """
    
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
    if predict and claw_port != "None":
        claw = serial.Serial(claw_port, 9600, timeout=1)
        claw.flush()

    # create the boards
    board0 = Boards("Board0", addr0, file)
    board1 = Boards("Board1", addr1, file)

    # create the main plotting window
    fig, ax = plt.subplots(1, 1, figsize=(12, 6))
    ax = [ax]
    fig.tight_layout(pad=3)

    fig.subplots_adjust(bottom=0.25)

    lookback_ax = fig.add_axes([0.12, 0.05, 0.22, 0.03])
    threshold_ax = fig.add_axes([0.12, 0.10, 0.22, 0.03])

    look_back = 0
    threshold = 3.0

    gaussian_filtering = False

    # setup slider for lookback selection
    lookback_slider = Slider(
        ax=lookback_ax,
        label='Lookback ',
        valmin=0,
        valmax=keep//2,
        valinit=look_back,
        valstep=1,
    )

    # setup slider for threshold selection
    threshold_slider = Slider(
        ax=threshold_ax,
        label='Threshold [v] ',
        valmin=0,
        valmax=3.3,
        valinit=threshold,
        valstep=0.1,
    )

    # setup button for gaussian filtering
    filter_button = Button(plt.axes([0.85, 0.05, 0.1, 0.075]), 'Enable \nGaussian Filter')

    activations = [0]*keep
    activations_x = np.linspace(-keep, 0, keep)
    activation_plot = ax[0].plot(activations_x, activations, label="Activation")

    threshold_plot = ax[0].plot(activations_x, np.ones_like(activations_x)*threshold, c='k')
    look_back_plot = ax[0].plot([-look_back, -look_back], [0, 3.3], c='k')

    # callback for lookback slider
    def update_lookback(val):
        global look_back
        look_back = int(val)
        look_back_plot[0].set_xdata([-look_back, -look_back])
        ax[0].draw_artist(look_back_plot[0])
        fig.canvas.blit(ax[0].bbox)

    # callback for threshold slider
    def update_threshold(val):
        global threshold
        threshold = val
        threshold_plot[0].set_ydata(np.ones_like(activations_x)*threshold)
        ax[0].draw_artist(threshold_plot[0])
        fig.canvas.blit(ax[0].bbox)

    # callback for gaussian filter button
    def update_filter(val):
        global gaussian_filtering
        gaussian_filtering = val
        print(val)
        if gaussian_filtering:
            filter_button.label.set_text('Disable \nGaussian Filter')
        else:
            filter_button.label.set_text('Enable \nGaussian Filter')

    lookback_slider.on_changed(update_lookback)
    threshold_slider.on_changed(update_threshold)
    filter_button.on_clicked(update_filter)

    # create the sensor plots
    s_plot0 = SensorPlot(ax, board0, keep)
    s_plot1 = SensorPlot(ax, board1, keep)

    # register the close event
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
    with open('model.pkl', 'rb') as f:
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
            # shape the input data for the model
            X = np.vstack([s_plot0.y0, s_plot0.y1, s_plot0.y2, s_plot1.y0, s_plot1.y1, s_plot1.y2]).T
            
            # run the model
            y = tfc.predict(X)
            if gaussian_filtering:
                # y = np.convolve(y, np.ones(2)/2, 'same')
                y = np.convolve(y, [0.25, 0.5, 0.25], 'same')

            # pack last 8 activations into a single byte
            claw_cmd = b"1" if np.mean(y[-look_back])>(threshold/3.3) else b"2"
            # print(claw_byte)


            # send to claw
            if claw:
                claw.write(bytes(claw_cmd))

                if claw.inWaiting() > 0:
                        try:
                            print(float(claw.read(claw.inWaiting())))
                        except:
                            pass

            activation_plot[0].set_ydata(3.3 * y)
            ax[0].draw_artist(activation_plot[0])
        else:
            run_model = time.time() - tts > 0

        # fill in the axes rectangle
        fig.canvas.blit(ax[0].bbox)

        # run at 30 fps
        plt.pause(1000/30)

if __name__ == '__main__':
    if len(sys.argv) == 5:
        main("0.0.0.0", int(sys.argv[1]), sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        print(f"Usage: python {sys.argv[1]} <port> <file> <predict> <claw port>")
    sys.exit(0)
