# SENTINEL-X Arduino UNO Q Pin Mapping V1.0

## I2C Bus

A4 (SDA)
- BME688
- MPU6050
- INA219
- OLED Display

A5 (SCL)
- BME688
- MPU6050
- INA219
- OLED Display

---------------------------------

## HC-SR04 Sensors

Front Left
TRIG -> D2
ECHO -> D3

Front Center
TRIG -> D4
ECHO -> D5

Front Right
TRIG -> D6
ECHO -> D7

---------------------------------

## RGB Status LED

Red -> D8

Green -> D9

Blue -> D10

---------------------------------

## Active Buzzer

D11

---------------------------------

## Emergency Stop Button

D12

---------------------------------

## TB6612 Motor Driver

Driver 1

PWMA -> PWM Pin
AIN1
AIN2

PWMB -> PWM Pin
BIN1
BIN2

STBY

Driver 2

PWMA -> PWM Pin
AIN1
AIN2

PWMB -> PWM Pin
BIN1
BIN2

STBY

(Note: Final motor pin assignment depends on the Arduino UNO Q PWM-capable pins and will be finalized after confirming the board's available I/O.)

---------------------------------

## Power

5V
GND
VIN (if required)

---------------------------------

## Charging

Battery -> BMS -> LM2596 -> Arduino