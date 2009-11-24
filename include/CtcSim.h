#ifndef _CTC_SIM_H_INCLUDE_
#define _CTC_SIM_H_INCLUDE_

#ifdef __LYNXOS
/* only for kernel */
#include <dldd.h>
#include <kernel.h>
#include <kern_proto.h>

#include <errno.h>
#include <sys/types.h>
#include <conf.h>
#include <io.h>
#include <sys/ioctl.h>
#include <time.h>
#include <param.h>
#include <string.h>
#include <proto.h>
#include <proto_int.h>
#include <fcntl.h>
#include <dir.h>
#include <string.h>
#include <unistd.h>
#include <param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <termio.h>
#include <termios.h>
#include <ces/vmelib.h>	  /* for 'pdparam_xxx' structure */
#include <inode.h>

/* system thread */
#include <conf.h>
#include <st.h>
#endif	/* __LYNXOS */

#include "dg/vme_am.h" /* for VMEbus Address Modifier Codes */
#include "dg/ServiceRegIoctl.h"
#include "dg/ModuleHeader.h"

#if defined(__linux__) && defined (__KERNEL__)
#undef sel
#define sel poll_table_struct	/* for select entry point */
/* only for Linux kernel */
#include <cdcm/cdcm.h>
#include "dg/swab-extra-linux.h"
#endif

/* for kernel only */
#if defined(__LYNXOS) || defined (__KERNEL__)
/* to be able to use CDCM utils inside the driver */
#include <cdcm/cdcmBoth.h>
#endif

/* Provides debug information printing. */
#define DBG_BEG(f_nm)							\
do {									\
  if (s->info->debugFlag & DBGIOCTL) {					\
    swait(&sem_drvr, SEM_SIGIGNORE); /* lock calling thread */		\
    cprintf("(pid %d) (tid %d) %s() ", getpid(), st_getstid(), f_nm);	\
  }									\
} while (0)

/* information about the address that will be read/write */
#define DBG_ADDR(ra, idx, en)					   \
do {								   \
  if (s->info->debugFlag & DBGIOCTL) {				   \
    unsigned addr = (unsigned)ra;				   \
    cprintf("[(elIdx %d) (elAm %d) @ 0x%x] ", idx, (int)en, addr); \
  }								   \
} while (0)

/* ioctl completion tag */
#define DBG_OK()							\
do {									\
	if (s->info->debugFlag & DBGIOCTL) {				\
		cprintf(" - OK.\n");					\
		ssignal(&sem_drvr);	/* wake up waiting thread */	\
	}								\
 } while (0)

/* ioctl completion tag */
#define DBG_FAIL()					\
do {							\
  if (s->info->debugFlag & DBGIOCTL) {			\
    cprintf(" - FAIL.\n");				\
    ssignal(&sem_drvr);	/* wake up waiting thread */	\
  }							\
} while (0)

/* start timing measurements. Different in case of Lynx/Linux */
#ifdef __Lynx__

#define START_TIME_STAT()								\
do {											\
  if (s->info->debugFlag & DBGTIMESTAT) {						\
    /* because of the spatial locality reference phenomenon, fill in cache		\
       memory so that it will keep recently accessed values (in our case		\
       it is 'nanotime' sys function stuff), thus trying to avoid additional		\
       time loss induced by cache miss.							\
       Normally, '__builtin_prefetch' gcc function should be used to minimize		\
       cache-miss latency by moving data into a cache before it is accessed.		\
       But with currently used Lynx gcc (version 2.95.3 20010323 (Lynx) as		\
       of 24.04.06) it's not possible, as it's probably not supported.			\
       For more info see gcc 4.0.1 reference manual at:					\
       http://gcc.gnu.org/onlinedocs/gcc-4.0.1/gcc/Other-Builtins.html#Other-Builtins	\
       So let's wait until newer gcc will be installed and then try the new		\
       approach. */									\
    GlobStartN = nanotime(&GlobStartS);							\
											\
    /* wait for the better times to implement... */					\
    /*__builtin_prefetch(&GlobStartN, 1, 1); */						\
											\
    /* now we can start real timing measurements */					\
    GlobStartN = nanotime(&GlobStartS);							\
  }											\
} while (0)

