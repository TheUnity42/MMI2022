import serial
import time
import numpy as np
import matplotlib.pyplot as plt
import sys

# random constants for handshake
HANDSHAKE_START = 0x7E
HANDSHAKE_RESPONSE = 0x7F
START_CODE = 0x80


def main(port):
    # open the provided port at 115200 baud.
    ser = serial.Serial(port, 115200)

    # send the handshake start byte, and wait for the response.
    ser.write(bytes([HANDSHAKE_START]))
    # response = ser.read()

    # if the response is not the handshake response, exit.
    # if response != bytes([HANDSHAKE_RESPONSE]):
    #     print("Incorrect Handshake response. Please restart the arduino and try again.")
    #     sys.exit(1)

    # prompt user for the duration of the recording.
    duration = int(input("Enter the duration of the recording in seconds: "))
    print("Recording for {} seconds...".format(duration))

    # convert to milliseconds, and send the start code.
    millis_duration = duration * 1000
    # send the 4 byte duration to the arduino.
    ser.write(bytes([START_CODE]))
    ser.write(millis_duration.to_bytes(4, byteorder='big'))

    # switch to non blocking mode.
    ser.timeout = 0

    samples = list()
    sample_times = list()
    start_time = time.time()
    while time.time() - start_time < duration:
        # read a 4 byte integer from the arduino.        
        data = ser.read(4)
        if len(data) == 4:
            sample = int.from_bytes(data, byteorder='big')
            samples.append(sample)
            sample_times.append(time.time() - start_time)

    print(f"Done recording. Recorded {len(samples)} samples.")

    # save the samples to a csv file.
    with open("samples.csv", "w") as f:
        for sample, sample_time in zip(samples, sample_times):
            f.write("{},{}\n".format(sample_time, sample))

    print("Saved samples to samples.csv")

    # plot the samples.
    fig, ax = plt.subplots(3, 1)
    ax[0].plot(sample_times, samples)
    ax[0].set(xlabel='time (s)', ylabel='sample',
           title=f"Samples Recorded on {port} for {duration} seconds")
    ax[0].grid()

    # plot the difference between samples.
    ax[1].plot(sample_times[1:], np.diff(samples))
    print(np.diff(samples))
    ax[1].set(xlabel='time (s)', ylabel='sample',
              title=f"Sample Differences")
    ax[1].grid()

    # plot the running sum of the samples.
    ax[2].plot(sample_times, np.cumsum(samples))
    ax[2].set(xlabel='time (s)', ylabel='sample',
                title=f"Running Sum of Samples")
    ax[2].grid()

    # tight layout and show the plot.
    fig.tight_layout()

    fig.savefig("samples.png")

    print("Saved plot to samples.png")
    plt.show()
    sys.exit(0)


if __name__ == '__main__':
    # ensure that the user has provided a port
    if len(sys.argv) < 2:
        print('Usage: python3 recorder.py <port>')
        sys.exit(1)
    main(sys.argv[1])
