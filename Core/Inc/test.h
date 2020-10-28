/*
 * test.h
 *
 *  Created on: 28/10/2020
 *      Author: Timothy
 */

#ifndef __TEST_H__
#define __TEST_H__


#define ASSERT_EQ_INT(var1, var2, message) if (var1 != var2) {\
	printf("assert failed: " #var1 " is not equal to " #var2 "\n\t values are %d and %d\n",var1, var2);\
	printf("\t%s\n",message);\
	return -1;\
}

typedef struct {
	char *name;
	int (*setup)(void);
	int (*run)(void);
	int (*teardown)(void);
} Test;

#endif