#else  /* linux */

#define START_TIME_STAT() \
	if (s->info->debugFlag & DBGTIMESTAT) getnstimeofday(&_t_beg)

#endif /* defined __Lynx__ */

/* printout measured time statistics */
#define FINISH_TIME_STAT()						\
do {									\
	if (s->info->debugFlag & DBGTIMESTAT) {				\
		unsigned int retN; /* for timing measurements */	\
		retN = ComputeTime(GlobTimeStatS);			\
		if (retN == -1)						\
			cprintf("Done in %s\n", GlobTimeStatS);		\
		else							\
			cprintf("Done in %dns (%s)\n", retN, GlobTimeStatS); \
	}								\
 } while (0)


#define CTC_MAX_NUM_CARDS 21 /* VME crates have 21 slots max */

/* CTC module ioctl numbers */
#define CTC_IOCTL_MAGIC 'C'

#define CTC_IO(nr)      _IO(CTC_IOCTL_MAGIC, nr)
#define CTC_IOR(nr,sz)  _IOR(CTC_IOCTL_MAGIC, nr, sz)
#define CTC_IOW(nr,sz)  _IOW(CTC_IOCTL_MAGIC, nr, sz)
#define CTC_IOWR(nr,sz) _IOWR(CTC_IOCTL_MAGIC, nr, sz)

#define CTC_GET_STATUS (_FIRST__IOCTL_ + 0)
#define CTC_GET_CNTR_ENABLE (_FIRST__IOCTL_ + 1)
#define CTC_SET_CNTR_ENABLE (_FIRST__IOCTL_ + 2)
#define CTC_GET_CONFCHAN (_FIRST__IOCTL_ + 3)
#define CTC_SET_CONFCHAN (_FIRST__IOCTL_ + 4)
#define CTC_GET_CLOCK1DELAY (_FIRST__IOCTL_ + 5)
#define CTC_SET_CLOCK1DELAY (_FIRST__IOCTL_ + 6)
#define CTC_GET_CLOCK2DELAY (_FIRST__IOCTL_ + 7)
#define CTC_SET_CLOCK2DELAY (_FIRST__IOCTL_ + 8)
#define CTC_GET_OUTPUTCNTR (_FIRST__IOCTL_ + 9)
#define CTC_GET_CNTR1CURVAL (_FIRST__IOCTL_ + 10)
#define CTC_GET_CNTR2CURVAL (_FIRST__IOCTL_ + 11)
#define CTC_GET_CHANNEL_1 (_FIRST__IOCTL_ + 12)
#define CTC_SET_CHANNEL_1 (_FIRST__IOCTL_ + 13)
#define CTC_GET_CHANNEL_2 (_FIRST__IOCTL_ + 14)
#define CTC_SET_CHANNEL_2 (_FIRST__IOCTL_ + 15)
#define CTC_GET_CHANNEL_3 (_FIRST__IOCTL_ + 16)
#define CTC_SET_CHANNEL_3 (_FIRST__IOCTL_ + 17)
#define CTC_GET_CHANNEL_4 (_FIRST__IOCTL_ + 18)
#define CTC_SET_CHANNEL_4 (_FIRST__IOCTL_ + 19)
#define CTC_GET_CHANNEL_5 (_FIRST__IOCTL_ + 20)
#define CTC_SET_CHANNEL_5 (_FIRST__IOCTL_ + 21)
#define CTC_GET_CHANNEL_6 (_FIRST__IOCTL_ + 22)
#define CTC_SET_CHANNEL_6 (_FIRST__IOCTL_ + 23)
#define CTC_GET_CHANNEL_7 (_FIRST__IOCTL_ + 24)
#define CTC_SET_CHANNEL_7 (_FIRST__IOCTL_ + 25)
#define CTC_GET_CHANNEL_8 (_FIRST__IOCTL_ + 26)
#define CTC_SET_CHANNEL_8 (_FIRST__IOCTL_ + 27)
#define CTC_GET_ALL_CHANNELS (_FIRST__IOCTL_ + 28)
#define CTC_SET_ALL_CHANNELS (_FIRST__IOCTL_ + 29)

