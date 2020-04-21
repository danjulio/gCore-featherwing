/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
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


 /*
  * Ported to gCore by Dan Julio
  */



/*
 * TFT Display signals:
 *   SCK  => GPIO 5
 *   MOSI => GPIO 18
 *   CS   => GPIO 15
 *   D/C  => GPIO 33
 *   RESX => UNUSED
 */



#include "fabgl.h"



fabgl::ILI9341Controller DisplayController;



#define TFT_SCK    5
#define TFT_MOSI   18
#define TFT_CS     15
#define TFT_DC     33
#define TFT_RESET  GPIO_UNUSED
#define TFT_SPIBUS VSPI_HOST

#define TS_CS      32
#define SD_CS      14
#define PWR_HOLD 2



void test(Color bcolor, char const * msg)
{
  Canvas cv(&DisplayController);
  cv.setBrushColor(bcolor);
  cv.clear();
  cv.setBrushColor(Color::BrightRed);
  int w = cv.getWidth();
  int h = cv.getHeight();
  Serial.printf("%d %d\n", w, h);
  Point pts[3] = { {w/2, 0}, {w-1, h-1}, {0, h-1} };
  cv.fillPath(pts, 3);
  cv.drawPath(pts, 3);
  cv.selectFont(&fabgl::FONT_8x14);
  cv.drawTextFmt(w/3, h/2, "%s", msg);
  delay(2000);
}


void setup()
{
  pinMode(PWR_HOLD, OUTPUT);
  digitalWrite(PWR_HOLD, HIGH);
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  
  //Serial.begin(115200); delay(500); Serial.write("\n\n\n"); // DEBUG ONLY

  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS);
  DisplayController.setResolution(TFT_320x480);
}


void loop()
{
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate0);
  test(Color::Blue, "Rotate0");
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate90);
  test(Color::Green, "Rotate90");
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate180);
  test(Color::Yellow,"Rotate180");
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate270);
  test(Color::Magenta, "Rotate270");
}
