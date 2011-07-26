/*
 * Extended boot option ROM support.
 *
 * Copyright IBM, Corp. 2007
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#include "hw.h"
#include "pc.h"
#include "isa.h"
#include "block.h"

/* Extended Boot ROM suport */

union extboot_cmd
{
    uint16_t type;
    struct {
	uint16_t type;
	uint16_t cylinders;
	uint16_t heads;
	uint16_t sectors;
	uint64_t nb_sectors;
    } query_geometry;
    struct {
	uint16_t type;
	uint16_t nb_sectors;
	uint16_t segment;
	uint16_t offset;
	uint64_t sector;
    } xfer;
};

static void get_translated_chs(BlockDriverState *bs, int *c, int *h, int *s)
{
    bdrv_get_geometry_hint(bs, c, h, s);

    if (*c <= 1024) {
	*c >>= 0;
	*h <<= 0;
    } else if (*c <= 2048) {
	*c >>= 1;
	*h <<= 1;
    } else if (*c <= 4096) {
	*c >>= 2;
	*h <<= 2;
    } else if (*c <= 8192) {
	*c >>= 3;
	*h <<= 3;
    } else {
	*c >>= 4;
	*h <<= 4;
    }

    /* what is the correct algorithm for this?? */
    if (*h == 256) {
	*h = 255;
	*c = *c + 1;
    }
}

static void extboot_write_cmd(void *opaque, uint32_t addr, uint32_t value)
{
    union extboot_cmd cmd;
    BlockDriverState *bs = opaque;
    int cylinders, heads, sectors, err;
    uint64_t nb_sectors;
    target_phys_addr_t pa = 0;
    int blen = 0;
    void *buf = NULL;

    cpu_physical_memory_read((value & 0xFFFF) << 4, (uint8_t *)&cmd,
                             sizeof(cmd));

    if (cmd.type == 0x01 || cmd.type == 0x02) {
	pa = cmd.xfer.segment * 16 + cmd.xfer.offset;
        blen = cmd.xfer.nb_sectors * 512;
        buf = qemu_memalign(512, blen);
    }

    switch (cmd.type) {
    case 0x00:
        get_translated_chs(bs, &cylinders, &heads, &sectors);
	bdrv_get_geometry(bs, &nb_sectors);
	cmd.query_geometry.cylinders = cylinders;
	cmd.query_geometry.heads = heads;
	cmd.query_geometry.sectors = sectors;
	cmd.query_geometry.nb_sectors = nb_sectors;
	break;
    case 0x01:
	err = bdrv_read(bs, cmd.xfer.sector, buf, cmd.xfer.nb_sectors);
	if (err)
	    printf("Read failed\n");

        cpu_physical_memory_write(pa, buf, blen);

	break;
    case 0x02:
        cpu_physical_memory_read(pa, buf, blen);

	err = bdrv_write(bs, cmd.xfer.sector, buf, cmd.xfer.nb_sectors);
	if (err)
	    printf("Write failed\n");

	break;
    }

    cpu_physical_memory_write((value & 0xFFFF) << 4, (uint8_t *)&cmd,
                              sizeof(cmd));
    if (buf)
        qemu_free(buf);
}

void extboot_init(BlockDriverState *bs)
{
    register_ioport_write(0x405, 1, 2, extboot_write_cmd, bs);
}
