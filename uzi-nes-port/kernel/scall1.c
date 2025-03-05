#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <scall.h>
#include <unix.h>

#include <lib/string.h>

#pragma code-name (push, "SCALL1_CODE")

int doclose(int16 uindex)
{
	return 0;
}

/****************************************
sync()
***************************************/
void _sync(void)
{
    register j;
    register inoptr ino;
    register char *buf;
    char *bread();

    /* Write out modified inodes */

    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if ((ino->c_refs) > 0 && ino->c_dirty != 0)
	{
	    wr_inode(ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread(j, 1, 1);
	    bcopy((char *)&fs_tab[j], buf, 512);
	    bfree(buf, 2);
	}
    }

    bufsync();   /* Clear buffer pool */
}

#pragma code-name (pop)
