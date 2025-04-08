/**************************************************
UZI (Unix Z80 Implementation) Kernel:  unix.h
***************************************************/

#ifndef __UNIX_H__
#define __UNIX_H__

#include <config.h>

#ifdef UZI
#define UZI_TYPE(x)	x
#define UZI_TYPE_T(x)	x##_t
#else
#define UZI_TYPE(x)	uzi_##x
#define UZI_TYPE_T(x)	uzi_##x##_t
#endif

#define UFTSIZE 10    /* Number of user files */
#define OFTSIZE 15    /* Open file table size */
#define ITABSIZE 20   /* Inode table size */
#define PTABSIZE 20   /* Process table size */

#define NSIGS 16        /* Number of signals <= 16 */

#define ROOTINODE 1  /* Inode # of /  for all mounted filesystems. */

#define TICKSPERSEC 10  /*Ticks per second */
#define MAXTICKS   10  /* Max ticks before swapping out (time slice) */

#define ARGBLK 0        /* Block number on SWAPDEV for arguments */
#define ENVBLK 1        /* Block number on SWAPDEV for environment */

/* This must be kept in sync with the PROGBASE defition
   in include/asm/uzi_nes.inc, or execve will break */
#define PROGBASE ((char *)(0x6000)) /* Hard coded to PRG mode 3, permanent RAM for now */
#define MAXEXEC 0       /* Max no of blks of executable file */

/*
 * Note that the executable header byte is an absolute jump instruction.  This
 * is necessary so that we can always start execution at program address 0 and
 * know (hope) that we'll end up somewhere sane.
 */
#define EMAGIC 0x4c     /* Header of executable */
#define CMAGIC 24721    /* Random number for cinode c_magic */
#define SMOUNTED 12742  /* Magic number to specify mounted filesystem */
#ifdef UZI
#define NULL 0
#endif


/* These macros are simply to trick the compiler into generating
   more compact code. */

#define ifnull(e) if(e){}else
#define ifnot(e) if(e){}else
#define ifzero(e) if(e){}else



/*
 * TODO the CPM stuff is meaningless any more.  This should probably
 * use the UZI define or something.
 */
#ifdef CPM
    typedef unsigned uint16;
    typedef int int16;
#else
    typedef unsigned short uint16;
    typedef short int16;
#endif


typedef struct s_queue {
    char *q_base;   /* Pointer to data */
    char *q_head;  /* Pointer to addr of next char to read. */
    char *q_tail;  /* Pointer to where next char to insert goes. */
    int16 q_size; /* Max size of queue */
    int16 q_count;        /* How many characters presently in queue */
    int16 q_wakeup;       /* Threshold for waking up processes waiting on queue */
} queue_t;



typedef struct UZI_TYPE_T(time) {
    uint16 t_time;
    uint16 t_date;
} UZI_TYPE_T(time);


/* User's structure for times() system call */

struct tms {
	UZI_TYPE_T(time)  tms_utime;
	UZI_TYPE_T(time)  tms_stime;
	UZI_TYPE_T(time)  tms_cutime;
	UZI_TYPE_T(time)  tms_cstime;
	UZI_TYPE_T(time)  tms_etime;      /* Elapsed real time */
} ;


/* Flags for setftime() */
#define A_TIME 1
#define M_TIME 2
#define C_TIME 4


typedef struct UZI_TYPE_T(off) {
    uint16 o_blkno;  /* Block number */
    int16 o_offset;     /* Offset within block 0-511 */
} UZI_TYPE_T(off);


typedef uint16 blkno_t;  /* Can have 65536 512-byte blocks in filesystem */
#define NULLBLK ((blkno_t)-1)


typedef struct blkbuf {
    char        bf_data[512];    /* This MUST be first ! */
    char        bf_dev;
    blkno_t     bf_blk;
    char        bf_dirty;
    char        bf_busy;
    uint16      bf_time;        /* LRU time stamp */
/*    struct blkbuf *bf_next;     LRU free list pointer */
} blkbuf, *bufptr;


