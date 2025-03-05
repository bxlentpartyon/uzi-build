/**************************************************
UZI (Unix Z80 Implementation) Kernel:  filesys.c
***************************************************/

#include <string.h>

#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <extras.h>
#include <filesys.h>
#include <machdep.h>
#include <process.h>
#include <scall.h>


char *bread();

/* Filename is given a path name, and returns a pointer
to the final component of it. */

char *filename(char *path)
{
    register char *ptr;

    ptr = path;
    while (*ptr)
	++ptr;
    while (*ptr != '/' && ptr-- > path)
	;
    return (ptr+1);
}

/* Newfile is given a pointer to a directory and a name, and
   creates an entry in the directory for the name, dereferences
   the parent, and returns a pointer to the new inode.
   It allocates an inode number,
   and creates a new entry in the inode table for the new file,
   and initializes the inode table entry for the new file.  The new file
   will have one reference, and 0 links to it.
   Better make sure there isn't already an entry with the same name. */

inoptr newfile(inoptr pino, char *name)
{

    register inoptr nindex;
    register int j;
    inoptr i_open();

    ifnot (nindex = i_open(pino->c_dev, 0))
	goto nogood;

    nindex->c_node.i_mode_lo = 0;   /* For the time being */
    nindex->c_node.i_mode_hi = F_REG;   /* For the time being */
    nindex->c_node.i_nlink = 1;
    nindex->c_node.i_size.o_offset = 0;
    nindex->c_node.i_size.o_blkno = 0;
    for (j=0; j <20; j++)
	nindex->c_node.i_addr[j] = 0;
    wr_inode(nindex);

    ifnot (ch_link(pino,"",filename(name),nindex))
    {
	i_deref(nindex);
	goto nogood;
    }

    i_deref (pino);
    return(nindex);

nogood:
    i_deref (pino);
    return (NULLINODE);
}

/* Oft_alloc and oft_deref allocate and dereference (and possibly free)
entries in the open file table. */

int oft_alloc(void)
{
    register int j;

    for (j=0; j < OFTSIZE ; ++j)
    {
	ifnot (of_tab[j].o_refs)
	{
	    of_tab[j].o_refs = 1;
	    of_tab[j].o_inode = NULLINODE;
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}

void oft_deref(register int of)
{
    register struct oft *ofptr;

    ofptr = of_tab + of;

    if (!(--ofptr->o_refs) && ofptr->o_inode)
    {
	i_deref(ofptr->o_inode);
	ofptr->o_inode = NULLINODE;
    }
}



/* Uf_alloc finds an unused slot in the user file table. */

int uf_alloc(void)
{
    register int j;

    for (j=0; j < UFTSIZE ; ++j)
    {
	if (udata.u_files[j] & 0x80)  /* Portable, unlike  == -1 */
	{
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}

/* isdevice(ino) returns true if ino points to a device */
int isdevice(inoptr ino)
{
    return (ino->c_node.i_mode_hi & F_CDEV);
}


/* This returns the device number of an inode representing a device */
devnum(inoptr ino)
{
    return (*(ino->c_node.i_addr));
}

/* This returns the inode pointer associated with a user's
file descriptor, checking for valid data structures */

inoptr getinode(int uindex)
{
    register int oftindex;
    register inoptr inoindex;

    if (uindex < 0 || uindex >= UFTSIZE || udata.u_files[uindex] & 0x80 )
    {
	udata.u_error = EBADF;
	return (NULLINODE);
    }

    if ((oftindex = udata.u_files[uindex]) < 0 || oftindex >= OFTSIZE)
	panic("Getinode: bad desc table");

    if ((inoindex = of_tab[oftindex].o_inode) < i_tab ||
	        inoindex >= i_tab+ITABSIZE)
	panic("Getinode: bad OFT");

    magic(inoindex);

    return(inoindex);
}
