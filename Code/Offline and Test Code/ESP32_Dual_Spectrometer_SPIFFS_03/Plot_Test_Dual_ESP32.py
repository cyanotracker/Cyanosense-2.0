# Written by MAS18195

import time
import serial
import matplotlib.pyplot as plt
import numpy as np

xu = yu = []                    # declare data arrays
xd = yd = []                    # declare data arrays
data_index = 1                  # data index counter
datapoints = 288                # number of X axis datapoints

# configure serial port
arduino = serial.Serial('COM5', 115200)  # set your COM port here
time.sleep(0.1)

# to run GUI event loop
plt.ion()

# create sub-plots
figure, (ax1, ax2) = plt.subplots(1 , 2, figsize=(15, 8))
figure.tight_layout(pad=3)
line1, = ax1.plot(xu, yu)
line2, = ax2.plot(xd, yd)

# configure the graph
ax1.title.set_text('Up Sensor')
ax1.set_xlabel('Wavelength (nm)')
ax1.set_ylabel('Intensity')
ax2.title.set_text('Down Sensor')
ax2.set_xlabel('Wavelength (nm)')
ax2.set_ylabel('Intensity')

ax1.grid(True)
ax2.grid(True)

# set the graph limits
ax1.set_xlim([340, 850])               # X-axis range
ax1.set_ylim([0, 8200])              # Y-axis range
ax2.set_xlim([340, 850])               # X-axis range
ax2.set_ylim([0, 8200])              # Y-axis range

# Loop for streaming
for i in range(50):
    # read data from serial port
    while data_index <= datapoints:
        arduino.write('X'.encode())
        wavelength_u = float(arduino.readline())
        xu = np.append(xu, wavelength_u)
        arduino.write('U'.encode())
        intensity_u = float(arduino.readline())
        yu = np.append(yu, intensity_u)

        arduino.write('W'.encode())
        wavelength_d = float(arduino.readline())
        xd = np.append(xd, wavelength_d)
        arduino.write('D'.encode())
        intensity_d = float(arduino.readline())
        yd = np.append(yd, intensity_d)

        data_index += 1
    arduino.write('E'.encode())         # tell sensor retrieved all data and start new scan
    data_index = 1                      # reset data index

    # updating data values
    line1.set_xdata(xu)
    line1.set_ydata(yu)
    line2.set_xdata(xd)
    line2.set_ydata(yd)

    # drawing updated values
    ax1.figure.canvas.draw()
    ax2.figure.canvas.draw()

    # This will run the GUI event
    # loop until all UI events currently waiting have been processed
    ax1.figure.canvas.flush_events()
    ax2.figure.canvas.flush_events()

    arr_up = np.asarray([xu, yu])
    arr_up = np.transpose(arr_up)
    np.savetxt('up_sensor.csv', arr_up, fmt='%d', delimiter=",")
    arr_down = np.asarray([xd, yd])
    arr_down = np.transpose(arr_down)
    np.savetxt('down_sensor.csv', arr_down, fmt='%d', delimiter=",")

    # clear arrays
    xu = xu[: len(xu) - datapoints]
    yu = yu[: len(yu) - datapoints]
    xd = xd[: len(xd) - datapoints]
    yd = yd[: len(yd) - datapoints]

#    time.sleep(0.02)
arduino.close()                         # close serial port