typedef struct dinode {
    uint16 i_mode;
    uint16 i_nlink;
    uint16 i_uid;
    uint16 i_gid;
    UZI_TYPE_T(off)	i_size;
    UZI_TYPE_T(time)	i_atime;
    UZI_TYPE_T(time)	i_mtime;
    UZI_TYPE_T(time)	i_ctime;
    blkno_t  i_addr[20];
} dinode;               /* Exactly 64 bytes long! */


struct  UZI_TYPE(stat)    /* Really only used by users */
{
	int16   st_dev;
	uint16  st_ino;
	uint16  st_mode;
	uint16  st_nlink;
	uint16  st_uid;
	uint16  st_gid;
	uint16  st_rdev;
	UZI_TYPE_T(off)		st_size;
#ifdef UZI
	UZI_TYPE_T(time)	st_atime;
	UZI_TYPE_T(time)	st_mtime;
	UZI_TYPE_T(time)	st_ctime;
#else
	UZI_TYPE_T(time)	uzi_st_atime;
	UZI_TYPE_T(time)	uzi_st_mtime;
	UZI_TYPE_T(time)	uzi_st_ctime;
#endif
};

/* Bit masks for i_mode and st_mode */

#define OTH_EX  0001
#define OTH_WR  0002
#define OTH_RD  0004
#define GRP_EX  0010
#define GRP_WR  0020
#define GRP_RD  0040
#define OWN_EX  0100
#define OWN_WR  0200
#define OWN_RD  0400

#define SAV_TXT 01000
#define SET_GID 02000
#define SET_UID 04000

#define MODE_MASK 07777

#define F_REG	0100000
#define F_DIR	 040000
#define F_PIPE	 010000
#define F_BDEV	 060000
#define F_CDEV	 020000

#define F_MASK	0170000



typedef struct cinode {
    int16    c_magic;             /* Used to check for corruption. */
    int16    c_dev;               /* Inode's device */
    uint16   c_num;           /* Inode # */
    dinode   c_node;
    char     c_refs;            /* In-core reference count */
    char     c_dirty;           /* Modified flag. */
} cinode, *inoptr;

#define NULLINODE ((inoptr)NULL)
#define NULLINOPTR ((inoptr*)NULL)


typedef struct direct {
    uint16 d_ino;
    char     d_name[14];
} direct;



typedef struct filesys {
    int16       s_mounted;
    uint16      s_isize;
    uint16      s_fsize;
    int16       s_nfree;
    blkno_t     s_free[50];
    int16       s_ninode;
    uint16      s_inode[50];
    int16       s_fmod;
    UZI_TYPE_T(time)	s_time;
    blkno_t     s_tfree;
    uint16      s_tinode;
    inoptr      s_mntpt; /* Mount point */
} filesys, *fsptr;

typedef struct oft {
    UZI_TYPE_T(off)	o_ptr;   /* File position point16er */
    inoptr      o_inode; /* Pointer into in-core inode table */
    char        o_access; /* O_RDONLY, O_WRONLY, or O_RDWR */
    char        o_refs;  /* Reference count: depends on # of active children*/
} oft;


/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_XSLEEP        4    /* Sleeping, don't wake up for signal */
#define P_PAUSE         5    /* Sleeping for pause(); can wakeup for signal */
#define P_FORKING       6    /* In process of forking; do not mess with */
#define P_WAIT          7    /* Executed a wait() */
#define P_ZOMBIE        8    /* Exited. */


#define SIGHUP  1       
#define SIGINT  2      
#define SIGQUIT 3     
#define SIGILL  4    
#define SIGTRAP 5   
#define SIGIOT  6  
#define SIGEMT  7 
#define SIGFPE  8 
#define SIGKILL 9
#define SIGBUS  10
#define SIGSEGV 11
#define SIGSYS  12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15

#define SIG_DFL         (int16 (*)())0
#define SIG_IGN         (int16 (*)())1

