#ifdef __Lynx__
/* for __inb, __outb etc... */
#include "dg/port_ops_lynx.h"
#else  /* __linux__ */
#include "dg/port_ops_linux.h"
#endif
#include "CtcSim.h"
#include "CtcUserDefinedSim.h"
#include "CtcVersionSim.h"

int sem_drvr = 1; /* initial semaphore value */

/* next are for timing measurements */
#define TSTR_LEN 32 /* time string lengths */
/* time statistic string */
char GlobTimeStatS[TSTR_LEN] = { [0 ... sizeof(GlobTimeStatS)-1] = 0 };
#ifdef __Lynx__
unsigned long GlobStartS = 0, GlobEndS = 0; /* seconds */
unsigned int  GlobStartN = 0, GlobEndN = 0; /* nanoseconds */
unsigned int  GlobCalVal = 0; /* timing measurements calibration value */
#else /* __linux__ */
static struct timespec _t_beg, _t_end, _t_delta;
static s64 _ac_etime;
#endif	/* __Lynx__ */

/* -------> CTC Simulator version info <------- */
/* driver compilation date and time */
static char Ctc_compile_date[]  = __DATE__;
static char Ctc_compile_time[]  = __TIME__;

/* which driverGen version was used to generate current code */
static const char Ctc_version[] = "v2.1.4";

/* generation date in hex and human representation */
static const char Ctc_generation_time_str[] = "Wed Aug 26 15:46:08 2009";
#define CTC_GENERATION_TIME_HEX 0x4a953ca0
/* ------------------------------------------------------------------------- */

/* to suppress implisit declaration warnings */
extern int nanotime(unsigned long *);
extern int ksprintf(char *, char *, ...);
extern int unmapVmeAddr(DevInfo_t*) __attribute__ ((weak));

/**
 * @brief Current driver/simulator version.
 *
 * @param vp -- if not @e NULL - driver version string will be placed here
 *
 * If @b vp is @e NULL, then driver compilation date and @e driverGen version,
 * by which current code was generated are printed out to the terminal output.
 * If @b vp is not null, then it is set to the address with version info.
 * No terminal printout is performed in this case.
 *
 * @return current driver version number
 */
static int DisplayVersion(char **vp)
{
	static char vers_info_str[MAX_STR] =
		{ [0 ... sizeof(vers_info_str)-1] = 0 };


	if (!vers_info_str[0]) { /* not initialized yet */
		Ctc_compile_date[11] = 0;
		Ctc_compile_time [9] = 0;

		/* build-up version info string */
		ksprintf(vers_info_str, "Compiled on: %s %s."
			 " Generated by driverGen <%s> on [ %s ]",
			 Ctc_compile_date, Ctc_compile_time,
			 Ctc_version, Ctc_generation_time_str);
	}

	if (vp)
		*vp = vers_info_str;
	else
		kkprintf("\nCTC Simulator <Vers. %d>\n%s",
			 CTC_SIM_CURRENT_VERSION,
			 vers_info_str);

	return CTC_SIM_CURRENT_VERSION;
}

#ifdef __Lynx__

/**
 * @brief Calibration for timing measurements
 *
 * @param info -- dev info table
 *
 * Find out how much time does @e timing @e measurements algorithm itself
 * takes. This time inaccuracy will be taken into account during timing
 * measurements computation. Consider it like timing collibration mechanism.
 *
 * For Linux - calibration is always zero.
 *
 * @return rough estimation for timing measurement in nanoseconds for Lynx
 * @return 0 - in case of Linux
 */
static unsigned int CalibrateTime(DevInfo_t *info)
{
	int cntr;
	unsigned int nsS, nsF, calVal; /* ns */
	unsigned long sS, sF;	         /* s */
	unsigned long long sum = 0;

	/* 100 sampling */
	for (cntr = 0; cntr < 0x64; cntr++) {
		/* hope that is less then 4s (if no - then you are fucked!) */
		nsS = nanotime(&sS);
		nsF = nanotime(&sF);
		calVal = ((sF-sS) && nsF < nsS) ? nsF + 1000000000 - nsS :
			nsF - nsS;
		sum += calVal;
	}

	calVal = sum/0x64;
	return calVal;
}

#else  /* __linux__ */

/* form kernel/lib/div64.c */
uint32_t __attribute__((weak)) __div64_32(uint64_t *n, uint32_t base)
{
	uint64_t rem = *n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t) high << 32;
		rem -= (uint64_t) (high*base) << 32;
	}

	while ((int64_t)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}

#endif


