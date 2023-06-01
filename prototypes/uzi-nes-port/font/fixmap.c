#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// This is where the ';' should be, but I put a '<'.  We have to
// subtract one from the char index to account for the missing character.
#define MISTAKE_OFFSET		'<' - 1
// 2 chars shy of a full ascii table because the last char is blank anyways,
// and we have to subtract 1 to account for the missing char again.
#define LAST_TILE_INDEX		'\x7f' - 2	// One char shy of full ascii....

#define TILE_SIZE		16
#define tile_offset(t)		(t) * TILE_SIZE
#define next_tile_offset(t)	tile_offset(t + 1)

struct nes_tile {
	unsigned char tile_bytes[TILE_SIZE];
	unsigned int tile_index;
};

void dump_tile(struct nes_tile *t)
{
	printf("Tile %d\n", t->tile_index);
	for (int i = 0; i < TILE_SIZE / 2; i++) {
		for (int b = 8; b >= 0; b--) {
			unsigned char cur_bit = 1 << b;
			if (cur_bit & t->tile_bytes[i])
				printf("1");
			else
				printf("0");
		}
		printf("\n");
	}
	printf("\n");
}



int copy_tile_foward(int chr_rom_fd, unsigned char tile_index)
{
	struct nes_tile cur_tile;
	ssize_t read_size;
	off_t seek_off = lseek(chr_rom_fd, tile_offset(tile_index), SEEK_SET);

	if (seek_off < 0) {
		printf("error: Could not lseek to %d, errno %d\n", tile_offset(tile_index), errno);
		return 1;
	}

	read_size = read(chr_rom_fd, &cur_tile, sizeof(cur_tile));
	cur_tile.tile_index = tile_index;
	dump_tile(&cur_tile);

	seek_off = lseek(chr_rom_fd, next_tile_offset(tile_index), SEEK_SET);
	if (seek_off < 0) {
		printf("error: Could not lseek to %d, errno %d\n", tile_offset(tile_index), errno);
		return 1;
	}

	write(chr_rom_fd, cur_tile.tile_bytes, TILE_SIZE);
}

int main(int argc, char **argv)
{
	int chr_rom_fd;
	int cur_tile_idx, prev_tile_idx;
	int ret;
	struct stat statbuf;
	char *file_path = argv[1];

	printf("Opening CHR-ROM file at %s\n", file_path);
	chr_rom_fd = open(file_path, O_RDWR);
	if (chr_rom_fd < 0) {
		printf("error: Could not open %s, errno %d\n", file_path, errno);
		goto out_bad;
	}

	printf("Moving tiles at offset %d ahead one slot\n", (unsigned int) MISTAKE_OFFSET);

	for (cur_tile_idx = LAST_TILE_INDEX;
	     cur_tile_idx >= MISTAKE_OFFSET;
	     cur_tile_idx--) {
		if (!copy_tile_foward(chr_rom_fd, cur_tile_idx))
			break;
	}

	return 0;
out_bad:
	return 1;
}