#define sigmask(sig)    (1<<(sig))

/* Process table entry */

typedef struct p_tab {
    char        p_status;       /* Process status */
    int16 p_pid;    /* Process ID */
    int16 p_uid;
    struct p_tab *p_pptr;    /* Process parent's table entry */
    blkno_t     p_swap;   /* Starting block of swap space */
    uint16	p_alarm;        /* Seconds until alarm goes off */
    uint16	p_exitval;      /* Exit value */
    /* Everything below here is overlaid by time info at exit */
    char       *p_wait;         /* Address of thing waited for */
    int16 p_priority;     /* Process priority */
    uint16      p_pending;      /* Pending signals */
    uint16      p_ignored;      /* Ignored signals */
} p_tab, *ptptr;

/* Per-process data (Swapped with process) */

typedef struct u_data {
    struct p_tab *u_ptab;       /* Process table pointer */
    char        u_insys;        /* True if in kernel */
    char        u_callno;       /* sys call being executed. */
    char        *u_retloc;     /* Return location from sys call */
    int16         u_retval;       /* Return value from sys call */
    int16         u_error;                /* Last error number */
    char        *u_sp;          /* Used when a process is swapped. */
    char        *u_bc;          /* Place to save user's frame pointer */
    int16         u_argn;         /* Last arg */
    int16         u_argn1;        /* This way because args on stack backwards */
    int16         u_argn2;
    int16         u_argn3;        /* args n-3, n-2, n-1, and n */

    char *      u_base;         /* Source or dest for I/O */
    uint16    u_count;        /* Amount for I/O */
    UZI_TYPE_T(off)	u_offset;       /* Place in file for I/O */
    struct blkbuf *u_buf;

    int16         u_gid;
    int16         u_euid;
    int16         u_egid;
    int16         u_mask;         /* umask: file creation mode mask */
    UZI_TYPE_T(time)	u_time;         /* Start time */
    char        u_files[UFTSIZE];       /* Process file table:
	                        contains indexes into open file table. */
    inoptr      u_cwd;          /* Index into inode table of cwd. */
    char        *u_break;        /* Top of data space */

    inoptr      u_ino;  /* Used during execve() */
    char        *u_isp;  /* Value of initial sp (argv) */

    int16         (*u_sigvec[NSIGS])();   /* Array of signal vectors */
    int16         u_cursig;       /* Signal currently being caught */
    char        u_name[8];      /* Name invoked with */
    UZI_TYPE_T(time)	u_utime;        /* Elapsed ticks in user mode */
    UZI_TYPE_T(time)	u_stime;        /* Ticks in system mode */
    UZI_TYPE_T(time)	u_cutime;       /* Total childrens ticks */
    UZI_TYPE_T(time)	u_cstime;

} u_data;


/* Struct to temporarily hold arguments in execve */
struct s_argblk {
    int16 a_argc;
    int16 a_arglen;
    int16 a_envc;
    char a_buf[512-3*sizeof(int16)];
};


/* The device driver switch table */

struct devsw {
    int16 minor;          /* The minor device number (an argument to below) */
    int16 (*dev_open)(int16 minor);  /* The routines for reading, etc */
    int16 (*dev_close)(int16 minor); /* format: op(minor,blkno,offset,count,buf); */
    int16 (*dev_read)(int16 minor, int16 rawflag);  /* offset would be ignored for block devices */
    int16 (*dev_write)(int16 minor, int16 rawflag); /* blkno and offset ignored for tty, etc. */
    int16 (*dev_ioctl)(int16 dev, int16 request, char *data); /* count is rounded to 512 for block devices */
};



/* Open() parameters. */

#ifdef UZI
#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2
#else
#define UZI_O_RDONLY        0
#define UZI_O_WRONLY        1
#define UZI_O_RDWR          2
#endif

#include "generated/errno.h"
#include "config.h"

extern struct devsw dev_tab[NDEVS];  /* The device driver switch table */

#endif /* __UNIX_H__ */
