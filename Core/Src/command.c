/*
 * command.c
 *
 *  Created on: 28/10/2020
 *      Author: timothy
 */


#include "my_monitor.h"

#include "yaffsfs.h"
#include "yaffs_spi_nand.h"
#include "yaffs_guts.h"
#include "test_rig.h"
#include "logger.h"
#include "shared.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int command_run(char *input_cmd);

static int dump_directory_tree_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		logger_print("opendir failed\n");
		return -1;
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			snprintf(str,sizeof(str), "%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			logger_print("%s inode %d length %d mode %X ",
				str, s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: logger_print("data file"); break;
				case S_IFDIR: logger_print("directory"); break;
				case S_IFLNK: logger_print("symlink -->");
							  if (yaffs_readlink(str,str,100) < 0) {
								logger_print("no alias");
							  } else {
								logger_print("\"%s\"",str);
							  }
							  break;
				default: logger_print("unknown"); break;
			}

			logger_print("\n");

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive)
				dump_directory_tree_worker(str,1);

		}

		yaffs_closedir(d);
	}

	return 0;
}

static void generate_pattern(char * path, int length, char *outputString) {
	//stat the file.
	//combine the inode and the path to create a seed for the random generator
	//write n chars to the output string.
	//The output string needs to already be malloced.

	//set string to nulls first

	//for now copy a dummy pattern.
	for (int i = 0; i < length; i ++) {
		outputString[i] = i % 255;
	}

}

#define FILE_LENGTH 10
//size needs to be 1mb.
//have to partially read the data in chunks due to limited stack memory on the stm32

static int vertify_existing_file( char *path) {
	//create the actual file.

	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fileHandle = yaffs_open(path, O_RDWR, mode) ;

	if (fileHandle == -1){
		logger_print("\n");
		logger_print("vertify_existing_file could not open file %s\n", path);
		return -1;
	}
	logger_print("opened file %s\n", path);
	char pattern_string[FILE_LENGTH];
	char read_pattern[FILE_LENGTH];

	//set memory to 0;
	generate_pattern(path, FILE_LENGTH, pattern_string);
	//copy pattern to file.
	int ret = yaffs_read(fileHandle, read_pattern, FILE_LENGTH);
	logger_print("read %d chars\n",ret);
	yaffs_close(fileHandle);

	if (!ret) {
		logger_print("\n");
		logger_print("vertify_existing_file could not read pattern from %s\n",path);
		return -1;
	}

	//memcmp
	ret = strncmp(pattern_string, read_pattern, FILE_LENGTH);
	if (!ret) {
		logger_print("\n");
		logger_print("vertify_existing_file: read pattern is different than expected pattern for file %s\n",path);
		logger_print("pattern read from file is:\n\t");
		for (int i=0; i< FILE_LENGTH; i++){
			logger_print("%d ",read_pattern[i]);
		}
		logger_print("\n");
		logger_print("expected pattern is:\n\t");
		for (int i=0; i< FILE_LENGTH; i++){
			logger_print("%d ",pattern_string[i]);
		}
		logger_print("\n");


		return -1;
	}
	return 0;

}

static int create_vertify_file( char *path) {
	//create the actual file.

	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fileHandle = yaffs_open(path, O_RDWR| O_CREAT | O_TRUNC, mode) ;

	if (fileHandle == -1){
		logger_print("\n");
		logger_print("create_vertify_file could not open file %s\n", path);
		return -1;
	}

	logger_print("opened file %s\n", path);

	char pattern_string[FILE_LENGTH];
	//set memory to 0;
	generate_pattern(path, FILE_LENGTH, pattern_string);
	//copy pattern to file.
	int ret = yaffs_write(fileHandle, pattern_string, FILE_LENGTH);

	char read_string[FILE_LENGTH];
	//quickly read it
	yaffs_lseek(fileHandle, 0, SEEK_SET); //reset the cursor to the start of the file.
	yaffs_read(fileHandle, read_string, FILE_LENGTH);
	logger_print("pattern written to file is:\n\t");
	for (int i=0; i< FILE_LENGTH; i++){
		logger_print("%d ",pattern_string[i]);
	}
	logger_print("\n");
	logger_print("pattern read from file is:\n\t");
	for (int i=0; i< FILE_LENGTH; i++){
		logger_print("%d ",read_string[i]);
	}
	logger_print("\n");

	yaffs_close(fileHandle);

	if (!ret) {
		logger_print("\n");
		logger_print("create_vertify_file could not write pattern to %s\n",path);
		return -1;
	}
	return 0;

}

