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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "gcore_power.h"
#include "gui.h"
#include "life.h"
#include "life_obj.h"
#include "lvgl/lvgl.h"


//
// Constants
//
#define TAG "GUI"


typedef enum
{
	STOPPED,
	SINGLE,
	RUNNING
} gui_run_state_t;


typedef enum
{
	EDIT_CELL,
	ADD_OBJ
} gui_edit_type_t;


struct gui_edit_state_t
{
	int cur_selection_index;
	gui_edit_type_t edit_type;
	struct life_obj_t* life_obj;
};


//
// Variables
//

// Overall theme
static lv_theme_t* theme;

// LVGL Objects
static lv_obj_t* main_screen;
static lv_obj_t* lbl_gen_count;
static lv_obj_t* btn_clear_grid;
static lv_obj_t* btn_randomize_grid;
static lv_obj_t* dd_edit_type;
static lv_obj_t* btn_edit_grid;
static lv_obj_t* btn_pause;
static lv_obj_t* btn_run;
static lv_obj_t* btn_single_step;
static lv_obj_t* lbl_status;
static lv_obj_t* btn_power_button;
static lv_obj_t* canvas_grid;

// Styles for cell rectangles drawn or cleared on the canvas
static lv_style_t cell_set_style;
static lv_style_t cell_clear_style;

// Canvas buffer
static lv_color_t* canvas_buffer;

// LVGL sub-tasks
static lv_task_t* gui_life_subtask;
static lv_task_t* gui_status_subtask;

// String created for drop-down
static char* dd_edit_list;

// State
static gui_run_state_t run_state;
static struct gui_edit_state_t edit_state;
static bool enable_edit;
static lv_point_t prev_cell;      // Last touched location



//
// Forward Declarations for internal functions
//
static void gui_screen_create();
static void gui_life_obj_add();
static void gui_update_gen_count();
static void gui_update_status();
static void gui_set_run_state(gui_run_state_t s);
static void gui_update_grid();
static void gui_modify_grid(lv_point_t grid_cell, bool val);
static void gui_add_obj_to_grid(lv_point_t start_cell, struct life_obj_t* life_obj);

static void gui_eval_life_subtask(lv_task_t * task);
static void gui_eval_status_subtask(lv_task_t * task);

static void cb_clear_grid(lv_obj_t * btn, lv_event_t event);
static void cb_randomize_grid(lv_obj_t * btn, lv_event_t event);
static void cb_edit_type(lv_obj_t * dd, lv_event_t event);
static void cb_edit_grid(lv_obj_t * btn, lv_event_t event);
static void cb_pause(lv_obj_t * btn, lv_event_t event);
static void cb_run(lv_obj_t * btn, lv_event_t event);
static void cb_single_step(lv_obj_t * btn, lv_event_t event);
static void cb_power_button(lv_obj_t * btn, lv_event_t event);
static void cb_canvas_grid(lv_obj_t * btn, lv_event_t event);

static bool touch_to_grid(lv_point_t pixel_point, lv_point_t* grid_point);


//
// GUI API
//
void gui_init()
{
	// Allocate buffer memory
	canvas_buffer = (lv_color_t*) heap_caps_malloc(2*GUI_LIFE_CANVAS_WIDTH*GUI_LIFE_CANVAS_HEIGHT, MALLOC_CAP_SPIRAM);
	if (canvas_buffer == NULL) {
		ESP_LOGE(TAG, "malloc canvas_buffer failed");
		return;
	}
	
	// Initialize the life engine
	if (!life_init(LIFE_NUM_HORIZONTAL, LIFE_NUM_VERTICAL)) {
		ESP_LOGE(TAG, "malloc life buffers failed");
		return;
	}
	
	// Initialize the graphics
	gui_screen_create();
	gui_set_run_state(STOPPED);
	
	// Load an initial pattern as our title
	prev_cell.x = 1;
	prev_cell.y = 2;
	gui_add_obj_to_grid(prev_cell, get_title_obj());
	
	// Variables
	edit_state.cur_selection_index = 0;
	edit_state.edit_type = EDIT_CELL;
	edit_state.life_obj = NULL;
	enable_edit = false;
	prev_cell.x = -1;
	prev_cell.y = -1;
	
	// Start the sub-tasks
	gui_life_subtask = lv_task_create(gui_eval_life_subtask, 125, LV_TASK_PRIO_HIGH, NULL);
	gui_status_subtask = lv_task_create(gui_eval_status_subtask, 1000, LV_TASK_PRIO_LOW, NULL);
}


