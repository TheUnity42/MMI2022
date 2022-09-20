import serial
import sys
import time
import matplotlib.pyplot as plt
import numpy as np

def main(port, dur):
    print("Connecting to Arduino...")
    ser = serial.Serial(port, 115200, timeout=1)

    start_time = time.time()

    vals = list()

    # Toggle DTR to reset Arduino
    ser.setDTR(False)
    time.sleep(2)
    # toss any data already received
    ser.flushInput()
    # ser.setDTR(True)

    duration = int(dur)

    print(f"Recording for {duration} seconds...")

    ser.flushInput()
    while time.time() - start_time < duration:
        line = ser.readline()
        if line:
            try:
                val = int(line.decode())
                if val >= 0 or val < 1024:
                    val = np.abs(val - 512)
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
    ax[1].plot(vals[0,1:], np.diff(vals[1,:])/np.diff(vals[0,:]))
    ax[1].set(xlabel='time (s)', ylabel='sample',
              title=f"Sample Differences")
    ax[1].grid()

    # plot the gaussian convolution
    gauss_size = 10
    gaussian = np.array([np.exp(-0.5 * x**2) for x in np.linspace(-1,1,gauss_size)])
    gaussian = gaussian / gaussian.sum()
    print(f"Using gaussian kernel: {gaussian}")
    gaussian_res = np.convolve(vals[1,:], gaussian, 'same')


    ax[2].plot(vals[0,:], gaussian_res)
    ax[2].set(xlabel='time (s)', ylabel='sample',
              title=f"Gaussian Filter Applied over Samples")
    ax[2].grid()

    # tight layout and show the plot.
    fig.tight_layout()

    fig.savefig("samples.png")

    print("Saved plot to samples.png")
    plt.show()
    sys.exit(0)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"{sys.argv[0]} <port> <duration>")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])