static int dump_directory_tree(const char *dname)
{
	int ret;

	ret = dump_directory_tree_worker(dname,1);
	if (!ret) {
		logger_print("\n");
		logger_print("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
	}

	return ret;
}

static int recursive_rm_worker(const char *dname,int recursive)
{
	logger_increase_indent_level(1);

	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		logger_print("opendir failed\n");
		logger_increase_indent_level(-1);

		return -1;
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			snprintf(str,sizeof(str), "%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			logger_print("%s inode %d length %d mode %X ",
				str, s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: logger_print("data file"); break;
				case S_IFDIR: logger_print("directory"); break;
				case S_IFLNK: logger_print("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0) {
								logger_print("no alias");
							  } else {
								logger_print("\"%s\"",str);
							  }
							  break;
				default: logger_print("unknown"); break;
			}

			logger_print("\n");

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive) {
				recursive_rm_worker(str,1);
				yaffs_rmdir(str);
			} else {
				yaffs_unlink(str);
			}
		}

		yaffs_closedir(d);
	}
	logger_increase_indent_level(-1);
	return 0;
}

static int recursive_rm(const char *dname)
{
	int ret;
	ret = recursive_rm_worker(dname,1);
	return ret;
}

static int cmd_dir(int argc, char *argv[])
{
	if (argc < 2)
		return -1;

	return dump_directory_tree(argv[1]);
}

static int cmd_mntcreate(int argc, char *argv[])
{
	int ret;
	int start_block;
	int end_block;

	logger_increase_indent_level(1);


	if (argc < 4){
		logger_print("mntcreate needs more arguments\n");
		logger_increase_indent_level(-1);

		return -1;
	}

	start_block = atoi(argv[2]);
	end_block = atoi(argv[3]);

	logger_print("Creating mount point %s start %d end %d\n",
				argv[1], start_block, end_block);

	if (end_block - start_block < 5) {
		logger_print("That looks too small\n");
		logger_increase_indent_level(-1);

		return -1;
	}
	ret = yaffs_spi_nand_load_driver(argv[1], start_block, end_block);

	if (ret == YAFFS_OK)
		ret = 0;
	else
		ret = -1;

	logger_print("Creating mount point %s returned %d\n", argv[1], ret);

	logger_increase_indent_level(-1);
	return ret;
}

