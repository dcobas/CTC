#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include "CtcTest.h"

#ifdef __linux__
#define STDIN   0
#define STDOUT  1
#define STDERR  2
#endif

/* to suppress 'implicit declaration' warnings, and 'undefined vars' errors */
extern char *optarg;

/*==========================Global data=====================================*/
static char *curModName;	 /* Module Name exactly as in the DataBase */
METHOD      access_mode = IOCTL; /* default access method */
char        *ClearScreen;	 /* terminal clear screen esc sequence */
const char  *g_drvrVersInfo;     /* Module version info string */
int         g_drvrVers;		 /* Module driver version */
int         g_isDrvr;		 /* What is currently testing,
				    driver or simulator */

char block00RegData[BLOCK00_NUM_REGISTERS][2][MAX_STR] = {
  { "STATUS", "Hardware version" },
  { "CNTR_ENABLE", "Enable/disable output channel" },
};

char block01RegData[BLOCK01_NUM_REGISTERS][2][MAX_STR] = {
  { "confChan", "Select External start (i.e. input channel) and Clock1/Clock2 settings" },
  { "clock1Delay", "How many ticks to wait (i.e. delay) for the 1'st counter" },
  { "clock2Delay", "How many ticks to wait (i.e. delay) for the 2'nd counter" },
  { "outputCntr", "Output pulse counter that occured in the channel" },
  { "cntr1CurVal", "Current value of the 1'st counter" },
  { "cntr2CurVal", "Current value of the 2'nd counter" },
};

char block02RegData[BLOCK02_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_1", "All registers of channel 1" },
};

char block03RegData[BLOCK03_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_2", "All registers of channel 2" },
};

char block04RegData[BLOCK04_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_3", "All registers of channel 3" },
};

char block05RegData[BLOCK05_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_4", "All registers of channel 4" },
};

char block06RegData[BLOCK06_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_5", "All registers of channel 5" },
};

char block07RegData[BLOCK07_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_6", "All registers of channel 6" },
};

char block08RegData[BLOCK08_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_7", "All registers of channel 7" },
};

char block09RegData[BLOCK09_NUM_REGISTERS][2][MAX_STR] = {
  { "channel_8", "All registers of channel 8" },
};

char block10RegData[BLOCK10_NUM_REGISTERS][2][MAX_STR] = {
  { "ALL_CHANNELS", "All CTC channels to access in one chunk" },
};

char extraRegData[EXTRANEOUS_NUM_REGISTERS][2][MAX_STR] = {
};

char woRegData[WRITEONLY_NUM_REGISTERS][2][MAX_STR] = {
};

char serviceRegData[SERVICE_NUM_REGISTERS][2][MAX_STR] = {
  { "__SRV__DEBUG_FLAG", "Flag Register. Enable/Disable various driver flags" },
  { "__SRV__DEVINFO_T", "Full device information (info file)" },
  { "__SRV__DRVR_VERS", "Driver version info (current version number and version info string)" },
  { "__SRV__DAL_CONSISTENT", "driver/DAL consistency checking (comparision between the creation dates)" },
  { "__SRV__REP_REG_RW", "Special ioctl number for repetitive register r/w access" },
  { "__SRV__RW_BOUNDS", "Special ioctl number to enable/disable r/w bounds checking during ioctl call" },
};
/*==================END of global data=======================================*/

/*======================Static function declararion==========================*/
static void  DisplayUsage(char*);
static char* ParseProgArgs(int, char*[], int*, int*);
static int   SetReg(HANDLE, int);
static int   SetSrvRegInfo(HANDLE, int);
static int   GetReg(HANDLE, int);
static int   GetSrvRegInfo(HANDLE, int);
static void  DisplayRegSummaries(int, char(*)[2][MAX_STR]);
static void  PrntDbgFlag(int);
/*===================END of static function declaration======================*/

/**
 * @brief Guess what...
 *
 * @param progName -- program name
 */
static void DisplayUsage(char *progName)
{
	printf("Usage:  %s <module_name> -U<lun> -C<chan_num>"
	       " -a[ioctl | iommap | iodma]\n", progName);
	printf("where:\n");
	printf(" <module_name> %sOptional%s\n", WHITE_CLR, END_CLR);
	printf(" DataBase Module Name to test. Default (CTC) will be"
	       " used, if not provided.\n");
	printf(" -U %sCompulsory%s\n", WHITE_CLR, END_CLR);
	printf("    Provides the Logical Unit Number (LUN) of the device to"
	       " access.\n");
	printf("    In case of Driver Simulator should be negative.\n");
	printf(" -C %sOptional%s\n", WHITE_CLR, END_CLR);
	printf("    Provides the Minor Device Number (or Channel number).\n");
	printf("    There can be several entry points (Channel Numbers) for\n");
	printf("    current Logical Unit Number. If not provided - then"
	       " will be\n    set to zero.\n\n");
	printf("If Driver or Simulator is currently installed in the system,"
	       " then files like\n /dev/CTCD.lXX.cXX"
	       " (for Driver)\nor\n /dev/CTCS.lXX.cXX"
	       " (for Simulator)\nshould exist. These are module"
	       " Device/Simulator drivers. First 'XX' is a LUN.\n"
	       "Second 'XX' is a Minor Device Number (or Channel number)."
	       " Notice once more,\nthat if you are working with Simulator"
	       " ( i.e. /dev/CTC-SimLXXCXX ),\nthen '-U' option"
	       " parameter should be negative. As an example, let's"
	       " consider\nthat you've got /dev/CTC-SimL01C00. It is"
	       " a Simulator.\nLUN is '-1' (-U-1) and channel number is zero"
	       " '0' (-C0).\n\n");
	printf(" -a %sOptional%s\n", WHITE_CLR, END_CLR);
	printf("    Wich Access mode to use for Register accessing."
	       " Possible choices are:\n");
	printf("    -a[ioctl] %sDefault%s\n", WHITE_CLR, END_CLR);
	printf("      Registers accessed in a standard way using standard"
	       " 'ioctl' system call.\n");
	printf("    -a[iommap]\n");
	printf("      Registers accessed directly through the module Mapped"
	       " Memory. Can be\n");
	printf("      used only in case of real Driver but not in case of"
	       " Driver simulator.\n\n");
	printf("    -a[iodma] %sLinux only%s\n"
	       "      Registers accessed using DMA.", WHITE_CLR, END_CLR);
	printf("EXAMPLES: %s CIBC -U0\n", progName);
	printf("          %s VXI_MUX_HF -U-2 -C4 -aiommap\n", progName);
	printf("          %s TG8 -U3 -aioctl\n\n", progName);
}

