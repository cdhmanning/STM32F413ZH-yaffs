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

static char *accept_input(void)
{
	static char str[100];
	uint32_t n = 0;
	int ch;

	memset(str, 0, sizeof(str));

	while(1) {
		ch = getchar();

		//printf("0x%02x\n", ch);

		if (ch == '\r') {
			printf("\n");
			break;
		}
		else if (ch == '\b') {
			if(n > 0) {
				n--;
				str[n] = 0;
				printf("\b \b");
			}
		} else if (n < sizeof(str) - 1) {
			str[n] = ch;
			n++;
			printf("%c",ch);
		}
	}

	return str;
}

void my_monitor(void)
{
	char *input;


	printf("Yaffs test monitor\n\n");

	command_run("test-all");
	printf("\n");
	command_run("setup-env");
	printf("\n");

	while(1) {
		print_prompt();
		input = accept_input();
		//printf("Got input \"%s\"\n", input);
		command_run(input);
	}
}
