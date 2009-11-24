/**
 * @file CtcUserDefinedAccess.h
 *
 * @brief CTC library header.
 *
 * @author Yury GEORGIEVSKIY. CERN BE/CO
 *
 * @date Created on 04/03/2009
 *
 * Function declarations and needed definitions are here.
 *
 * @version 2.0  ygeorgie  04/03/2009  1. Fix ctc_getClock[1/2] functions.
 *                                     2. Add ctc_getChannelStatus() function.
 *                                     3. Generate DOXYGEN docs for the lib.
 */
#ifndef _CTC_USER_DEFINED_ACCESS_H_INCLUDE_
#define _CTC_USER_DEFINED_ACCESS_H_INCLUDE_

#include <dg/dal.h>
#include <user/CtcDefinitions.h>

/* OK and SYSERR are from 'kernel.h' */
#ifndef OK
#define OK 0
#endif

#ifndef SYSERR
#define SYSERR -1
#endif

/*! @name library errors */
//@{
typedef enum _tagCTC_UDA_ERR {
	CTC_CH_OUT_OF_RANGE = 100, //!< input/output channel out of range
	CTC_VAL_OUT_OF_RANGE,      //!< value out-of-range
	CTC_ERR,                   //!< internal lib error
	CTC_DAL_ERR                //!< DAL error occurs
} ctc_uda_err;
//@}

static inline long IS_CTC_ERR(const void *ptr)
{
  return ((WITHIN_RANGE(CTC_CH_OUT_OF_RANGE, (unsigned long)ptr, CTC_DAL_ERR))?1:0);
}

static inline long PTR_CTC_ERR(const void *ptr)
{
  return (long) ptr;
}

/** @defgroup ctclib CTC library API
 *@{
 */
#ifdef __cplusplus
extern "C" {
#endif

	int ctc_setInputChan(HANDLE handle, int chan, unsigned long newVal);

	int ctc_getClock1(HANDLE handle, int chan, unsigned long *res);
	int ctc_setClock1(HANDLE handle, int chan, unsigned long newVal);

	int ctc_getClock2(HANDLE handle, int chan, unsigned long *res);
	int ctc_setClock2(HANDLE handle, int chan, unsigned long newVal);

	int ctc_getClock1Tick(HANDLE handle, int chan, unsigned long *res);
	int ctc_setClock1Tick(HANDLE handle, int chan, unsigned long newVal);

	int ctc_getClock2Tick(HANDLE handle, int chan, unsigned long *res);
	int ctc_setClock2Tick(HANDLE handle, int chan, unsigned long newVal);

	int ctc_getOutPutCounter(HANDLE handle, int chan, unsigned long *res);
	int ctc_getCntr1CurVal(HANDLE handle, int chan, unsigned long *res);
	int ctc_getCntr2CurVal(HANDLE handle, int chan, unsigned long *res);
	int ctc_getModuleStatus(HANDLE handle, unsigned long *res);
	int ctc_resetModule(HANDLE handle);
	int ctc_enableChannel(HANDLE handle, int outChan, int inpChan);
	int ctc_disableChannel(HANDLE handle, int outChan);
	ctc_cfg_reg_t* ctc_getChanConf(HANDLE handle, int outChan);
	int ctc_dbgPrintout(int toggle);
	int ctc_getChannelStatus(HANDLE handle, int outChan[8]);

#ifdef __cplusplus
}
#endif
//@} end of group

#endif /* _CTC_USER_DEFINED_ACCESS_H_INCLUDE_ */
