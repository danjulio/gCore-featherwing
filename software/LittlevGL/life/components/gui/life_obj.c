/*
 * Bitmap objects that can be added to the Life grid.
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
#include "life_obj.h"

//
// Life bitmap objects are arrays organized as h (height) rows of w (width)
// uint8_t values containing 0 or 1.
//


//
// Bitmaps
//

// =======
// TITLE
// =======
static const uint8_t title_bitmask[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0
};

static struct life_obj_t title_obj = {"Title", 45, 21, (uint8_t*) title_bitmask};


// =======
// GLIDERS
// =======
static const uint8_t glider_r_u_bitmask[] = {
	1, 1, 1,
	0, 0, 1,
	0, 1, 0
};

static struct life_obj_t glider_r_u_obj = {"Glider_R_U", 3, 3, (uint8_t*) glider_r_u_bitmask};


static const uint8_t glider_l_u_bitmask[] = {
	1, 1, 1,
	1, 0, 0,
	0, 1, 0
};

static struct life_obj_t glider_l_u_obj = {"Glider_L_U", 3, 3, (uint8_t*) glider_l_u_bitmask};


static const uint8_t glider_r_d_bitmask[] = {
	0, 1, 0,
	0, 0, 1,
	1, 1, 1
};

static struct life_obj_t glider_r_d_obj = {"Glider_R_D", 3, 3, (uint8_t*) glider_r_d_bitmask};


static const uint8_t glider_l_d_bitmask[] = {
	0, 1, 0,
	1, 0, 0,
	1, 1, 1
};

static struct life_obj_t glider_l_d_obj = {"Glider_L_D", 3, 3, (uint8_t*) glider_l_d_bitmask};


// ==========
// GLIDER GUN
// ==========
static const uint8_t glider_gun_bitmask[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static struct life_obj_t glider_gun_obj = {"Glider Gun", 36, 9, (uint8_t*) glider_gun_bitmask};


// ========
// BLINKERS
// ========
static const uint8_t blinker_1_bitmask[] = {
	0, 1, 0, 0,
	0, 0, 1, 1,
	1, 1, 0, 0,
	0, 0, 1, 0
};

static struct life_obj_t blinker_1_obj = {"Blinker 1", 4, 4, (uint8_t*) blinker_1_bitmask};


static const uint8_t blinker_2_bitmask[] = {
	0, 1, 0,
	1, 1, 1,
	0, 1, 0

};

static struct life_obj_t blinker_2_obj = {"Blinker 2", 3, 3, (uint8_t*) blinker_2_bitmask};


static const uint8_t blinker_3_bitmask[] = {
	0, 0, 1, 1,
	0, 0, 1, 1,
	1, 1, 0, 0,
	1, 1, 0, 0
};

static struct life_obj_t blinker_3_obj = {"Blinker 3", 4, 4, (uint8_t*) blinker_3_bitmask};


// ====
// EDEN
// ====
static const uint8_t eden_bitmask[] = {
	0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0,
	0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,
	0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0,
	0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0,
	1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

static struct life_obj_t eden_obj = {"Eden", 12, 11, (uint8_t*) eden_bitmask};


// ==========
// SPACESHIPS
// ==========
static const uint8_t spaceship_r_bitmask[] = {
	0, 0, 0, 1, 0,
	0, 0, 0, 0, 1,
	1, 0, 0, 0, 1,
	0, 1, 1, 1, 1
};

static struct life_obj_t spaceship_r_obj = {"Spaceship R", 5, 4, (uint8_t*) spaceship_r_bitmask};


static const uint8_t spaceship_l_bitmask[] = {
	0, 1, 0, 0, 0,
	1, 0, 0, 0, 0,
	1, 0, 0, 0, 1,
	1, 1, 1, 1, 0
};

static struct life_obj_t spaceship_l_obj = {"Spaceship L", 5, 4, (uint8_t*) spaceship_l_bitmask};


// ======
// STABLE
// ======
static const uint8_t stable_1_bitmask[] = {
	0, 1, 0,
	1, 0, 1,
	1, 0, 1,
	0, 1, 0
};

static struct life_obj_t stable_1_obj = {"Stable 1", 3, 4, (uint8_t*) stable_1_bitmask};


static const uint8_t stable_2_bitmask[] = {
	0, 1, 0,
	1, 0, 1,
	0, 1, 1
};

static struct life_obj_t stable_2_obj = {"Stable 2", 3, 3, (uint8_t*) stable_2_bitmask};


static const uint8_t stable_3_bitmask[] = {
	0, 0, 1, 0,
	0, 1, 0, 1,
	1, 0, 0, 1,
	0, 1, 1, 0
};

static struct life_obj_t stable_3_obj = {"Stable 3", 4, 4, (uint8_t*) stable_3_bitmask};


static const uint8_t stable_4_bitmask[] = {
	0, 1, 0, 0,
	1, 0, 1, 0,
	0, 1, 0, 1,
	0, 0, 1, 0
};

static struct life_obj_t stable_4_obj = {"Stable 4", 4, 4, (uint8_t*) stable_4_bitmask};



//
// API
//
struct life_obj_t* get_title_obj()
{
	return &title_obj;
}


struct life_obj_t* get_edit_obj(int index)
{
	struct life_obj_t* obj;
	
	switch (index) {
		case LIFE_OBJ_BLINKER_1:
			obj = &blinker_1_obj;
			break;
		case LIFE_OBJ_BLINKER_2:
			obj = &blinker_2_obj;
			break;
		case LIFE_OBJ_BLINKER_3:
			obj = &blinker_3_obj;
			break;
		case LIFE_OBJ_EDEN:
			obj = &eden_obj;
			break;
		case LIFE_OBJ_GLIDER_R_U:
			obj = &glider_r_u_obj;
			break;
		case LIFE_OBJ_GLIDER_L_U:
			obj = &glider_l_u_obj;
			break;
		case LIFE_OBJ_GLIDER_R_D:
			obj = &glider_r_d_obj;
			break;
		case LIFE_OBJ_GLIDER_L_D:
			obj = &glider_l_d_obj;
			break;
		case LIFE_OBJ_GLIDER_GUN:
			obj = &glider_gun_obj;
			break;
		case LIFE_OBJ_SPACESHIP_R:
			obj = &spaceship_r_obj;
			break;
		case LIFE_OBJ_SPACESHIP_L:
			obj = &spaceship_l_obj;
			break;
		case LIFE_OBJ_STABLE_1:
			obj = &stable_1_obj;
			break;
		case LIFE_OBJ_STABLE_2:
			obj = &stable_2_obj;
			break;
		case LIFE_OBJ_STABLE_3:
			obj = &stable_3_obj;
			break;
		case LIFE_OBJ_STABLE_4:
			obj = &stable_4_obj;
			break;
		default:
			obj = 0;
	}
	
	return obj;
}

