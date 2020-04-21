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
#ifndef LIFE_H
#define LIFE_H

#include <stdint.h>
#include <stdbool.h>

//
// Constants
//

//
// API
//
bool life_init(int w, int h);
void life_clear();
void life_step();

void life_set_cell(int x, int y, bool val);
bool life_get_cell(int x, int y);
bool life_cell_changed(int x, int y, bool* val);
int life_get_gen_count();

#endif /* LIFE_H */