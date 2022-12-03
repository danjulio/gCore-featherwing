/*
 * HX8357 LCD driver for littlevgl using SPI.
 * 
 * Initialization commands based on the Adafruit driver
 * 
 */
// ==================================================
// Constants
//

//
// Display pins
//
const int tft_cs_pin =  15;
const int tft_dc_pin =  33;


//
// Display Type - set this to the chip-type in your display (HX8357B - default; or HX8357D)
// Try the other one if the display is inverted.
//
#define DEFAULT_TFT      HX8357B


//
// Display SPI settings
//
#define TFT_SPI_SETTINGS SPISettings(26000000, MSBFIRST, SPI_MODE0)


//
// HX8357B/D REGS
//
#define HX8357D                    0xD  ///< Our internal const for D type
#define HX8357B                    0xB  ///< Our internal const for B type

#define HX8357_TFTWIDTH            320  ///< 320 pixels wide
#define HX8357_TFTHEIGHT           480  ///< 480 pixels tall

#define HX8357_NOP                0x00  ///< No op
#define HX8357_SWRESET            0x01  ///< software reset
#define HX8357_RDDID              0x04  ///< Read ID
#define HX8357_RDDST              0x09  ///< (unknown)

#define HX8357_RDPOWMODE          0x0A  ///< Read power mode Read power mode
#define HX8357_RDMADCTL           0x0B  ///< Read MADCTL
#define HX8357_RDCOLMOD           0x0C  ///< Column entry mode
#define HX8357_RDDIM              0x0D  ///< Read display image mode
#define HX8357_RDDSDR             0x0F  ///< Read dosplay signal mode

#define HX8357_SLPIN              0x10  ///< Enter sleep mode
#define HX8357_SLPOUT             0x11  ///< Exit sleep mode
#define HX8357B_PTLON             0x12  ///< Partial mode on
#define HX8357B_NORON             0x13  ///< Normal mode

#define HX8357_INVOFF             0x20  ///< Turn off invert
#define HX8357_INVON              0x21  ///< Turn on invert
#define HX8357_DISPOFF            0x28  ///< Display on
#define HX8357_DISPON             0x29  ///< Display off

#define HX8357_CASET              0x2A  ///< Column addr set
#define HX8357_PASET              0x2B  ///< Page addr set
#define HX8357_RAMWR              0x2C  ///< Write VRAM
#define HX8357_RAMRD              0x2E  ///< Read VRAM

#define HX8357B_PTLAR             0x30  ///< (unknown)
#define HX8357_TEON               0x35  ///< Tear enable on
#define HX8357_TEARLINE           0x44  ///< (unknown)
#define HX8357_MADCTL             0x36  ///< Memory access control
#define HX8357_COLMOD             0x3A  ///< Color mode

#define HX8357_SETOSC             0xB0  ///< Set oscillator
#define HX8357_SETPWR1            0xB1  ///< Set power control
#define HX8357B_SETDISPLAY        0xB2  ///< Set display mode
#define HX8357_SETRGB             0xB3  ///< Set RGB interface
#define HX8357D_SETCOM            0xB6  ///< Set VCOM voltage

#define HX8357B_SETDISPMODE       0xB4  ///< Set display mode
#define HX8357D_SETCYC            0xB4  ///< Set display cycle reg
#define HX8357B_SETOTP            0xB7  ///< Set OTP memory
#define HX8357D_SETC              0xB9  ///< Enable extension command

#define HX8357B_SET_PANEL_DRIVING 0xC0  ///< Set panel drive mode
#define HX8357D_SETSTBA           0xC0  ///< Set source option
#define HX8357B_SETDGC            0xC1  ///< Set DGC settings
#define HX8357B_SETID             0xC3  ///< Set ID
#define HX8357B_SETDDB            0xC4  ///< Set DDB
#define HX8357B_SETDISPLAYFRAME   0xC5  ///< Set display frame
#define HX8357B_GAMMASET          0xC8  ///< Set Gamma correction
#define HX8357B_SETCABC           0xC9  ///< Set CABC
#define HX8357_SETPANEL           0xCC  ///< Set Panel

#define HX8357B_SETPOWER          0xD0  ///< Set power control
#define HX8357B_SETVCOM           0xD1  ///< Set VCOM
#define HX8357B_SETPWRNORMAL      0xD2  ///< Set power normal

#define HX8357B_RDID1             0xDA  ///< Read ID #1
#define HX8357B_RDID2             0xDB  ///< Read ID #2
#define HX8357B_RDID3             0xDC  ///< Read ID #3
#define HX8357B_RDID4             0xDD  ///< Read ID #4

