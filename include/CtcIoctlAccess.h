#ifndef _CTC_IOCTL_ACCESS_H_INCLUDE_
#define _CTC_IOCTL_ACCESS_H_INCLUDE_

#include "./user/CtcUserDefinedAccess.h"

/* this API is obsolete! */
#warning WARNING! Deprecated library. Use DrvrAccess library instead.

#ifdef __cplusplus
extern "C" {
#endif

/* see CtcIoctlAccess.c for precise parameter description */

int  CtcEnableAccess(int, int); /* open  Device driver */
void CtcDisableAccess(int);     /* close Device driver */

int  CtcGetSTATUS(int fd, unsigned long *result);
int  CtcGetCNTR_ENABLE(int fd, unsigned long *result);
int  CtcSetCNTR_ENABLE(int fd, unsigned long arg);
int  CtcGetconfChan(int fd, unsigned long *result);
int  CtcSetconfChan(int fd, unsigned long arg);
int  CtcGetclock1Delay(int fd, unsigned long *result);
int  CtcSetclock1Delay(int fd, unsigned long arg);
int  CtcGetclock2Delay(int fd, unsigned long *result);
int  CtcSetclock2Delay(int fd, unsigned long arg);
int  CtcGetoutputCntr(int fd, unsigned long *result);
int  CtcGetcntr1CurVal(int fd, unsigned long *result);
int  CtcGetcntr2CurVal(int fd, unsigned long *result);
int  CtcGetWindowchannel_1(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_1(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_1(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_1(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_2(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_2(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_2(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_2(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_3(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_3(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_3(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_3(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_4(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_4(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_4(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_4(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_5(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_5(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_5(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_5(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_6(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_6(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_6(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_6(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_7(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_7(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_7(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_7(int fd, unsigned long arg[6]);
int  CtcGetWindowchannel_8(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetchannel_8(int fd, unsigned long result[6]);
int  CtcSetWindowchannel_8(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetchannel_8(int fd, unsigned long arg[6]);
int  CtcGetWindowALL_CHANNELS(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CtcGetALL_CHANNELS(int fd, unsigned long result[48]);
int  CtcSetWindowALL_CHANNELS(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CtcSetALL_CHANNELS(int fd, unsigned long arg[48]);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_IOCTL_ACCESS_H_INCLUDE_ */
