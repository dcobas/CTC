/**
 * @file CtcUserDefinedAccess.c
 *
 * @brief CTC library.
 *
 * @author Yury GEORGIEVSKIY. CERN BE/CO
 *
 * @date Created on 04/03/2009
 *
 * <long description>
 *
 * @version 2.0  ygeorgie  04/03/2009\n
 *                   1. Fix ctc_getClock[1/2] functions.\n
 *                   2. Add ctc_getChannelStatus() function.\n
 *                   3. Generate DOXYGEN docs for the lib.\n
 *
 * @version 1.0  ygeorgie  03/04/2008\n Creation date.
 */
/** @mainpage CTC Library Index Page
 *
 * @section Introduction
 *
 * Here you will find @b CTC library API descripiton.\n
 * Don't forget that you also can use @b DAL library to communicate with the
 * module.\n
 * @b CTC library is using @b DAL library to access the hardware.
 *
 * @section library_sec Library.
 *
 * @ref ctclib
 *
 * @subsection ctcapifunc API functions.
 * @subsection ctcliberr  Library error codes.
 */
#include "CtcUserDefinedAccess.h"
#include "CtcRegId.h"
#include "CtcDrvr.h"
#include <stddef.h>		/* for offsetof */
#include "user/CtcDefinitions.h"


/* by default debug is OFF (i.e. 0) */
static int ctc_lib_glob_dbg_flag = 0;

#define DBG_PRINTF(a)				\
do {						\
  if (ctc_lib_glob_dbg_flag) {			\
    printf a ;					\
  }						\
} while (0)

#define CTC_PRNT_DAL_ERR(dalf)												\
do {															\
  fprintf(stderr, "\nCTClib@%s(): DAL %s() FAILED! rc %d [%s]\n", __FUNCTION__, dalf, dalrc, DaGetErrCode(dalrc)->da_err_code);	\
} while (0)


static int __attribute__((unused)) ctc_getInputChan(HANDLE, int, unsigned long*);


/**
 * @brief Get input channel (i.e. external start) of output channel
 *        in question.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - [1, 40] external start channel
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - channel out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
static int ctc_getInputChan(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  unsigned long locRes;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &locRes, sizeof(unsigned long)) != 1) ) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  locRes = ((locRes & CHCFG_ESC_MASK) >> CHCFG_ESC_SHIFT) + 1;
  *res = locRes;

  return(OK);
}


/**
 * @brief Input (i.e. external start) <-> output channel mapping.
 *
 * @param handle - DAL handle
 * @param chan   - output channel. [1, 8] range
 * @param newVal - input channel. [1, 40] range.
 *                 If zero - then module resetting requested.
 *                 Will be set to default external trigger (i.e. input channel)
 *                 namely the first one.
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_setInputChan(HANDLE handle, int chan, unsigned long newVal)
{
  int rid = 0;
  unsigned long curVal;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if (!WITHIN_RANGE(0, newVal, L_INP_CH)) {
    fprintf(stderr, "\nCTClib@%s(): Input channel (#%ld) out of range!\n", __FUNCTION__, newVal);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  /* get configuration register */
  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  DBG_PRINTF(("@%s(): Input 0x%x (%d) Output 0x%x (%d) \n", __FUNCTION__, (int)newVal, (int)newVal, chan, chan));
  DBG_PRINTF(("@%s(): Current register value is 0x%08x\n", __FUNCTION__, (int)curVal));

  curVal &= ~(CHCFG_ESC_MASK);	/* cleanout old value */
  if (newVal)
    curVal |= (((newVal - 1) & CHCFG_ESC_SIGBITS_MASK) << CHCFG_ESC_SHIFT);

  DBG_PRINTF(("@%s(): After conversion, value to set is 0x%08x\n", __FUNCTION__, (int)curVal));

  /* set new value */
  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  if (ctc_lib_glob_dbg_flag) {	/* check if register is set */
    /* get configuration register */
    if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
      CTC_PRNT_DAL_ERR("DaGetRegChunk");
      return(-CTC_DAL_ERR);
    }
    DBG_PRINTF(("@%s(): New register value is 0x%08x\n", __FUNCTION__, (int)curVal));
  }

  return(OK);
}


/**
 * @brief Returns current clock setting for the first counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @note Real register value will be interpreted here, since the clock setting,
 *       as seen by the user is (reg_value + 1), i.e.\n
 *       0 - clock1\n
 *       1 - clock2 etc...
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getClock1(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  unsigned long locRes;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &locRes, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  locRes = (locRes &CHCFG_C1S_MASK) >> CHCFG_C1S_SHIFT;
  *res = locRes + 1;

  return(OK);
}


/**
 * @brief Set external clock for the first counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param newVal - value to set [1, 6]
 *
 * @note Provided user value will be interpreted, since the the real clock
 *       setting is (user_value - 1), i.e.\n
 *       0 - clock1\n
 *       1 - clock2 etc...
 *
 * @return OK                    - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE  - input/output chan out of range
 * @return -CTC_VAL_OUT_OF_RANGE - requested external clock is out-of-range
 * @return -CTC_DAL_ERR          - DAL return error
 */