#define HX8357D_SETGAMMA          0xE0  ///< Set Gamma

#define HX8357B_SETGAMMA          0xC8 ///< Set Gamma
#define HX8357B_SETPANELRELATED   0xE9 ///< Set panel related


#define MADCTL_MY  0x80  ///< Bottom to top
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04  ///< LCD refresh right to left


//
// Initialization command structure
//
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;



//
// HX8357 Initialization commands
//
// Taken from the Adafruit driver
static const uint8_t
  initb[] = {
    HX8357B_SETPOWER, 3,
      0x44, 0x41, 0x06,
    HX8357B_SETVCOM, 2,
      0x40, 0x10,
    HX8357B_SETPWRNORMAL, 2,
      0x05, 0x12,
    HX8357B_SET_PANEL_DRIVING, 5,
      0x14, 0x3b, 0x00, 0x02, 0x11,
    HX8357B_SETDISPLAYFRAME, 1,
      0x0c,                      // 6.8mhz
    HX8357B_SETPANELRELATED, 1,
      0x01,                      // BGR
    0xEA, 3,                     // seq_undefined1, 3 args
      0x03, 0x00, 0x00,
    0xEB, 4,                     // undef2, 4 args
      0x40, 0x54, 0x26, 0xdb,
    HX8357B_SETGAMMA, 12,
      0x00, 0x15, 0x00, 0x22, 0x00, 0x08, 0x77, 0x26, 0x66, 0x22, 0x04, 0x00,
    HX8357_MADCTL, 1,
      0xC0,
    HX8357_COLMOD, 1,
      0x55,
    HX8357_PASET, 4,
      0x00, 0x00, 0x01, 0xDF,
    HX8357_CASET, 4,
      0x00, 0x00, 0x01, 0x3F,
    HX8357B_SETDISPMODE, 1,
      0x00,                      // CPU (DBI) and internal oscillation ??
    HX8357_SLPOUT, 0x80 + 120/5, // Exit sleep, then delay 120 ms
    HX8357_DISPON, 0x80 +  10/5, // Main screen turn on, delay 10 ms
    0                            // END OF COMMAND LIST
  }, initd[] = {
    HX8357_SWRESET, 0x80 + 100/5, // Soft reset, then delay 10 ms
    HX8357D_SETC, 3,
      0xFF, 0x83, 0x57,
    0xFF, 0x80 + 500/5,          // No command, just delay 300 ms
    HX8357_SETRGB, 4,
      0x80, 0x00, 0x06, 0x06,    // 0x80 enables SDO pin (0x00 disables)
    HX8357D_SETCOM, 1,
      0x25,                      // -1.52V
    HX8357_SETOSC, 1,
      0x68,                      // Normal mode 70Hz, Idle mode 55 Hz
    HX8357_SETPANEL, 1,
      0x05,                      // BGR, Gate direction swapped
    HX8357_SETPWR1, 6,
      0x00,                      // Not deep standby
      0x15,                      // BT
      0x1C,                      // VSPR
      0x1C,                      // VSNR
      0x83,                      // AP
      0xAA,                      // FS
    HX8357D_SETSTBA, 6,
      0x50,                      // OPON normal
      0x50,                      // OPON idle
      0x01,                      // STBA
      0x3C,                      // STBA
      0x1E,                      // STBA
      0x08,                      // GEN
    HX8357D_SETCYC, 7,
      0x02,                      // NW 0x02
      0x40,                      // RTN
      0x00,                      // DIV
      0x2A,                      // DUM
      0x2A,                      // DUM
      0x0D,                      // GDON
      0x78,                      // GDOFF
    HX8357D_SETGAMMA, 34,
      0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b,
      0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03, 0x02, 0x0A,
      0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b, 0x42, 0x3A,
      0x27, 0x1B, 0x08, 0x09, 0x03, 0x00, 0x01,
    HX8357_COLMOD, 1,
      0x55,                      // 16 bit
    HX8357_MADCTL, 1,
      0xC0,
    HX8357_TEON, 1,
      0x00,                      // TW off
    HX8357_TEARLINE, 2,
      0x00, 0x02,
    HX8357_SLPOUT, 0x80 + 150/5, // Exit Sleep, then delay 150 ms
    HX8357_DISPON, 0x80 +  50/5, // Main screen turn on, delay 50 ms
    0,                           // END OF COMMAND LIST
  };



