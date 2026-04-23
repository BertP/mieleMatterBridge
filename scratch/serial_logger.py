import serial
import sys

# Konfiguration
port = '/dev/ttyACM0'
baud = 115200
logfile = 'bridge_live.log'

try:
    ser = serial.Serial(port, baud, timeout=1)
    print(f"Verbunden mit {port} bei {baud} Baud.")
    with open(logfile, 'w') as f:
        f.write(f"--- Log-Start: Verbunden mit {port} ---\n")
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='replace')
                print(line, end='') # Auch im Agent-Output zeigen
                f.write(line)
                f.flush()
except Exception as e:
    print(f"Fehler: {e}")
    sys.exit(1)
