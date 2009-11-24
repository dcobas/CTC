#ifndef _CTC_REG_ID_H_INCLUDE_
#define _CTC_REG_ID_H_INCLUDE_

#include "dg/ServiceRegId.h"

/* CTC module registers ID. Used by DAL */
typedef enum _tag_CTC_rid {
  STATUS_ID = LAST_SRV_REG, /* Hardware version (RWMODE rc) */
  CNTR_ENABLE_ID, /* Enable/disable output channel (RWMODE rw) */
  CONFCHAN_ID, /* Select External start (i.e. input channel) and Clock1/Clock2 settings (RWMODE rw) */
  CLOCK1DELAY_ID, /* How many ticks to wait (i.e. delay) for the 1'st counter (RWMODE rw) */
  CLOCK2DELAY_ID, /* How many ticks to wait (i.e. delay) for the 2'nd counter (RWMODE rw) */
  OUTPUTCNTR_ID, /* Output pulse counter that occured in the channel (RWMODE r) */
  CNTR1CURVAL_ID, /* Current value of the 1'st counter (RWMODE r) */
  CNTR2CURVAL_ID, /* Current value of the 2'nd counter (RWMODE r) */
  CHANNEL_1_ID, /* All registers of channel 1 (RWMODE rw) */
  CHANNEL_2_ID, /* All registers of channel 2 (RWMODE rw) */
  CHANNEL_3_ID, /* All registers of channel 3 (RWMODE rw) */
  CHANNEL_4_ID, /* All registers of channel 4 (RWMODE rw) */
  CHANNEL_5_ID, /* All registers of channel 5 (RWMODE rw) */
  CHANNEL_6_ID, /* All registers of channel 6 (RWMODE rw) */
  CHANNEL_7_ID, /* All registers of channel 7 (RWMODE rw) */
  CHANNEL_8_ID, /* All registers of channel 8 (RWMODE rw) */
  ALL_CHANNELS_ID, /* All CTC channels to access in one chunk (RWMODE rw) */
} CTC_rid_t;

#endif /* _CTC_REG_ID_H_INCLUDE_ */
