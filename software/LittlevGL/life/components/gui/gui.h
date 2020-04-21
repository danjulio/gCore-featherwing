/*
 * Main GUI screen related functions, callbacks and event handlers
 *
 * Copyright 2020 Dan Julio
 *
 * This file is part of life.
 *
 * life is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * life is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef GUI_H
#define GUI_H


//
// GUI Constants
//

// Theme hue (0-360)
#define GUI_THEME_HUE           210

// Control area layout configuration
#define GUI_CONTROL_AREA_TOP    0
#define GUI_CONTROL_BTN_Y_OFF   8
#define GUI_CONTROL_LBL_Y_OFF   20
#define GUI_CONTROL_BTN_Y       (GUI_CONTROL_AREA_TOP + GUI_CONTROL_BTN_Y_OFF)
#define GUI_CONTROL_LBL_Y       (GUI_CONTROL_AREA_TOP + GUI_CONTROL_LBL_Y_OFF)
#define GUI_CONTROL_HEIGHT      40
#define GUI_CONTROL_WIDTH       30

#define GUI_CTRL_GEN_COUNT_X    5
#define GUI_CTRL_GEN_COUNT_W    50
#define GUI_CTRL_CLR_GRID_X     60
#define GUI_CTRL_CLR_GRID_W     GUI_CONTROL_WIDTH
#define GUI_CTRL_RND_GRID_X     95
#define GUI_CTRL_RND_GRID_W     GUI_CONTROL_WIDTH
#define GUI_CTRL_EDIT_DD_X      130
#define GUI_CTRL_EDIT_DD_W      115
#define GUI_CTRL_EDIT_X         250
#define GUI_CTRL_EDIT_W         GUI_CONTROL_WIDTH
#define GUI_CTRL_PAUSE_X        285
#define GUI_CTRL_PAUSE_W        GUI_CONTROL_WIDTH
#define GUI_CTRL_RUN_X          320
#define GUI_CTRL_RUN_W          GUI_CONTROL_WIDTH
#define GUI_CTRL_STEP_X         355
#define GUI_CTRL_STEP_W         GUI_CONTROL_WIDTH
#define GUI_CTRL_STATUS_X       400
#define GUI_CTRL_STATUS_W       40
#define GUI_CTRL_PWROFF_X       445
#define GUI_CTRL_PWROFF_W       GUI_CONTROL_WIDTH

// Life 2-dimensional grid canvas configuration
#define GUI_LIFE_CANVAS_TOP     55
#define GUI_LIFE_CANVAS_LEFT    5

#define GUI_LIFE_CANVAS_WIDTH   470
#define GUI_LIFE_CANVAS_HEIGHT  260

// Life cells dimensions must fit
#define GUI_LIFE_CELL_WIDTH     10
#define GUI_LIFE_CELL_HEIGHT    10

// Life array parameters
#define LIFE_NUM_HORIZONTAL     (GUI_LIFE_CANVAS_WIDTH / GUI_LIFE_CELL_WIDTH)
#define LIFE_NUM_VERTICAL       (GUI_LIFE_CANVAS_HEIGHT / GUI_LIFE_CELL_HEIGHT)
#define LIFE_NUM_CELLS          (LIFE_NUM_HORIZONTAL * LIFE_NUM_VERTICAL)

// Battery state-of-charge curve
//   Based on 0.2C discharge rate on https://www.richtek.com/battery-management/en/designing-liion.html
//   This isn't particularly accurate...
#define BATT_75_THRESHOLD    3.9
#define BATT_50_THRESHOLD    3.72
#define BATT_25_THRESHOLD    3.66
#define BATT_0_THRESHOLD     3.6
#define BATT_CRIT_THRESHOLD  3.4


//
// GUI API
//
void gui_init();

#endif /* GUI_H */