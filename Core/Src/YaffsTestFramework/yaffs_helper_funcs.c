/*
 * These are the Yaffs porting functions used adapt Yaffs to the RTOS or
 * bare metal.
 */

#include "yaffs_spi_nand.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "spi_nand.h"
#include "my_malloc.h"


#include <stdio.h>

void yaffs_sizes(void)
{
	printf("sizeof(struct yaffs_dev)=%d\n", sizeof(struct yaffs_dev));
	printf("sizeof(struct yaffs_obj)=%d\n", sizeof(struct yaffs_obj));
	printf("sizeof(struct yaffs_block_info)=%d\n", sizeof(struct yaffs_block_info));

}

void yaffsfs_SetError(int err)
{
	errno = err;
}
void yaffs_bug_fn(const char *file_name, int line_no)
{
	printf("Yaffs bug at %s, line %d\n", file_name, line_no);
}

void yaffsfs_Lock(void)
{

}
void yaffsfs_Unlock(void)
{

}

int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
	(void) write_request;

	if (!addr)
		return -1;
	return 0;
}


void *yaffsfs_malloc(size_t size)
{
	return my_malloc(size);
}

void yaffsfs_free(void *ptr)
{
		my_free(ptr);
}

u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

