/*
 * Implementation of John Conway's Life program - evaluate steps and
 * allow access to the two-dimensional grid.
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
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "life.h"

//
// Constants
//
#define TAB "LIFE"


//
// Variables
//

static int life_w;
static int life_h;
static int num_cells;

// 2-dimensional grid is linear array of bytes storing 0 or 1 for each cell.
// There are two arrays: Current and Next generation.
static uint8_t* life_array[2];
static int life_cur_index;

static int gen_count;



//
// Forward declarations for internal functions
//
int cells_adjacent_to(int x, int y);



//
// API
//
bool life_init(int w, int h)
{
	life_w = w;
	life_h = h;
	num_cells = w * h;
	
	// Allocate the arrays in SPIRAM
	life_array[0] = heap_caps_malloc(num_cells, MALLOC_CAP_SPIRAM);
	if (life_array[0] == NULL) {
		return false;
	}
	life_array[1] = heap_caps_malloc(num_cells, MALLOC_CAP_SPIRAM);
	if (life_array[1] == NULL) {
		return false;
	}
	
	life_clear();
	
	return true;
}


void life_clear()
{
	int n;
	
	// Clear the bitmasks
	n = num_cells;
	while (n--) {
		*(life_array[0] + n) = 0;
		*(life_array[1] + n) = 0;
	}
	
	life_cur_index = 0;
	gen_count = 0;
}


void life_step()
{
	int x, y;
	int n;
	int next_index;
	int cell_num = 0;
	
	next_index = (life_cur_index) ? 0 : 1;
	
	/*
	 * C   N                 new C
	 * 1   0,1             ->  0  # Lonely
	 * 1   4,5,6,7,8       ->  0  # Overcrowded
	 * 1   2,3             ->  1  # Lives
	 * 0   3               ->  1  # It takes three to give birth!
	 * 0   0,1,2,4,5,6,7,8 ->  0  # Barren
     */
	for (y=0; y<life_h; y++) {
		for (x=0; x<life_w; x++) {
//			cell_num = x + y*life_w;
			n = cells_adjacent_to(x, y);
		
			// Implement the rules of life!
//			if (n < 2) {
//				*(life_array[next_index] + cell_num) = 0;
//			}
//			else 
			if (n == 2) {
				*(life_array[next_index] + cell_num) = *(life_array[life_cur_index] + cell_num);
			}
			else if (n == 3) {
				*(life_array[next_index] + cell_num) = 1;
			}		
			else {
				*(life_array[next_index] + cell_num) = 0;
			}
			cell_num++;
		}
	}
	
	life_cur_index = next_index;
	gen_count++;
}


void life_set_cell(int x, int y, bool val)
{
	*(life_array[life_cur_index] + x + y*life_w) = val ? 1 : 0;
}


bool life_get_cell(int x, int y)
{
	return (*(life_array[life_cur_index] + x + y*life_w)) ? true : false;
}


bool life_cell_changed(int x, int y, bool* val)
{
	bool val_cur;
	bool val_prev;
	int prev_index;
	
	prev_index = (life_cur_index) ? 0 : 1;
	
	val_cur = *(life_array[life_cur_index] + x + y*life_w);
	val_prev = *(life_array[prev_index] + x + y*life_w);
	
	*val = val_cur;
	return (val_cur != val_prev);
}


int life_get_gen_count()
{
	return gen_count;
}



//
// Internal functions
//

// World ends at borders, does not wrap
int cells_adjacent_to(int x, int y)
{
	int i, j;
	int x1, x2;
	int y1, y2;
	int n;
	
	// Compute legal boundaries
	x1 = (x == 0) ? 0 : (x - 1);
	x2 = (x == life_w-1) ? (life_w-1) : (x + 1);
	y1 = (y == 0) ? 0 : (y - 1);
	y2 = (y == life_h-1) ? (life_h-1) : (y + 1);
	n  = 0;
	
	// Count surrounding cells
	for (j=y1; j<=y2; j++) {
		for (i=x1; i<=x2; i++) {
			if ((i != x) || (j != y)) {
				n += *(life_array[life_cur_index] + i + j*life_w);
			}
		}
	}
	
	return n;
}