//
// Internal functions
//
static void gui_screen_create()
{
	static lv_obj_t* btn_clear_grid_label;
	static lv_obj_t* btn_randomize_grid_label;
	static lv_obj_t* btn_edit_grid_label;
	static lv_obj_t* btn_pause_label;
	static lv_obj_t* btn_run_label;
	static lv_obj_t* btn_single_step_label;
	static lv_obj_t* btn_power_button_label;
	
	// Setup a global theme and Initialize the underlying screen object
	theme = lv_theme_alien_init(GUI_THEME_HUE, NULL);
	lv_theme_set_current(theme);
	
	main_screen = lv_obj_create(NULL, NULL);
	lv_obj_set_size(main_screen, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	// Create the drawing canvas
	canvas_grid = lv_canvas_create(main_screen, NULL);
	lv_canvas_set_buffer(canvas_grid, canvas_buffer, GUI_LIFE_CANVAS_WIDTH, GUI_LIFE_CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
	lv_obj_set_pos(canvas_grid, GUI_LIFE_CANVAS_LEFT, GUI_LIFE_CANVAS_TOP);
	lv_canvas_fill_bg(canvas_grid, LV_COLOR_BLACK);
	lv_obj_set_click(canvas_grid, true);
	lv_obj_set_event_cb(canvas_grid, cb_canvas_grid);
	
	// Create the GUI controls and assign callbacks
	lbl_gen_count = lv_label_create(main_screen, NULL);
	lv_obj_set_pos(lbl_gen_count, GUI_CTRL_GEN_COUNT_X, GUI_CONTROL_LBL_Y);
	lv_obj_set_width(lbl_gen_count, GUI_CTRL_GEN_COUNT_W);
	lv_label_set_align(lbl_gen_count, LV_LABEL_ALIGN_RIGHT);
	gui_update_gen_count();
	
	btn_clear_grid = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_clear_grid, GUI_CTRL_CLR_GRID_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_clear_grid, GUI_CTRL_CLR_GRID_W, GUI_CONTROL_HEIGHT);
	btn_clear_grid_label = lv_label_create(btn_clear_grid, NULL);
	lv_label_set_static_text(btn_clear_grid_label, LV_SYMBOL_TRASH);
	lv_obj_set_event_cb(btn_clear_grid, cb_clear_grid);
	
	btn_randomize_grid = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_randomize_grid, GUI_CTRL_RND_GRID_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_randomize_grid, GUI_CTRL_RND_GRID_W, GUI_CONTROL_HEIGHT);
	btn_randomize_grid_label = lv_label_create(btn_randomize_grid, NULL);
	lv_label_set_static_text(btn_randomize_grid_label, LV_SYMBOL_REFRESH);
	lv_obj_set_event_cb(btn_randomize_grid, cb_randomize_grid);
	
	dd_edit_type = lv_ddlist_create(main_screen, NULL);
	gui_life_obj_add();
	lv_ddlist_set_selected(dd_edit_type, 0);
	lv_obj_set_pos(dd_edit_type, GUI_CTRL_EDIT_DD_X, GUI_CONTROL_BTN_Y);
	lv_ddlist_set_fix_width(dd_edit_type, GUI_CTRL_EDIT_DD_W);
	lv_ddlist_set_fix_height(dd_edit_type, 310);
	lv_ddlist_set_sb_mode(dd_edit_type, LV_SB_MODE_AUTO);
	lv_obj_set_event_cb(dd_edit_type, cb_edit_type);
	
	btn_edit_grid = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_edit_grid, GUI_CTRL_EDIT_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_edit_grid, GUI_CTRL_EDIT_W, GUI_CONTROL_HEIGHT);
	btn_edit_grid_label = lv_label_create(btn_edit_grid, NULL);
	lv_label_set_static_text(btn_edit_grid_label, LV_SYMBOL_EDIT);
	lv_obj_set_event_cb(btn_edit_grid, cb_edit_grid);
	
	btn_pause = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_pause, GUI_CTRL_PAUSE_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_pause, GUI_CTRL_PAUSE_W, GUI_CONTROL_HEIGHT);
	btn_pause_label = lv_label_create(btn_pause, NULL);
	lv_label_set_static_text(btn_pause_label, LV_SYMBOL_PAUSE);
	lv_obj_set_event_cb(btn_pause, cb_pause);
	
	btn_run = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_run, GUI_CTRL_RUN_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_run, GUI_CTRL_RUN_W, GUI_CONTROL_HEIGHT);
	btn_run_label = lv_label_create(btn_run, NULL);
	lv_label_set_static_text(btn_run_label, LV_SYMBOL_PLAY);
	lv_obj_set_event_cb(btn_run, cb_run);
	
	btn_single_step = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_single_step, GUI_CTRL_STEP_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_single_step, GUI_CTRL_STEP_W, GUI_CONTROL_HEIGHT);
	btn_single_step_label = lv_label_create(btn_single_step, NULL);
	lv_label_set_static_text(btn_single_step_label, LV_SYMBOL_NEXT);
	lv_obj_set_event_cb(btn_single_step, cb_single_step);
	
	lbl_status = lv_label_create(main_screen, NULL);
	lv_obj_set_pos(lbl_status, GUI_CTRL_STATUS_X, GUI_CONTROL_LBL_Y);
	lv_obj_set_width(lbl_status, GUI_CTRL_STATUS_W);
	lv_label_set_align(lbl_status, LV_LABEL_ALIGN_LEFT);
	gui_update_status();
	
	btn_power_button = lv_btn_create(main_screen, NULL);
	lv_obj_set_pos(btn_power_button, GUI_CTRL_PWROFF_X, GUI_CONTROL_BTN_Y);
	lv_obj_set_size(btn_power_button, GUI_CTRL_PWROFF_W, GUI_CONTROL_HEIGHT);
	btn_power_button_label = lv_label_create(btn_power_button, NULL);
	lv_label_set_static_text(btn_power_button_label, LV_SYMBOL_POWER);
	lv_obj_set_event_cb(btn_power_button, cb_power_button);
	
	// Create the canvas cell styles
	lv_style_copy(&cell_set_style, &lv_style_plain_color);
	
	lv_style_copy(&cell_clear_style, &lv_style_plain);
	cell_clear_style.body.main_color = LV_COLOR_BLACK;
	cell_clear_style.body.grad_color = LV_COLOR_BLACK;
	
	// Tell LVGL to use this set of objects
	lv_scr_load(main_screen);
}


