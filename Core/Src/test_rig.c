#include "test_rig.h"
#include "tests/lists_of_all_tests.h"

int test_rig_run_all(void){
	Test *current = test_list[0];
	printf("running test_rig_run_all\n");
	while(current) {
		int ret = 0;

		ret = current->setup();
		if (ret != 0) {
			printf("setup for test '%s' failed, returned %d\n",current->name, ret);
			return -1;
		}

		ret = current->run();
		if (ret != 0) {
			printf("ran test '%s', returned %d\n",current->name, ret);
				return -1;
		}

		ret = current->teardown();
		if (ret != 0) {
			printf("teardown for test '%s' failed, returned %d\n",current->name, ret);
			return -1;
		}


		current++;
		break;
	}
}