/* First allowed number for user-defined ioctl */
#define CTC_FIRST_USR_IOCTL (_FIRST__IOCTL_ + 30)

/* keeps last written value of the 'write only' registers */
typedef  struct {
} CTCWriteonly_t;


/* user-defined extraneous registers */
typedef  struct {
} CTCExtraneous_t;


/* Blk[#0]@addr[#1] Offs 0x0. Sz 8 bytes. 2 reg(s). 0 gap(s) */
typedef volatile struct {
  /* 0x0 */ unsigned long STATUS;
  /* 0x4 */ unsigned long CNTR_ENABLE;
} CTCBlock00_t;

/* Blk[#1]@addr[#1] Offs 0x8. Sz 24 bytes. 6 reg(s). 0 gap(s) */
typedef volatile struct {
  /* 0x0 */ unsigned long confChan;
  /* 0x4 */ unsigned long clock1Delay;
  /* 0x8 */ unsigned long clock2Delay;
  /* 0xc */ unsigned long outputCntr;
  /* 0x10 */ unsigned long cntr1CurVal;
  /* 0x14 */ unsigned long cntr2CurVal;
} CTCBlock01_t;

/* Blk[#2]@addr[#1] Offs 0x8. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_1[6];
} CTCBlock02_t;

/* Blk[#3]@addr[#1] Offs 0x20. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_2[6];
} CTCBlock03_t;

/* Blk[#4]@addr[#1] Offs 0x38. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_3[6];
} CTCBlock04_t;

/* Blk[#5]@addr[#1] Offs 0x50. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_4[6];
} CTCBlock05_t;

/* Blk[#6]@addr[#1] Offs 0x68. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_5[6];
} CTCBlock06_t;

/* Blk[#7]@addr[#1] Offs 0x80. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_6[6];
} CTCBlock07_t;

/* Blk[#8]@addr[#1] Offs 0x98. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_7[6];
} CTCBlock08_t;

/* Blk[#9]@addr[#1] Offs 0xb0. Sz 24 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x14] */ unsigned long channel_8[6];
} CTCBlock09_t;

/* Blk[#10]@addr[#1] Offs 0x8. Sz 192 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0xbc] */ unsigned long ALL_CHANNELS[48];
} CTCBlock10_t;

/* CTC module master topology */
typedef struct {
  CTCWriteonly_t  *wo; /* write only registers last history */
  CTCExtraneous_t *ex; /* extraneous registers */
  CTCBlock00_t *block00;
  CTCBlock01_t *block01;
  CTCBlock02_t *block02;
  CTCBlock03_t *block03;
  CTCBlock04_t *block04;
  CTCBlock05_t *block05;
  CTCBlock06_t *block06;
  CTCBlock07_t *block07;
  CTCBlock08_t *block08;
  CTCBlock09_t *block09;
  CTCBlock10_t *block10;
} CTCTopology_t;

/* compiler should know variable type. So get rid of incomplete type */
typedef struct CTCUserStatics_t CTCUserStatics_t;

/* CTC statics data table */
typedef struct {
  CTCTopology_t *card; /* hardware module topology */
  DevInfo_t *info; /* device information table */
  CTCUserStatics_t *usrst; /* user-defined statics data table */
} CTCStatics_t;

#endif /* _CTC_SIM_H_INCLUDE_ */