/**
 * @brief Checks if command line arguments are correct.
 *
 * @param argc  -- command line arg number.
 * @param argv  -- command line arg.
 * @param lun   -- Logical Unit Number goes here.
 * @param chanN -- Minor Device Number (Channel Number) goes here.
 *
 * @return Module Name (exactly as in the DataBase)
 * @return Logical Unit Number in @b lun param.
 * @return Channel number in @b chanN param.\n
 *         Terminate programm in case of error. Printing out error info
 *         to stderr.
 */
static char* ParseProgArgs(int argc, char *argv[], int *lun, int *chanN)
{
	static char modName[MAX_STR] = "CTC"; /* default module name */
	int         argCntr = 0; /* counter of non-option args */
	int         cmdOpt;

	/* set default params */
	*lun   = NO_LUN;
	*chanN = 0;	/* channel number */

	/* Scan params of the command line */
	while ((cmdOpt = getopt(argc, argv, "-ha:U:C:")) != EOF) {
		switch (cmdOpt) {
		case 0:	/* this is non-option arg, i.e. module name */
			if (argCntr) {
				fprintf(stderr,
					"Too many module names detected.\n");
				goto wrongOption;
			}
			strncpy(modName, optarg, MAX_STR);
			argCntr++;
			break;
		case 'h':	/* Help information */
			DisplayUsage(argv[0]);
			exit(EXIT_SUCCESS); /* 0 */
			break;
		case 'a':	/* register access mode option */
			if (!strcmp(optarg, "ioctl"))
				access_mode = IOCTL;
			else if (!strcmp(optarg, "iommap"))
				access_mode = IOMMAP;
			else if (!strcmp(optarg, "iodma"))
                                access_mode = IODMA;
			else {
				fprintf(stderr,
					"Invalid argument -l%s\n.", optarg);
				goto wrongOption;
			}
			break;
		case 'U':	/* Logical Unit Number. Compulsory option. */
			*lun = atoi(optarg);
			break;
		case 'C':	/* Channel Number. (Minor device number). */
			*chanN = atoi(optarg);
			break;
		case '?':	/* wrong option */
		wrongOption:
			fprintf(stderr,
				"Try \'%s -h\' for more information.\n",
				argv[0]);
		exit(EXIT_FAILURE); /* 1 */
		break;
		default:
			break;
		}
	}

	if (*lun == NO_LUN) { /* Not set by command args. error */
		fprintf(stderr, "LUN is expected.\n");
		goto wrongOption;
	}

	if ((*lun < 0) && (access_mode == IOMMAP)) { /* unacceptable mode */
		fprintf(stderr, "Driver Simulators can not be tested using"
			" mmaped I/O access.\n");
		goto wrongOption;
	}

	return modName;
}


/**
 * @brief Function provides the register setting along with light graphical
 *        interface for user.
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess call
 * @param rId    -- Register ID
 *
 * @return -1 - in case of error.
 * @return  0 - otherwise.
 */
