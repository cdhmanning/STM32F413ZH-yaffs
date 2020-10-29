/*
 * list_of_all_tests.c
 *
 *  Created on: 28/10/2020
 *      Author: timothy
 */

#ifndef SRC_TESTS_LIST_OF_ALL_TESTS_C_
#define SRC_TESTS_LIST_OF_ALL_TESTS_C_

#include "tests/lists_of_all_tests.h"

#include "tests/test_rm.h"
#include "tests/test_mount.h"

Test *tests[]={
		&test_mount,
		&test_rm,
		0//Terminator
};

Test **test_list = tests;

#endif /* SRC_TESTS_LIST_OF_ALL_TESTS_C_ */