static void gui_life_obj_add()
{
	const char* init_items = "Edit Cell\n";
	char* cp;
	int i;
	int n;
	struct life_obj_t* obj;
	
	// "Edit Cell\n" + "[item]\n" + ... + "[itme]0"
	
	// Determine how much space we need for the string
	n = strlen(init_items);
	for (i=0; i<LIFE_OBJ_MENU_NUM; i++) {
		obj = get_edit_obj(i);
		if (obj != NULL) {
			n += strlen(obj->name) + 1;   // +1 includes "\n"
		}
	}
	
	// Allocate our string
	dd_edit_list = malloc(n);
	if (dd_edit_list == NULL) {
		return;
	}
	
	// Add edit objects to the drop-down menu
	n = 0;
	cp = (char*) &init_items[0];
	while (*cp != 0) dd_edit_list[n++] = *cp++;
	for (i=0; i<LIFE_OBJ_MENU_NUM; i++) {
		obj = get_edit_obj(i);
		if (obj != NULL) {
			cp = (char*) &obj->name[0];
			while (*cp != 0) dd_edit_list[n++] = *cp++;
			if (i != (LIFE_OBJ_MENU_NUM-1)) {
				// Add trailing "\n" to all but last entry
				dd_edit_list[n++] = '\n';
			}
		}
	}
	dd_edit_list[n] = 0;  // Terminate string	
	
	// Finally, add the list of items to the drop-down menu
	lv_ddlist_set_options(dd_edit_type, dd_edit_list);
}