static int SetReg(HANDLE handle, int rId)
{
	unsigned int	 nmemb;	/* number of elements in register */
	REGSIZE	 elSize; /* register element size in bytes */
	unsigned long  newVal; /*  */
	unsigned char  choise; /*  */
	int		 da; /* (Digit Amount) number of digits to print */
	int		 count, j, i; /* counters */
	void		 *resPtr = NULL;
	ARIGHTS        amod;	/* register access rights */
	int		 retCode;
	/* for now only three register types are possible */
	unsigned char  *charReg  = NULL;
	unsigned short *shortReg = NULL;
	unsigned long  *longReg  = NULL;

	if (rId < LAST_SRV_REG)
		return SetSrvRegInfo(handle, rId); /* service register */

	amod = DaGetRegAccessRights(handle, rId);
	if (amod & (AMRD | AMEX)) { /*===REGISTER CAN BE READ====*/
		/* register can be read. In this case first display
		   current register value and then propose user to change it */
		nmemb  = DaGetRegDepth(handle, rId);
		elSize = DaGetRegSize(handle, rId);
		resPtr = calloc(nmemb, elSize);
		if ((retCode = DaGetRegister(handle, rId, resPtr,
					     (elSize*nmemb))) <= 0) {
			printf("\n\n%s: ERROR! CAN'T GET CURRENT REGISTER"
			       " VALUE! (coco %d)\n\n<enter> to continue",
			       DaGetRegName(handle, rId), retCode);
			getchar();
			return -1; /* error */
		}

		/* init pointers */
		charReg  = (unsigned char*) resPtr;
		shortReg = (unsigned short*)resPtr;
		longReg  = (unsigned long*) resPtr;

		da = elSize * 2; /* char - 2 hex digit, short - 4,
				    long - 8 hex digits */

		if (nmemb == 1) { /* register consists only of one element */
			printf("--> %s = 0x%0*X\n",
			       DaGetRegName(handle, rId), da,
			       (unsigned int)((elSize==SZCHAR)?*charReg:(elSize==SZSHORT)?*shortReg:*longReg));
			printf("\n  Please enter the desired value in"
			       " hexadecimal > 0x");
			scanf("%lx", &newVal);
			if (elSize == SZCHAR)
				*charReg  = (char)newVal;
			else if (elSize == SZSHORT)
				*shortReg = (short)newVal;
			else
				*longReg  = (long)newVal;
			getchar();
			DaSetRegister(handle, rId, resPtr, elSize);
		} else {	/* register consists from several elements */
			int upper, lower;

			/* init values for drawing */
			count = 0;
			lower = 0;
			upper = MIN(3, nmemb);

			do {	/* here comes value depicting */
				printf("%s", ClearScreen);

				for (j = 2; j > count; j--)
					printf("\n");

				for (j = lower; j < count; j++)
					printf("    %s[%04d] = 0x%0*X\n",
					       DaGetRegName(handle, rId), j, da,
					       (unsigned int)((elSize==SZCHAR)?charReg[j]:(elSize==SZSHORT)?shortReg[j]:longReg[j]));

				printf("\n--> %s[%04d] = 0x%0*X\n\n",
				       DaGetRegName(handle, rId), count,
				       da, (unsigned int)((elSize==SZCHAR)?charReg[count]:(elSize==SZSHORT)?shortReg[count]:longReg[count]));

				for (j = (count + 1); j < upper; j++)
					printf("    %s[%04d] = 0x%0*X\n",
					       DaGetRegName(handle, rId), j, da,
					       (unsigned int)((elSize==SZCHAR)?charReg[j]:(elSize==SZSHORT)?shortReg[j]:longReg[j]));

				for (j = (count - nmemb + 3); j > 0; j--)
					printf("\n");

				printf("\n-----\n");
				printf("'n' - set new value for %s[%04d]\n",
				       DaGetRegName(handle, rId), count);
				printf("'.' - set new value for"
				       " %s[%04d - %04d] and quit\n",
				       DaGetRegName(handle, rId), count,
				       nmemb-1);
				printf("'s' - skip element\n");
				printf("'g' - goto element\n");
				printf("'q' - save and quit\n");
				printf("'a' - abort\n\n> ");
				choise = getchar();

				if (choise == 'n') {
					printf("\n  %s[%04d] new value = 0x",
					       DaGetRegName(handle, rId),
					       count);
					scanf("%lx", &newVal);
					if (elSize == SZCHAR)
						charReg[count]  = (char)newVal;
					else if (elSize == SZSHORT)
						shortReg[count] = (short)newVal;
					else
						longReg[count]  = (long)newVal;
				}

				if (choise == '.') {
					printf("\n  %s[%04d - %04d] new"
					       " value = 0x",
					       DaGetRegName(handle, rId),
					       count, nmemb-1);
					scanf("%lx", &newVal);
					if (elSize == SZCHAR)
						charReg[count]  = (char)newVal;
					else if (elSize == SZSHORT)
						shortReg[count] = (short)newVal;
					else
						longReg[count]  = (long)newVal;
				}

				if (choise == 'g') {
					printf("\n  goto which element > ");
					scanf("%ld", &newVal);
					if (newVal < 0 ||
					    newVal > (nmemb - 1)) {
						printf("\n  element out of"
						       " range. <enter> to"
						       " continue");
						getchar();
					} else {
						count = newVal;
						lower = MAX(0, count - 2);
						upper = MIN(nmemb, count + 3);
					}
					getchar();
				}

				if (choise == 'n' || choise == 's') {
					if (count > (lower + 1))
						lower++;

					if ((nmemb - 1) >= (count + 3))
						upper++;

					count++;
					getchar();
				}

			} while ((count < nmemb) && (choise != '.')
				 && (choise != 'q') && (choise != 'a'));

			if (choise == '.')
				for (i = count; i < nmemb; i++) {
					if (elSize == SZCHAR)
						charReg[i]  = charReg[count];
					else if (elSize == SZSHORT)
						shortReg[i] = shortReg[count];
					else
						longReg[i]  = longReg[count];
				}
			if (choise != 'a')
				DaSetRegister(handle, rId, resPtr,
					      (nmemb * elSize));
		}
	} else { /*=============REGISTER CAN'T BE READ======================*/
		/* register can't be read. In this case user can't get
		   current register value and should set all register elements
		   without knowing their current values. If some elements
		   will not be set by user, they will be set to zero
		   automatically. */
		nmemb  = DaGetRegDepth(handle, rId);
		elSize = DaGetRegSize(handle, rId);
		resPtr = calloc(nmemb, elSize);

		/* init pointers */
		charReg  = (unsigned char*) resPtr;
		shortReg = (unsigned short*)resPtr;
		longReg  = (unsigned long*) resPtr;

		if (nmemb == 1) { /* register consists only of one element */
			printf("\n  Please enter the desired value in"
			       " hexadecimal > 0x");
			scanf("%lx", &newVal);
			if (elSize == SZCHAR)
				*charReg  = (char)newVal;
			else if (elSize == SZSHORT)
				*shortReg = (short)newVal;
			else
				*longReg  = (long)newVal;
			getchar();
			DaSetRegister(handle, rId, resPtr, elSize);
		} else {	/* register consists from several elements */
			count = 0;

			do {
				printf("%s", ClearScreen);
				printf("\nRegister can not be read. Can't"
				       " obtain current values.\nIf some of"
				       " [%04d - %04d] elements will not be"
				       " set, they will be truncated\nto zero"
				       " automatically.", 0, (nmemb -1));

				printf("\n-----\n");
				printf("'n' - set new value for %s[%04d]\n",
				       DaGetRegName(handle, rId), count);
				printf("'.' - set new value for"
				       " %s[%04d - %04d] and quit\n",
				       DaGetRegName(handle, rId), count,
				       nmemb-1);
				printf("'s' - skip element\n");
				printf("'g' - goto element\n");
				printf("'q' - save and quit\n");
				printf("'a' - abort\n\n> ");
				choise = getchar();

				if (choise == 'n') {
					printf("\n  %s[%04d] new value = 0x",
					       DaGetRegName(handle, rId),
					       count);
					scanf("%lx", &newVal);
					if (elSize == SZCHAR)
						charReg[count]  = (char)newVal;
					else if (elSize == SZSHORT)
						shortReg[count] = (short)newVal;
					else
						longReg[count]  = (long)newVal;
				}

				if (choise == '.') {
					printf("\n  %s[%04d - %04d] new"
					       " value = 0x",
					       DaGetRegName(handle, rId),
					       count, nmemb-1);
					scanf("%lx", &newVal);
					if (elSize == SZCHAR)
						charReg[count]  = (char)newVal;
					else if (elSize == SZSHORT)
						shortReg[count] = (short)newVal;
					else
						longReg[count]  = (long)newVal;
				}

				if (choise == 'g') {
					printf("\n  goto which element > ");
					scanf("%ld", &newVal);
					if (newVal < 0 ||
					    newVal > (nmemb - 1)) {
						printf("\n  element out of"
						       " range. <enter> to"
						       " continue");
						getchar();
					} else
						count = newVal;
					getchar();
				}

				if (choise == 'n' || choise == 's') {
					count++;
					getchar();
				}
			} while ((count < nmemb) && (choise != '.')
				 && (choise != 'q') && (choise != 'a'));

			if (choise == '.')
				for (i = count; i < nmemb; i++) {
					if (elSize == SZCHAR)
						charReg[i]  = charReg[count];
					else if (elSize == SZSHORT)
						shortReg[i] = shortReg[count];
					else
						longReg[i]  = longReg[count];
				}
			if (choise != 'a')
				DaSetRegister(handle, rId, resPtr,
					      (nmemb * elSize));
		}
	} /* end of else */

	free(resPtr);
	return 0;
}


