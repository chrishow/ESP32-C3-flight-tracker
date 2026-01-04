# ESP32-C3 Flight Tracker

A real-time flight tracking display system built for the ESP32-C3 microcontroller with ST7735/ST7789 TFT display. This project displays live flight data and weather information, featuring automatic day/night update intervals and WiFi connectivity.

## Features

- **Real-time Flight Tracking**: Displays flight number, aircraft type, and departure airport
- **Weather Information**: Shows current temperature and humidity
- **Time Display**: Real-time clock with automatic updates
- **WiFi Status Indicator**: Visual signal strength indicator
- **Smart Update Intervals**: 
  - Day mode (7 AM - 10 PM): Flight data updates every 20 seconds
  - Night mode (10 PM - 7 AM): Flight data updates every hour
  - Weather updates every 10 minutes
- **WiFi Manager**: Easy WiFi configuration through captive portal

## Hardware Requirements

- **Microcontroller**: ESP32-C3 (Adafruit QT Py ESP32-C3 or compatible)
- **Display**: ST7735 or ST7789 TFT LCD (128x128 pixels)
- **Connections**:
  - TFT_CS: GPIO 2
  - TFT_RST: GPIO 5
  - TFT_DC: GPIO 0
  - TFT_MOSI: GPIO 4
  - TFT_SCLK: GPIO 3

## Software Dependencies

This project uses PlatformIO and requires the following libraries:

- `Adafruit ST7735 and ST7789 Library` (^1.10.4)
- `Adafruit GFX Library` (^1.11.9)
- `ArduinoJson` (^7.4.1)
- `WiFiManager` (^2.0.16-rc.2)

## Installation

### Prerequisites

- [PlatformIO IDE](https://platformio.org/) or PlatformIO Core
- USB cable for programming

### Setup

1. Clone this repository:
   ```bash
   git clone https://github.com/chrishow/ESP32-C3-flight-tracker.git
   cd ESP32-C3-flight-tracker
   ```

2. Open the project in PlatformIO:
   ```bash
   pio project init --ide vscode
   ```

3. Build the project:
   ```bash
   pio run
   ```

4. Upload to your ESP32-C3:
   ```bash
   pio run --target upload
   ```

5. Monitor serial output (optional):
   ```bash
   pio device monitor -b 9600
   ```

## Configuration

### WiFi Setup

On first boot, the device will create a WiFi access point:
1. Connect to the "ESP32-C3-Flight-Tracker" WiFi network
2. Navigate to the captive portal (usually opens automatically)
3. Select your WiFi network and enter credentials
4. The device will automatically connect and start displaying data

### API Endpoint

The flight data is fetched from a custom API endpoint. To use your own data source, modify the `API_URL` constant in [src/flight_data_manager.cpp](src/flight_data_manager.cpp):

```cpp
const char *API_URL = "https://your-api-endpoint.com/flight-data";
```

### Expected API Response Format

The flight data API should return JSON in the following format:

```json
{
  "flightDataAvailable": true,
  "flightNumber": "AA123",
  "aircraftType": "B738",
  "departureAirport": "LAX"
}
```

The weather API should return:

```json
{
  "temperature": 72,
  "humidity": 45
}
```

## Project Structure

```
.
├── include/
│   ├── display_manager.h          # Display control and rendering
│   ├── flight_data_manager.h      # Flight data API integration
│   ├── weather_manager.h          # Weather data API integration
│   ├── ft_wifi_manager.h          # WiFi connection management
│   └── DSEG*.h                    # Custom fonts for display
├── src/
│   ├── main.cpp                   # Main application loop
│   ├── display_manager.cpp        # Display implementation
│   ├── flight_data_manager.cpp    # Flight data fetch logic
│   ├── weather_manager.cpp        # Weather data fetch logic
│   └── ft_wifi_manager.cpp        # WiFi management implementation
├── platformio.ini                 # PlatformIO configuration
└── README.md                      # This file
```

## Display Modes

The display cycles through different information screens:

1. **Time & Weather Mode**: Shows current time, temperature, and humidity
2. **Flight Data Mode**: Shows flight number, aircraft type, and departure airport
3. **Error Mode**: Displays error messages when connectivity issues occur
4. **WiFi Setup Mode**: Shown during initial WiFi configuration

## Customization

### Update Intervals

Modify the timing constants in [src/main.cpp](src/main.cpp):

```cpp
const unsigned long NIGHT_FLIGHT_UPDATE_INTERVAL = 3600000; // 1 hour
const unsigned long DAY_FLIGHT_UPDATE_INTERVAL = 20000;     // 20 seconds
const unsigned long WEATHER_UPDATE_INTERVAL = 600000;       // 10 minutes
```

### Night Hours

Adjust when night mode begins/ends:

```cpp
const unsigned long NIGHT_START_HOUR = 22;  // 10 PM
const unsigned long NIGHT_END_HOUR = 7;     // 7 AM
```

## Troubleshooting

### Display Not Working

- Check wiring connections match the pin definitions
- Verify display is ST7735 or ST7789 compatible
- Try adjusting SPI speed in display initialization

### WiFi Connection Failed

- Ensure WiFi credentials are correct
- Check WiFi signal strength
- Reset device and reconfigure through captive portal

### No Data Displayed

- Check serial monitor for API connection errors
- Verify API endpoint is accessible
- Confirm JSON response format matches expected structure

## Serial Monitor Output

The device outputs detailed logging at 9600 baud:
- System initialization status
- WiFi connection status
- API request/response information
- Flight and weather data updates
- Error messages

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


## Author

Chris How - [GitHub](https://github.com/chrishow)

---

**Note**: This project requires a backend API to provide flight and weather data. The API endpoints need to be configured according to your data source.
