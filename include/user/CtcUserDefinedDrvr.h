#ifndef _CTC_USER_DEFINED_DRVR_H_INCLUDE_
#define _CTC_USER_DEFINED_DRVR_H_INCLUDE_

#include "CtcDrvr.h"

#define CTC_TEST_BENCH (CTC_FIRST_USR_IOCTL + 0)

#if defined(__LYNXOS) || defined(__KERNEL__)
struct sel; /* preliminary structure declaration to supress warnings during
	       user code compilation */

/* user-defined statics data table for CTC module */
struct CTCUserStatics_t {
  /*
    -=-=-=-=-=-=-=-=-=-=-=-=-
    | insert your code here |
    -=-=-=-=-=-=-=-=-=-=-=-=-
  */

};



/*
  -=-=-=-=-=-=-=-=-=-=-=-=-
  | insert your code here |
  -=-=-=-=-=-=-=-=-=-=-=-=-
*/


int CtcUserOpen(int*, register CTCStatics_t*, int, struct file*);
int CtcUserClose(int*, register CTCStatics_t*, struct file*);
int CtcUserRead(int*, register CTCStatics_t*, struct file*, char*, int);
int CtcUserWrite(int*, register CTCStatics_t*, struct file*, char*, int);
int CtcUserSelect(int*, register CTCStatics_t*, struct file*, int, struct sel*);
int CtcUserIoctl(int*, register CTCStatics_t*, struct file*, int, int, char*);
char* CtcUserInst(int*, register DevInfo_t*, register CTCStatics_t*);
int CtcUserUnInst(int*, CTCStatics_t*);
#endif /* __LYNXOS || __KERNEL__ */

#endif /* _CTC_USER_DEFINED_DRVR_H_INCLUDE_ */