/**
 * @brief Set (and printout if needed) service register information.
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess call
 * @param rId    -- Register ID
 *
 * These regs are of the same format and content for all of the
 * driverGen-generated drivers.
 *
 * @return -1 - in case of error.
 * @return  0 - otherwise.
 */
static int SetSrvRegInfo(HANDLE handle, int rId)
{
	char  bitstr[64];
	char  errstr[64];
	int   tbits[32]; /* all flag bits */
	int   tbam = 0;  /* detected amount user wants to change */
	char *bsptr;
	int   cntr;
	int   dflg = DaDbgFlag(handle, 0, OPRD);

	/* service register is allowed to be modified */
	switch (rId) {
	case __SRV__DEBUG_FLAG_ID:
	doagain:
		memset(bitstr, 0, sizeof(bitstr));
		memset(errstr, 0, sizeof(errstr));
		PrntDbgFlag(dflg);
		printf("\nMultiple toggle bits can be provided.\nUse white"
		       " space to separate them.\nBits to toggle"
		       " (enter to quit) > ");

		/* here some magic comes */
		scanf("%[ 0-9]", bitstr);   /* get digits only */
		scanf("%[ 0-9a-zA-Z,;:/.-]", errstr); /* store erroneous
							 input */
		scanf("%*[^\n]"); /* skip to the end of the line */
		getchar(); /* swallow \n */

		if (errstr[0]) {		/* erroneous input detected */
			printf("Wrong input!\n<enter> to continue");
			getchar();
			goto doagain;
		}

		if (bitstr[0] == 0 || bitstr[0] == ' ') /* user wants out */
			return 0;

		bsptr = strtok(bitstr, " ");
		tbam = 0;
		while (bsptr) {
			tbits[tbam] = atoi(bsptr);
			/* check if within allowable toggle bit range */
			if (!WITHIN_RANGE(1, tbits[tbam], DA_DBG_LAST)) {
				printf("Can't modify bit %d!\n\n<enter> to"
				       " continue", tbits[tbam]);
				getchar();
				goto doagain;
			}
			++tbam;
			bsptr = strtok(NULL, " ");
		}

		/* set toggle bits */
		for (cntr = 0, dflg = 0; cntr < tbam; cntr++)
			dflg |= (1 << tbits[cntr]);

		DaDbgFlag(handle, dflg, OPWR); /* set new value */
		dflg = DaDbgFlag(handle, 0, OPRD);
		PrntDbgFlag(dflg);
		printf("\n<enter> to continue");
		getchar();
		goto doagain;
		break;
	default:
		return -1;
	}

	return 0; /* we cool */
}


