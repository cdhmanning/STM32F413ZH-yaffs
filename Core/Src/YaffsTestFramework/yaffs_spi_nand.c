/*
 * These are the Yaffs Driver functions to set up a Yaffs Device.
 * This mostly calls the underlying SPI NAND functions.
 */
#include "yaffs_spi_nand.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "spi_nand.h"
#include "my_malloc.h"


#include <stdio.h>

#define PAGE_TAGS_OFFSET	0x820

/*
 * This is a function to do the flash writes from Yaffs.
 */
static int yaffs_spi_nand_write_chunk (struct yaffs_dev *dev, int nand_chunk,
			   const u8 *data, int data_len,
			   const u8 *oob, int oob_len)
{
	int ret;
	struct spi_nand_buffer_op op[2];
	int n_ops = 0;

	if (data && data_len) {
		op[n_ops].offset = 0;
		op[n_ops].buffer = (uint8_t *)data;
		op[n_ops].nbytes = data_len;
		n_ops++;
	}
	/* If there are tags, then store them at the offset in the spare area. */
	if (oob && oob_len) {
		op[n_ops].offset = PAGE_TAGS_OFFSET;
		op[n_ops].buffer = (uint8_t *)oob;
		op[n_ops].nbytes = oob_len;
		n_ops++;
	}

	ret =  spi_nand_write_page(nand_chunk, op, n_ops, NULL);

	if (ret < 0)
		return YAFFS_FAIL;

	return YAFFS_OK;
}


/*
 * This is the function to do the flash reads from Yaffs.
 */
static int yaffs_spi_nand_read_chunk (struct yaffs_dev *dev, int nand_chunk,
			   u8 *data, int data_len,
			   u8 *oob, int oob_len,
			   enum yaffs_ecc_result *ecc_result)
{
	int ret;
	struct spi_nand_buffer_op op[2];
	int n_ops = 0;
	uint8_t status;

	if (data && data_len) {
		op[n_ops].offset = 0;
		op[n_ops].buffer = (uint8_t *)data;
		op[n_ops].nbytes = data_len;
		n_ops++;
	}
	/*
	 * If there are tags read them from the desired offset in the spare area.
	 */
	if (oob && oob_len) {
		op[n_ops].offset = PAGE_TAGS_OFFSET;
		op[n_ops].buffer = (uint8_t *)oob;
		op[n_ops].nbytes = oob_len;
		n_ops++;
	}

	ret =  spi_nand_read_page(nand_chunk, op, n_ops, &status);

	/* Handle ECC errors. */
	if (ecc_result) {
		uint8_t ecc_status = (status >>4) & 7; /* Just the ECC status bits. */

		if (ecc_status == 0 || ecc_status == 1)
			*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
		else if (ecc_status == 2)
			*ecc_result = YAFFS_ECC_RESULT_UNFIXED;
		else
			*ecc_result = YAFFS_ECC_RESULT_FIXED;
	}
	if (ret < 0)
		return YAFFS_FAIL;

	return YAFFS_OK;
}

static int yaffs_spi_nand_erase_block (struct yaffs_dev *dev, int block_no)
{
	int ret;
	uint8_t status;

	ret = spi_nand_erase_block(block_no, &status);

	if (ret < 0 || (status & 0x04))
		return YAFFS_FAIL;

	return YAFFS_OK;
}


static int yaffs_spi_nand_mark_bad_block(struct yaffs_dev *dev, int block_no)
{
	int ret = 0;
	uint8_t status;

	(void) status;

	ret = spi_nand_mark_block_bad(block_no, &status);

	if (ret < 0)
		return YAFFS_FAIL;

	return YAFFS_OK;
}

static int yaffs_spi_nand_check_bad_block (struct yaffs_dev *dev, int block_no)
{
	int ret;
	uint32_t is_ok;

	ret = spi_nand_check_block_ok(block_no, &is_ok);

	if (ret == 0 && is_ok)
		return YAFFS_OK;
	return YAFFS_FAIL;
}

static int yaffs_spi_nand_initialise (struct yaffs_dev *dev)
{
	int ret;

	ret = spi_nand_init();
	if (ret < 0)
		return YAFFS_FAIL;
	return YAFFS_OK;
}

int yaffs_spi_nand_deinitialise (struct yaffs_dev *dev)\
{
	return YAFFS_OK;
}

/* Main yaffs driver loading function.
 * This sets up the Yaffs device then registers with
 * Yaffs.
 *
 * The name parameter is used by yaffs as the root name for the
 * directory for this device.
 *
 * For example if the name is "flashroot", then the files in this device
 * will be called:
 * /flashroot/xx/yy
 * /flashroot/file
 * ...
 */
int yaffs_spi_nand_load_driver(const char *name,
							   uint32_t start_block,
							   uint32_t end_block)
{
	struct yaffs_dev *dev;
	char *name_copy = strdup(name);
	struct yaffs_param *param;
	struct yaffs_driver *drv;

	dev = malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	if(!name_copy) {
		free(name_copy);
		return YAFFS_FAIL;
	}

	/* Set up parmeters */
	param = &dev->param;
	drv = &dev->drv;

	param->name = name_copy;

	param->total_bytes_per_chunk = 2048;
	param->chunks_per_block = 64;
	param->spare_bytes_per_chunk = 32;
	param->no_tags_ecc = 1;
	param->n_reserved_blocks = 5;
	param->start_block = start_block;
	param->end_block = end_block;
	param->use_nand_ecc = 1;
	param->is_yaffs2 = 1;
	param->inband_tags = 0;

	param->n_caches = 5;
	param->disable_soft_del = 1;

	/* Set up driver function. */
	drv->drv_write_chunk_fn = yaffs_spi_nand_write_chunk;
	drv->drv_read_chunk_fn = yaffs_spi_nand_read_chunk;
	drv->drv_erase_fn = yaffs_spi_nand_erase_block;
	drv->drv_mark_bad_fn = yaffs_spi_nand_mark_bad_block;
	drv->drv_check_bad_fn = yaffs_spi_nand_check_bad_block;
	drv->drv_initialise_fn = yaffs_spi_nand_initialise;
	drv->drv_deinitialise_fn = yaffs_spi_nand_deinitialise;

	dev->driver_context = (void *) 1;	// Used to identify the device in fstat.

	/* Register device under the specified name */
	yaffs_add_device(dev);

	return YAFFS_OK;
}

