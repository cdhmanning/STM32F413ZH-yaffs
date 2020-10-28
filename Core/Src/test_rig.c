#include "test_rig.h"
#include "tests/lists_of_all_tests.h"
#include <stdio.h>
#include "logger.h"

int test_rig_run_all(void){
	logger_increase_indent_level(1);
	Test *current = test_list[0];
	logger_print("running test_rig_run_all\n");
	while (current != 0) {
		int ret = 0;
		logger_print("current is: %p\n", current);
		logger_print("running test %s\n", current->name);
		ret = current->setup();
		if (ret != 0) {
			logger_print("setup for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-1);
			return -1;
		} else {
			logger_print("setup returned %d\n", ret);
		}

		ret = current->run();
		if (ret != 0) {
			logger_print("ran test '%s', returned %d\n",current->name, ret);
			logger_increase_indent_level(-1);
			return -1;
		} else {
			logger_print("run returned %d\n", ret);
		}

		ret = current->teardown();
		if (ret != 0) {
			logger_print("teardown for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-1);
			return -1;
		} else {
			logger_print("teardown returned %d\n", ret);
		}

		logger_print("curr before\n");
		current++;
		logger_print("curr after\n");

	}
	logger_print("all finished\n");
	logger_increase_indent_level(-1);

	return 0;
}