int ctc_setClock1(HANDLE handle, int chan, unsigned long newVal)
{
  int rid = 0;
  unsigned long curVal;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if (!WITHIN_RANGE(1, newVal, 6)) {
	  fprintf(stderr, "\nCTClib@%s(): Requested xternal clock (#%ld)"
		  "is out of range!\n", __func__, newVal);
	  return -CTC_VAL_OUT_OF_RANGE;
  }

  /* get current */
  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  DBG_PRINTF(("Before setting Clock1, CONFIG reg is 0x%x\n", (int)curVal));

  curVal &= ~(CHCFG_C1S_MASK);	/* cleanout old value */
  curVal |= (((newVal - 1) & CHCFG_C1S_SIGBITS_MASK) << CHCFG_C1S_SHIFT);

  DBG_PRINTF(("After setting Clock1 reg is 0x%x\n", (int)curVal));

  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief Returns current clock setting for the second counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @note Real register value will be interpreted here, since the clock setting,
 *       as seen by the user is (reg_value + 1), i.e.\n
 *       0 - clock1\n
 *       1 - clock2 etc...
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getClock2(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  unsigned long locRes;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &locRes, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  locRes = (locRes & CHCFG_C2S_MASK) >> CHCFG_C2S_SHIFT;
  *res = locRes + 1;

  return(OK);
}


/**
 * @brief Set external clock for the second counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param newVal - value to set [1, 6]
 *
 * @note Provided user value will be interpreted, since the the real clock
 *       setting is (user_value - 1), i.e.\n
 *       0 - clock1\n
 *       1 - clock2 etc...
 *
 * @return OK                    - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE  - input/output chan out of range
 * @return -CTC_VAL_OUT_OF_RANGE - requested external clock is out-of-range
 * @return -CTC_DAL_ERR          - DAL return error
 */
int ctc_setClock2(HANDLE handle, int chan, unsigned long newVal)
{
  int rid = 0;
  unsigned long curVal;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if (!WITHIN_RANGE(1, newVal, 6)) {
	  fprintf(stderr, "\nCTClib@%s(): Requested xternal clock (#%ld)"
		  "is out of range!\n", __func__, newVal);
	  return -CTC_VAL_OUT_OF_RANGE;
  }

  /* get current */
  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  DBG_PRINTF(("Before setting Clock2, CONFIG reg is 0x%x\n", (int)curVal));

  curVal &= ~(CHCFG_C2S_MASK);	/* cleanout old value */
  curVal |= (((newVal - 1) & CHCFG_C2S_SIGBITS_MASK) << CHCFG_C2S_SHIFT);

  DBG_PRINTF(("After setting Clock2 reg is 0x%x\n", (int)curVal));

  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &curVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getClock1Tick(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock1Delay)/sizeof(unsigned long), 1, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param newVal - value to set
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_setClock1Tick(HANDLE handle, int chan, unsigned long newVal)
{
  int rid = 0;
  unsigned long tmpTickVal = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  /* first - reset it to 0 */
  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock1Delay)/sizeof(unsigned long), 1, &tmpTickVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  /* then set user value */
  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock1Delay)/sizeof(unsigned long), 1, &newVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getClock2Tick(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock2Delay)/sizeof(unsigned long), 1, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param newVal - value to set
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_setClock2Tick(HANDLE handle, int chan, unsigned long newVal)
{
  int rid = 0;
  unsigned long tmpTickVal = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  /* first, reset it to 0 */
  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock2Delay)/sizeof(unsigned long), 1, &tmpTickVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  /* now set user value */
  if ( (dalrc = DaSetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock2Delay)/sizeof(unsigned long), 1, &newVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getOutPutCounter(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, outputCntr)/sizeof(unsigned long), 1, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief Get current value of the 1'st counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getCntr1CurVal(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, cntr1CurVal)/sizeof(unsigned long), 1, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief Get current value of the 2'nd counter.
 *
 * @param handle - DAL handle
 * @param chan   - [1, 8] range
 * @param res    - reg val will be put here
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_getCntr2CurVal(HANDLE handle, int chan, unsigned long *res)
{
  int rid = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, chan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, cntr2CurVal)/sizeof(unsigned long), 1, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegChunk");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief Obtain module status register (get current Hardware version).
 *
 * @param handle - DAL handle
 * @param res    - reg val will be put here
 *
 * @return OK           - if succeed. i.e. 0
 * @return -CTC_DAL_ERR - DAL return error
 */
int ctc_getModuleStatus(HANDLE handle, unsigned long *res)
{
  int dalrc;

  if ( (dalrc = DaGetRegister(handle, STATUS_ID, res, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/*
   RESET register bit layout:

   Only 9 LSB are taken into account [0 - 8].
   First one is to reset the whole module.
   All the rest are to enable/disable channels. bit 1 - channel 8,
   bit 2 - channel 7 ... bit 8 - channel 1

   bit:    8     7     6     5     4     3     2     1        0
        +-----+-----+-----+-----+-----+-----+-----+-----+-----------+
        | ch1 | ch2 | ch3 | ch4 | ch5 | ch6 | ch7 | ch8 | mod reset |
        +-----+-----+-----+-----+-----+-----+-----+-----+-----------+

   To enable the channel  - write 1 to appropriate bit.
   To disable the channel - write 0 to appropriate bit.
   To reset the module    - write 1 to the bit zero.
 */

/**
 * @brief Reset module and disable all the channels.
 *
 * @param handle - DAL handle
 *
 * @return OK           - if succeed. i.e. 0
 * @return -CTC_DAL_ERR - DAL return error
 * @return -CTC_ERR     - ctc lib function error
 */
int ctc_resetModule(HANDLE handle)
{
  volatile unsigned long curRegVal;
  int cntr;
  int dalrc;

  if ( (dalrc = DaGetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return(-CTC_DAL_ERR);
  }

  curRegVal = 1; /*  */

  if ( (dalrc = DaSetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegister");
    return(-CTC_DAL_ERR);
  }

  curRegVal = 0; /* change module reset bit to zero and reset all the channels,
		    so that next time we'll write some value into reset
		    register, no module reset will be performed by accident */

  if ( (dalrc = DaSetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegister");
    return(-CTC_DAL_ERR);
  }

  if ( (dalrc = DaGetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return(-CTC_DAL_ERR);
  }

  /* set all channel inputs and counter delays to zero */
  for (cntr = 1; cntr <= 8; cntr++)
    if ( ctc_setInputChan(handle, cntr, 0) ||
	 ctc_setClock1Tick(handle, cntr,0) ||
	 ctc_setClock2Tick(handle, cntr,0) ) {
      fprintf(stderr, "      `----> called from %s()\n", __FUNCTION__);
      return(-CTC_ERR);
    }

  return(OK);
}


/**
 * @brief Set input <-> output channel mapping and enable specified
 *        output channel.
 *
 * @param handle  - DAL handle
 * @param outChan - output channel.[1,  8] range
 * @param inpChan - input channel. [1, 40] range
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 * @return -CTC_ERR             - ctc lib function error
 */
int ctc_enableChannel(HANDLE handle, int outChan, int inpChan)
{
  int rid = 0;
  volatile unsigned long curRegVal = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(outChan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, outChan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if (!WITHIN_RANGE(F_INP_CH, inpChan, L_INP_CH)) {
    fprintf(stderr, "\nCTClib@%s(): Input channel (#%d) out of range!\n", __FUNCTION__, inpChan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  /* set input channel in config register of the channel */
  if (ctc_setInputChan(handle, outChan, inpChan)) {
    fprintf(stderr, "      `----> called from %s()\n", __FUNCTION__);
    return(-CTC_ERR);
  }

  if ( (dalrc = DaGetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return(-CTC_DAL_ERR);
  }

  curRegVal |= 1 << (F_OUT_CH + L_OUT_CH - outChan);

  if ( (dalrc = DaSetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegister");
    return(-CTC_DAL_ERR);
  }

  return(OK);
}


/**
 * @brief Disable specified output channel and unset input <-> output channel
 *        mapping.
 *
 * @param handle  - DAL handle
 * @param outChan - output channel to disable. [1, 8] range
 *
 * To disable channel, we should write 0 to the corresponding bit of the reset
 * register. Moreover, current external start (input channel) is set to zero.
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
int ctc_disableChannel(HANDLE handle, int outChan)
{
  int rid = 0;
  volatile unsigned long curRegVal = 0;
  int dalrc;

  if ( (rid = CHECK_OUT_CHAN(outChan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, outChan);
    return(-CTC_CH_OUT_OF_RANGE);
  }

  if ( (dalrc = DaGetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return(-CTC_DAL_ERR);
  }

  curRegVal &= ~(1 << (F_OUT_CH + L_OUT_CH - outChan));

  if ( (dalrc = DaSetRegister(handle, CNTR_ENABLE_ID, (void*)&curRegVal, sizeof(unsigned long))) != 1) {
    CTC_PRNT_DAL_ERR("DaSetRegister");
    return(-CTC_DAL_ERR);
  }

  return(OK);
  //return(ctc_setInputChan(handle, outChan, 0));
}


/**
 * @brief To get channel configuration data.
 *
 * @param handle  - DAL handle
 * @param outChan - output channel to exploit. [1, 8] range (0 for all of them)
 *
 * @return OK                   - if succeed. i.e. 0
 * @return -CTC_CH_OUT_OF_RANGE - input/output chan out of range
 * @return -CTC_DAL_ERR         - DAL return error
 */
ctc_cfg_reg_t* ctc_getChanConf(HANDLE handle, int outChan)
{
  static ctc_cfg_reg_t ctc_conf_data[L_OUT_CH];
  CTCBlock01_t blk_data[L_OUT_CH];
  int dalrc, cntr;
  int rid = 0;

  if ( (rid = CHECK_OUT_CHAN(outChan)) == -1) {
    fprintf(stderr, "\nCTClib@%s(): Output channel (#%d) out of range!\n", __FUNCTION__, outChan);
    return((ctc_cfg_reg_t*)-CTC_CH_OUT_OF_RANGE);
  }

  /* get info about all CTC output channels */
  if ( (dalrc = DaGetRegister(handle, ALL_CHANNELS_ID, (void*)blk_data, sizeof(blk_data))) != DaGetRegDepth(handle, ALL_CHANNELS_ID)) {
    CTC_PRNT_DAL_ERR("DaGetRegister");
    return((ctc_cfg_reg_t*)-CTC_DAL_ERR);
  }

  /* put config data into the structure for the user */
  for (cntr = 0; cntr < L_OUT_CH; cntr++) {
    ctc_conf_data[cntr].cr_ext_start = ((blk_data[cntr].confChan&CHCFG_ESC_MASK)>>CHCFG_ESC_SHIFT) + 1;
    ctc_conf_data[cntr].cr_cntr1_clk = ((blk_data[cntr].confChan&CHCFG_C1S_MASK)>>CHCFG_C1S_SHIFT) + 1;
    ctc_conf_data[cntr].cr_cntr2_clk = ((blk_data[cntr].confChan&CHCFG_C2S_MASK)>>CHCFG_C2S_SHIFT) + 1;
    ctc_conf_data[cntr].cr_mode      = blk_data[cntr].confChan & CHCFG_MOD_MASK;
    ctc_conf_data[cntr].cr_direction = blk_data[cntr].confChan & CHCFG_DIR_MASK;
  }

  return(ctc_conf_data);
}


/**
 * @brief Enable library debug printout and toggle debug flag.
 *
 * @param toggle - if 1 - will toggle current flag and return the previous
 *                 flag value.\n
 *                 if 0 - just give current value without flag toggling.
 *
 * @return previous/current debug flag
 */
int ctc_dbgPrintout(int toggle)
{
  int old_flg = ctc_lib_glob_dbg_flag;

  if (toggle)
    ctc_lib_glob_dbg_flag ^= 1;

  return(old_flg);
}


/**
 * ctc_getChannelStatus - Get current status of 8 output channels.
 *
 * @param handle  - DAL handle
 * @param outChan - massive to store current status in. Must be 8 integers long.
 *
 * Output channel can be either enabled or disabled.\n
 * Each value in @outChan represents current status of the output channel.\n
 * (outChan[0] - for channel 1, outChan[7] - for channel 8, etc...)\n
 * If channel value is 0 - output channel is disabled.\n
 * If channle value is one of [1 - 40] - output channel is active and
 * connected to the corresponding input channel.
 *
 * @return OK           - all OK.
 * @return -CTC_DAL_ERR - DAL return error.
 */
int ctc_getChannelStatus(HANDLE handle, int outChan[8])
{
	ctc_cfg_reg_t *data_p = NULL;
	int count_enb_reg;
	int cntr;

	if ( (DaGetRegister(handle, CNTR_ENABLE_ID, &count_enb_reg,
			    sizeof(count_enb_reg))) <= 0) {
		printf("Can't get channel status.\n");
		return -CTC_DAL_ERR;
	}

	for (cntr = 0; cntr < L_OUT_CH; cntr++)
		outChan[cntr] =
			(count_enb_reg & (1 << (L_OUT_CH - cntr))) >>
			(L_OUT_CH - cntr);

	data_p = ctc_getChanConf(handle, 0/*get all the channels*/);
	if (IS_CTC_ERR(data_p)) {
		printf("Can't get channel configuration.\n");
		return PTR_CTC_ERR(data_p);
	}

	for (cntr = 0; cntr < L_OUT_CH; cntr++)
		if (outChan[cntr])
			outChan[cntr] = data_p[cntr].cr_ext_start;

	return OK;
}