/**
 * @brief Process timing data, that was measured during driver code execution.
 *
 * @param tStr   -- measured time in %E format
 *
 * As currently @e LynxOS doesn't support %E output format, time string in
 * seconds (%E) representation is build-up and store in @e tStr parameter.
 * Note, that accuracy is limited up to 4 sec MAX. So if exec time is more,
 * than buildup string will contain notification message saying, that time is
 * more then 4 secs.
 *
 * @return Computed time in ns, @b LIMITED up to 4s (i.e. 4E+9ns) If time
 *         exeeds 4s, then -1 (i.e. 0xffffffff) is returnded. @e tStr string
 *         is set to the measured time in sec (%E format), or if it exeeds 4
 *         sec, then string will contain appropriate message.
 */
static unsigned int ComputeTime(char tStr[TSTR_LEN])
{
	int tseCntr, power;
	unsigned int  execTN = 0; /* total execution time in ns */
	unsigned long execTS = 0; /* total execution time in sec */

	static int e[10] = { /* all possible exp values */
		1,	   /* s  */
		10,	   /* ms */
		100,	   /* ms */
		1000,	   /* ms */
		10000,	   /* us */
		100000,	   /* us */
		1000000,   /* us */
		10000000,  /* ns */
		100000000, /* ns */
		1000000000 /* ns */
	};

	/* finish time measurements */
#ifdef __Lynx__
	GlobEndN = nanotime(&GlobEndS);
	execTS = GlobEndS - GlobStartS; /* total exec time in seconds */
#else
	getnstimeofday(&_t_end);
	_t_delta = timespec_sub(_t_end, _t_beg);
	_ac_etime = timespec_to_ns(&_t_delta);
	execTN = __div64_32(&_ac_etime, e[9]);
	execTS = _ac_etime;
#endif

	/* exec time is more or equal 4 sec. printout and return */
	if (execTS >= 4) {
		ksprintf(tStr, "> 4.00000E+00s");
		return -1;
	}

	/* init time string */
	bzero(tStr, sizeof(tStr));

	/* printout in XXs XXns format */
	/*
	cprintf("\nDone in %ds %dns.\n",
		execTS, (execTS && GlobEndN < GlobStartN) ?
		GlobEndN + e[9] - GlobStartN : GlobEndN - GlobStartN);
	*/

	/* convert seconds to nanoseconds */
	execTS *= e[9];

#ifdef __Lynx__
	/* compute total exec time in nanosec */
	execTN = execTS + GlobEndN - GlobStartN - GlobCalVal;
#else
	execTN += execTS;
#endif

	/* get current time dimension */
	for (power = 9; power >= 0; power--)
		if (execTN/e[power])/* we've got our dimension */
			break;

	/* build-up time string tail. precision will be */
	/* limited to 5 digits after the decimal point  */
	ksprintf(tStr, "%u", execTN);
	for (tseCntr = power; tseCntr >= 1; tseCntr--)
		/* shift one position to the right */
		tStr[tseCntr+1] = tStr[tseCntr];
	tStr[1] = '.';
	tStr[7] = 'E';
	tStr[8] = (9-power) ? '-' : '+';
	ksprintf(&tStr[9], "%02ds", 9-power);
	return execTN;
}


/* Standart getting/setting register operations for CTC module
   are located in included file */
#include "CtcGetSetReg.inc.c"


/**
 * @brief Entry point function. Initializes minor devices.
 *
 * @param statPtr - statics table pointer
 * @param devno   - contains major/minor dev numbers
 * @param flp     - file pointer
 *
 * @return OK     - if succeed.
 * @return SYSERR - in case of failure.
 */
int Ctc_open(register CTCStatics_t *statPtr, int devno, struct file *flp)
{
  int minN = minor(devno); /* get minor device number */
  int proceed;	/* if standard code execution should be proceed after call to
		   user entry point function  */
  int usrcoco;	/* completion code of user entry point function */

  /* TODO. Wrong philosophy? */
  if (minN >= CTC_MAX_NUM_CARDS) {
    pseterr(ENXIO);
    return(SYSERR); /* -1 */
  }

  kkprintf("Ctc: Open Logical Unit %d.\n", statPtr->info->mlun);

  /* user entry point function call */
  usrcoco = CtcUserOpen(&proceed, statPtr, devno, flp);
  if (!proceed) /* all done by user */
    return(usrcoco);

  kkprintf("Ctc: Done\n\n");

  return(OK); /* 0 */
}


/**
 * @brief Entry point function.Called to close open file descriptor.
 *
 * @param statPtr -- statics table pointer
 * @param flp     -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 *
 * @return OK     - if succeed.
 * @return SYSERR - in case of failure.
 */
