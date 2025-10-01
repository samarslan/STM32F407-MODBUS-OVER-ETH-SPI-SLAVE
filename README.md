# SPI Slave LED Controller - STM32F4

A robust SPI slave implementation for STM32F4 that receives ASCII commands over SPI and controls local LEDs.

## 🎯 Overview

This firmware turns an STM32F4 into an SPI slave device that:
- Listens for human-readable ASCII commands via SPI
- Controls 4 on-board LEDs (Green, Orange, Red, Blue)
- Responds with status acknowledgments
- Operates in interrupt-driven or polling mode

```
[SPI Master Bridge] ←SPI→ [STM32F4 SPI Slave] ←→ [4 LEDs]
                              (This Project)
```

## ✨ Features

- **ASCII Protocol**: Human-readable commands for easy debugging
- **Dual Mode**: Interrupt-driven or polling-based operation
- **Robust Parsing**: Handles variable-length commands
- **Status Reporting**: Query all LED states with single command
- **Bulk Operations**: Control all LEDs simultaneously
- **Low Latency**: < 5ms command processing time

## 📋 Hardware Requirements

- **STM32F4 Discovery Board** (or compatible)
- **4 LEDs** (on-board LEDs)
- **SPI Connection** to master device
- **Common Ground** with master

## 🔌 Pin Configuration

### SPI1 (Slave Mode)
| Pin  | Function | Description |
|------|----------|-------------|
| PA4  | SPI1_NSS | Chip Select (Hardware NSS) |
| PA5  | SPI1_SCK | Serial Clock (input) |
| PA6  | SPI1_MISO | Master In Slave Out (output) |
| PA7  | SPI1_MOSI | Master Out Slave In (input) |

### LEDs
| Pin  | LED Color | Command Code |
|------|-----------|--------------|
| PD12 | Green     | G            |
| PD13 | Orange    | O            |
| PD14 | Red       | R            |
| PD15 | Blue      | B            |

## 🛠️ STM32CubeMX Configuration

### SPI1 Settings
```
Mode: Full-Duplex Slave
Hardware NSS: Input Signal
Data Size: 8 Bits
CPOL: Low
CPHA: 1 Edge
First Bit: MSB First
CRC: Disabled
```

### NVIC Settings (Interrupt Mode)
```
☑ Enable SPI1 global interrupt
```

### GPIO Configuration
```
PA4 (NSS):  Alternate Function, Pull-up
PA5 (SCK):  Alternate Function
PA6 (MISO): Alternate Function
PA7 (MOSI): Alternate Function
PD12-15:    GPIO Output Push-Pull (LEDs)
```

## 📂 Project Structure

```
Core/
├── Inc/
│   ├── main.h
│   └── spi_slave.h
├── Src/
│   ├── main.c
│   └── spi_slave.c
```

## 🔧 Integration

### Add Files to Project
- Copy `spi_slave.h` to `Core/Inc/`
- Copy `spi_slave.c` to `Core/Src/`

### Update main.c

**Interrupt Mode (Recommended):**
```c
#include "main.h"
#include "spi_slave.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    SPI_Slave_Init();
    
    while (1) {
        HAL_Delay(100);
    }
}
```

**Polling Mode:**
```c
#include "main.h"
#include "spi_slave.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    SPI_Slave_Init();
    
    while (1) {
        SPI_Slave_Poll();
    }
}
```

## 📡 Command Protocol

### Commands

| Command | Description |
|---------|-------------|
| `LED:G1\n` | Turn ON Green LED |
| `LED:G0\n` | Turn OFF Green LED |
| `LED:O1\n` | Turn ON Orange LED |
| `LED:O0\n` | Turn OFF Orange LED |
| `LED:R1\n` | Turn ON Red LED |
| `LED:R0\n` | Turn OFF Red LED |
| `LED:B1\n` | Turn ON Blue LED |
| `LED:B0\n` | Turn OFF Blue LED |
| `LED:A0\n` | Turn OFF ALL LEDs |
| `LED:A1\n` | Turn ON ALL LEDs |
| `GET:LED\n` | Query LED status |

### Responses

| Response | Description |
|----------|-------------|
| `OK\n` | Command executed successfully |
| `ERR\n` | Invalid command |
| `STA:GORB\n` | Status (0=OFF, 1=ON) |
| `RDY\n` | Ready state |

### Example

**Turn on Green LED:**
```
Master → Slave: LED:G1\n
Slave → Master: OK\n
```

**Query status:**
```
Master → Slave: GET:LED\n
Slave → Master: STA:1010\n
                    ↑↑↑↑
                    ||||└─ Blue: OFF
                    |||└── Red: ON
                    ||└─── Orange: OFF
                    |└──── Green: ON
```

## 🚀 Building

### STM32CubeIDE
1. Import project
2. Build: `Ctrl+B`
3. Flash: `F11`

### Command Line
```bash
make
st-flash write build/spi_slave.bin 0x8000000
```

## 🧪 Testing

### Arduino Test Code
```c
#include <SPI.h>

void setup() {
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    pinMode(SS, OUTPUT);
    digitalWrite(SS, HIGH);
}

void sendCommand(const char* cmd) {
    digitalWrite(SS, LOW);
    delay(1);
    
    for (int i = 0; i < 32; i++) {
        SPI.transfer(cmd[i]);
    }
    
    delay(1);
    digitalWrite(SS, HIGH);
    delay(10);
}

void loop() {
    sendCommand("LED:G1\n");
    delay(1000);
    sendCommand("LED:G0\n");
    delay(1000);
}
```

## 🐛 Troubleshooting

### No Response
- Check SPI wiring (especially GND)
- Verify NSS goes LOW during communication
- Confirm CPOL=0, CPHA=1 on both devices
- Use oscilloscope to verify SCK signal

### LEDs Not Changing
- Verify GPIO pin definitions match your board
- Test with `SPI_Slave_SetLED(0, 1)` directly
- Check LED polarity
- Ensure adequate power supply

### Corrupted Commands
- Reduce SPI clock speed (< 1 MHz)
- Add 10kΩ pull-up on NSS
- Shorten wires (< 10 cm)
- Check solid GND connection

### Interrupt Issues
- Enable SPI1 interrupt in NVIC
- Verify `HAL_SPI_TxRxCpltCallback()` exists
- Increase stack size if hard faults occur

## 📊 Performance

- **Processing Time**: < 5 ms
- **SPI Speed**: Up to 2 MHz
- **Buffer Size**: 32 bytes
- **Response Latency**: < 2 ms

## 📝 API Reference

```c
void SPI_Slave_Init(void);
// Initialize SPI slave and LEDs

void SPI_Slave_Poll(void);
// Polling mode handler (call in main loop)

void SPI_Slave_SetLED(uint8_t index, uint8_t state);
// Manually set LED (index: 0-3, state: 0-1)

uint8_t SPI_Slave_GetLED(uint8_t index);
// Get LED state (returns 0 or 1)
```

## 👤 Author

**Samet Arslan**  
Created: September 2025

## 🔗 Related Projects

- [Modbus/TCP-SPI Bridge](https://github.com/samarslan/STM32F407-MODBUS-OVER-ETH)
- [C# Modbus Client](https://github.com/samarslan/ModbusMaster)

---

**Status**: ✅ Tested  
**Version**: 1.0.0  
**Updated**: September 30, 2025
