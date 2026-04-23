import serial
import time

try:
    ser = serial.Serial('COM9', 115200, timeout=1)
    print("Connected to COM9. Waiting for logs...")
    start_time = time.time()
    while time.time() - start_time < 10:
        line = ser.readline()
        if line:
            print(line.decode('utf-8', errors='ignore').strip())
except Exception as e:
    print(f"Error: {e}")