int Ctc_close(register CTCStatics_t *statPtr, struct file *flp)
{
	int minN;    /* minor device number */
	int proceed; /* if standard code execution should be proceed
			after call to user entry point function  */
	int usrcoco; /* completion code of user entry point function */

#ifdef __Lynx__
	minN = minor(flp->dev);
#else
	minN = iminor(flp->f_path.dentry->d_inode);
#endif

	/* user entry point function call */
	usrcoco = CtcUserClose(&proceed, statPtr, flp);
	if (!proceed) /* all done by user */
		return usrcoco;

	return OK; /* 0 */
}


/**
 * @brief Entry point function. Reads data from the device.
 *
 * @param statPtr -- statics table pointer
 * @param flp     -- file pointer
 * @param buff    -- character buffer pointer
 * @param count   -- number of bytes to copy
 *
 * @return number of bytes actually copied, including zero - if succeed.
 * @return SYSERR                                          - if fails.
 */
int Ctc_read(register CTCStatics_t *statPtr,
		     struct file *flp, char *buff, int count)
{
	int minN;    /* minor device number */
	int proceed; /* if standard code execution should be proceed
			after call to user entry point function  */
	int usrcoco; /* completion code of user entry point function */

#ifdef __Lynx__
	minN = minor(flp->dev);
#else
	minN = iminor(flp->f_path.dentry->d_inode);
#endif

	/* user entry point function call */
	usrcoco = CtcUserRead(&proceed, statPtr, flp, buff, count);
	if (!proceed) /* all done by user */
		return usrcoco;

	return usrcoco; /* number of read bytes */
}


/**
 * @brief Entry point function. Sends data to the device.
 *
 * @param statPtr -- statics table pointer
 * @param flp     -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param which   -- condition to monitor. Not valid in case of Linux!\n
 * @param buff    -- char buffer pointer
 * @param count   -- the number of bytes to copy
 *
 * @return number of bytes actually copied, including zero - if succeed.
 * @return SYSERR                                          - if fails.
 */
int Ctc_write(register CTCStatics_t *statPtr,
		      struct file *flp, char *buff, int count)
{
	int minN; /* minor device number */
	int proceed;	/* if standard code execution should be proceed after
			   call to user entry point function  */
	int usrcoco;	/* completion code of user entry point function */

#ifdef __Lynx__
	minN = minor(flp->dev);
#else  /* __linux__ */
	minN = iminor(flp->f_path.dentry->d_inode);
#endif

	/* user entry point function call */
	usrcoco = CtcUserWrite(&proceed, statPtr, flp, buff, count);
	if (!proceed) /* all done by user */
		return usrcoco;

	return usrcoco; /* number of written bytes */
}


/**
 * @brief Entry point function. Supports I/O polling or multiplexing.
 *
 * @param statPtr -- statics table pointer
 * @param flp     -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param which   -- condition to monitor. Not valid in case of Linux!\n
 *                   <b> SREAD, SWRITE, SEXCEPT </b> in case of Lynx.
 * @param ffs     -- select data structure in case of Lynx.\n
 *                   struct poll_table_struct in case of Linux.
 *
 * @return OK     - if succeed.
 * @return SYSERR - in case of failure.
 */
int Ctc_select(register CTCStatics_t *statPtr,
		       struct file *flp, int which, struct sel *ffs)
{
	int minN;    /* minor device number */
	int proceed; /* if standard code execution should be proceed after
			call to user entry point function  */
	int usrcc; /* completion code of user entry point function */

#ifdef __Lynx__
	minN = minor(flp->dev);
#else  /* __linux__ */
	minN = iminor(flp->f_path.dentry->d_inode);
#endif

	/* user entry point function call */
	usrcc = CtcUserSelect(&proceed, statPtr, flp, which, ffs);
	if (!proceed) /* all done by user */
		return usrcc;

	return OK; /* 0 */
}


/**
 * @brief CTC ioctl entry point
 *
 * @param s   -- statics table
 * @param f   -- file pointer. Lynx/Linux specific.
 *               See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param com -- ioctl number
 * @param arg -- arguments
 *
 * By default - we do check r/w bounds while accessing the register.
 *
 * @return OK     - if succeed.
 * @return SYSERR - in case of failure.
 */
int Ctc_ioctl(register CTCStatics_t *s,
		      struct file *f, int com, char *arg)
{
	static int c_rwb = 1; /* check r/w bounds (1 - yes, 0 - no)
				 Valid _ONLY_ for Lynx! */
	int minN;     /* minor device number (LUN) */
	int proceed;  /* if standard code execution should be proceed after
			 call to user entry point function  */
	int usrcoco;  /* completion code of user entry point function */
	int r_rw = 0; /* repetitive r/w (1 - yes, 0 - no) */

	START_TIME_STAT(); /* timing measurements */

#ifdef __Lynx__
	minN = minor(f->dev);
#else
	minN = iminor(f->f_path.dentry->d_inode);
#endif

