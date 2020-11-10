/*
 * logger.c
 *
 *  Created on: 29/10/2020
 *      Author: timothy
 */
#include "logger.h"

int indent_level = 0;

void logger_increase_indent_level(int inc){
	indent_level += inc;
}

int logger_get_indent_level(){
	return indent_level;
}
