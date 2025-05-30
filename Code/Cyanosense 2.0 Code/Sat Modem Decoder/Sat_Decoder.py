import numpy as np
import math

data_start_index = 30  # This value should not change as the index will always start from 30 (coded in ESP32 side)

cal_water = np.genfromtxt('cal_down.csv', delimiter=",")  # Import sensor specific coefficients for down sensor (sensor looking at water)
cal_sky = np.genfromtxt('cal_up.csv', delimiter=",") # Import sensor specific coefficients for up sensor (sensor looking at sky)

date = input("Enter Data Date (Example: XX-XX-XXXX): ")
hex_sky = input("Enter Hex for Sky Sensor (first transmission of the day): ")
hex_down = input("Enter Hex for Water Sensors (second transmission of the day): ")

# Checks to make sure that lengths of each data is the same. If it is not there is an issue.
if len(hex_sky) != len(hex_down):
    print("\nLength of Sky and Water Data do not match!")
    print("Either it was copied wrong or data is missing from Transmission!")
    quit()

hex_value = hex_sky + hex_down

# Converts hex to buffer value (high byte and low byte)
buffer = [int(hex_value[i:i+2], 16) for i in range(0, len(hex_value), 2)]

decimal = []

# Converts buffer value to decimal
for i in range(0, len(buffer), 2):
    high = buffer[i] << 8
    low = buffer[i + 1]
    decode = high + low
    decimal.append(decode)

# Splits the decimal array into sky_sensor and water_sensor intensities
sky_sensor = decimal[:len(decimal)//2]
water_sensor = decimal[len(decimal)//2:]

# Tells the wavelength for loop when to stop based on the index
data_end_index = len(sky_sensor) + data_start_index

wavelength_sky = []
wavelength_water = []

# Calculates the Wavelength of the sky and water sensor from coefficients
# The range is ~400nm to ~700nm
#for i in range(30, 189+1):
for i in range(data_start_index, data_end_index):
    wavelength_sky.append(int(round(float(cal_sky[0]) + (float(cal_sky[1]) * i) + (float(cal_sky[2]) * math.pow(i, 2)) +
                                    (float(cal_sky[3]) * math.pow(i, 3)) + (float(cal_sky[4]) * math.pow(i, 4)) +
                                    (float(cal_sky[5]) * math.pow(i, 5)), 0)))
    wavelength_water.append(int(round(float(cal_water[0]) + (float(cal_water[1]) * i) + (float(cal_water[2]) * math.pow(i, 2)) +
                                      (float(cal_water[3]) * math.pow(i, 3)) + (float(cal_water[4]) * math.pow(i, 4)) +
                                      (float(cal_water[5]) * math.pow(i, 5)), 0)))

# Creates CSV File
data = np.asarray([wavelength_sky, sky_sensor, wavelength_water, water_sensor])
data = np.transpose(data)
np.savetxt(str(date) + '.csv', data, fmt='%d', delimiter=",",
           header='Sky Wavelength, Sky Intensity, Water Wavelength, Water Intensity')
print("\nCreated CSV File")
