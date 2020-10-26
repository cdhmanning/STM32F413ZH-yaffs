/*
 * yaffs_spi_nand.h
 *
 *  Created on: 30/07/2020
 *      Author: charles
 */

#ifndef __YAFFS_SPI_NAND_H__
#define __YAFFS_SPI_NAND_H__

#include <stdint.h>

int yaffs_spi_nand_load_driver(const char *name,
							   uint32_t start_block,
							   uint32_t end_block);

#endif