static void gui_update_gen_count()
{
	static char cnt_buf[12];   // Statically allocated for lv_label_set_static_text
	static int prev_cnt = -1;  // State to prevent unnecessary updates
	int cur_cnt;
	
	cur_cnt = life_get_gen_count();
	if (cur_cnt != prev_cnt) {
		sprintf(cnt_buf, "%d", cur_cnt);
		lv_label_set_static_text(lbl_gen_count, cnt_buf);
		prev_cnt = cur_cnt;
	}
}


static void gui_update_status()
{
	static char batt_buf[8];  // Statically allocated for lv_label_set_static_text
	static int prev_bs = -1;
	static gcore_charge_t prev_cs = CHARGE_FAULT;
	int cur_bs;
	gcore_charge_t cur_cs;
	float bv;
	
	// Get current battery and charge state
	bv = gcore_get_batt_voltage();
	cur_cs = gcore_get_charge_state();
	
	// Compute current battery level
	if (bv <= BATT_CRIT_THRESHOLD) cur_bs = 0;
	else if (bv <= BATT_0_THRESHOLD) cur_bs = 1;
	else if (bv <= BATT_25_THRESHOLD) cur_bs = 2;
	else if (bv <= BATT_50_THRESHOLD) cur_bs = 3;
	else if (bv <= BATT_75_THRESHOLD) cur_bs = 4;
	else cur_bs = 5;
	
	// Update if necessary
	if ((cur_bs != prev_bs) || (cur_cs != prev_cs)) {
		// Set battery charge condition icon
		switch (cur_bs) {
			case 5:
				strcpy(&batt_buf[0], LV_SYMBOL_BATTERY_FULL);
				break;
			case 4:
				strcpy(&batt_buf[0], LV_SYMBOL_BATTERY_3);
				break;
			case 3:
				strcpy(&batt_buf[0], LV_SYMBOL_BATTERY_2);
				break;
			case 2:
				strcpy(&batt_buf[0], LV_SYMBOL_BATTERY_1);
				break;
			default:
				strcpy(&batt_buf[0], LV_SYMBOL_BATTERY_EMPTY);
				break;
		}
	
		// Space between
		batt_buf[3] = ' ';
	
		// Set charge/fault icon
		switch (cur_cs) {
			case CHARGE_IDLE:
				strcpy(&batt_buf[4], "   ");
				break;
			case CHARGE_COMPLETE:
			case CHARGE_IN_PROGRESS:
				strcpy(&batt_buf[4], LV_SYMBOL_CHARGE);
				break;
			default:
				strcpy(&batt_buf[4], LV_SYMBOL_WARNING);
				break;
		}
	
		// Null terminator
		batt_buf[7] = 0;
	
		lv_label_set_static_text(lbl_status, batt_buf);
		
		prev_bs = cur_bs;
		prev_cs = cur_cs;
	}
}


static void gui_set_run_state(gui_run_state_t s)
{
	switch (s) {
		case STOPPED:
			lv_btn_set_state(btn_pause, LV_BTN_STATE_PR);
			lv_btn_set_state(btn_run, LV_BTN_STATE_REL);
			lv_btn_set_state(btn_single_step, LV_BTN_STATE_REL);
			break;
			
		case SINGLE:
			lv_btn_set_state(btn_pause, LV_BTN_STATE_REL);
			lv_btn_set_state(btn_run, LV_BTN_STATE_REL);
			lv_btn_set_state(btn_single_step, LV_BTN_STATE_PR);
			break;
			
		case RUNNING:
			lv_btn_set_state(btn_pause, LV_BTN_STATE_REL);
			lv_btn_set_state(btn_run, LV_BTN_STATE_PR);
			lv_btn_set_state(btn_single_step, LV_BTN_STATE_REL);
			break;
	}
	
	run_state = s;
}


