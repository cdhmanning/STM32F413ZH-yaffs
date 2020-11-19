/*
 * test_vertify.c
 *
 *  Created on: 19/11/2020
 *      Author: timothy
 */


#include "tests/test_vertify.h"
#include "shared.h"
#include "command.h"

int test_vertify_setup(void) {
	//make sure the test file does not exist.
	command_run("rm " MOUNTPOINT_PATH "/testfile1");
	return 0;
};

int test_vertify_run(void) {
	logger_increase_indent_level(1);

	int ret = command_run("create-verify-file " MOUNTPOINT_PATH "/testfile1");
	ASSERT_EQ_INT(ret, 0, "error: could not create a verification file");

	ret = command_run("verify-file " MOUNTPOINT_PATH "/testfile1");
	ASSERT_EQ_INT(ret, 0, "error: could not verify file");

	logger_increase_indent_level(-1);
	return 0;
};

int test_vertify_teardown(void) {
	return 0;
};


Test test_vertify = {"test_vertify", test_vertify_setup, test_vertify_run, test_vertify_teardown};
