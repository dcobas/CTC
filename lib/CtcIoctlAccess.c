#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdlib.h>
#include "CtcDrvr.h"

#define _DRVR_NM_ "CTC"

#define matchEndian(bytes, size, depth) { }

/**
 * @brief Open device driver node
 *
 * @param lun   -- Logical Unit Number assigned to the module. Negative in case
 *                 of driver simulator
 * @param chanN -- Minor Device Number. There can be several entry points for
 *                 current Logical Unit Number (ChannelNumber).
 *
 * @return Open file decriptor(normally >= 3) - if success.
 * @return -1 - if fails. Error message is printing out.
 */
int CtcEnableAccess(int lun, int chanN)
{
	int  fd;			/* open file descriptor */
	char fileName[0x100];		/* device file name */
	char *tmp;

	if (!MODULE_NAME_OK(_DRVR_NM_)) {
		fprintf(stderr, "Spurious Module Name '%s'.\n"
			"Normally _should not_ contain any lowercase"
			" letters!\n",  _DRVR_NM_);
		return -1;
	}

	tmp = _ncf(_DRVR_NM_);
	sprintf(fileName, "/dev/" NODE_NAME_FMT,
		tmp, (lun < 0)?_SIML_:_DRVR_, abs(lun), chanN);
	free(tmp);
	if ((fd = open(fileName, O_RDWR)) < 0) { /* error */
		perror(NULL);
		fprintf(stderr, "Error [%s] in CtcEnableAccess()"
			" while opening '%s' file.\nCheck if '%s' module is"
			" installed.\n", strerror(errno), fileName, _DRVR_NM_);
		return -1;
	}

	return fd;
}


/**
 * @brief  Close driver file descriptor.
 *
 * @param fd -- open file descriptor, retuned by CtcEnableAccess call
 *
 * @return void
 */
void CtcDisableAccess(int fd)
{
	close(fd);
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetSTATUS(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_STATUS, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetCNTR_ENABLE(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CNTR_ENABLE, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetCNTR_ENABLE(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = 1;		    /* number of elements write */
	arguments[2] = 0;		    /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CTC_SET_CNTR_ENABLE, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetconfChan(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CONFCHAN, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetconfChan(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = 1;		    /* number of elements write */
	arguments[2] = 0;		    /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CTC_SET_CONFCHAN, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetclock1Delay(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CLOCK1DELAY, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetclock1Delay(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		    /* number of elements write */
	arguments[2] = 0;		    /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CTC_SET_CLOCK1DELAY, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetclock2Delay(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CLOCK2DELAY, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetclock2Delay(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		    /* number of elements write */
	arguments[2] = 0;		    /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CTC_SET_CLOCK2DELAY, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetoutputCntr(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_OUTPUTCNTR, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetcntr1CurVal(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CNTR1CURVAL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetcntr2CurVal(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CTC_GET_CNTR2CURVAL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_1(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_1, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_1(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_1(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_1(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_1, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_1(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_1(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_2(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_2, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_2(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_2(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_2(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_2, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_2(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_2(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_3(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_3, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_3(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_3(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_3(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_3, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_3(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_3(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_4(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_4, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_4(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_4(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_4(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_4, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_4(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_4(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_5(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_5, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_5(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_5(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_5(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_5, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_5(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_5(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_6(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_6, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_6(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_6(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_6(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_6, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_6(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_6(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_7(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_7, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_7(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_7(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_7(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_7, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_7(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_7(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowchannel_8(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_CHANNEL_8, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 6);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetchannel_8(
			     int fd,
			     unsigned long result[6])
{
	return CtcGetWindowchannel_8(fd, 0, 6, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowchannel_8(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 6);

	/* driver call */
	return ioctl(fd, CTC_SET_CHANNEL_8, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetchannel_8(
			     int fd,
			     unsigned long arg[6])
{
	return CtcSetWindowchannel_8(fd, 0, 6, arg);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to get
 * @param result -- buffer for the results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetWindowALL_CHANNELS(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result;
	arguments[1] = depth;
	arguments[2] = elOffs;

	/* driver call */
	if (ioctl(fd, CTC_GET_ALL_CHANNELS, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 48);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcGetALL_CHANNELS(
			     int fd,
			     unsigned long result[48])
{
	return CtcGetWindowALL_CHANNELS(fd, 0, 48, result);
}


/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param elOffs -- element offset (expressed in elements)
 * @param depth  -- number of elemets to set
 * @param arg    -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetWindowALL_CHANNELS(
				   int fd,
				   unsigned int elOffs,
				   unsigned int depth,
				   unsigned long *arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) arg; /* where to take data from */
	arguments[1] = depth;               /* number of elements write */
	arguments[2] = elOffs;              /* element index */

	/* handle endianity */
	matchEndian((char*)arg, sizeof(unsigned long), 48);

	/* driver call */
	return ioctl(fd, CTC_SET_ALL_CHANNELS, (char *)arguments);
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- buffer holds new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CtcSetALL_CHANNELS(
			     int fd,
			     unsigned long arg[48])
{
	return CtcSetWindowALL_CHANNELS(fd, 0, 48, arg);
}


