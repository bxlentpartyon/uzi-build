int uninsq(register struct s_queue *q, char *cp);
int d_ioctl(int dev, int request, char *data);
void bufdump(void);
int swapread(int dev, blkno_t blkno, unsigned nbytes, char *buf);
int swapwrite(int dev, blkno_t blkno, unsigned nbytes, char *buf);

/* static to devio.h */
int bdread(bufptr bp);
int bdwrite(bufptr bp);
