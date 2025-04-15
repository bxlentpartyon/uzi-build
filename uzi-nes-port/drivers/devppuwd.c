#include <extern.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>

#include <lib/string.h>

int ppuwd_open(int minor)
{
	return (0);
}

/* General PPU bank descriptive data */
#define NUM_BANKS		1024
#define BANK_SIZE		4096
#define BANK_UPPER_SHIFT	8
#define BANK_UPPER_MASK		0b11
#define BANK_LOWER_MASK		0xff
#define BLOCKS_PER_BANK		(BANK_SIZE / BLOCK_SIZE)
#define DISK_BANK_START		0x1000
#define BANK_BYTE_PAD		1

/* R/W specific data */
#define READ_CHUNK_SIZE		PPU_MAX_READ
#define WRITE_CHUNK_SIZE	PPU_MAX_WRITE
#define WRITE_CHUNK_BUF_SIZE	WRITE_CHUNK_SIZE + BANK_BYTE_PAD
#define READ_PASSES		(BLOCK_SIZE / PPU_MAX_READ)
#define WRITE_PASSES		(BLOCK_SIZE / PPU_MAX_WRITE)

/* General block location macros */
#define blk_bank(blkno)			(blkno / BLOCKS_PER_BANK)
#define blk_base_addr(blkno)		(DISK_BANK_START + ((blkno % BLOCKS_PER_BANK) * BLOCK_SIZE))
#define	bank_upper_bits(ppu_bank)	((ppu_bank >> BANK_UPPER_SHIFT) & BANK_UPPER_MASK)
#define bank_lower_byte(ppu_bank)	(ppu_bank & BANK_LOWER_MASK)

/* R/W specific macros */
#define blk_chunk_addr_rd(blkno, chunk)	(blk_base_addr(blkno) + (chunk * READ_CHUNK_SIZE))
#define blk_chunk_addr_wr(blkno, chunk)	(blk_base_addr(blkno) + (chunk * WRITE_CHUNK_SIZE))

struct ppuwd_bank_desc
{
	char lower_byte;
	char upper_bits;
};

void populate_bank_desc(blkno_t blk, struct ppuwd_bank_desc *bankdesc)
{
	int bank = blk_bank(udata.u_buf->bf_blk);
	bankdesc->lower_byte = bank_lower_byte(bank);
	bankdesc->upper_bits = bank_upper_bits(bank);
}

int ppuwd_read(int minor, int rawflag)
{
	struct ppu_desc ppudesc;
	struct ppuwd_bank_desc bankdesc;
	char pass;
	blkno_t blk = udata.u_buf->bf_blk + minor;

	populate_bank_desc(blk, &bankdesc);

	ppudesc.size = READ_CHUNK_SIZE;
	ppudesc.flags = bankdesc.upper_bits | PPU_DESC_FLAG_READ;

	for (pass = 0; pass < READ_PASSES; pass++) {
		ppudesc.target = (char *) blk_chunk_addr_rd(blk, pass);
		queue_descriptor(&ppudesc, &bankdesc.lower_byte);

		while (!ppu_readbuf_dirty);

		bcopy(ppu_readbuf,
		      (char * ) &udata.u_buf->bf_data + READ_CHUNK_SIZE * pass,
		      READ_CHUNK_SIZE);

		ppu_readbuf_dirty = 0;
	}

	return 0;
}

extern void wait_frame(void);

int ppuwd_write(int minor, int rawflag)
{
	struct ppu_desc ppudesc;
	struct ppuwd_bank_desc bankdesc;
	char pass;
	char write_chunk[WRITE_CHUNK_BUF_SIZE] = { 0 };
	blkno_t blk = udata.u_buf->bf_blk + minor;

	populate_bank_desc(blk, &bankdesc);

	ppudesc.size = WRITE_CHUNK_SIZE;
	ppudesc.flags = bankdesc.upper_bits | PPU_DESC_FLAG_WRITE;
	write_chunk[0] = bankdesc.lower_byte;

	for (pass = 0; pass < WRITE_PASSES; pass++) {
		bcopy((char * ) &udata.u_buf->bf_data + WRITE_CHUNK_SIZE * pass,
		      write_chunk + BANK_BYTE_PAD,
		      WRITE_CHUNK_SIZE);

		ppudesc.target = (char *) blk_chunk_addr_wr(blk, pass);
		queue_descriptor(&ppudesc, write_chunk);
		wait_frame();
	}
}
