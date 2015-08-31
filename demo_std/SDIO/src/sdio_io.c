#include "type.h"
#include "common.h"
#include "string.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sd.h"
#include "sdio.h"
#include "sdio_func.h"
#include "sdio_ids.h"
#include "if_sdio.h"

/**
 *	sdio_enable_func - enables a SDIO function for usage
 *	@func: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register
 *	access is possible.
 */
int sdio_enable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;
	unsigned long timeout;

	//BUG_ON(!func);
	//BUG_ON(!func->card);

	pr_debug("SDIO: Enabling device %s...\n", sdio_func_id(func));

	ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg |= 1 << func->num;

	ret = mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	timeout =func->enable_timeout;

	while (1) {
		ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IORx, 0, &reg);
		if (ret)
			goto err;
		if (reg & (1 << func->num))
			break;

		ms_delay();
		timeout--;
		ret = -ETIME;
		if (timeout==0)
			goto err;
	}

	pr_debug("SDIO: Enabled device %s\n", sdio_func_id(func));

	return 0;

err:
	pr_debug("SDIO: Failed to enable device %s\n", sdio_func_id(func));
	return ret;
}







/**
 *	sdio_set_block_size - set the block size of an SDIO function
 *	@func: SDIO function to change
 *	@blksz: new block size or 0 to use the default.
 *
 *	The default block size is the largest supported by both the function
 *	and the host, with a maximum of 512 to ensure that arbitrarily sized
 *	data transfer use the optimal (least) number of commands.
 *
 *	A driver may call this to override the default block size set by the
 *	core. This can be used to set a block size greater than the maximum
 *	that reported by the card; it is the driver's responsibility to ensure
 *	it uses a value that the card supports.
 *
 *	Returns 0 on success, -EINVAL if the host does not support the
 *	requested block size, or -EIO (etc.) if one of the resultant FBR block
 *	size register writes failed.
 *
 */
int sdio_set_block_size(struct sdio_func *func, unsigned blksz)
{
	int ret;

	if (blksz > func->card->host->max_blk_size)
		return -EINVAL;

	if (blksz == 0) {
		blksz = min(func->max_blksize, func->card->host->max_blk_size);
		blksz = min(blksz, 512u);
	}

	ret = mmc_io_rw_direct(func->card, 1, 0,
		SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE,
		blksz & 0xff, NULL);
	if (ret)
		return ret;
	ret = mmc_io_rw_direct(func->card, 1, 0,
		SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE + 1,
		(blksz >> 8) & 0xff, NULL);
	if (ret)
		return ret;
	func->cur_blksize = blksz;
	return 0;
}

 
#if 0 //这个是发送任意长块数据的函数，上面是针对stm32修改后的数据结果
/* Split an arbitrarily sized data transfer into several
 * IO_RW_EXTENDED commands. */
static int sdio_io_rw_ext_helper(struct sdio_func *func, int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remainder = size;
	unsigned max_blocks;
	unsigned blocks;
	int ret;

	/* Do the bulk of the transfer using block mode (if supported). */
	//这里做点相应的改动，让设备无条件执行以设备对齐的方式写入数据
	//if (func->card->cccr.multi_block && (size > sdio_max_byte_size(func))) {
	if (func->card->cccr.multi_block ) {
		/* Blocks per command is limited by host count, host transfer
		 * size (we only use a single sg entry) and the maximum for
		 * IO_RW_EXTENDED of 511 blocks. */
		max_blocks = min(func->card->host->max_blk_count,
			func->card->host->max_seg_size / func->cur_blksize);
		max_blocks = min(max_blocks, 511);

		//while (remainder > func->cur_blksize) {
		while(remainder){
			blocks =(remainder< func->cur_blksize)?1:(remainder / func->cur_blksize);
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func->cur_blksize;

			ret = mmc_io_rw_extended(func->card, write,
				func->num, addr, incr_addr, buf,
				blocks, func->cur_blksize);
			if (ret)
				return ret;

			remainder -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}

	/* Write the remainder using byte mode. */
	while (remainder > 0) {
	#ifdef MASK_DEBUG
		size = min(remainder, sdio_max_byte_size(func));

		ret = mmc_io_rw_extended(func->card, write, func->num, addr,
			 incr_addr, buf, 1, size);
		if (ret)
			return ret;

		remainder -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	#endif
		printk("some remainder data can't send!\n");
		try_bug(0);
	}
	return 0;
}
#endif

 


/**
 *	sdio_readb - read a single byte from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xff
 *	is returned and @err_ret will contain the error code.
 */
u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	//BUG_ON(!func);

	if (err_ret)
		*err_ret = 0;

	ret = mmc_io_rw_direct(func->card, 0, func->num, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
		return 0xFF;
	}

	return val;
}

/**
 *	sdio_writeb - write a single byte to a SDIO function
 *	@func: SDIO function to access
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
void sdio_writeb(struct sdio_func *func, u8 b, unsigned int addr, int *err_ret)
{
	int ret;

	//BUG_ON(!func);
	ret = mmc_io_rw_direct(func->card, 1, func->num, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

