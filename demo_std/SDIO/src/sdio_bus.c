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

static const struct sdio_device_id *sdio_match_one(struct sdio_func *func,
	const struct sdio_device_id *id)
{
	if (id->class != ( u8)SDIO_ANY_ID && id->class != func->class)
		return NULL;
	if (id->vendor != (u16)SDIO_ANY_ID && id->vendor != func->vendor)
		return NULL;
	if (id->device != (u16)SDIO_ANY_ID && id->device != func->device)
		return NULL;
	return id;
}

const struct sdio_device_id if_sdio_ids[3]={//wo support marvell8385 and marvell 8686
	SDIO_ANY_ID,SDIO_VENDOR_ID_MARVELL,SDIO_DEVICE_ID_MARVELL_LIBERTAS,//marvell 8385
	SDIO_ANY_ID,SDIO_VENDOR_ID_MARVELL,SDIO_DEVICE_ID_MARVELL_8688WLAN,//marvell 8686
	0,0,0
};
static struct sdio_device_id *sdio_match_device(struct sdio_func *func)
{
	struct sdio_device_id *ids=(struct sdio_device_id *)if_sdio_ids;
	pr_debug("function vendor=0x%x	device=0x%x\n",func->vendor,func->device);
	if (ids) {
		while (ids->class || ids->vendor || ids->device) {
			if (sdio_match_one(func, ids))
				return ids;
			ids++;
		}
	}
	return NULL;
}



/*******************************************
*函数：sdio_bus_probe
*描述：设备 总线匹配
*********************************************/
struct lbs_private *sdio_bus_probe(struct sdio_func *func)
{
	struct sdio_device_id *id;
	int ret;

	id = sdio_match_device(func);
	if (!id){
		marvell_error("Cann't find support modules!\n");
		return (struct lbs_private *)NULL;
	}
	/* Set the default block size so the driver is sure it's something
	 * sensible. */
	ret = sdio_set_block_size(func, 0);
	if (ret)
		return (struct lbs_private *)NULL;
	return if_sdio_probe(func,id);
}


