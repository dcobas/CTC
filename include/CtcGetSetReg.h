#ifndef _CTC_GET_SET_REG_H_INCLUDE_
#define _CTC_GET_SET_REG_H_INCLUDE_

/* Get/Set functions that can be used inside the user part of the driver */

/* Service register operations */
int get___SRV__DEBUG_FLAG(register CTCStatics_t*, char*, int, int);
int set___SRV__DEBUG_FLAG(register CTCStatics_t*, char*, int, int);
int get___SRV__DEVINFO_T(register CTCStatics_t*, char*, int, int);
int get___SRV__DRVR_VERS(register CTCStatics_t*, char*, int, int);
int get___SRV__DAL_CONSISTENT(register CTCStatics_t*, char*, int, int);

/* Hardware version */
int get_STATUS(register CTCStatics_t*, char*, int, int);

/* Enable/disable output channel */
int get_CNTR_ENABLE(register CTCStatics_t*, char*, int, int);
int set_CNTR_ENABLE(register CTCStatics_t*, char*, int, int);

/* Select External start (i.e. input channel) and Clock1/Clock2 settings */
int get_confChan(register CTCStatics_t*, char*, int, int);
int set_confChan(register CTCStatics_t*, char*, int, int);

/* How many ticks to wait (i.e. delay) for the 1'st counter */
int get_clock1Delay(register CTCStatics_t*, char*, int, int);
int set_clock1Delay(register CTCStatics_t*, char*, int, int);

/* How many ticks to wait (i.e. delay) for the 2'nd counter */
int get_clock2Delay(register CTCStatics_t*, char*, int, int);
int set_clock2Delay(register CTCStatics_t*, char*, int, int);

/* Output pulse counter that occured in the channel */
int get_outputCntr(register CTCStatics_t*, char*, int, int);

/* Current value of the 1'st counter */
int get_cntr1CurVal(register CTCStatics_t*, char*, int, int);

/* Current value of the 2'nd counter */
int get_cntr2CurVal(register CTCStatics_t*, char*, int, int);

/* All registers of channel 1 */
int get_channel_1(register CTCStatics_t*, char*, int, int);
int set_channel_1(register CTCStatics_t*, char*, int, int);

/* All registers of channel 2 */
int get_channel_2(register CTCStatics_t*, char*, int, int);
int set_channel_2(register CTCStatics_t*, char*, int, int);

/* All registers of channel 3 */
int get_channel_3(register CTCStatics_t*, char*, int, int);
int set_channel_3(register CTCStatics_t*, char*, int, int);

/* All registers of channel 4 */
int get_channel_4(register CTCStatics_t*, char*, int, int);
int set_channel_4(register CTCStatics_t*, char*, int, int);

/* All registers of channel 5 */
int get_channel_5(register CTCStatics_t*, char*, int, int);
int set_channel_5(register CTCStatics_t*, char*, int, int);

/* All registers of channel 6 */
int get_channel_6(register CTCStatics_t*, char*, int, int);
int set_channel_6(register CTCStatics_t*, char*, int, int);

/* All registers of channel 7 */
int get_channel_7(register CTCStatics_t*, char*, int, int);
int set_channel_7(register CTCStatics_t*, char*, int, int);

/* All registers of channel 8 */
int get_channel_8(register CTCStatics_t*, char*, int, int);
int set_channel_8(register CTCStatics_t*, char*, int, int);

/* All CTC channels to access in one chunk */
int get_ALL_CHANNELS(register CTCStatics_t*, char*, int, int);
int set_ALL_CHANNELS(register CTCStatics_t*, char*, int, int);

#endif /* _CTC_GET_SET_REG_H_INCLUDE_ */