/**
 * @brief Function obtains of the register value and print it out to the user.
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess call
 * @param rId    -- Register ID
 *
 * @return -1 - in case of error.
 * @return  0 - otherwise.
 */
static int GetReg(HANDLE handle, int rId)
{
	unsigned int	 nmemb; /* number of elements in register */
	REGSIZE	 elSize; /* register element size in bytes */
	unsigned char  choise;
	int		 da; /* (Digit Amount) number of digits to print */
	void		 *resPtr = NULL;
	int		 retCode;
	/* for now only three register types are possible */
	unsigned char  *charReg  = NULL;
	unsigned short *shortReg = NULL;
	unsigned long  *longReg  = NULL;

	if (rId < LAST_SRV_REG)
		return GetSrvRegInfo(handle, rId);  /* service register */

	nmemb  = DaGetRegDepth(handle, rId);
	elSize = DaGetRegSize(handle, rId);
	resPtr = calloc(nmemb, elSize);
	if ((retCode = DaGetRegister(handle, rId, resPtr, (elSize*nmemb)))
	    <= 0) {
		printf("\n\n%s: ERROR! CAN'T GET REGISTER!"
		       " (coco %d)\n\n<enter> to continue",
		       DaGetRegName(handle, rId), retCode);
		getchar();
		return -1; /* error */
	}

	/* init pointers */
	charReg  = (unsigned char*)resPtr;
	shortReg = (unsigned short*)resPtr;
	longReg  = (unsigned long*)resPtr;

	da = elSize * 2; /* char - 2 hex digit, short - 4,
			    long - 8 hex digits */

	if (nmemb == 1) {	/* register consists only of one element */
		printf("\n\n%s: 0x%0*X\n\n<enter> to continue",
		       DaGetRegName(handle, rId), da
		       , (unsigned int)((elSize==SZCHAR)?*charReg:(elSize==SZSHORT)?*shortReg:*longReg));
		getchar();
	} else {	/* register consists from several elements */
		int count, i;

		count = 0;

		do {
			printf("%s", ClearScreen);
			for (i = 0; count <  nmemb &&
				     (i < screenHeight() - 2); i++) {
				printf("%s[%04d]: 0x%0*X\n",
				       DaGetRegName(handle, rId), count, da,
				       (unsigned int)((elSize==SZCHAR)?charReg[count]:(elSize==SZSHORT)?shortReg[count]:longReg[count]));
				count++;
			}

			if (count != nmemb)
				printf("\n<enter> to continue -- q <enter> to"
				       " quit ");
			else
				printf("\n<enter> to continue ");

			choise = getchar();
		} while ((count < nmemb) && (choise != 'q'));
	}

	free(resPtr);
	return 0;
}


/**
 * @brief Get and printout service register information.
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess call
 * @param rId    -- Register ID
 *
 * These regs are of the same format and content for all of the
 * driverGen-generated drivers.
 *
 * @return -1 - in case of error.
 * @return  0 - otherwise.
 */
static int GetSrvRegInfo(HANDLE handle, int rId)
{
	const char  *vs;    /* DAL/driver version string pointer */
	time_t  dal_gt, drvr_gt; /* DAL and driver generation time */
	char *dal_t_str, *drvr_t_str; /* generation time human representation */
	int dv;    /* driver version */
	int isCons; /* if driver<->dal are consistent */

	switch (rId) {
	case __SRV__DEBUG_FLAG_ID:
		PrntDbgFlag(DaDbgFlag(handle, 0, OPRD));
		printf("\n<enter> to continue");
		getchar();
		break;
	case __SRV__DEVINFO_T_ID:
		printf("Not implemented yet!\n\n<enter> to continue");
		getchar();
		break;
	case __SRV__DRVR_VERS_ID:
		dv = DaDrvrVers(handle, &vs, NULL);
		printf("\n\n[Drvr Version] -> %d\n[Drvr String]  ->"
		       " %s\n\n<enter> to continue", dv, vs);
		getchar();
		break;
	case __SRV__DAL_CONSISTENT_ID:
		DaDrvrVers(handle, NULL, &drvr_gt);
		isCons = DaLibVers(handle, &vs, &dal_gt);

		dal_t_str = ctime(&dal_gt);
		dal_t_str[strlen(dal_t_str)-1] = 0; /* get rid of '\n' */

		drvr_t_str = ctime(&drvr_gt);
		drvr_t_str[strlen(drvr_t_str)-1] = 0; /* get rid of '\n' */

		printf("\n\n[DAL Version]       -> %s\n[DAL Generated on]"
		       "  -> %s\n\n", vs, dal_t_str);
		if (isCons == DA_ERR_ALL_OK)
			printf("OK: DAL <--> %s are consistent.\nI.e. driver"
			       " source code was generated (on %s) AFTER DAL"
			       " was generated\n",
			       (g_isDrvr)?"Driver":"Simulator", drvr_t_str);
		else if (isCons == -DA_ERR_INCOMPAT)
			printf("NOT OK: DAL <--> %s are NOT consistent!\nI.e."
			       " driver source code was generated (on %s)"
			       " BEFORE DAL was generated.\nYou should"
			       " regenerate it using latest driverGen"
			       " version.\n",
			       (g_isDrvr)?"Driver":"Simulator", drvr_t_str);
		else
			printf("ERROR: Can't perform consistency"
			       " checking: [%s] -> %s\n",
			       DaGetErrCode(isCons)->da_err_code,
			       DaGetErrCode(isCons)->da_err_str);

		printf("\n<enter> to continue");
		getchar();
		break;
	default:
		return -1;	/* normally not reached */
	}

	return 0;
}


