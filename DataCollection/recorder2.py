import serial
import sys
import time
import matplotlib.pyplot as plt
import numpy as np

def main(port):
    ser = serial.Serial(port, 115200, timeout=1)

    start_time = time.time()

    vals = list()

    # Toggle DTR to reset Arduino
    ser.setDTR(False)
    time.sleep(1)
    # toss any data already received
    ser.flushInput()
    ser.setDTR(True)

    duration = 20

    print(f"Recording for {duration} seconds...")

    while time.time() - start_time < duration:
        line = ser.readline()
        if line:
            try:
                val = int(line.decode())
                vals.append((time.time() - start_time, val))
            except:
                pass

    print(f"Done recording. Recorded {len(vals)} samples.")

    with open("samples.csv", "w") as f:
        for sample_time, sample in vals:
            f.write("{},{}\n".format(sample_time, sample))

    print("Saved samples to samples.csv")

    vals = np.array(vals).T

    # plot the samples.
    fig, ax = plt.subplots(3, 1)
    ax[0].plot(vals[0,:], vals[1,:])
    ax[0].set(xlabel='time (s)', ylabel='sample',
              title=f"Samples Recorded on {port} for {5} seconds")
    ax[0].grid()

    # plot the difference between samples.
    ax[1].plot(vals[0,1:], np.diff(vals[1,:]))
    ax[1].set(xlabel='time (s)', ylabel='sample',
              title=f"Sample Differences")
    ax[1].grid()

    # plot the running sum of the samples.
    ax[2].plot(vals[0,:], np.cumsum(vals[1,:]))
    ax[2].set(xlabel='time (s)', ylabel='sample',
              title=f"Running Sum of Samples")
    ax[2].grid()

    # tight layout and show the plot.
    fig.tight_layout()

    fig.savefig("samples.png")

    print("Saved plot to samples.png")
    plt.show()


if __name__ == "__main__":
    main(sys.argv[1])