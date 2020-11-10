/*
 * test_rm.c
 *
 *  Created on: 29/10/2020
 *      Author: timothy
 */

#include "test.h"
#include "command.h"
#include "logger.h"

int test_rm_setup(void) {
	int ret = command_run("setup-env");
	return ret;
};

int test_rm_run(void) {
	logger_increase_indent_level(1);
	logger_print("entering test_rm_run function\n");
	int ret = command_run("rm /test/non_existent_file");
	ASSERT_EQ_INT(ret, -1, "error: removed a non-existing file");

	logger_increase_indent_level(-1);
	return 0;
};

int test_rm_teardown(void) {
	return 0;
};


Test test_rm = {"test_rm", test_rm_setup, test_rm_run, test_rm_teardown};