static void gui_update_grid()
{
	int x, y;
	int x1, y1;
	bool set;
	
	for (y=0; y<LIFE_NUM_VERTICAL; y++) {
		y1 = y*GUI_LIFE_CELL_HEIGHT;
		for (x=0; x<LIFE_NUM_HORIZONTAL; x++) {
			if (life_cell_changed(x, y, &set)) {
				x1 = x*GUI_LIFE_CELL_WIDTH;
				if (set) {
					lv_canvas_draw_rect(canvas_grid, x1+1, y1+1, GUI_LIFE_CELL_WIDTH-2, GUI_LIFE_CELL_HEIGHT-2, &cell_set_style);
				} else {
					lv_canvas_draw_rect(canvas_grid, x1, y1, GUI_LIFE_CELL_WIDTH, GUI_LIFE_CELL_HEIGHT, &cell_clear_style);
				}
			}
		}
	}
	
}


static void gui_modify_grid(lv_point_t grid_cell, bool val)
{
	int x, y;
	
	life_set_cell(grid_cell.x, grid_cell.y, val);
	
	x = grid_cell.x*GUI_LIFE_CELL_WIDTH;
	y = grid_cell.y*GUI_LIFE_CELL_HEIGHT;
	
	if (val) {
		lv_canvas_draw_rect(canvas_grid, x+1, y+1, GUI_LIFE_CELL_WIDTH-2, GUI_LIFE_CELL_HEIGHT-2, &cell_set_style);
	} else {
		lv_canvas_draw_rect(canvas_grid, x, y, GUI_LIFE_CELL_WIDTH, GUI_LIFE_CELL_HEIGHT, &cell_clear_style);
	}
}


static void gui_add_obj_to_grid(lv_point_t start_cell, struct life_obj_t* life_obj)
{
	lv_point_t cur_cell;
	int index;
	bool val;
	
	
	for (cur_cell.y=start_cell.y; cur_cell.y<(start_cell.y + life_obj->h); cur_cell.y++) {
		if (cur_cell.y < LIFE_NUM_VERTICAL) {
			for (cur_cell.x=start_cell.x; cur_cell.x<(start_cell.x + life_obj->w); cur_cell.x++) {
				if (cur_cell.x < LIFE_NUM_HORIZONTAL) {
					index = (cur_cell.y - start_cell.y)*life_obj->w + (cur_cell.x - start_cell.x);
					val = *(life_obj->array + index) ? true : false;
					gui_modify_grid(cur_cell, val);
				}
			}
		}
	}
}


static void gui_eval_life_subtask(lv_task_t * task)
{
	if (run_state != STOPPED) {
		life_step();
		gui_update_grid();
		gui_update_gen_count();
		
		if (run_state == SINGLE) {
			gui_set_run_state(STOPPED);
		}
	}
}


static void gui_eval_status_subtask(lv_task_t * task)
{
	gui_update_status();
}


static void cb_clear_grid(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		life_clear();
		lv_canvas_fill_bg(canvas_grid, LV_COLOR_BLACK);
		gui_update_gen_count();
		
		if (run_state != STOPPED) {
			gui_set_run_state(STOPPED);
		}
	}
}


static void cb_randomize_grid(lv_obj_t * btn, lv_event_t event)
{
	int x, y;
	
	if (event == LV_EVENT_CLICKED) {
		life_clear();
		lv_canvas_fill_bg(canvas_grid, LV_COLOR_BLACK);
		
		for (y=0; y<LIFE_NUM_VERTICAL; y++) {
			for (x=0; x<LIFE_NUM_HORIZONTAL; x++) {
				// Less than 1/3 density seems ok
				if (esp_random() > 0xB7FFFFFF) {
					life_set_cell(x, y, true);
				}
			}
		}
		gui_update_grid();
		gui_update_gen_count();
	}
}