/**
 * @brief Compute MAX field lengths of the register name
 *
 * @param numReg  --
 * @param regDesc --
 *
 * @return maximum string lengths.
 */
static int ComputeRegNameFieldSize(int  numReg, char regDesc[][2][MAX_STR])
{
	int max_sz, cntr, sz;

	max_sz = strlen("Reg Name");

	for (cntr = 0; cntr < numReg; cntr++)
		if ( (sz = strlen(regDesc[cntr][0])) > max_sz)
			max_sz = sz;

	return max_sz;
}


/**
 * @brief Displays register summaries using low level @e graphics
 *
 * @param numReg  --
 * @param regDesc --
 */
static void DisplayRegSummaries(int  numReg, char regDesc[][2][MAX_STR])
{
	char format[32];
	int  i, count = 0, rcntr;
	int sz = ComputeRegNameFieldSize(numReg, regDesc);

	/* setup format string */
	sprintf(format, "%%-%ds | %%.%ds\n", sz, screenWidth() - sz - 3);

	/* print first page header */
	printf("%-*s | Comment\n", sz, "Reg Name");
	for (i = 0; i < sz+1; i++)
		printf("-");
	printf("+");
	for (i = 0; i < (screenWidth() - sz - 2); i++)
		printf("-");
	printf("\n");


	/* loop through all the registers */
	for (rcntr = 0; rcntr < numReg; rcntr++) {
		printf(format, regDesc[rcntr][0], regDesc[rcntr][1]);
		count++;

		if ( (count == screenHeight() - 4) && (rcntr != numReg - 1) ) {
			printf("\n<enter> to continue");
			getchar();
			printf("%s", ClearScreen);
			count = 0;

			/* printout nex page header */
			printf("%-*s | Comment\n", sz, "Reg Name");
			for (i = 0; i < sz+1; i++)
				printf("-");
			printf("+");
			for (i = 0; i < (screenWidth() - sz - 2); i++)
				printf("-");
			printf("\n");
		}
	}

	printf("\n<enter> to continue");
	getchar();
}


/* get bit position from number */
int bp(int x)
{
	int p = 0;
	for(;;) {
		if (x & 1<<p)
			break;
		++p;
	}
	return p;
}

/**
 * @brief Formatted printout of current debug flag bits.
 *
 * @param flg -- debug flag to print
 */
static void PrntDbgFlag(int flg)
{
  char *horizont = "\n+--------+---------+-----+----+"
	  "+-----+-----+------+----+----+----+\n";

  printf("%s", ClearScreen);

  printf("%s| Bits   | 31 - %2d | %2d  | %2d || %2d  | %2d  | %2d   | %2d | %2d |  0 |%s| Name   | reserv  | RWO | EI || DIT | PTS | PIOC | PD | PI | ER |%s| ON/OFF | reserv  |  %c  |  %c ||  %c  |  %c  |   %c  |  %c |  %c |  %c |%sDriver Flags:\n[ 0] ER   - ERror/invalid state (read-only)\n[%2d] PI   - Trace Installation\n[%2d] PD   - Trace Deinstallation\n[%2d] PIOC - Trace IOCTL calls\n[%2d] PTS  - Trace register access Time Statistics\n[%2d] DIT  - Printout all Device Info Table (during istall only)\n\nDAL Flags:\n[%2d] EI  - print error info\n[%2d] RWO - trace dma read/write operations\n",
	 horizont, DA_DBG_LAST+1, bp(DA_DBG_DMA_RW_OPS), bp(DA_DBG_ERR_INFO),
	 bp(DBGALLDI), bp(DBGTIMESTAT), bp(DBGIOCTL), bp(DBGDEINSTL),
	 bp(DBGINSTL), horizont, horizont, (flg&DA_DBG_DMA_RW_OPS)?'X':' ',
	 (flg&DA_DBG_ERR_INFO)?'X':' ', (flg&DBGALLDI)?'X':' ',
	 (flg&DBGTIMESTAT)?'X':' ', (flg&DBGIOCTL)?'X':' ',
	 (flg&DBGDEINSTL)?'X':' ', (flg&DBGINSTL)?'X':' ',
	 (flg&DBGERR)?'X':' ', horizont,
	 bp(DBGINSTL), bp(DBGDEINSTL), bp(DBGIOCTL), bp(DBGTIMESTAT),
	 bp(DBGALLDI), bp(DA_DBG_ERR_INFO), bp(DA_DBG_DMA_RW_OPS));
}


