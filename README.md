# ST7789-STM32

STM32 HAL driver for ST7789-based TFT LCD displays, using hardware SPI with optional DMA support.  
Written in C++ with a **camelCase** function naming convention.

> **Based on the original work by [Floyd-Fish](https://github.com/Floyd-Fish/ST7789-STM32).**  
> Fonts derived from [afiskon's stm32-st7735](https://github.com/afiskon/stm32-st7735) and [ananevilya's Arduino-ST7789-Library](https://github.com/ananevilya/Arduino-ST7789-Library).  
> Contributors to the upstream project: [JasonLrh](https://github.com/JasonLrh), [ZiangCheng](https://github.com/ZiangCheng).

---

## Supported Displays

| Resolution | Notes                             |
|------------|-----------------------------------|
| 135 × 240  |                                   |
| 240 × 240  | Tested                            |
| 170 × 320  | Tested                            |
| 240 × 320  | Set all shifts to 0               |

Any resolution supported by the ST7789 controller can be used — just pass the correct width, height, and shift values to the constructor.

---

## Quick Start

### 1. Add the files to your project

Copy `Inc/st7789.h`, `Inc/tft_fonts.h`, `Src/st7789.cpp`, and `Src/tft_fonts.c` into your STM32 project.  
Add the `Inc/` folder to your include path.

### 2. Configure STM32CubeMX / CubeIDE

Set up an SPI peripheral in **Full-Duplex Master** mode:

- Data Size: **8 bits**
- First Bit: **MSB First**
- Clock speed: up to **40 MHz** on PCB traces, keep ≤ 21 MHz on jumper wires

If using DMA, enable **DMA for SPI TX** in the DMA settings tab.

Define the following GPIO pins in CubeMX (or name them yourself):

| Signal | GPIO label example    |
|--------|-----------------------|
| RST    | `ST7789_RST`          |
| DC     | `ST7789_DC`           |
| CS     | `ST7789_CS` (optional)|

### 3. Include the header

```cpp
#include "st7789.h"
```

### 4. Construct the driver object

```cpp
// With CS pin
ST7789 tft(
    &hspi1,
    ST7789_RST_GPIO_Port, ST7789_RST_Pin,
    ST7789_DC_GPIO_Port,  ST7789_DC_Pin,
    ST7789_CS_GPIO_Port,  ST7789_CS_Pin,
    240, 320           // width, height
);

// Without CS pin (CS tied to GND)
ST7789 tft(
    &hspi1,
    ST7789_RST_GPIO_Port, ST7789_RST_Pin,
    ST7789_DC_GPIO_Port,  ST7789_DC_Pin,
    nullptr, 0,        // no CS
    240, 240
);

// With display offset (e.g. 170x320 panel)
ST7789 tft(&hspi1,
    RST_GPIO_Port, RST_Pin,
    DC_GPIO_Port,  DC_Pin,
    CS_GPIO_Port,  CS_Pin,
    170, 320,
    35, 0              // xShift, yShift
);
```

### 5. Initialise

Call `init()` once after power-on, typically in `main()` after HAL and peripheral init:

```cpp
tft.init();
```

### 6. (Optional) Run the built-in test

```cpp
tft.test();   // cycles through colours, fonts, and shapes
```

### 7. Don't forget the backlight

The backlight pin is not managed by this library. Drive it high (or via PWM) separately.

---

## DMA

DMA is enabled by default via the `#define USE_DMA` at the top of `st7789.h`.  
Comment it out to fall back to blocking SPI transfers.

With DMA enabled, bulk writes (`fillColor`, `drawImage`) hand off to DMA and poll for completion — freeing the CPU from byte-by-byte work.  
Small transfers below `DMA_MIN_SIZE` (16 bytes) always use blocking SPI to avoid DMA overhead on tiny payloads.

The internal framebuffer is `HOR_LEN` (default 5) rows tall. Increase it if you have spare RAM for a small speed gain on full-screen fills.

---

## API Reference

### Core

```cpp
void init();
void setRotation(uint8_t m);   // 0–3
```

### Basic drawing

```cpp
void fillColor(uint16_t color);
void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void drawPixel4px(uint16_t x, uint16_t y, uint16_t color);   // 3×3 fat pixel
void fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);
```

### Primitives

```cpp
void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void drawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
void drawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t x3, uint16_t y3, uint16_t color);
void drawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t x3, uint16_t y3, uint16_t color);
```

### Image & text

```cpp
void drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void writeChar(uint16_t x, uint16_t y, char ch,
               TFT_FontDef font, uint16_t color, uint16_t bgcolor);
void writeString(uint16_t x, uint16_t y, const char *str,
                 TFT_FontDef font, uint16_t color, uint16_t bgcolor);
```

Available fonts: `TFT_Font_7x10`, `TFT_Font_11x18`, `TFT_Font_16x26`

### Misc

```cpp
void invertColors(bool invert);
void tearEffect(bool enable);
uint16_t width()  const;
uint16_t height() const;
```

---

## Colour Constants

Pre-defined RGB565 values live in the `Color::` namespace:

```cpp
Color::WHITE      Color::BLACK      Color::RED        Color::GREEN
Color::BLUE       Color::CYAN       Color::YELLOW     Color::MAGENTA
Color::GRAY       Color::BROWN      Color::DARKBLUE   Color::LIGHTBLUE
Color::GRAYBLUE   Color::LIGHTGREEN Color::LGRAY      Color::LGRAYBLUE
Color::LBBLUE
```

Custom colours can be assembled with the standard RGB565 formula:

```cpp
uint16_t myColor = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
```

---

## Example

```cpp
#include "st7789.h"

ST7789 tft(&hspi1,
    ST7789_RST_GPIO_Port, ST7789_RST_Pin,
    ST7789_DC_GPIO_Port,  ST7789_DC_Pin,
    nullptr, 0,
    240, 240);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();

    tft.init();

    tft.fillColor(Color::BLACK);
    tft.writeString(10, 10, "Hello STM32!", TFT_Font_16x26, Color::GREEN, Color::BLACK);
    tft.drawCircle(120, 120, 50, Color::CYAN);
    tft.drawFilledRectangle(10, 150, 80, 40, Color::RED);

    while (1) {}
}
```

---

## SPI Speed Notes

| Connection type | Max reliable clock |
|-----------------|--------------------|
| 20 cm jumper wire (no probe) | ~21.25 MHz |
| 20 cm jumper wire + logic analyser | ~10.6 MHz  |
| PCB trace | up to 40 MHz |

---

## License

See [LICENSE](LICENSE) — original MIT licence from Floyd-Fish's upstream project is preserved.
