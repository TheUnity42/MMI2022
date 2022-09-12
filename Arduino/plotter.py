import numpy as np
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation
import serial
import time
plt.style.use('seaborn-pastel')

def main():
    fig = plt.figure()
    ax = plt.axes(xlim=(0, 4), ylim=(-2, 2))
    line, = ax.plot([], [], lw=3)

    ser = serial.Serial('COM3', 115200, timeout=0)

    x, y = [time.time()], [0.0]

    def init():
        line.set_data(x, y)
        return line,

    def animate(i):
        v = ser.readline()

        if v is not None:
            x.append(time.time())
            y.append(float(v))

        line.set_data(x, y)
        return line,

    anim = FuncAnimation(fig, animate, init_func=init,
                        frames=200, interval=20, blit=True)

    plt.show()

if __name__ == '__main__':
    main()