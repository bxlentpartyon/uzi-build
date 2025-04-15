/**************************************************
UZI (Unix Z80 Implementation) Kernel:  config.h
***************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

extern int tty_open();
extern int tty_close();
extern int tty_read();
extern int tty_write();
extern unsigned int mem_read();
extern unsigned int mem_write();
extern unsigned int null_write();

#define NDEVS	5		/* Devices 0..NDEVS-1 are capable of being mounted */
#define TTYDEV	2		/* Device used by kernel for messages, panics */
#define SWAPDEV	1		/* Device for swapping. */
#define NBUFS	4		/* Number of block buffers */

#endif				/* __CONFIG_H__ */
