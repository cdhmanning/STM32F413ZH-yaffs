#include "test_rig.h"
#include "tests/lists_of_all_tests.h"
#include <stdio.h>
#include "logger.h"
#include "shared.h"
#include "command.h"
#include "yaffsfs.h"
#include "yaffs_spi_nand.h"



u32 yaffs_trace_mask= 0;

static void dump_directory_tree_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			printf("%s inode %d length %d mode %X ",
				str, s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);
							  break;
				default: printf("unknown"); break;
			}

			printf("\n");

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive)
				dump_directory_tree_worker(str,1);

		}

		yaffs_closedir(d);
	}

}

/* Test code */

static void dump_directory_tree(const char *dname)
{
	dump_directory_tree_worker(dname,1);
	printf("\n");
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
}

static uint8_t local_buffer[1000];

void fill_local_buffer(void)
{
	int i;
	uint32_t *local_buffer_u32 = (uint32_t *)local_buffer;

	for (i = 0; i < 250; i++)
		local_buffer_u32[i] = i * 10;
}

void create_a_file(const char *name, int size)
{
	int i;
	int h;
	int n = size;
	int start = HAL_GetTick();
	int n_writes = 0;

	h = yaffs_open(name, O_CREAT | O_RDWR | O_TRUNC, 0666);

	while (n > 0) {
			i = n;
			if (i > sizeof(local_buffer))
				i = sizeof(local_buffer);
			yaffs_write(h, local_buffer, i);
			n -= i;
			n_writes++;
	}
	yaffs_close(h);
	printf("Writing file %s, handle %d, size %d took %d writes and %d milliseconds\n",
			name, h, size, n_writes, HAL_GetTick() - start);
}

void read_a_file(const char *name)
{
	int h;
	int size;
	int start = HAL_GetTick();
	int n_reads = 0;

	h = yaffs_open(name, O_RDONLY, 0);
	size = yaffs_lseek(h, 0, SEEK_END);
	yaffs_lseek(h, 0, SEEK_SET);

	while(yaffs_read(h, local_buffer, sizeof(local_buffer)) > 0)
		n_reads++;

	printf("Reading file %s, handle %d, size %d took %d reads and %d milliseconds\n",
			name, h, size, n_reads, HAL_GetTick() - start);
	yaffs_close(h);
}


void create_a_pattern_file(const char *name, int size)
{
	int i;
	int h;
	int n = size;
	int start = HAL_GetTick();
	int n_writes = 0;
	uint8_t ch;
	char *x;

	ch = 0;
	x = (char *)name;

	while(*x) {
		ch += *x;
		x++;
	}

	h = yaffs_open(name, O_CREAT | O_RDWR | O_TRUNC, 0666);


	while (n > 0) {
			for(i = 0; i < sizeof(local_buffer); i++) {
				local_buffer[i] = ch;
				ch++;
			}
			i = n;
			if (i > sizeof(local_buffer))
				i = sizeof(local_buffer);
			yaffs_write(h, local_buffer, i);
			n -= i;
			n_writes++;
	}
	yaffs_close(h);
	printf("Writing file %s, handle %d, size %d took %d writes and %d milliseconds\n",
			name, h, size, n_writes, HAL_GetTick() - start);
}

void check_a_pattern_file(const char *name)
{
	int h;
	int i;
	int size;
	int start = HAL_GetTick();
	int n_reads = 0;
	int read_diff = 0;
	uint8_t ch;
	char *x;

	ch = 0;
	x = (char *)name;

	while(*x) {
		ch += *x;
		x++;
	}

	h = yaffs_open(name, O_RDONLY, 0);
	size = yaffs_lseek(h, 0, SEEK_END);
	yaffs_lseek(h, 0, SEEK_SET);

	while(!read_diff && yaffs_read(h, local_buffer, sizeof(local_buffer)) > 0) {
		for(i = 0; !read_diff && i < sizeof(local_buffer); i++) {
			if (local_buffer[i] != ch) {
				printf("file %s differs at %d: %02x %02x\n",
						name, n_reads * sizeof(local_buffer) + i,
						local_buffer[i], ch);
				return;
			}
			ch++;
		}
		n_reads++;
	}

	printf("Reading file %s, handle %d, size %d took %d reads and %d milliseconds\n",
			name, h, size, n_reads, HAL_GetTick() - start);
	yaffs_close(h);
}


