#include "test_rig.h"
#include "tests/lists_of_all_tests.h"
#include <stdio.h>
#include "logger.h"

int test_rig_run_all(void){

	Test *current = test_list[0];
	logger_print("running test_rig_run_all\n");
	logger_increase_indent_level(1);
	while (current != 0) {
		int ret = 0;
		logger_print("\n");
		logger_print("running test %s\n", current->name);
		logger_increase_indent_level(1);

		logger_print("calling test setup\n");
		ret = current->setup();
		if (ret != 0) {
			logger_print("setup for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
			logger_print("setup returned %d\n", ret);
		}
		logger_print("calling test run \n");
		ret = current->run();
		if (ret != 0) {
			logger_print("ran test '%s', returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
			logger_print("run returned %d\n", ret);
		}

		logger_print("calling test teardown\n");
		ret = current->teardown();
		if (ret != 0) {
			logger_print("teardown for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
			logger_print("teardown returned %d\n", ret);
		}

		logger_print("curr before\n");
		current++;
		logger_print("curr after\n");
		logger_increase_indent_level(-1);

	}
	logger_print("all finished\n");
	logger_increase_indent_level(-1);

	return 0;
}
