/*
 Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
 Copyright (c) 2019-2020 Fabrizio Di Vittorio.
 All rights reserved.

 This file is part of FabGL Library.

 FabGL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 FabGL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "fabutils.h"
#include "TFTControllerSpecif.h"



namespace fabgl {



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// ST7789


#define ST7789_SWRST      0x01
#define ST7789_RDDCOLMOD  0x0C
#define ST7789_SLPOUT     0x11
#define ST7789_PTLON      0x12
#define ST7789_NORON      0x13
#define ST7789_INVOFF     0x20
#define ST7789_INVON      0x21
#define ST7789_DISPON     0x29
#define ST7789_PTLAR      0x30
#define ST7789_COLMOD     0x3A
#define ST7789_WRDISBV    0x51
#define ST7789_WRCTRLD    0x53
#define ST7789_WRCACE     0x55
#define ST7789_WRCABCMB   0x5E
#define ST7789_RAMCTRL    0xB0
#define ST7789_PORCTRL    0xB2
#define ST7789_GCTRL      0xB7
#define ST7789_VCOMS      0xBB
#define ST7789_LCMCTRL    0xC0
#define ST7789_VDVVRHEN   0xC2
#define ST7789_VRHS       0xC3
#define ST7789_VDVS       0xC4
#define ST7789_FRCTRL2    0xC6
#define ST7789_PWCTRL1    0xD0
#define ST7789_PVGAMCTRL  0xE0
#define ST7789_NVGAMCTRL  0xE1


void ST7789Controller::softReset()
{
  // software reset
  SPIBeginWrite();
  writeCommand(ST7789_SWRST);
  SPIEndWrite();
  vTaskDelay(150 / portTICK_PERIOD_MS);

  SPIBeginWrite();

  // Sleep Out
  writeCommand(ST7789_SLPOUT);
  vTaskDelay(120 / portTICK_PERIOD_MS);

  // Normal Display Mode On
  writeCommand(ST7789_NORON);

  setupOrientation();

  // 0x55 = 0 (101) 0 (101) => 65K of RGB interface, 16 bit/pixel
  writeCommand(ST7789_COLMOD);
  writeByte(0x55);
  vTaskDelay(10 / portTICK_PERIOD_MS);

  // Porch Setting
  writeCommand(ST7789_PORCTRL);
  writeByte(0x0c);
  writeByte(0x0c);
  writeByte(0x00);
  writeByte(0x33);
  writeByte(0x33);

  // Gate Control
  // VGL = -10.43V
  // VGH = 13.26V
  writeCommand(ST7789_GCTRL);
  writeByte(0x35);

  // VCOM Setting
  // 1.1V
  writeCommand(ST7789_VCOMS);
  writeByte(0x28);

  // LCM Control
  // XMH, XMX
  writeCommand(ST7789_LCMCTRL);
  writeByte(0x0C);

  // VDV and VRH Command Enable
  // CMDEN = 1, VDV and VRH register value comes from command write.
  writeCommand(ST7789_VDVVRHEN);
  writeByte(0x01);
  writeByte(0xFF);

  // VRH Set
  // VAP(GVDD) = 4.35+( vcom+vcom offset+vdv) V
  // VAN(GVCL) = -4.35+( vcom+vcom offset-vdv) V
  writeCommand(ST7789_VRHS);
  writeByte(0x10);

  // VDV Set
  // VDV  = 0V
  writeCommand(ST7789_VDVS);
  writeByte(0x20);

  // Frame Rate Control in Normal Mode
  // RTNA = 0xf (60Hz)
  // NLA  = 0 (dot inversion)
  writeCommand(ST7789_FRCTRL2);
  writeByte(0x0f);

  // Power Control 1
  // VDS  = 2.3V
  // AVCL = -4.8V
  // AVDD = 6.8v
  writeCommand(ST7789_PWCTRL1);
  writeByte(0xa4);
  writeByte(0xa1);

  // Positive Voltage Gamma Control
  writeCommand(ST7789_PVGAMCTRL);
  writeByte(0xd0);
  writeByte(0x00);
  writeByte(0x02);
  writeByte(0x07);
  writeByte(0x0a);
  writeByte(0x28);
  writeByte(0x32);
  writeByte(0x44);
  writeByte(0x42);
  writeByte(0x06);
  writeByte(0x0e);
  writeByte(0x12);
  writeByte(0x14);
  writeByte(0x17);

  // Negative Voltage Gamma Control
  writeCommand(ST7789_NVGAMCTRL);
  writeByte(0xd0);
  writeByte(0x00);
  writeByte(0x02);
  writeByte(0x07);
  writeByte(0x0a);
  writeByte(0x28);
  writeByte(0x31);
  writeByte(0x54);
  writeByte(0x47);
  writeByte(0x0e);
  writeByte(0x1c);
  writeByte(0x17);
  writeByte(0x1b);
  writeByte(0x1e);

  // Display Inversion On
  writeCommand(ST7789_INVON);

  // Display On
  writeCommand(ST7789_DISPON);

  SPIEndWrite();
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// ILI9341


#define ILI9341_SWRESET           0x01
#define ILI9341_SLEEPOUT          0x11
#define ILI9341_NORON             0x13
#define ILI9341_GAMMASET          0x26
#define ILI9341_DISPON            0x29
#define ILI9341_PIXELFORMATSET    0x3A
#define ILI9341_FRAMERATECTRL1    0xB1
#define ILI9341_DISPLAYFUNCCTRL   0xB6
#define ILI9341_POWERCTR1         0xC0
#define ILI9341_POWERCTR2         0xC1
#define ILI9341_VCOMCTR1          0xC5
#define ILI9341_VCOMCTR2          0xC7
#define ILI9341_POWERCTRLA        0xCB
#define ILI9341_POWERCTRLB        0xCF
#define ILI9341_POSGAMMACORR      0xE0
#define ILI9341_NEGGAMMACORR      0xE1
#define ILI9341_DRIVERTIMINGCTRLA 0xE8
#define ILI9341_DRIVERTIMINGCTRLB 0xEA
#define ILI9341_POWERONSEQCTRL    0xED
#define ILI9341_DEVICECODE        0xEF
#define ILI9341_ENABLE3G          0xF2
#define ILI9341_PUMPRATIOCTRL     0xF7



void ILI9341Controller::softReset()
{
  m_reverseHorizontal = true;

  // software reset
  SPIBeginWrite();
  writeCommand(ILI9341_SWRESET);
  SPIEndWrite();
  vTaskDelay(150 / portTICK_PERIOD_MS);

  SPIBeginWrite();

  // unknown but required init sequence!
  writeCommand(ILI9341_DEVICECODE);
  writeByte(0x03);
  writeByte(0x80);
  writeByte(0x02);

  // Power control B
  writeCommand(ILI9341_POWERCTRLB);
  writeByte(0x00);
  writeByte(0XC1);
  writeByte(0X30);

  // Power on sequence control
  writeCommand(ILI9341_POWERONSEQCTRL);
  writeByte(0x64);
  writeByte(0x03);
  writeByte(0X12);
  writeByte(0X81);

  // Driver timing control A
  writeCommand(ILI9341_DRIVERTIMINGCTRLA);
  writeByte(0x85);
  writeByte(0x00);
  writeByte(0x78);

  // Power control A
  writeCommand(ILI9341_POWERCTRLA);
  writeByte(0x39);
  writeByte(0x2C);
  writeByte(0x00);
  writeByte(0x34);
  writeByte(0x02);

  // Pump ratio control
  writeCommand(ILI9341_PUMPRATIOCTRL);
  writeByte(0x20);

  // Driver timing control B
  writeCommand(ILI9341_DRIVERTIMINGCTRLB);
  writeByte(0x00);
  writeByte(0x00);

  // Power Control 1
  writeCommand(ILI9341_POWERCTR1);
  writeByte(0x23);

  // Power Control 2
  writeCommand(ILI9341_POWERCTR2);
  writeByte(0x10);

  // VCOM Control 1
  writeCommand(ILI9341_VCOMCTR1);
  writeByte(0x3e);
  writeByte(0x28);

  // VCOM Control 2
  writeCommand(ILI9341_VCOMCTR2);
  writeByte(0x86);

  setupOrientation();

  // COLMOD: Pixel Format Set
  writeCommand(ILI9341_PIXELFORMATSET);
  writeByte(0x55);

  // Frame Rate Control (In Normal Mode/Full Colors)
  writeCommand(ILI9341_FRAMERATECTRL1);
  writeByte(0x00);
  writeByte(0x13); // 0x18 79Hz, 0x1B 70Hz (default), 0x13 100Hz

  // Display Function Control
  writeCommand(ILI9341_DISPLAYFUNCCTRL);
  writeByte(0x08);
  writeByte(0x82);
  writeByte(0x27);

  // Enable 3G (gamma control)
  writeCommand(ILI9341_ENABLE3G);
  writeByte(0x00);  // bit 0: 0 => disable 3G

  // Gamma Set
  writeCommand(ILI9341_GAMMASET);
  writeByte(0x01);  // 1 = Gamma curve 1 (G2.2)

  // Positive Gamma Correction
  writeCommand(ILI9341_POSGAMMACORR);
  writeByte(0x0F);
  writeByte(0x31);
  writeByte(0x2B);
  writeByte(0x0C);
  writeByte(0x0E);
  writeByte(0x08);
  writeByte(0x4E);
  writeByte(0xF1);
  writeByte(0x37);
  writeByte(0x07);
  writeByte(0x10);
  writeByte(0x03);
  writeByte(0x0E);
  writeByte(0x09);
  writeByte(0x00);

  // Negative Gamma Correction
  writeCommand(ILI9341_NEGGAMMACORR);
  writeByte(0x00);
  writeByte(0x0E);
  writeByte(0x14);
  writeByte(0x03);
  writeByte(0x11);
  writeByte(0x07);
  writeByte(0x31);
  writeByte(0xC1);
  writeByte(0x48);
  writeByte(0x08);
  writeByte(0x0F);
  writeByte(0x0C);
  writeByte(0x31);
  writeByte(0x36);
  writeByte(0x0F);

  // Sleep Out
  writeCommand(ILI9341_SLEEPOUT);

  // Normal Display Mode On
  writeCommand(ILI9341_NORON);

  SPIEndWrite();

  vTaskDelay(120 / portTICK_PERIOD_MS);

  SPIBeginWrite();

  // Display ON
  writeCommand(ILI9341_DISPON);
  
  SPIEndWrite();
}




////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// HX8357

/*******************
 * HX8357B/D REGS
*********************/
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
#define HX8357_RAMRD              0x2E  ///< Read VRAm

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