/**
 * @brief Calculates screen height.
 *
 * @return Screen height.
 */
int screenHeight()
{
	struct winsize termsize;

	if (ioctl(STDOUT, TIOCGWINSZ, (char *) &termsize) == -1)
		return(24);
	else if (termsize.ws_row < 1)
		return(24);
	else
		return(termsize.ws_row);
}


/**
 * @brief Calculates screen width.
 *
 * @return Screen width.
 */
int screenWidth()
{
	struct winsize termsize;

	if (ioctl(STDOUT, TIOCGWINSZ, (char *)&termsize) == -1)
		return(80);
	else if (termsize.ws_col < 1)
		return(80);
	else
		return(termsize.ws_col);
}


int main(int argc, char *argv[], char *envp[])
{
	char   termcapEntry[1024];
	char   termcapValueHold[64];
	char   *termcapValue;
	char   *terminalType;
	int    choice;
	char   *modName; /* Module Name (exactly as in the DataBase) */
	int    lun;      /* Logical Unit Number (Major Number) */
	int    chanN;    /* Minor Device Number (Channel Number) */
	HANDLE handle;   /* library handle */
	int    i;

	termcapValue = (char*)termcapValueHold;
	terminalType = (char*)getenv("TERM");

	tgetent(termcapEntry, terminalType);
	ClearScreen = (char*)tgetstr("cl", &termcapValue);

	modName = ParseProgArgs(argc, argv, &lun, &chanN);

	/* init global vars */
	curModName = strdup(modName);
	g_isDrvr = (lun >= 0)? 1 : 0;

	printf("Test %s module with LUN %d\n", curModName, lun);
	handle = DaEnableAccess(curModName, access_mode, lun, chanN);
	if (handle < 0) {
		fprintf(stderr, "Unable to test %s module.\n", curModName);
		exit(EXIT_FAILURE); /* 1 */
	}

	/* get driver version and version info string */
	g_drvrVers = DaDrvrVers(handle, &g_drvrVersInfo, NULL);

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Main Menu\n[%s]\n",
		       ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator", g_drvrVers,
		       g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");
		printf("01 - Exit\n");
		printf("02 - User Defined Menu\n");
		printf("03 - Service Registers\n");
		printf("04 - Extraneous Registers\n");
		printf("05 - WriteOnly Registers history buffer"
		       " (last written values)\n");
    printf("06 - Block 0\n");
    printf("07 - Block 1\n");
    printf("08 - Block 2\n");
    printf("09 - Block 3\n");
    printf("10 - Block 4\n");
    printf("11 - Block 5\n");
    printf("12 - Block 6\n");
    printf("13 - Block 7\n");
    printf("14 - Block 8\n");
    printf("15 - Block 9\n");
    printf("16 - Block 10\n");
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      exit(1);
    case 2:
      UserDefinedMenu(handle, lun);
      break;
    case 3:
      ServiceRegsOps(handle);
      break;
    case 4:
      Extraneous(handle);
      break;
    case 5:
      WORegsMenu(handle);
      break;
    case 6:
      Block00(handle);
      break;
    case 7:
      Block01(handle);
      break;
    case 8:
      Block02(handle);
      break;
    case 9:
      Block03(handle);
      break;
    case 10:
      Block04(handle);
      break;
    case 11:
      Block05(handle);
      break;
    case 12:
      Block06(handle);
      break;
    case 13:
      Block07(handle);
      break;
    case 14:
      Block08(handle);
      break;
    case 15:
      Block09(handle);
      break;
    case 16:
      Block10(handle);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  } /* end of 'for' */

  free(curModName);
  DaDisableAccess(handle); /* disconnect from the DAL library */
  return(EXIT_SUCCESS);	/* 0 */
}


/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int WORegsMenu(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;
	unsigned char  *charPointer;
	unsigned short *shortPointer;
	unsigned long  *longPointer;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	charPointer  = NULL;
	shortPointer = NULL;
	longPointer  = NULL;
	format[0]    = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - WriteOnly Registers"
		       " history buffer (last written values)\n[%s]\n",
		       ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

		if (access_mode == IOMMAP) {
			printf("Can't read WO register last history using"
			       " IOMMAP access method.\n\n<enter> to continue");
			getchar();
			return OK;
		}

    printf("This module does not have Write Only registers\n\n<enter> to continue");
    getchar();
    break;
  }

  return 1;
}



/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Extraneous(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;
	unsigned char  *charPointer;
	unsigned short *shortPointer;
	unsigned long  *longPointer;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	charPointer  = NULL;
	shortPointer = NULL;
	longPointer  = NULL;
	format[0]    = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Extraneous"
		       " Registers\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("This module does not have extraneous registers\n\n<enter>"
	   " to continue");
    getchar();
    break;
  }

  return 1;
}

