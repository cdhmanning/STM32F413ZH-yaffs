/*
 * test_open.c
 *
 *  Created on: 30/10/2020
 *      Author: timothy
 */


#include "test.h"
#include "stdio.h"
#include <stddef.h>
#include "command.h"
#include "shared.h"

int test_open_setup(void) {
	return 0;
};

int test_open_run(void) {

	int ret = command_run("open badmountpoint/new_file");
	ASSERT_EQ_INT(ret, -1, "error: opened or created a file with a bad path");

	ret = command_run("open " MOUNTPOINT_PATH"/new_file");
	ASSERT_EQ_INT(ret, 1, "error: could not open a new file");

	return 0;
};

int test_open_teardown(void) {
	return 0;
};


Test test_open = {"test_open", test_open_setup, test_open_run, test_open_teardown};