static int cmd_mount(int argc, char *argv[])
{
	int ret;
	logger_increase_indent_level(1);

	if (argc < 2) {
		logger_print("Mount needs more args\n");
		logger_increase_indent_level(-1);

		return -1;
	}

	ret = yaffs_mount(argv[1]);

	logger_print("Mounting %s returned %d\n", argv[1], ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_mkdir(int argc, char *argv[])
{
	int ret;
	logger_increase_indent_level(1);

	if (argc < 2) {
		logger_increase_indent_level(-1);

		return -1;
	}

	ret = yaffs_mkdir(argv[1], 0666);

	logger_print("yaffs_mkdir %s returned %d\n", argv[1], ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_rm(int argc, char *argv[])
{
	int ret;
	int recursive = 0;
	char *name;
	struct yaffs_stat s;
	logger_increase_indent_level(1);
	logger_print("running cmd_rm\n");

	if (argc < 2){
		logger_print("rm needs more arguments.\n");
		logger_increase_indent_level(-1);

		return -1;
	}
	logger_print("rm running\n");
	recursive = (0 == strcmp(argv[1], "-r"));

	if (recursive && argc < 3) {
		logger_print("rm needs more arguments.\n");
		logger_increase_indent_level(-1);

		return -1;
	}

	if (recursive) {
		logger_print("running recursive rm\n");
		name = argv[2];
		ret = recursive_rm(argv[2]);
		logger_print("recursive rm %s returned %d\n", name, ret);
		logger_increase_indent_level(-1);

		return ret;
	}

	name = argv[1];
	logger_print("about to run yaffs_stat\n");
	ret = yaffs_stat(name, &s);

	logger_print("yaffs_stat returned %d\n", ret);

	if ((s.st_mode & S_IFMT) == S_IFDIR)
		ret = yaffs_rmdir(name);
	else
		ret = yaffs_unlink(name);
	logger_print("rm %s returned %d\n", name, ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_setup_env(int argc, char *argv[]) {
	logger_increase_indent_level(1);
	int ret =-1;

	logger_print("running setup test environment\n");

	ret = command_run(MOUNTPOINT_CREATE_COMMAND);
	ret =  ret && command_run("mount "MOUNTPOINT_PATH);

	logger_increase_indent_level(-1);
	return ret;

}

static int cmd_test_all(int argc, char *argv[]){
	logger_increase_indent_level(1);

	logger_print("running cmd_test_all\n");
	int ret = test_rig_run_all();
	logger_increase_indent_level(-1);
	return ret;
}

static int cmd_umount(int argc, char *argv[])
{
	int ret;
	logger_increase_indent_level(-1);

	if (argc < 2) {
		logger_increase_indent_level(-1);

		return -1;
	}
	ret = yaffs_unmount(argv[1]);

	logger_print("Unmounting %s returned %d\n", argv[1], ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_verify_file(int argc, char *argv[])
{
	int ret;
	logger_increase_indent_level(-1);

	if (argc < 2) {
		logger_increase_indent_level(-1);
		logger_print("Verify-file needs more arguments\n");
		return -1;
	}
	ret = vertify_existing_file(argv[1]);

	logger_print("Verify-file %s returned %d\n", argv[1], ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_create_verify_file(int argc, char *argv[])
{
	int ret;
	logger_increase_indent_level(-1);

	if (argc < 2) {
		logger_increase_indent_level(-1);
		logger_print("create-verify-file needs more arguments\n");
		return -1;
	}
	ret = create_vertify_file(argv[1]);

	logger_print("create-verify-file %s returned %d\n", argv[1], ret);
	logger_increase_indent_level(-1);

	return ret;
}

static int cmd_open(int argc, char *argv[]) {

	logger_increase_indent_level(1);

	if (argc < 2) {
		logger_print("Open needs more arguments.\n");
		logger_increase_indent_level(-1);
		return -1;
	}
	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fileHandle = yaffs_open(argv[1], O_RDWR| O_CREAT, mode) ;

	logger_print("Open %s returned the file handle %d\n", argv[1], fileHandle);
	logger_increase_indent_level(-1);

	int ret =-1;
	if (fileHandle >=0) {
		ret = 1;
	}
	return ret;
}

struct cmd_def {
	char *name;
	int (*fn)(int argc, char *argv[]);
	char *help_str;
};

#define CMD_DEF(name, fn, help_str) {name, fn, help_str}

static int cmd_help(int argc, char *argv[]);

struct cmd_def cmd_list[] = {

	CMD_DEF("help", cmd_help, 			"help\t\tGet help for commands"),
	CMD_DEF("mntcreate", cmd_mntcreate, "mntcreate name start_block end_block\tCreate mount point for name, start block end block"),
	CMD_DEF("mount", cmd_mount, 		"mount name\t\tMount specified yaffs device"),
	CMD_DEF("umount", cmd_umount, 		"umount name\t\tUnmount specified yaffs device"),
	CMD_DEF("dir", cmd_dir, 			"dir name\t\tDump directory tree for specified directory"),
	CMD_DEF("mkdir", cmd_mkdir, 		"mkdir name\t\tCreate specified directory"),
	CMD_DEF("rm", cmd_rm, 				"rm [-r] name\t\tDelete obj [-r] for recursive"),
	CMD_DEF("setup-env", cmd_setup_env, "setup-env\t\tCreates a test mountpoint with files and dirs and mounts it."),
	CMD_DEF("test-all", cmd_test_all, "test-all\t\tRuns all test scripts."),
	CMD_DEF("open", cmd_open, "open path [oflag] [mode]\t\t runs yaffs_open, defaults are mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) and oflag =( O_RDWR| O_CREAT) "),
	CMD_DEF("verify-file", cmd_verify_file, "verify-file name\t\tverify an existing file."),
	CMD_DEF("create-verify-file", cmd_create_verify_file, "cmd_create-verify_file name\t\tcreates a file with a known pattern that can be verified with verify-file command."),

	{}
};

static int cmd_help(int argc, char *argv[])
{
	struct cmd_def *cmd = cmd_list;

	while(cmd->name) {
		logger_print("%s\n", cmd->help_str);
		cmd++;
	}

	return 0;
}

static int my_monitor_process(int argc, char *argv[])
{
	struct cmd_def *cmd = cmd_list;
	uint32_t start_time;
	uint32_t end_time;
	int ret;

	if (argc < 1)
		return 0;

	while(cmd->name) {
		if (strcmp(cmd->name, argv[0]) == 0) {
			start_time = HAL_GetTick();
			ret =  cmd->fn(argc, argv);
			end_time = HAL_GetTick();

			logger_print("command %s ran and returned %d in %u msec\n", cmd-> name, ret, end_time - start_time);

			return ret;
		}

		cmd++;
	}
	logger_print("Command '%s' not found\n", argv[0]);
	return -1;
}


int command_run(char *input_cmd) {

	char *argv[10];
	int argc;
	int ret;
	argc = 0;
	memset(argv, 0, sizeof(argv));

	//copy the string because we are modifying it.
	//there were issues with this being used with string literals, which cannot be modified.
	int string_length = strlen(input_cmd);
	char *cmd = (char *) malloc(sizeof(char)*(string_length+1));
	strcpy(cmd, input_cmd);

	char *cmd_iter = cmd;

	//parse the string into args
	while(*cmd_iter && argc < 10) {

		//replace all whitespaces and tabs with null until we hit not tab or null char.
		//this partitions the string into several substrings
		while (*cmd_iter == ' ' || *cmd_iter == '\t') {
			*cmd_iter = 0;
			cmd_iter++;
		}
		//if the char is not null.
		if (*cmd_iter) {
			//just copy the entire string?
			argv[argc] = cmd_iter;
			argc++;
		}
		//move pointer to next whitespace, tab, or null.
		while (*cmd_iter && !(*cmd_iter == ' ' || *cmd_iter == '\t')) {
			cmd_iter++;
		}
	}

	ret = my_monitor_process(argc, argv);

	free(cmd);
	return ret;
}