static void cb_edit_type(lv_obj_t * dd, lv_event_t event)
{
	int new_sel;
	
	if (event == LV_EVENT_VALUE_CHANGED) {
		new_sel = lv_ddlist_get_selected(dd);
		if (new_sel != edit_state.cur_selection_index) {
			edit_state.cur_selection_index = new_sel;
			if (new_sel == 0) {
				edit_state.edit_type = EDIT_CELL;
				edit_state.life_obj = NULL;
			} else {
				edit_state.edit_type = ADD_OBJ;
				edit_state.life_obj = get_edit_obj(new_sel - 1);
			}
		}
	}
}


static void cb_edit_grid(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (enable_edit) {
			enable_edit = false;
			lv_btn_set_state(btn, LV_BTN_STATE_REL);
		} else {
			enable_edit = true;
			lv_btn_set_state(btn, LV_BTN_STATE_PR);
		}
	}
}


static void cb_pause(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (run_state != STOPPED) {
			gui_set_run_state(STOPPED);
		} else {
			// Reset the "set" status of the button since pressing it clears that
			lv_btn_set_state(btn, LV_BTN_STATE_PR);
		}
	}
}


static void cb_run(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (run_state != RUNNING) {
			gui_set_run_state(RUNNING);
		} else {
			// Reset the "set" status of the button since pressing it clears that
			lv_btn_set_state(btn, LV_BTN_STATE_PR);
		}
	}
}


static void cb_single_step(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (run_state != SINGLE) {
			gui_set_run_state(SINGLE);
		}
	}
}


static void cb_power_button(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		gcore_power_down();
	}
}


static void cb_canvas_grid(lv_obj_t * btn, lv_event_t event)
{
	lv_indev_t* touch;          // Input device
	lv_point_t cur_point;
	lv_point_t cur_cell;
	bool initial_press;         // Differentiate between the first touch and dragging
	static bool adding_cells;   // Determined on initial press during EDIT_CELL operations
	
	if ((event == LV_EVENT_PRESSED) || (event == LV_EVENT_PRESSING)) {
		initial_press = (event == LV_EVENT_PRESSED);
		touch = lv_indev_get_act();
		lv_indev_get_point(touch, &cur_point);
		if (touch_to_grid(cur_point, &cur_cell)) {
			if ((cur_cell.x != prev_cell.x) || (cur_cell.y != prev_cell.y)) {
				if (enable_edit) {
					if (edit_state.edit_type == EDIT_CELL) {
						if (initial_press) {
							// Get the content of the cell being touched to determine what
							// we will do to it and subsequent cells as we drag along
							adding_cells = !life_get_cell(cur_cell.x, cur_cell.y);
						}
						if (adding_cells) {
							gui_modify_grid(cur_cell, true);
						} else {
							gui_modify_grid(cur_cell, false);
						}
					} else {
						// Only add an object on the first touch to avoid "smearing" them
						// during an accidental drag
						if ((edit_state.life_obj != NULL) && initial_press) {
							gui_add_obj_to_grid(cur_cell, edit_state.life_obj);
						}
					}
					prev_cell.x = cur_cell.x;
					prev_cell.y = cur_cell.y;
				}
			}
		}
	} else if ((event == LV_EVENT_PRESS_LOST) || (event == LV_EVENT_RELEASED)) {
		prev_cell.x = -1;
		prev_cell.y = -1;
	}
}


static bool touch_to_grid(lv_point_t pixel_point, lv_point_t* grid_point)
{
	grid_point->x = (pixel_point.x - GUI_LIFE_CANVAS_LEFT) / GUI_LIFE_CELL_WIDTH;
	grid_point->y = (pixel_point.y - GUI_LIFE_CANVAS_TOP) / GUI_LIFE_CELL_HEIGHT;
	
	return ((grid_point->x < LIFE_NUM_HORIZONTAL) && (grid_point->y < LIFE_NUM_VERTICAL));
}
