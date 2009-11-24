#include "CtcUserDefinedDrvr.h"
#ifdef __powerpc__
#include <vme.h>
#endif

#ifdef __Lynx__
/* for __inb, __outb etc... */
#include "dg/port_ops_lynx.h"
#else  /* __linux__ */
#include "dg/port_ops_linux.h"
#endif

static ulong __buf[(1<<16)] = { 0 };

/*
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  User Entry Poins into Driver/Simulator are located here.
  This functions are called in an appropriate driver/simulator routines.
  All user-defined driver/simulator code should be added in this functions.
  Also Interrupt Service Routine is located here, as it's implementation is
  _completely_ up to the user.
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcISR.
 * DESCRIPTION: Interrupt Service Routine. Implementation is _completely_ up to
 *		the user.
 * RETURNS:	void.
 *-----------------------------------------------------------------------------
 */
static void  __attribute__((unused))
CtcISR(
	       CTCStatics_t *stPtr) /* Statics table pointer */
{

}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserOpen.
 * DESCRIPTION: User entry point in driver/simulator open routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserOpen(
	    int *proceed, /* if standard code execution should be proceed */
	    register CTCStatics_t *sptr, /*  */
	    int         d,  /*  */
	    struct file *f) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;


  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserClose.
 * DESCRIPTION: User entry point in driver/simulator close routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserClose(
	     int *proceed, /* if standard code execution should be proceed */
	     register CTCStatics_t *sptr, /*  */
	     struct file   *f) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;


  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserRead.
 * DESCRIPTION: User entry point in driver/simulator read routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserRead(
	    int *proceed, /* if standard code execution should be proceed */
	    register CTCStatics_t *sptr, /*  */
	    struct file *f,    /*  */
	    char	*buff, /*  */
	    int		count) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;


  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserWrite.
 * DESCRIPTION: User entry point in driver/simulator write routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserWrite(
	     int *proceed, /* if standard code execution should be proceed */
	     register CTCStatics_t *sptr, /*  */
	     struct file *f,    /*  */
	     char	 *buff, /*  */
	     int	 count) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;


  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserSelect.
 * DESCRIPTION: User entry point in driver/simulator select routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserSelect(
	      int *proceed, /* if standard code execution should be proceed */
	      register CTCStatics_t *sptr, /*  */
	      struct file *f,    /*  */
	      int	  which, /*  */
	      struct sel  *ffs)  /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;


  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserIoctl.
 * DESCRIPTION: User entry point in driver/simulator ioctl routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserIoctl(
	     int *proceed, /* if standard code execution should be proceed */
	     register CTCStatics_t *sptr, /*  */
             struct file *f,   /*  */
             int         lun,  /*  */
             int         com,  /*  */
             char        *arg) /*  */
{
	CTCUserStatics_t *usp =  sptr->usrs; /* user statistics table */
	int i;
	CTCBlock01_t *ptr = sptr->card->block01;

	switch (com) {
	case CTC_TEST_BENCH:
		/* will write repetedly into Delay Counter Registers[1/2] */
		for (i = 0; i < (1<<16)-1; i++) {
			__buf[i] = i;
		}
		kkprintf("starting CTC_TEST_BENCH"
			 " (Array is %d elements long)\n", ARRAY_SIZE(__buf));

		for (i = 0; i < 8; i++, ptr++) {
			kkprintf("Channel %d --- ", i+1);
			__rep_outl((__port_addr_t)&ptr->clock1Delay,
				   __buf, ARRAY_SIZE(__buf));
			__rep_outl((__port_addr_t)&ptr->clock2Delay,
				   __buf, ARRAY_SIZE(__buf));
			kkprintf("Done\n");
		}


		*proceed = FALSE;
		kkprintf("CTC_TEST_BENCH done\n");
		break;
	default:
		*proceed = TRUE; /* continue standard code execution */
	}


	return OK; /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserInst.
 * DESCRIPTION: User entry point in driver/simulator installation routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard execution should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		pointer to a statics data structure - if succeed.
 *		SYSERR                              - in case of failure.
 *-----------------------------------------------------------------------------
 */
char*
CtcUserInst(
	    int *proceed, /* if standard code execution should be proceed */
	    register DevInfo_t *info, /*  */
	    register CTCStatics_t *sptr) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */
  int iVec = 0;			/* interrupt vector */

  usp  = sptr->usrst;
  iVec = info->iVector;		/* set up interrupt vector */

#if 0
  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    | user-defined code is here. |
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    Uncomment the following code to register ISR:
    !WARNING! It's worth to do 'man iointset' before to start.
  */
  if (iVec > 0) {
    int coco = 0; /* completion code */
    DBG_install(("ISR ( vector number [%d] ) installation - ", iVec));
#ifdef __powerpc__ /* in this case we are using CES BSP */
    coco = vme_intset(iVec, (int (*)())CtcISR, (char*)sptr, 0);
#else  /* use standard system call otherwise */
    coco = iointset(iVec, (int (*)())CtcISR, (char*)sptr);
#endif
    if (coco < 0) {
      DBG_install(("Failed.\n"));
      pseterr(EFAULT);		/* TODO. what error to set? */
      return((char*)SYSERR);	/* -1 */
    }
    DBG_install(("interrupt vector managed.\n"));
  }
#endif

  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return((char*)sptr); /* succeed */
}


/*-----------------------------------------------------------------------------
 * FUNCTION:    CtcUserUnInst.
 * DESCRIPTION: User entry point in driver/simulator unistall routine.
 *		It's up to user to set kernel-level errno (by means of
 *		'pseterr' call). 'proceed' parameter denotes if further
 *		standard actions should be proceed after function returns.
 *		FALSE - means that user-desired operation done all that user
 *		wants and there is no further necessaty to perfom any standard
 *		operations that follow function call. TRUE - means that code
 *		that follows function call will be executed.
 * RETURNS:	return value is the same as in entry point function.
 *		OK     - if succeed.
 *		SYSERR - in case of failure.
 *-----------------------------------------------------------------------------
 */
int
CtcUserUnInst(
	      int *proceed, /* if standard code execution should be proceed */
	      CTCStatics_t *sptr) /*  */
{
  CTCUserStatics_t *usp; /* user statistics table */

  usp = sptr->usrst;

#if 0
  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
    Uncomment the following code to unregister ISR:
  */
  kkprintf("Ctc: Interrupt routine managment cleanup ( vector number [%d] ) - ", sptr->info->iVector);
#ifdef __powerpc__
  /* in this case we are using CES BSP */
  vme_intclr(sptr->info->iVector, 0);
#else
  iointclr(sptr->info->iVector);
#endif
   kkprintf("OK\n");
#endif


  if (proceed)
    *proceed = TRUE; /* continue standard code execution */

  return(OK); /* succeed */
}
