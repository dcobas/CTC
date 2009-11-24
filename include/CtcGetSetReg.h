#ifndef _CTC_GET_SET_REG_H_INCLUDE_
#define _CTC_GET_SET_REG_H_INCLUDE_

/*
  These functions are used to deliver register values directly to the user
  space.

  API is the following:
  1 param -- statics table

  2 param -- ioctl argument in predefined format:
             Massive of 3 elements, each is 4 bytes long.
             [0] - user-space address
             [1] - number of elements to r/w
             [2] - element index, starting from zero

             In case of service registers -- ioctl arguments can vary.
             Their amount depends on specific ioctl number.
             See service routines (those are with __SRV__ subword)
             for more details on parameter amount.

             For example, if this is a repetitive r/w request
             (ioctl number is SRV__REP_REG_RW) then we should have 4 arguments,
             that are packed as follows:

             [0] -- ioctl number
             [1] -- user-space address
             [2] -- number of elements to r/w
             [3] -- element index, starting from zero

  3 param -- check r/w bounds (1 - yes, 0 - no)
             valid only in case of Lynx
  4 param -- repeatedly read register (1 - yes, 0 - no)


  Bear in mind, that r/w operation results goes diretly to the user space.
  If you want to operate on the HW registers inside the driver -- use
  low-level port operation functions from port_ops_[linux/lynx].h like:
  __inb      -- read a byte from a port
  __inw      -- read a word from a port
  __in       -- lread a long from a port
  __outb     -- write a byte to a port
  __outw     -- write a word to a port
  __outl     -- write a long to a port
  __rep_inb  -- read multiple bytes from a port into a buffer
  __rep_inw  -- read multiple words from a port into a buffer
  __rep_inl  -- read multiple longs from a port into a buffer
  __rep_outb -- write multiple bytes to a port from a buffer
  __rep_outw -- write multiple words to a port from a buffer
  __rep_outl -- write multiple longs to a port from a buffer

  These functions are used to r/w HW registers inside the driver.
  Never access registers directly. Use this function to do this.
*/

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
