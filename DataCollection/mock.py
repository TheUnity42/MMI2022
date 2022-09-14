import serial
import sys
import time

def main(port):
    ser = serial.Serial(port, 115200)
    print(ser.read())
    ser.write(b'\x7F')
    
    # wait for start code
    while ser.read() != b'\x80':
        pass
    # read duration
    duration = int.from_bytes(ser.read(4), byteorder='big')
    print(duration)
    # return a random sample each millisecond
    for i in range(duration):
        ser.write((i**2 - i).to_bytes(4, byteorder='big'))
        # sleep for a millisecond
        time.sleep(0.001)
        
    
if __name__ == "__main__":
    main(sys.argv[1])

