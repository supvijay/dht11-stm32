# dht11-stm32
DHT11 sensor driver implementation on STM32 using datasheet-based GPIO timing without an external DHT library.

## Features

- DHT11 temperature and humidity measurement
- DHT11 communication implemented directly from the datasheet
- Custom microsecond delay implementation using TIM1
- Checksum verification
- Temperature and humidity calculation
- Independent Watchdog (IWDG) implementation
- USB CDC communication for transmitting sensor readings
- STM32CubeMX peripheral configuration

## Pinout

| Component | Pin | STM32 Pin | Description |
|-----------|-----|-----------|-------------|
| DHT11 | DATA | PC13 | Bidirectional data communication |
| DHT11 | VCC | 3.3V | Sensor power supply |
| DHT11 | GND | GND | Common ground |

The project includes the STM32CubeMX configuration settings and the DHT11 datasheet used as a reference for the implementation.
