import serial
import time

fluke87v = serial.Serial(port='COM28', baudrate=9600, timeout=.1)

prev_second = -1;

while True:
    current_time = time.localtime()

    if(current_time.tm_sec != prev_second):
        message = bytes(str(f"{current_time.tm_year:04d}")+
                        str(f"{current_time.tm_mon:02d}")+
                        str(f"{current_time.tm_mday:02d}")+
                        str(f"{current_time.tm_hour:02d}")+
                        str(f"{current_time.tm_min:02d}")+
                        str(f"{current_time.tm_sec:02d}")+
                        str(f"{current_time.tm_wday:02d}") +
                        str(f"{current_time.tm_yday:03d}") + "\n", 'utf-8')
        print(message);
        fluke87v.write(message)
        prev_second = current_time.tm_sec;