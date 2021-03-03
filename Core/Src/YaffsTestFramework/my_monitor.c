/*
 * Note this is a very basic monitor. It handles backspace, but it gets broken by things like up arrow.
 */

#include "my_monitor.h"

#include "yaffsfs.h"
#include "yaffs_spi_nand.h"
#include "yaffs_guts.h"
#include "test_rig.h"

#include <stdio.h>
#include <string.h>

#include "command.h"

static void print_prompt(void)
{
	printf("YM>");
}


static void accept_input(char *str, int length)
{
	uint32_t n = 0;
	int ch;

	memset(str, 0, length);

	while(1) {
		ch = getchar();

		//printf("0x%02x\n", ch);

		if (ch == '\r') {
			printf("\n");
			break;
		}
		else if (ch == '\b') {
			// handle backspace
			if(n > 0) {
				n--;
				str[n] = 0;
				printf("\b \b");
			}
		} else if (n < length - 1) {
			str[n] = ch;
			n++;
			printf("%c",ch);
		}


#if 0
		int i;
		for(i = 0; i < n; i++)
			printf("[%02x]", str[i]);
		printf("\n");
#endif

	}
}

void my_monitor(void)
{
	char str[100];

	printf("Yaffs test monitor\n\n");

	//command_run("test-all");
	//printf("\n");
	//command_run("setup-env");
	//printf("\n");

	while(1) {
		print_prompt();
		accept_input(str, 100);
		//printf("Got input \"%s\"\n", input);
		command_run(str);
	}
}
