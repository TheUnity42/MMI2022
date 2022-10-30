# import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import sys

def main(log):
    log_df = pd.read_csv(log)

    fig, ax = plt.subplots()
    ax.plot(log_df['Timestamp'], log_df['Value'])
    ax.set_title(f"Log {log}")
    ax.set_xlabel("Pico Timestamp (us since boot)")
    ax.set_ylabel("ADC Reading (V)")
    ax.set_ylim((0.0,3.4))
    plt.show()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage {sys.argv[0]}: <log file>")
    main(sys.argv[1])