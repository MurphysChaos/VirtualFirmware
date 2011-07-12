// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "disk.h" // struct ata_s
#include "biosvar.h" // GET_GLOBAL
#include "cmos.h" // inb_cmos
#include "util.h" // dprintf
#include "ata.h" // process_ata_op
#include "usb-msc.h" // process_usb_op
#include "virtio-blk.h" // process_virtio_op

struct drives_s Drives VAR16VISIBLE;

struct drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    if (extdriveoffset >= ARRAY_SIZE(Drives.idmap[0]))
        return NULL;
    struct drive_s *drive_gf = GET_GLOBAL(Drives.idmap[exttype][extdriveoffset]);
    if (!drive_gf)
        return NULL;
    return GLOBALFLAT2GLOBAL(drive_gf);
}


/****************************************************************
 * Disk geometry translation
 ****************************************************************/

static u8
get_translation(struct drive_s *drive_g)
{
    u8 type = GET_GLOBAL(drive_g->type);
    if (! CONFIG_COREBOOT && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 ataid = GET_GLOBAL(drive_g->cntl_id);
        u8 channel = ataid / 2;
        u8 translation = inb_cmos(CMOS_BIOS_DISKTRANSFLAG + channel/2);
        translation >>= 2 * (ataid % 4);
        translation &= 0x03;
        return translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = GET_GLOBAL(drive_g->pchs.heads);
    u16 cylinders = GET_GLOBAL(drive_g->pchs.cylinders);
    u16 spt = GET_GLOBAL(drive_g->pchs.spt);
    u64 sectors = GET_GLOBAL(drive_g->sectors);
    u64 psectors = (u64)heads * cylinders * spt;
    if (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        return TRANSLATION_LBA;

    if (cylinders <= 1024 && heads <= 16 && spt <= 63)
        return TRANSLATION_NONE;
    if (cylinders * heads <= 131072)
        return TRANSLATION_LARGE;
    return TRANSLATION_LBA;
}

void
setup_translation(struct drive_s *drive_g)
{
    u8 translation = get_translation(drive_g);
    SET_GLOBAL(drive_g->translation, translation);

    u16 heads = GET_GLOBAL(drive_g->pchs.heads);
    u16 cylinders = GET_GLOBAL(drive_g->pchs.cylinders);
    u16 spt = GET_GLOBAL(drive_g->pchs.spt);
    u64 sectors = GET_GLOBAL(drive_g->sectors);
    const char *desc = NULL;

    switch (translation) {
    default:
    case TRANSLATION_NONE:
        desc = "none";
        break;
    case TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        if (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            break;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        if (heads>128)
            heads = 255;
        else if (heads>64)
            heads = 128;
        else if (heads>32)
            heads = 64;
        else if (heads>16)
            heads = 32;
        else
            heads = 16;
        cylinders = sect / heads;
        break;
    case TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        if (heads==16) {
            if (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    case TRANSLATION_LARGE:
        if (translation == TRANSLATION_LARGE)
            desc = "large";
        while (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            if (heads > 127)
                break;
        }
        break;
    }
    // clip to 1024 cylinders in lchs
    if (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%d\n"
            , drive_g
            , drive_g->pchs.cylinders, drive_g->pchs.heads, drive_g->pchs.spt
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    SET_GLOBAL(drive_g->lchs.heads, heads);
    SET_GLOBAL(drive_g->lchs.cylinders, cylinders);
    SET_GLOBAL(drive_g->lchs.spt, spt);
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
static void
fill_fdpt(struct drive_s *drive_g, int hdid)
{
    if (hdid > 1)
        return;

    u16 nlc   = GET_GLOBAL(drive_g->lchs.cylinders);
    u16 nlh   = GET_GLOBAL(drive_g->lchs.heads);
    u16 nlspt = GET_GLOBAL(drive_g->lchs.spt);

    u16 npc   = GET_GLOBAL(drive_g->pchs.cylinders);
    u16 nph   = GET_GLOBAL(drive_g->pchs.heads);
    u16 npspt = GET_GLOBAL(drive_g->pchs.spt);

    struct fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nlspt;

    if (nlc != npc || nlh != nph || nlspt != npspt) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = npspt;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, sizeof(*fdpt));
    }

    if (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 struct extended_bios_data_area_s, fdpt[0])));
    else
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 struct extended_bios_data_area_s, fdpt[1])));
}

