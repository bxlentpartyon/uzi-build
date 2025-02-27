#include <stdio.h>
#include <strings.h>

#include <unix.h>
#include "linux_fs.h"

int yes(void)
{
    char line[20];
    /* int  fgets(); - HP */

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}

inoptr root;
int16 ROOTDEV;

struct u_data udata;
struct oft of_tab[OFTSIZE];
struct filesys fs_tab[1];

    inoptr srch_dir();
    inoptr srch_mt();
inoptr n_open(char *name, inoptr *parent)
{
    inoptr wd;  /* the directory we are currently searching. */
    inoptr ninode;
    inoptr temp;

    if (*name == '/')
	wd = root;
    else
	wd = udata.u_cwd;

    i_ref(ninode = wd);
    i_ref(ninode);

    for(;;)
    {
	if (ninode)
	    magic(ninode);

	/* See if we are at a mount point */
	if (ninode)
	    ninode = srch_mt(ninode);

	while (*name == '/')    /* Skip (possibly repeated) slashes */
	    ++name;
	ifnot (*name)           /* No more components of path? */
	    break;
	ifnot (ninode)
	{
	    udata.u_error = UZI_ENOENT;
	    goto nodir;
	}
	i_deref(wd);
	wd = ninode;
	if (getmode(wd) != F_DIR)
	{
	    udata.u_error = UZI_ENOTDIR;
	    goto nodir;
	}
	ifnot (getperm(wd) & OTH_EX)
	{
	    udata.u_error = UZI_EPERM;
	    goto nodir;
	}

	/* See if we are going up through a mount point */
	if ( wd->c_num == ROOTINODE && wd->c_dev != ROOTDEV && name[1] == '.')
	{
	   temp = fs_tab[wd->c_dev].s_mntpt;
	   ++temp->c_refs;
	   i_deref(wd);
	   wd = temp;
	}

	ninode = srch_dir(wd,name);

	while (*name != '/' && *name )
	    ++name;
    }

    if (parent)
	*parent = wd;
    else
	i_deref(wd);
    ifnot (parent || ninode)
	udata.u_error = UZI_ENOENT;
    return (ninode);

nodir:
    if (parent)
	*parent = NULLINODE;
    i_deref(wd);
    return(NULLINODE);

}

int oft_alloc(void)
{
	int j;

	for (j = 0; j < OFTSIZE; ++j) {
		ifnot (of_tab[j].o_refs) {
			of_tab[j].o_refs = 1;
			of_tab[j].o_inode = NULLINODE;
			return (j);
		}
	}
	udata.u_error = UZI_ENFILE;
	return(-1);
}

int uf_alloc(void)
{
	int j;

	for (j = 0; j < UFTSIZE; ++j) {
		if (udata.u_files[j] & 0x80) { /* Portable, unlike  == -1 */
			return (j);
		}

	}
	udata.u_error = UZI_ENFILE;
	return(-1);
}

void i_ref(inoptr ino)
{
    if (++(ino->c_refs) == 2*ITABSIZE)  /* Arbitrary limit. */
	panic("too many i-refs");
}

void i_deref(register inoptr ino)
{
    magic(ino);

    ifnot (ino->c_refs)
	panic("inode freed.");

    if ((ino->c_node.i_mode & F_MASK) == F_PIPE)
	wakeup((char *)ino);

    /* If the inode has no links and no refs, it must have
    its blocks freed. */

    ifnot (--ino->c_refs || ino->c_node.i_nlink)
	    f_trunc(ino);

    /* If the inode was modified, we must write it to disk. */
    if (!(ino->c_refs) && ino->c_dirty)
    {
	ifnot (ino->c_node.i_nlink)
	{
	    ino->c_node.i_mode = 0;
	    ino->c_node.i_mode = 0;
	    i_free(ino->c_dev, ino->c_num);
	}
	wr_inode(ino);
    }
}

int uzifs_open(char *path, short flag)
{
	short uindex;
	short oftindex;
	inoptr ino;
	short perm;

	if (flag < 0 || flag > 2) {
		udata.u_error = UZI_EINVAL;
		return (-1);
	}

	if ((uindex = uf_alloc()) == -1)
		return (-1);

	if ((oftindex = oft_alloc()) == -1)
		goto nooft;

	ifnot (ino = n_open(path ,NULLINOPTR))
		goto cantopen;

	of_tab[oftindex].o_inode = ino;

	perm = getperm(ino);
	if (((flag == UZI_O_RDONLY || flag == UZI_O_RDWR) && !(perm & OTH_RD)) ||
	    ((flag == UZI_O_WRONLY || flag == UZI_O_RDWR) && !(perm & OTH_WR))) {
		udata.u_error = UZI_EPERM;
		goto cantopen;
	}

	if (getmode(ino) == F_DIR &&
	    (flag == UZI_O_WRONLY || flag == UZI_O_RDWR)) {
		udata.u_error = UZI_EISDIR;
		goto cantopen;
	}

	if (isdevice(ino) && d_open((int)ino->c_node.i_addr[0]) != 0) {
		udata.u_error = UZI_ENXIO;
		goto cantopen;
	}

	udata.u_files[uindex] = oftindex;

	of_tab[oftindex].o_ptr.o_offset = 0;
	of_tab[oftindex].o_ptr.o_blkno = 0;
	of_tab[oftindex].o_access = flag;

	return (uindex);

cantopen:
	oft_deref(oftindex);  /* This will call i_deref() */
nooft:
	udata.u_files[uindex] = -1;
	return (-1);
}