/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int ServiceRegsOps(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Service"
		       " Registers\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator", g_drvrVers,
		       g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");
    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read __SRV__DEBUG_FLAG\n", 3);
    printf("%03d - Write __SRV__DEBUG_FLAG\n", 4);
    printf("%03d - Read __SRV__DEVINFO_T\n", 5);
    printf("%03d - Read __SRV__DRVR_VERS\n", 6);
    printf("%03d - Read __SRV__DAL_CONSISTENT\n", 7);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(SERVICE_NUM_REGISTERS, serviceRegData);
      break;
    case 3:
      GetReg(handle, __SRV__DEBUG_FLAG_ID);
      break;
    case 4:
      SetReg(handle, __SRV__DEBUG_FLAG_ID);
      break;
    case 5:
      GetReg(handle, __SRV__DEVINFO_T_ID);
      break;
    case 6:
      GetReg(handle, __SRV__DRVR_VERS_ID);
      break;
    case 7:
      GetReg(handle, __SRV__DAL_CONSISTENT_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block00(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 0\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read STATUS\n", 3);
    printf("%03d - Read CNTR_ENABLE\n", 4);
    printf("%03d - Write CNTR_ENABLE\n", 5);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK00_NUM_REGISTERS, block00RegData);
      break;
    case 3:
      GetReg(handle, STATUS_ID);
      break;
    case 4:
      GetReg(handle, CNTR_ENABLE_ID);
      break;
    case 5:
      SetReg(handle, CNTR_ENABLE_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block01(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 1\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read confChan\n", 3);
    printf("%03d - Write confChan\n", 4);
    printf("%03d - Read clock1Delay\n", 5);
    printf("%03d - Write clock1Delay\n", 6);
    printf("%03d - Read clock2Delay\n", 7);
    printf("%03d - Write clock2Delay\n", 8);
    printf("%03d - Read outputCntr\n", 9);
    printf("%03d - Read cntr1CurVal\n", 10);
    printf("%03d - Read cntr2CurVal\n", 11);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK01_NUM_REGISTERS, block01RegData);
      break;
    case 3:
      GetReg(handle, CONFCHAN_ID);
      break;
    case 4:
      SetReg(handle, CONFCHAN_ID);
      break;
    case 5:
      GetReg(handle, CLOCK1DELAY_ID);
      break;
    case 6:
      SetReg(handle, CLOCK1DELAY_ID);
      break;
    case 7:
      GetReg(handle, CLOCK2DELAY_ID);
      break;
    case 8:
      SetReg(handle, CLOCK2DELAY_ID);
      break;
    case 9:
      GetReg(handle, OUTPUTCNTR_ID);
      break;
    case 10:
      GetReg(handle, CNTR1CURVAL_ID);
      break;
    case 11:
      GetReg(handle, CNTR2CURVAL_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block02(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 2\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_1\n", 3);
    printf("%03d - Write channel_1\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK02_NUM_REGISTERS, block02RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_1_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_1_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block03(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 3\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_2\n", 3);
    printf("%03d - Write channel_2\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK03_NUM_REGISTERS, block03RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_2_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_2_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block04(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 4\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_3\n", 3);
    printf("%03d - Write channel_3\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK04_NUM_REGISTERS, block04RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_3_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_3_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block05(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 5\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_4\n", 3);
    printf("%03d - Write channel_4\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK05_NUM_REGISTERS, block05RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_4_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_4_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block06(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 6\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_5\n", 3);
    printf("%03d - Write channel_5\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK06_NUM_REGISTERS, block06RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_5_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_5_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block07(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 7\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_6\n", 3);
    printf("%03d - Write channel_6\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK07_NUM_REGISTERS, block07RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_6_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_6_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block08(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 8\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_7\n", 3);
    printf("%03d - Write channel_7\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK08_NUM_REGISTERS, block08RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_7_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_7_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block09(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 9\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read channel_8\n", 3);
    printf("%03d - Write channel_8\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK09_NUM_REGISTERS, block09RegData);
      break;
    case 3:
      GetReg(handle, CHANNEL_8_ID);
      break;
    case 4:
      SetReg(handle, CHANNEL_8_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
/**
 * @brief
 *
 * @param handle -- lib handle, ret. by @e DaEnableAccess() call
 *
 * @return
 */
int Block10(HANDLE handle)
{
	int		 i, j;
	int		 choice;
	int		 result;
	int		 count;
	char		 format[32];
	int		 upper, lower;
	unsigned char  charResult;
	unsigned short shortResult;
	unsigned long  longResult;

	/* just to suppress 'unused variable' warnings */
	i=j=choice=result=count=upper=lower=charResult=shortResult=longResult=0;
	format[0] = 0;

	for (;;) {
		printf("%s%s %s <V. %d> Test Program - Block"
		       " 10\n[%s]\n", ClearScreen, curModName,
		       (g_isDrvr)?"Driver":"Simulator",
		       g_drvrVers, g_drvrVersInfo);
		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");

    printf("%03d - Return to Main Menu\n", 1);
    printf("%03d - Register summaries\n", 2);
    printf("%03d - Read ALL_CHANNELS\n", 3);
    printf("%03d - Write ALL_CHANNELS\n", 4);
    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 1:
      return(OK);
      break;
    case 2:
      DisplayRegSummaries(BLOCK10_NUM_REGISTERS, block10RegData);
      break;
    case 3:
      GetReg(handle, ALL_CHANNELS_ID);
      break;
    case 4:
      SetReg(handle, ALL_CHANNELS_ID);
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }
  }
}
