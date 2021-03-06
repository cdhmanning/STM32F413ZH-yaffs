#include "test.h"
#include "stdio.h"
#include <stddef.h>
#include "command.h"
#include "shared.h"

int test_mount_setup(void) {
	return 0;
};

int test_mount_run(void) {

	int ret = command_run("mount nonexistingmountpoint 0 200");
	ASSERT_EQ_INT(ret, -1, "error: mounted a non-existing mountpoint");

	ret = command_run("mount " MOUNTPOINT_PATH " 0 200");
	ASSERT_EQ_INT(ret, 0, "error: could not mount an existing mountpoint");

	return 0;
};

int test_mount_teardown(void) {
	return 0;
};


Test test_mount = {"test_mount", test_mount_setup, test_mount_run, test_mount_teardown};


