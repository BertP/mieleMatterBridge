import serial
import time
import sys

def main():
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
        print("Listening to /dev/ttyACM0...")
        for _ in range(50):
            line = ser.readline()
            if line:
                print(line.decode('utf-8', errors='ignore').strip())
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
