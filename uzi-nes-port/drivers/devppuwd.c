#include <extern.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>

#include <lib/string.h>

int ppuwd_open(int minor)
{
	return (0);
}

#define NUM_BANKS		1024
#define BANK_SIZE		4096
#define BANK_UPPER_SHIFT	8
#define BANK_UPPER_MASK		0b11
#define BANK_LOWER_MASK		0xff
#define BLOCKS_PER_BANK		(BANK_SIZE / BLOCK_SIZE)
#define DISK_BANK_START		0x1000
#define	CHUNK_SIZE		PPU_MAX_READ

#define blk_bank(blkno)			(blkno / BLOCKS_PER_BANK)
#define blk_base_addr(blkno)		(DISK_BANK_START + ((blkno % BLOCKS_PER_BANK) * BLOCK_SIZE))
#define blk_chunk_addr(blkno, chunk)	(blk_base_addr(blkno) + (chunk * CHUNK_SIZE))
#define	bank_upper_bits(ppu_bank)	((ppu_bank >> BANK_UPPER_SHIFT) & BANK_UPPER_MASK)
#define bank_lower_byte(ppu_bank)	(ppu_bank & BANK_LOWER_MASK)

#define READ_PASSES	(BLOCK_SIZE / PPU_MAX_READ)

int ppuwd_read(int minor, int rawflag)
{
	struct ppu_desc desc;
	char pass;
	int bank = blk_bank(udata.u_buf->bf_blk);
	char data = bank_lower_byte(bank);

	desc.size = CHUNK_SIZE;
	desc.flags = bank_upper_bits(bank) | PPU_DESC_FLAG_READ;

	for (pass = 0; pass < READ_PASSES; pass++) {
		desc.target = (char *) blk_chunk_addr(udata.u_buf->bf_blk, pass);
		queue_descriptor(&desc, &data);

		while (!ppu_readbuf_dirty);

		bcopy(&ppu_readbuf,
		      (char * ) &udata.u_buf->bf_data + CHUNK_SIZE * pass,
		      CHUNK_SIZE);

		ppu_readbuf_dirty = 0;
	}

	return 0;
}
