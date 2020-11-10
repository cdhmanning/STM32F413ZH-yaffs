/*
 * logger.h
 *
 *  Created on: 29/10/2020
 *      Author: timothy
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#define logger_print(...) {for (int i=0; i < logger_get_indent_level(); i ++) {\
	printf("\t");\
}\
	printf( __VA_ARGS__);}\

void logger_increase_indent_level(int inc);
int logger_get_indent_level();

#endif /* INC_LOGGER_H_ */