// Map a drive (that was registered via add_bcv_hd)
void
map_hd_drive(struct drive_s *drive_g)
{
    // fill hdidmap
    u8 hdcount = GET_BDA(hdcount);
    if (hdcount >= ARRAY_SIZE(Drives.idmap[0])) {
        warn_noalloc();
        return;
    }
    dprintf(3, "Mapping hd drive %p to %d\n", drive_g, hdcount);
    Drives.idmap[EXTTYPE_HD][hdcount] = drive_g;
    SET_BDA(hdcount, hdcount + 1);

    // Fill "fdpt" structure.
    fill_fdpt(drive_g, hdcount);
}

// Find spot to add a drive
static void
add_ordered_drive(struct drive_s **idmap, u8 *count, struct drive_s *drive_g)
{
    if (*count >= ARRAY_SIZE(Drives.idmap[0])) {
        warn_noalloc();
        return;
    }
    struct drive_s **pos = &idmap[*count];
    *count = *count + 1;
    if (CONFIG_THREADS) {
        // Add to idmap with assured drive order.
        struct drive_s **end = pos;
        for (;;) {
            struct drive_s **prev = pos - 1;
            if (prev < idmap)
                break;
            struct drive_s *prevdrive = *prev;
            if (prevdrive->type < drive_g->type
                || (prevdrive->type == drive_g->type
                    && prevdrive->cntl_id < drive_g->cntl_id))
                break;
            pos--;
        }
        if (pos != end)
            memmove(pos+1, pos, (void*)end-(void*)pos);
    }
    *pos = drive_g;
}

// Map a cd
void
map_cd_drive(struct drive_s *drive_g)
{
    dprintf(3, "Mapping cd drive %p\n", drive_g);
    add_ordered_drive(Drives.idmap[EXTTYPE_CD], &Drives.cdcount, drive_g);
}

// Map a floppy
void
map_floppy_drive(struct drive_s *drive_g)
{
    // fill idmap
    dprintf(3, "Mapping floppy drive %p\n", drive_g);
    add_ordered_drive(Drives.idmap[EXTTYPE_FLOPPY], &Drives.floppycount
                      , drive_g);

    // Update equipment word bits for floppy
    if (Drives.floppycount == 1) {
        // 1 drive, ready for boot
        SETBITS_BDA(equipment_list_flags, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } else if (Drives.floppycount >= 2) {
        // 2 drives, ready for boot
        SETBITS_BDA(equipment_list_flags, 0x41);
        SET_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * 16bit calling interface
 ****************************************************************/

// Execute a disk_op request.
int
process_op(struct disk_op_s *op)
{
    ASSERT16();
    u8 type = GET_GLOBAL(op->drive_g->type);
    switch (type) {
    case DTYPE_FLOPPY:
        return process_floppy_op(op);
    case DTYPE_ATA:
        return process_ata_op(op);
    case DTYPE_ATAPI:
        return process_atapi_op(op);
    case DTYPE_RAMDISK:
        return process_ramdisk_op(op);
    case DTYPE_CDEMU:
        return process_cdemu_op(op);
    case DTYPE_USB:
        return process_usb_op(op);
    case DTYPE_VIRTIO:
	return process_virtio_op(op);
    default:
        op->count = 0;
        return DISK_RET_EPARAM;
    }
}

// Execute a "disk_op_s" request - this runs on a stack in the ebda.
static int
__send_disk_op(struct disk_op_s *op_far, u16 op_seg)
{
    struct disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop
               , op_seg, op_far
               , sizeof(dop));

    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , dop.drive_g, (u32)dop.lba, dop.buf_fl
            , dop.count, dop.command);

    int status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->count, dop.count);

    return status;
}

// Execute a "disk_op_s" request by jumping to a stack in the ebda.
int
send_disk_op(struct disk_op_s *op)
{
    ASSERT16();
    if (! CONFIG_DRIVES)
        return -1;

    return stack_hop((u32)op, GET_SEG(SS), __send_disk_op);
}


/****************************************************************
 * Setup
 ****************************************************************/

void
drive_setup(void)
{
    memset(&Drives, 0, sizeof(Drives));
}
