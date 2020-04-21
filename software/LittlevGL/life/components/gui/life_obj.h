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
#ifndef LIFE_OBJ_H
#define LIFE_OBJ_H

#include <stdint.h>
#include <stdbool.h>

//
// Constants
//

// Object indicies for edit items added to the drop-down menu
#define LIFE_OBJ_BLINKER_1   0
#define LIFE_OBJ_BLINKER_2   1
#define LIFE_OBJ_BLINKER_3   2
#define LIFE_OBJ_EDEN        3
#define LIFE_OBJ_GLIDER_R_U  4
#define LIFE_OBJ_GLIDER_L_U  5
#define LIFE_OBJ_GLIDER_R_D  6
#define LIFE_OBJ_GLIDER_L_D  7
#define LIFE_OBJ_GLIDER_GUN  8
#define LIFE_OBJ_SPACESHIP_R 9
#define LIFE_OBJ_SPACESHIP_L 10
#define LIFE_OBJ_STABLE_1    11
#define LIFE_OBJ_STABLE_2    12
#define LIFE_OBJ_STABLE_3    13
#define LIFE_OBJ_STABLE_4    14

#define LIFE_OBJ_MENU_NUM    15

// Object with parameters
struct life_obj_t
{
	char* name;
	int w;
	int h;
	uint8_t* array;
};



//
// API
//
struct life_obj_t* get_title_obj();
struct life_obj_t* get_edit_obj(int index);


#endif /* LIFE_OBJ_H */