void yaffs_call_all_funcs(void)
{
	int h;
	uint8_t b[200];
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	int ret;
	int l;
	int start;

	(void) ret;
	ret = yaffs_spi_nand_load_driver("/m", 0, 200);

	start = HAL_GetTick();
	ret = yaffs_mount("/m");
	printf("Mounting /m returned %d, took %d msec\n", ret, HAL_GetTick() - start);

	dump_directory_tree("/m");

	fill_local_buffer();
	printf("Start writing 10 Mbytes\n");
	start = HAL_GetTick();
	create_a_pattern_file("/m/0", 1000000);
	create_a_pattern_file("/m/1", 1000000);
	create_a_pattern_file("/m/2", 1000000);
	create_a_pattern_file("/m/3", 1000000);
	create_a_pattern_file("/m/4", 1000000);
	create_a_pattern_file("/m/5", 1000000);
	create_a_pattern_file("/m/6", 1000000);
	create_a_pattern_file("/m/7", 1000000);
	create_a_pattern_file("/m/8", 1000000);
	create_a_pattern_file("/m/9", 1000000);
	printf("End writing 10 Mbytes, took %d milliseconds\n",
			HAL_GetTick() - start);

	printf("Start reading 10 Mbytes\n");
	start = HAL_GetTick();
	check_a_pattern_file("/m/0");
	check_a_pattern_file("/m/1");
	check_a_pattern_file("/m/2");
	check_a_pattern_file("/m/3");
	check_a_pattern_file("/m/4");
	check_a_pattern_file("/m/5");
	check_a_pattern_file("/m/6");
	check_a_pattern_file("/m/7");
	check_a_pattern_file("/m/8");
	check_a_pattern_file("/m/9");

	printf("End reading 10 Mbytes, took %d milliseconds\n",
			HAL_GetTick() - start);

	h = yaffs_open("/m/0", O_RDWR, 0);

	if(h >= 0) {
		l = yaffs_lseek(h, 0, SEEK_END);
		printf("File /m/a exists, size %d\n", l);
		yaffs_close(h);
	}
	h = yaffs_open("/m/a", O_CREAT | O_TRUNC | O_RDWR, 0666);
	printf("opening file /m/a returned %d\n", h);
	ret = yaffs_write(h, b, sizeof(b)-20);
	printf("writing %d bytes returned %d\n", sizeof(b)-20, ret);
	ret = yaffs_lseek(h, 0, SEEK_END);
	printf("lseek to end returned %d\n", ret);
	ret = yaffs_lseek(h, 0, 0);
	printf("lseek to 0 returned %d\n", ret);

	ret = yaffs_read(h, b, sizeof(b));
	printf("reading %d bytes returned %d\n", sizeof(b), ret);

	ret = yaffs_ftruncate(h,0);
	ret = yaffs_unlink("/m/a");

	ret = yaffs_mkdir("/m/d", 0666) ;

	d = yaffs_opendir("/m/d") ;
	de = yaffs_readdir(d) ;
	(void) de;
	yaffs_rewinddir(d) ;
	yaffs_closedir(d) ;
	ret = yaffs_rmdir("/m/d") ;

	printf(" free space %ld\n", yaffs_freespace("/m/a"));
	printf(" total space %ld\n", yaffs_totalspace("/m/a"));
}

void yaffs_test(void)
{
	printf("<<<<<<<<<<<<<<<<<<<<< Start Yaffs test\n");
	yaffs_call_all_funcs();
	printf(">>>>>>>>>>>>>>>>>>>>> End Yaffs test\n");
}


int test_rig_run_all(void){

	Test *current = 0;
	int index = 0;

	command_run(MOUNTPOINT_CREATE_COMMAND);

	logger_print("running test_rig_run_all\n");
	logger_increase_indent_level(1);
	while (test_list[index] != 0) {
		current = test_list[index];
		int ret = 0;

		logger_print("\n");
		logger_print("running test %s\n", current->name);
		logger_increase_indent_level(1);

//		logger_print("calling test setup\n");
		ret = current->setup();
		if (ret != 0) {
			logger_print("setup for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
//			logger_print("setup returned %d\n", ret);
		}
//		logger_print("calling test run \n");
		ret = current->run();
		if (ret != 0) {
			logger_print("ran test '%s', returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
//			logger_print("run returned %d\n", ret);
		}

//		logger_print("calling test teardown\n");
		ret = current->teardown();
		if (ret != 0) {
			logger_print("teardown for test '%s' failed, returned %d\n",current->name, ret);
			logger_increase_indent_level(-2);
			return -1;
		} else {
//			logger_print("teardown returned %d\n", ret);
		}

		index++;
		logger_increase_indent_level(-1);
		logger_print("test %s ran successfully\n",current->name);

	}
	logger_print("all tests have run successfully\n");
	logger_increase_indent_level(-1);

	return 0;
}