	/* user entry point function call */
	usrcoco = CtcUserIoctl(&proceed, s, f, minN, com, arg);
	if (!proceed) /* all done by user */
		return usrcoco;

 rep_ioctl:
	switch (com) { /* default 'ioctl' driver operations */
	case SRV_GET_DEBUG_FLAG:
		return get___SRV__DEBUG_FLAG(s, arg, c_rwb, r_rw);
		break;
	case SRV_SET_DEBUG_FLAG:
		return set___SRV__DEBUG_FLAG(s, arg, c_rwb, r_rw);
		break;
	case SRV_GET_DEVINFO_T:
		return get___SRV__DEVINFO_T(s, arg, c_rwb, r_rw);
		break;
	case SRV_GET_DRVR_VERS:
		return get___SRV__DRVR_VERS(s, arg, c_rwb, r_rw);
		break;
	case SRV_GET_DAL_CONSISTENT:
		return get___SRV__DAL_CONSISTENT(s, arg, c_rwb, r_rw);
		break;
	case SRV__REP_REG_RW:
		return srv_func___SRV__REP_REG_RW(s, arg, c_rwb, r_rw);
		break;
	case SRV__RW_BOUNDS:
		return srv_func___SRV__RW_BOUNDS(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_STATUS:
		return get_STATUS(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CNTR_ENABLE:
		return get_CNTR_ENABLE(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CNTR_ENABLE:
		return set_CNTR_ENABLE(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CONFCHAN:
		return get_confChan(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CONFCHAN:
		return set_confChan(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CLOCK1DELAY:
		return get_clock1Delay(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CLOCK1DELAY:
		return set_clock1Delay(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CLOCK2DELAY:
		return get_clock2Delay(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CLOCK2DELAY:
		return set_clock2Delay(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_OUTPUTCNTR:
		return get_outputCntr(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CNTR1CURVAL:
		return get_cntr1CurVal(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CNTR2CURVAL:
		return get_cntr2CurVal(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_1:
		return get_channel_1(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_1:
		return set_channel_1(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_2:
		return get_channel_2(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_2:
		return set_channel_2(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_3:
		return get_channel_3(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_3:
		return set_channel_3(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_4:
		return get_channel_4(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_4:
		return set_channel_4(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_5:
		return get_channel_5(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_5:
		return set_channel_5(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_6:
		return get_channel_6(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_6:
		return set_channel_6(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_7:
		return get_channel_7(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_7:
		return set_channel_7(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_CHANNEL_8:
		return get_channel_8(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_CHANNEL_8:
		return set_channel_8(s, arg, c_rwb, r_rw);
		break;
	case CTC_GET_ALL_CHANNELS:
		return get_ALL_CHANNELS(s, arg, c_rwb, r_rw);
		break;
	case CTC_SET_ALL_CHANNELS:
		return set_ALL_CHANNELS(s, arg, c_rwb, r_rw);
		break;
	default:
		pseterr(EINVAL);
		return SYSERR; /* -1 */
	} /* end of 'ioctl' operations switch */

	return OK; /* 0 */
}


/**
 * @brief Cleanup and deallocation of the Simulator statics table.
 *
 * @param statPtr - deallocate this
 *
 * @return void
 */
static void CtcMemCleanup(CTCStatics_t *statPtr)
{
  DevInfo_t *dip = statPtr->info; /* device info pointer */

  if (statPtr->card != NULL) {
    kkprintf("Ctc: Cleaning up Logical Unit %d\n", dip->mlun);

    if (statPtr->card->wo != NULL) {
      kkprintf("Ctc: Writeonly structure deallocation - ");
      sysfree((char*)statPtr->card->wo, (long)sizeof(CTCWriteonly_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->ex != NULL) {
      kkprintf("Ctc: Extraneous structure deallocation - ");
      sysfree((char*)statPtr->card->ex, (long)sizeof(CTCExtraneous_t));
      kkprintf("OK\n");
    }
    if (statPtr->card->block00 != NULL) {
      kkprintf("Ctc: Block 0 deallocation - ");
      sysfree((char*)statPtr->card->block00, (long)sizeof(CTCBlock00_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block01 != NULL) {
      kkprintf("Ctc: Block 1 deallocation - ");
      sysfree((char*)statPtr->card->block01, (long)sizeof(CTCBlock01_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block02 != NULL) {
      kkprintf("Ctc: Block 2 deallocation - ");
      sysfree((char*)statPtr->card->block02, (long)sizeof(CTCBlock02_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block03 != NULL) {
      kkprintf("Ctc: Block 3 deallocation - ");
      sysfree((char*)statPtr->card->block03, (long)sizeof(CTCBlock03_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block04 != NULL) {
      kkprintf("Ctc: Block 4 deallocation - ");
      sysfree((char*)statPtr->card->block04, (long)sizeof(CTCBlock04_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block05 != NULL) {
      kkprintf("Ctc: Block 5 deallocation - ");
      sysfree((char*)statPtr->card->block05, (long)sizeof(CTCBlock05_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block06 != NULL) {
      kkprintf("Ctc: Block 6 deallocation - ");
      sysfree((char*)statPtr->card->block06, (long)sizeof(CTCBlock06_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block07 != NULL) {
      kkprintf("Ctc: Block 7 deallocation - ");
      sysfree((char*)statPtr->card->block07, (long)sizeof(CTCBlock07_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block08 != NULL) {
      kkprintf("Ctc: Block 8 deallocation - ");
      sysfree((char*)statPtr->card->block08, (long)sizeof(CTCBlock08_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block09 != NULL) {
      kkprintf("Ctc: Block 9 deallocation - ");
      sysfree((char*)statPtr->card->block09, (long)sizeof(CTCBlock09_t));
      kkprintf("OK\n");
    }

    if (statPtr->card->block10 != NULL) {
      kkprintf("Ctc: Block 10 deallocation - ");
      sysfree((char*)statPtr->card->block10, (long)sizeof(CTCBlock10_t));
      kkprintf("OK\n");
    }

    kkprintf("Ctc: Topology structure deallocation - ");
    sysfree((char*)statPtr->card, (long)sizeof(CTCTopology_t));
    kkprintf("OK\n");

    kkprintf("Ctc: All Logical Unit specific resources have been deallocated.\n");
  }

  if (statPtr->usrst != NULL) {
    kkprintf("Ctc: User-defined statics data table deallocation - ");
    sysfree((char*)statPtr->usrst, (long)sizeof(CTCUserStatics_t));
    kkprintf("OK\n");
  }

  kkprintf("Ctc: Device info table structure deallocation - ");
  sysfree((char*)statPtr->info, (long)sizeof(DevInfo_t));
  kkprintf("OK\n");

  kkprintf("Ctc: Statics structure deallocation - ");
  sysfree((char*)statPtr, (long)sizeof(CTCStatics_t));
  kkprintf("OK\n");
}




/**
 * @brief Entry point function.
 *
 * @param info --
 *
 * Initializes the hardware and allocates shared memory buffers. Invoked each
 * time the device driver is installed for a major device (i.e. each time
 * when @b cdv_install() or @b bdv_install() functions are called).
 *
 * @return pointer to a statics data structure - if succeed.
 * @return SYSERR                              - in case of failure.
 */
#define INST_OK()						\
do {								\
  kkprintf("Ctc: Simulator (LUN %d) ", info->mlun);	\
  kkprintf(" installation Complete. ");				\
  DisplayVersion(NULL);						\
  kkprintf("\n\n");						\
} while (0)

#define INST_FAIL()						\
do {								\
  kkprintf("Ctc: Simulator (LUN %d) ", info->mlun);	\
  kkprintf(" installation Fails. ");				\
  DisplayVersion(NULL);						\
  kkprintf("\n\n");						\
} while (0)

char* Ctc_install(void *infofile)
{
	register CTCStatics_t *statPtr;
	char *usrcoco; /* completion code of user entry point function */
	int cntr;
	AddrInfo_t *aiptr;
	DevInfo_t *info;
	int res __attribute__((unused));

	kkprintf("Ctc: Device info table structure allocation - ");
	info = (DevInfo_t *)sysbrk(sizeof(DevInfo_t));
	if (!info) {
		/* Horrible failure */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return (char *)SYSERR; /* -1 */
	}
	kkprintf("OK\n");

	kkprintf("Ctc: Copying info table from user space - ");
	memcpy(info, infofile, sizeof(DevInfo_t));
	kkprintf("OK\n");


  if (info->debugFlag & DBGINSTL) {	/* only in case of debugging */
    aiptr = &(info->addr1);
    kkprintf("\n\nSimulator install procedure parameters are:\n");
    kkprintf("+-----------------------------------------\n");
    /* info about 2 address spaces */
    for (cntr = 0; cntr < 2; cntr++) {
      kkprintf("|    Addr%d info:\n", cntr+1);
      kkprintf("|    -----------\n");
      if (aiptr->baseAddr != ADCG__NO_ADDRESS) {
	kkprintf("|     Base address      => 0x%x\n", aiptr->baseAddr);
	kkprintf("|     Address range     => 0x%x\n", aiptr->range);
	kkprintf("|     Address increment => 0x%x\n", aiptr->increment);
	kkprintf("|     Dataport size     => %d\n", aiptr->dpSize);
	kkprintf("|     Address modifier  => %s (0x%x)\n|\n", (aiptr->addrModif == DG_AM_SH)?"SH":(aiptr->addrModif == DG_AM_ST)?"ST":(aiptr->addrModif == DG_AM_EX)?"EX":(aiptr->addrModif == DG_AM_CR)?"CR":"UNKNOWN", aiptr->addrModif);
      } else kkprintf("|     Addr%d NOT defined.\n|\n", cntr+1);
      aiptr++;
    }

    kkprintf("| Debug flag                 => 0x%x\n", info->debugFlag);
    kkprintf("| Opaque parameter           => '%s'\n", info->opaque);
    kkprintf("| Interrupt level            => %d\n", info->iLevel);
    kkprintf("| Interrupt vector           => %d\n", info->iVector);
    kkprintf("| Interrupt vector increment => %d\n", info->iVectorInc);
    kkprintf("| Number of channels         => %d\n", info->chan);
    kkprintf("+-----------------------------------------\n\n");

    if (info->debugFlag & DBGALLDI) {
      /* user wants really everything, so let him have it! */
      REGDESC *rdp = info->regDesc;
      BLKDESC *bdp = info->blkDesc;
      aiptr = &(info->addr1);

      /* 2 address space info */
      for (cntr = 0; cntr < 2; cntr++) {
	kkprintf("+-----------------------------\n");
	kkprintf("| Addr[%d].baseAddr  => 0x%X\n", cntr, aiptr->baseAddr);
	kkprintf("| Addr[%d].range     => 0x%X\n", cntr, aiptr->range);
	kkprintf("| Addr[%d].increment => 0x%X\n", cntr, aiptr->increment);
	kkprintf("| Addr[%d].dpSize    => 0x%d\n", cntr, aiptr->dpSize);
	kkprintf("| Addr[%d].addrModif => 0x%d\n", cntr, aiptr->addrModif);
	kkprintf("+-----------------------------\n\n");
	aiptr++;
      }

      /* general info */
      kkprintf("+-----------------------------\n");
      kkprintf("| 'mtn'        => %d\n",   info->mtn);
      kkprintf("| 'debugFlag'  => 0x%X\n", info->debugFlag);
      kkprintf("| 'opaque'     => '%s'\n", info->opaque);
      kkprintf("| 'iLevel'     => %d\n",   info->iLevel);
      kkprintf("| 'iVector'    => %d\n",   info->iVector);
      kkprintf("| 'iVectorInc' => %d\n",   info->iVectorInc);
      kkprintf("| 'chan'       => %d\n",   info->chan);
      kkprintf("| 'chrindex'   => %d\n",   info->chrindex);
      kkprintf("| 'gen_date'   => %ld\n",  info->gen_date);
      kkprintf("| 'dg_vers'    => '%s'\n", info->dg_vers);
      kkprintf("| 'regAmount'  => %d\n",   info->regAmount);
      kkprintf("| 'maxRegSz'   => %d\n",   info->maxRegSz);
      kkprintf("| 'checksum'   => %d\n",   info->checksum);
      kkprintf("+-----------------------------\n\n");

      /* all block info */
      for (cntr = 0; cntr < info->blkAmount; cntr++) {
	kkprintf("+---------------------------------------------\n");
	kkprintf("| Blk[%d] 'block'       => %d\n", cntr, bdp[cntr].block);
	kkprintf("| Blk[%d] 'blkBaseAddr' => %d\n", cntr,
		 bdp[cntr].blkBaseAddr);
	kkprintf("| Blk[%d] 'offset'      => 0x%lX\n", cntr, bdp[cntr].offset);
	kkprintf("| Blk[%d] 'blksz_drvr'  => %d\n", cntr, bdp[cntr].blksz_drvr);
	kkprintf("| Blk[%d] 'blksz_sim'   => %d\n", cntr, bdp[cntr].blksz_sim);
	kkprintf("| Blk[%d] 'reg_am'      => %d\n", cntr, bdp[cntr].reg_am);
	kkprintf("+---------------------------------------------\n\n");
      }

      /* all registers info */
      for (cntr = 0; cntr < info->regAmount; cntr++) {
	kkprintf("+---------------------------------------------\n");
	kkprintf("| Reg[%d] 'ID'           => %d\n", cntr, rdp[cntr].regId);
	kkprintf("| Reg[%d] 'regName'      => %s\n", cntr, rdp[cntr].regName);
	kkprintf("| Reg[%d] 'busType'      => %s\n", cntr, rdp[cntr].busType);
	kkprintf("| Reg[%d] 'regType'      => %s (%d)\n", cntr, (rdp[cntr].regType == RT_REAL)?"RT_REAL":(rdp[cntr].regType == RT_EXTR)?"RT_EXTR":"RT_SRVS", rdp[cntr].regType);
	kkprintf("| Reg[%d] 'regSize'      => %d\n", cntr, rdp[cntr].regSize);
	kkprintf("| Reg[%d] 'b_a'          => %s\n", cntr, rdp[cntr].b_a);
	kkprintf("| Reg[%d] 'bid'          => %d\n", cntr, rdp[cntr].bid);
	kkprintf("| Reg[%d] 'regOffset'    => 0x%X\n", cntr, rdp[cntr].regOffset);
	kkprintf("| Reg[%d] 'regDepth'     => %d\n", cntr, rdp[cntr].regDepth);
	kkprintf("| Reg[%d] 'regtl'        => %d\n", cntr, rdp[cntr].regtl);
	kkprintf("| Reg[%d] 'regar'        => %d\n", cntr, rdp[cntr].regar);
	kkprintf("| Reg[%d] 'rwIoctlOpNum' => 0x%x\n", cntr, rdp[cntr].rwIoctlOpNum);
	kkprintf("+---------------------------------------------\n\n");
      }
    }
  }

  kkprintf("Ctc: Simulator (LUN %d) installation begins...\n", info->mlun);

#ifdef __Lynx__
  GlobCalVal = CalibrateTime(info); /* do timing calibration */
  if (info->debugFlag & DBGTIMESTAT) /* time statistics is enabled */
     kkprintf("Ctc: Calibrated timing is %dns per access.\n",
	      GlobCalVal);
#endif

  /*
     Should we disable interrupts here?
     int ps;
     disable(ps);
  */

	/* For device driver resources, resource allocation must be done during
	   the installation routine (e.g.: decriptor of system call interface:
	   semaphore, system thread, etc.) */

	/* 0x0 */
	kkprintf("Ctc: Statics data structure allocation - ");
	if(!(statPtr = (CTCStatics_t*)
	     sysbrk((long)sizeof(CTCStatics_t)))) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return (char*)SYSERR; /* -1 */
	}
	kkprintf("OK\n");

	/* 0x1 */
	kkprintf("Ctc: Hardware topology structure allocation - ");
	if( !(statPtr->card = (CTCTopology_t*)
	      sysbrk((long)sizeof(CTCTopology_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return (char*)SYSERR; /* -1 */
	}
	bzero((char*)statPtr->card, sizeof(CTCTopology_t));
	kkprintf("OK\n");

	/* 0x2 */
	if (sizeof(CTCWriteonly_t) > 0) {
		kkprintf("Ctc: Writeonly structure allocation - ");
		if ( !(statPtr->card->wo = (CTCWriteonly_t*)
		       sysbrk((long)sizeof(CTCWriteonly_t))) ) {
			/* Horrible, impossible failure. */
			kkprintf("FAILED\n");
			pseterr(ENOMEM);
			INST_FAIL();
			return (char*)SYSERR;	/* -1 */
		}
		bzero((char*)statPtr->card->wo, sizeof(CTCWriteonly_t));
		kkprintf("OK\n");
	} else {
		statPtr->card->wo = NULL;
	}

	/* 0x3 */
	if (sizeof(CTCExtraneous_t) > 0) {
		kkprintf("Ctc: Extraneous structure allocation - ");
		if ( !(statPtr->card->ex = (CTCExtraneous_t*)
		       sysbrk((long)sizeof(CTCExtraneous_t))) ) {
			/* Horrible, impossible failure. */
			kkprintf("FAILED\n");
			pseterr(ENOMEM);
			INST_FAIL();
			return (char*)SYSERR;	/* -1 */
		}
		bzero((char*)statPtr->card->ex, sizeof(CTCExtraneous_t));
		kkprintf("OK\n");
	} else {
		statPtr->card->ex = NULL;
	}

	/* 0x4 */
	kkprintf("Ctc: Block 0 allocation - ");
	if ( !(statPtr->card->block00 = (CTCBlock00_t *)sysbrk((long)sizeof(CTCBlock00_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block00, sizeof(CTCBlock00_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 1 allocation - ");
	if ( !(statPtr->card->block01 = (CTCBlock01_t *)sysbrk((long)sizeof(CTCBlock01_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block01, sizeof(CTCBlock01_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 2 allocation - ");
	if ( !(statPtr->card->block02 = (CTCBlock02_t *)sysbrk((long)sizeof(CTCBlock02_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block02, sizeof(CTCBlock02_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 3 allocation - ");
	if ( !(statPtr->card->block03 = (CTCBlock03_t *)sysbrk((long)sizeof(CTCBlock03_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block03, sizeof(CTCBlock03_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 4 allocation - ");
	if ( !(statPtr->card->block04 = (CTCBlock04_t *)sysbrk((long)sizeof(CTCBlock04_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block04, sizeof(CTCBlock04_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 5 allocation - ");
	if ( !(statPtr->card->block05 = (CTCBlock05_t *)sysbrk((long)sizeof(CTCBlock05_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block05, sizeof(CTCBlock05_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 6 allocation - ");
	if ( !(statPtr->card->block06 = (CTCBlock06_t *)sysbrk((long)sizeof(CTCBlock06_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block06, sizeof(CTCBlock06_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 7 allocation - ");
	if ( !(statPtr->card->block07 = (CTCBlock07_t *)sysbrk((long)sizeof(CTCBlock07_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block07, sizeof(CTCBlock07_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 8 allocation - ");
	if ( !(statPtr->card->block08 = (CTCBlock08_t *)sysbrk((long)sizeof(CTCBlock08_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block08, sizeof(CTCBlock08_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 9 allocation - ");
	if ( !(statPtr->card->block09 = (CTCBlock09_t *)sysbrk((long)sizeof(CTCBlock09_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block09, sizeof(CTCBlock09_t));
	kkprintf("OK\n");

	kkprintf("Ctc: Block 10 allocation - ");
	if ( !(statPtr->card->block10 = (CTCBlock10_t *)sysbrk((long)sizeof(CTCBlock10_t))) ) {
		/* Horrible, impossible failure. */
		kkprintf("FAILED\n");
		pseterr(ENOMEM);
		INST_FAIL();
		return((char*)SYSERR); /* -1 */
	}

	bzero((char*)statPtr->card->block10, sizeof(CTCBlock10_t));
	kkprintf("OK\n");

	/* 0x5 save info table pointer */
	statPtr->info = (void*)info;

	/* 0x6 */
	if (sizeof(CTCUserStatics_t) > 0) { /* only if it's not empty */
		kkprintf("Ctc: User-defined statics data table"
			 " allocation - ");
		if( !(statPtr->usrst = (CTCUserStatics_t*)
		      sysbrk( (long)sizeof(CTCUserStatics_t) )) ) {
			/* Horrible, impossible failure. */
			kkprintf("FAILED\n");
			pseterr(ENOMEM);
			INST_FAIL();
			return (char*)SYSERR; /* -1 */
		}
		bzero((char*)statPtr->usrst, sizeof(CTCUserStatics_t));
		kkprintf("OK\n");
	} else {
		statPtr->usrst = NULL;
	}

	/* user entry point function call */
	usrcoco = CtcUserInst(NULL, info, statPtr);
	if (usrcoco == (char*)SYSERR ) {
		INST_FAIL();
		CtcMemCleanup(statPtr); /* free allocated memory */
		return (char*)SYSERR; /* -1 */
	}

	INST_OK();
	return (char*)statPtr;
}


/**
 * @brief Driver uninstallation.
 *
 * @param statPtr -- statics table
 *
 * Deallocates shared memory and clears used interrupt vectors.
 *
 * @return OK     - if succeed.
 * @return SYSERR - in case of failure.
 */
int Ctc_uninstall(CTCStatics_t *statPtr)
{
	int rc;

	/*
	  Should we disable interrupts?
	  int ps;
	  disable(ps);
	*/

	kkprintf("Ctc: Uninstall...\n");

	/* user entry point function call */
	rc = CtcUserUnInst(NULL, statPtr);
	if (rc == SYSERR )
		return rc; /* -1 */

	/* cleanup and release statics table */
	CtcMemCleanup(statPtr);

	/*
	  Enable interrupts.
	  restore(ps);
	*/

	kkprintf("Ctc: Done\n\n");

	return OK;
}

/* CTC entry points */
#ifdef __linux__
struct dldd entry_points = {
	(int(*)(void*, int, struct cdcm_file*))
	/* open      */ Ctc_open,
	 (int(*)(void*, struct cdcm_file*))
	/* close     */ Ctc_close,
	 (int(*)(void*, struct cdcm_file*, char*, int))
	/* read      */ Ctc_read,
	 (int(*)(void*, struct cdcm_file*, char*, int))
	/* write     */ Ctc_write,
	 (int(*)(void*, struct cdcm_file*, int, struct cdcm_sel*))
	/* select    */ Ctc_select,
	 (int(*)(void*, struct cdcm_file*, int, char*))
	/* ioctl     */ Ctc_ioctl,
	 (char* (*)(void*))
	/* install   */ Ctc_install,
	 (int(*)(void*))
	/* uninstall */ Ctc_uninstall,
	/* memory-mapped devices access */ NULL
};
#else
struct dldd entry_points = {
  /* open      */ Ctc_open,
  /* close     */ Ctc_close,
  /* read      */ Ctc_read,
  /* write     */ Ctc_write,
  /* select    */ Ctc_select,
  /* ioctl     */ Ctc_ioctl,
  /* install   */ Ctc_install,
  /* uninstall */ Ctc_uninstall,
  /* memory-mapped devices access */ NULL
};
#endif