// ==================================================
// littlevgl integration
//
/* Display flushing */
void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_array)
{
  int16_t w, h;

  w = (int16_t) (area->x2 - area->x1 + 1);
  h = (int16_t) (area->y2 - area->y1 + 1);
  tft_writeRect((int16_t) area->x1, (int16_t) area->y1, w, h, (const uint16_t*) color_array);

  /*Tell the flushing is ready*/
  lv_disp_flush_ready(disp);
}



// ==================================================
// API Code
//
void tft_begin()
{
  // Initialize the hardware
  pinMode(tft_cs_pin, OUTPUT);
  digitalWrite(tft_cs_pin, HIGH);
  pinMode(tft_dc_pin, OUTPUT);
  digitalWrite(tft_dc_pin, HIGH);  // "Data" is default

  // Send the initialization commands
  const uint8_t *addr = (DEFAULT_TFT == HX8357B) ? initb : initd;
  uint8_t        cmd, x, numArgs;
  while((cmd = *addr++) > 0) { // '0' command ends list
    x = *addr++;
    numArgs = x & 0x7F;
    if (cmd != 0xFF) { // '255' is ignored
      if (x & 0x80) {  // If high bit set, numArgs is a delay time
        tft_send_cmd(cmd);
      } else {
        tft_send_cmd(cmd);
        tft_send_data(addr, numArgs);
        addr += numArgs;
      }
    }
    if (x & 0x80) {       // If high bit set...
      vTaskDelay(numArgs * 5 / portTICK_RATE_MS); // numArgs is actually a delay time (5ms units)
    }
  }

  tft_setRotation(1);
  //tft_send_cmd(HX8357_INVON);
}



// ==================================================
// Internal Module Code
//
void tft_setRotation(uint8_t m)
{
  m = m % 4; // can't be higher than 3
  
  SPI.beginTransaction(TFT_SPI_SETTINGS);
  digitalWrite(tft_dc_pin, LOW);
  digitalWrite(tft_cs_pin, LOW);
  SPI.transfer(HX8357_MADCTL);
  digitalWrite(tft_dc_pin, HIGH);

  switch (m) {
    case 0:
      SPI.transfer(MADCTL_MX | MADCTL_BGR);
      break;
    case 1:
      SPI.transfer(MADCTL_MV | MADCTL_BGR);
      break;
    case 2:
      SPI.transfer(MADCTL_MY | MADCTL_BGR);
      break;
    default: // case 3:
      SPI.transfer(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      break;
  }
  digitalWrite(tft_cs_pin, HIGH);
  SPI.endTransaction();
}


void tft_writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors)
{
  SPI.beginTransaction(TFT_SPI_SETTINGS);
  digitalWrite(tft_cs_pin, LOW);
  
  // Set addr
  digitalWrite(tft_dc_pin, LOW);
  SPI.transfer(HX8357_CASET); // Column addr set
  digitalWrite(tft_dc_pin, HIGH);
  SPI.transfer16(x);   // XSTART
  SPI.transfer16(x+w-1);   // XEND
  
  digitalWrite(tft_dc_pin, LOW);
  SPI.transfer(HX8357_PASET); // Row addr set
  digitalWrite(tft_dc_pin, HIGH);
  SPI.transfer16(y);   // YSTART
  SPI.transfer16(y+h-1);   // YEND

  // Load RAM
  digitalWrite(tft_dc_pin, LOW);
  SPI.transfer(HX8357_RAMWR);
  digitalWrite(tft_dc_pin, HIGH);
  /*
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      SPI.transfer16(*pcolors++);
    }
  }
  */
  SPI.writeBytes((const uint8_t*) pcolors, w*h*2);
  digitalWrite(tft_cs_pin, HIGH);
  SPI.endTransaction();
}


void tft_send_cmd(uint8_t cmd)
{
  SPI.beginTransaction(TFT_SPI_SETTINGS);
  digitalWrite(tft_dc_pin, LOW);
  digitalWrite(tft_cs_pin, LOW);
  SPI.transfer(cmd);
  digitalWrite(tft_cs_pin, HIGH);
  SPI.endTransaction();  
}


void tft_send_data(const uint8_t* data, uint16_t length)
{
  SPI.beginTransaction(TFT_SPI_SETTINGS);
  digitalWrite(tft_dc_pin, HIGH);
  digitalWrite(tft_cs_pin, LOW);
  while (length--) {
    SPI.transfer(*data++);
  }
  digitalWrite(tft_cs_pin, HIGH);
  SPI.endTransaction();
}
