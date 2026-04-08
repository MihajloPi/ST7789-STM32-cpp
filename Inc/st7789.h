#pragma once

#include "tft_fonts.h"
#include "main.h"
#include <cstdint>
#include <cstddef>

/* Comment out if you don't want to use DMA */
#define USE_DMA

/* -----------------------------------------------------------------------
 * RGB565 colour constants
 * --------------------------------------------------------------------- */
namespace Color {
    constexpr uint16_t WHITE      = 0xFFFF;
    constexpr uint16_t BLACK      = 0x0000;
    constexpr uint16_t BLUE       = 0x001F;
    constexpr uint16_t RED        = 0xF800;
    constexpr uint16_t MAGENTA    = 0xF81F;
    constexpr uint16_t GREEN      = 0x07E0;
    constexpr uint16_t CYAN       = 0x07FF;
    constexpr uint16_t YELLOW     = 0xFFE0;
    constexpr uint16_t GRAY       = 0x8430;
    constexpr uint16_t BROWN      = 0xBC40;
    constexpr uint16_t DARKBLUE   = 0x01CF;
    constexpr uint16_t LIGHTBLUE  = 0x7D7C;
    constexpr uint16_t GRAYBLUE   = 0x5458;
    constexpr uint16_t LIGHTGREEN = 0x07E4;
    constexpr uint16_t LGRAY      = 0xC618;
    constexpr uint16_t LGRAYBLUE  = 0x5D1C;
    constexpr uint16_t LBBLUE     = 0x2B12;
}

/* -----------------------------------------------------------------------
 * ST7789 TFT driver
 *
 * Usage:
 *   ST7789 tft(&hspi1,
 *              ST7789_RST_GPIO_Port, ST7789_RST_Pin,
 *              ST7789_DC_GPIO_Port,  ST7789_DC_Pin,
 *              ST7789_CS_GPIO_Port,  ST7789_CS_Pin,  // nullptr/0 if no CS
 *              240, 320);
 *   tft.init();
 * --------------------------------------------------------------------- */
class ST7789 {
public:
#ifdef USE_DMA
    /* DMA: minimum transfer size to bother using DMA over blocking SPI */
    static constexpr uint16_t DMA_MIN_SIZE = 16;
#endif

    /* DMA framebuffer strip height — increase if you have spare RAM */
    static constexpr uint16_t HOR_LEN = 5;

    ST7789(SPI_HandleTypeDef *spi,
           GPIO_TypeDef *rstPort, uint16_t rstPin,
           GPIO_TypeDef *dcPort,  uint16_t dcPin,
           GPIO_TypeDef *csPort,  uint16_t csPin,   // pass nullptr/0 for no CS
           uint16_t width, uint16_t height,
           uint16_t xShift = 0, uint16_t yShift = 0);

    /* Core */
    void init();
    void setRotation(uint8_t m);

    /* Basic drawing */
    void fillColor(uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawPixel4px(uint16_t x, uint16_t y, uint16_t color);
    void fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);

    /* Primitives */
    void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void drawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
    void drawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                      uint16_t x3, uint16_t y3, uint16_t color);
    void drawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                            uint16_t x3, uint16_t y3, uint16_t color);

    /* Image & text */
    void drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
    void writeChar(uint16_t x, uint16_t y, char ch,
                   TFT_FontDef font, uint16_t color, uint16_t bgcolor);
    void writeString(uint16_t x, uint16_t y, const char *str,
                     TFT_FontDef font, uint16_t color, uint16_t bgcolor);

    /* Misc */
    void invertColors(bool invert);
    void tearEffect(bool enable);
    void test();

    uint16_t width()  const { return _width;  }
    uint16_t height() const { return _height; }

private:
    SPI_HandleTypeDef *_spi;
    GPIO_TypeDef      *_rstPort;  uint16_t _rstPin;
    GPIO_TypeDef      *_dcPort;   uint16_t _dcPin;
    GPIO_TypeDef      *_csPort;   uint16_t _csPin;   // nullptr = no CS
    uint16_t _width, _height, _xShift, _yShift;

    /* DMA framebuffer — HOR_LEN rows of pixels */
    uint16_t _dispBuf[240 * HOR_LEN];   // sized for max supported width

    inline void select()   const;
    inline void unSelect() const;
    inline void dcSet()    const;
    inline void dcClr()    const;
    inline void rstSet()   const;
    inline void rstClr()   const;

    void writeCommand(uint8_t cmd);
    void writeData(const uint8_t *buff, size_t size);
    void writeSmallData(uint8_t data);
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    /* Byte-swap a RGB565 value for correct DMA wire order */
    static constexpr uint16_t swap16(uint16_t x) {
        return static_cast<uint16_t>((x << 8) | (x >> 8));
    }
    static constexpr int16_t absVal(int16_t x) { return x >= 0 ? x : -x; }
};