/**********************
 *  INITIALIZATION ARRAYS
 **********************/
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
  
void HX8357BController::softReset()
{
	const uint8_t *addr = initb;
	uint8_t        cmd, x, numArgs;
	
	// software reset
	SPIBeginWrite();
	writeCommand(HX8357_SWRESET);
	SPIEndWrite();
	vTaskDelay(150 / portTICK_PERIOD_MS);
	
	//Send all the commands
	while((cmd = *addr++) > 0) { // '0' command ends list
		x = *addr++;
		numArgs = x & 0x7F;
		if (cmd != 0xFF) { // '255' is ignored
			SPIBeginWrite();
			if (x & 0x80) {  // If high bit set, numArgs is a delay time
				writeCommand(cmd);
			} else {
				writeCommand(cmd);
				while (numArgs-- > 0) {
					writeByte(*addr++);
				}
			}
			SPIEndWrite();
		}
		if (x & 0x80) {       // If high bit set...
			vTaskDelay(numArgs * 5 / portTICK_RATE_MS); // numArgs is actually a delay time (5ms units)
		}
	}
	
	SPIBeginWrite();
	setupOrientation();
	SPIEndWrite();
}

void HX8357DController::softReset()
{
	const uint8_t *addr = initd;
	uint8_t        cmd, x, numArgs;
	
	// software reset
	SPIBeginWrite();
	writeCommand(HX8357_SWRESET);
	SPIEndWrite();
	vTaskDelay(150 / portTICK_PERIOD_MS);
	
	//Send all the commands
	while((cmd = *addr++) > 0) { // '0' command ends list
		x = *addr++;
		numArgs = x & 0x7F;
		if (cmd != 0xFF) { // '255' is ignored
			SPIBeginWrite();
			if (x & 0x80) {  // If high bit set, numArgs is a delay time
				writeCommand(cmd);
			} else {
				writeCommand(cmd);
				writeData((void *) addr, numArgs);
				addr += numArgs;
			}
			SPIEndWrite();
		}
		if (x & 0x80) {       // If high bit set...
			vTaskDelay(numArgs * 5 / portTICK_RATE_MS); // numArgs is actually a delay time (5ms units)
		}
	}
	
	SPIBeginWrite();
	setupOrientation();
	SPIEndWrite();
}


} // end of namespace
