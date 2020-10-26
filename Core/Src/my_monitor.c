#include "my_monitor.h"

#include "yaffsfs.h"
#include "yaffs_spi_nand.h"
#include "yaffs_guts.h"

#include <stdio.h>
#include <string.h>

static int dump_directory_tree_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
		return -1;
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			snprintf(str,sizeof(str), "%s/%s",dname,de->d_name);

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

	return 0;
}

static int dump_directory_tree(const char *dname)
{
	int ret;

	ret = dump_directory_tree_worker(dname,1);
	if (!ret) {
		printf("\n");
		printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
	}

	return ret;
}

static int recursive_rm_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
		return -1;
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			snprintf(str,sizeof(str), "%s/%s",dname,de->d_name);

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

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive) {
				recursive_rm_worker(str,1);
				yaffs_rmdir(str);
			} else {
				yaffs_unlink(str);
			}
		}

		yaffs_closedir(d);
	}

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

	if (argc < 4)
		return -1;

	start_block = atoi(argv[2]);
	end_block = atoi(argv[3]);

	printf("Creating mount point %s start %d end %d\n",
				argv[1], start_block, end_block);

	if (end_block - start_block < 5) {
		printf("That looks too small\n");
		return -1;
	}
	ret = yaffs_spi_nand_load_driver(argv[1], start_block, end_block);

	if (ret == YAFFS_OK)
		ret = 0;
	else
		ret = -1;

	printf("Creating mount point %s returned %d\n", argv[1], ret);

	return ret;
}

static int cmd_mount(int argc, char *argv[])
{
	int ret;

	if (argc < 2)
		return -1;

	ret = yaffs_mount(argv[1]);

	printf("Mounting %s returned %d\n", argv[1], ret);

	return ret;
}

static int cmd_mkdir(int argc, char *argv[])
{
	int ret;

	if (argc < 2)
		return -1;

	ret = yaffs_mkdir(argv[1], 0666);

	printf("yaffs_mkdir %s returned %d\n", argv[1], ret);

	return ret;
}

static int cmd_rm(int argc, char *argv[])
{
	int ret;
	int recursive = 0;
	char *name;
	struct yaffs_stat s;

	if (argc < 2)
		return -1;

	recursive = (0 == strcmp(argv[1], "-r"));

	if (recursive && argv < 3)
		return -1;

	if (recursive) {
		name = argv[2];
		ret = recursive_rm(argv[2]);
		printf("recursive rm %s returned %d\n", name, ret);
	}

	name = argv[1];
	ret = yaffs_stat(name, &s);

	if (s.st_mode & S_IFMT == S_IFDIR)
		ret = yaffs_rmdir(name);
	else
		ret = yaffs_unlink(name);
	printf("rm %s returned %d\n", name, ret);

	return ret;
}

static int cmd_umount(int argc, char *argv[])
{
	int ret;

	if (argc < 2)
		return -1;

	ret = yaffs_unmount(argv[1]);

	printf("Unmounting %s returned %d\n", argv[1], ret);

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
	{}
};

static int cmd_help(int argc, char *argv[])
{
	struct cmd_def *cmd = cmd_list;

	while(cmd->name) {
		printf("%s\n", cmd->help_str);
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

			printf("%u msec\n", end_time - start_time);

			return ret;
		}

		cmd++;
	}
	printf("Command not found\n");
	return -1;
}

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
	char *argv[10];
	int argc;
	int ret;

	printf("Yaffs test monitor\n");

	while(1) {
		print_prompt();
		input = accept_input();
		//printf("Got input \"%s\"\n", input);
		argc = 0;
		memset(argv, 0, sizeof(argv));

		while(*input && argc < 10) {
			while (*input == ' ' || *input == '\t') {
				*input = 0;
				input++;
			}
			if (*input) {
				argv[argc] = input;
				argc++;
			}
			while (*input && !(*input == ' ' || *input == '\t')) {
				input++;
			}
		}

		//for(i = 0; i < argc; i++)
		//	printf("\"%s\"\n", argv[i]);

		ret = my_monitor_process(argc, argv);
		printf("returned %d\n", ret);
	}
}
