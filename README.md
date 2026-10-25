# ESP32 AS7341 Spectrometer
## AS7341 spectrometer using an ESP32 "cheap yellow display" kit.
It includes XYZ & spectral reconstruction matrices from manufacturer application data.

<img width="2040" height="1536" alt="ResizedImage_2026-10-25_22-01-49_4627" src="https://github.com/user-attachments/assets/feb3caba-8893-42ec-a81c-b3039113a2da" />

The sensor is connected via I2C to CN1 connector on board with the following pinout:
| CN1 Pin | Out | I2C | Wire color (may vary) |
| :-----: | :-: | :-: | :-------------------: |
| 4       | GND | GND | Black                 |
| 3       | 22  | SDA | Blue                  |
| 2       | 27  | SCL | Yellow                |
| 1       | 3V3 | VCC | Red                   |

To set exposure in main.cpp:
```
#define ATIME 5
#define ASTEP 3596
#define ADC_GAIN AS7341_GAIN_128X
```
If the LCD is not working make sure the library flags are set correctly in platformio.ini for your CYD board:
```
build_flags =
	-D USER_SETUP_LOADED=1
	-D ILI9341_2_DRIVER=1
	-D TFT_WIDTH=240
	-D TFT_HEIGHT=320
	-D TFT_INVERSION_ON=1
	-D TFT_BL=21
	-D TFT_BACKLIGHT_ON=HIGH
	-D TFT_MISO=12
	-D TFT_MOSI=13
	-D TFT_SCLK=14
	-D TFT_CS=15
 	-D TFT_DC=2
	-D TFT_RST=-1
	-D LOAD_GLCD=1
	-D LOAD_FONT2=1
	-D TOUCH_CS=33
	-D SPI_FREQUENCY=55000000
	-D USE_HSPI_PORT=1
```
