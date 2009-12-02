#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "CtcTest.h"
#include "CtcUserDefinedTest.h"
#include "user/CtcUserDefinedAccess.h"
#include "user/CtcDefinitions.h"
#include <stddef.h>		/* for offsetof */
#include "CtcDrvr.h"
#include "user/CtcUserDefinedDrvr.h"

/**
 * @brief
 *
 * @param none
 *
 * @return void
 */
static void local_clear_screen()
{
  char termcapValueHold[64];
  char *termcapValue = termcapValueHold;
  char termcapEntry[1024];
  char *terminalType;
  char *ClearScreen;

  terminalType = (char *)getenv("TERM");
  tgetent(termcapEntry, terminalType);
  ClearScreen = (char *)tgetstr("cl", &termcapValue);
  printf("%s", ClearScreen);
}

/* get Output Channel */
static int get_output_ch(int first)
{
	int outch;

	printf("Output channel[%d - 8] -> ", first);
	scanf("%d", &outch);
	getchar();
	if (!WITHIN_RANGE(first, outch, L_OUT_CH)) {
		printf("ERROR! out-of-range [%d - %d]\n",
		       F_OUT_CH, L_OUT_CH);
		return -1;
	}
	return outch;
}

/* get Input Channel */
static int get_input_ch(void)
{
	int inpch;

	printf("Input channel[1 - 40] -> ");
	scanf("%d", &inpch);
	getchar();
	if (!WITHIN_RANGE(F_INP_CH, inpch, L_INP_CH)) {
		printf("ERROR! out-of-range [%d - %d]\n",
		       F_OUT_CH, L_OUT_CH);
		return -1;
	}
	return inpch;
}

/**
 * @brief
 *
 * @param handle - DAL handle
 * @param lun    - Logical Unit Number
 *
 * @return SYSERR - error occurs
 * @return OK     - we cool
 */
static int printout_cntr_enable_reg_and_ext_start(HANDLE handle, int lun)
{
  int cc, cntr, again_cntr = 0;
  int count_enb_reg;
  ctc_cfg_reg_t *data_p = NULL;
  char str[0x10]; /* user prompt data */

 get_again:
  if ( (cc = DaGetRegister(handle, CNTR_ENABLE_ID, &count_enb_reg, sizeof(count_enb_reg))) <= 0) {
    printf("Can't get channel status! (cc = %d)\n", cc);
    return(SYSERR);
  }

  data_p = ctc_getChanConf(handle, 0/*get all the channels*/);
  if (IS_CTC_ERR(data_p)) {
    printf("Can't get channel configuration! (cc = %ld)\n", PTR_CTC_ERR(data_p));
    return(SYSERR);
  }

  if (!ctc_dbgPrintout(0))
    local_clear_screen();
  printf("[%02d] Current register value is 0x%x\n\n", again_cntr++, count_enb_reg);
  printf("bit:      8     7     6     5     4     3     2     1        0\n       +-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n       | ch1 | ch2 | ch3 | ch4 | ch5 | ch6 | ch7 | ch8 | mod reset |\n       +-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n       |");
  for (cntr = F_OUT_CH; cntr <= L_OUT_CH; cntr++)
    printf(" %-3s |", (count_enb_reg & (1 << (F_OUT_CH + L_OUT_CH - cntr)))? "ON" : "OFF"); /* all 8 channels */
  printf("     %d     |\n", (count_enb_reg & 1)); /* mod reset bit */
  printf("       +-----+-----+-----+-----+-----+-----+-----+-----+-----------+\noutCh: |");
  for (cntr = F_OUT_CH; cntr <= L_OUT_CH; cntr++)
    printf(" %-2d  |", data_p[cntr-1].cr_ext_start); /* all 8 channels */
  printf("           |\n       +-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n");

  /* prompt user */
  printf("<enter> to renew\nq - to quit: ");
  fgets(str, sizeof(str), stdin);
  if (str[0] == '\n')
    goto get_again;

  return(OK); /* 0 */
}


/**
 * @brief
 *
 * @param handle - DAL handle
 * @param lun    - Logical Unit Number
 * @param chan   - output channel to query
 *
 * @return SYSERR - error occurs
 * @return OK     - we cool
 */
static int printout_conf_reg(HANDLE handle, int lun, int chan)
{
  int rid = 0;
  unsigned long locRes;
  int coco;
  unsigned long cntr12_cv_reg[2]; /* counter current values */
  unsigned long cntr12_del_reg[2]; /* counter delay values */
  char str[0x10]; /* user prompt data */
  int cntr = 0, data, is_error;

  if ( (rid = CHECK_OUT_CHAN(chan)) == -1) {
    printf("Can't get configuration register! Provided output channel is out of [%d - %d] range.\n", F_OUT_CH, L_OUT_CH);
    return(SYSERR);	/* -1 */
  }

 loop_read:
  is_error = 0;
  if ( (coco = DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, confChan)/sizeof(unsigned long), 1, &locRes, sizeof(unsigned long)) != 1) ) {
    printf("Can't get configuration register for channel[%d]! (coco = %d)\n", chan, coco);
    return(SYSERR); 	/* -1 */
  }

 prompt_again:
  local_clear_screen();
  printf("[%02d] Register value on chan[%d]@lun#%d is 0x%lx\n\n", cntr++, chan, abs(lun), locRes);

  printf("bit:  31 30 29 ....... 24 23 22 ..... 20 19 18 ..... 16 15..2   1       0\n     +-----+-------------+--+-----------+--+-----------+-----+----+---------+\n     |XXXXX|Ext Start sel|XX|cntr1 clock|XX|cntr2 clock|XXXXX|Mode|Direction|\n     |-----+-------------+--+-----------+--+-----------+-----+----+---------|\n     |xxxxx|     %2ld      |xx|    %ld      |xx|    %ld      |xxxxx|  %ld |    %ld    |\n     +-----+-------------+--+-----------+--+-----------+-----+----+---------+\n\n", ((locRes&CHCFG_ESC_MASK)>>CHCFG_ESC_SHIFT) + 1, ((locRes&CHCFG_C1S_MASK)>>CHCFG_C1S_SHIFT) + 1, ((locRes&CHCFG_C2S_MASK)>>CHCFG_C2S_SHIFT) + 1, (locRes & CHCFG_MOD_MASK), (locRes & CHCFG_DIR_MASK));

  if (!is_error) {
	  DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, clock1Delay)/sizeof(unsigned long), 2, cntr12_del_reg, sizeof(cntr12_del_reg));
	  DaGetRegChunk(handle, rid, offsetof(CTCBlock01_t, cntr1CurVal)/sizeof(unsigned long), 2, cntr12_cv_reg, sizeof(cntr12_cv_reg));
  }
  printf("Cntr1 delay => 0x%lx. current val => 0x%lx\nCntr2 delay => 0x%lx. current val => 0x%lx\n\n", cntr12_del_reg[0], cntr12_cv_reg[0], cntr12_del_reg[1], cntr12_cv_reg[1]);


  /* prompt user */
  printf("<enter>    - re-read register && refresh screen\n"
	 "se [1, 40] - set Ext Start sel\n"
	 "s1 [1, 6]  - set cntr1 clock\n"
	 "s2 [1, 6]  - set cntr2 clock\n"
	 "m          - Mode bit toggle\n"
	 "d          - Direction bit toggle\n"
	 "q          - to quit\n\n-> ");
  fgets(str, sizeof(str), stdin);
  if (str[0] == 'q')
	  return OK;
  else if (str[0] == 'm') {
	  printf("\nNOT IMPLEMENTED YET! <enter> to continue");
	  getchar();
	  is_error = 1;
	  goto prompt_again;
  }
  else if (str[0] == 'd') {
	  printf("\nNOT IMPLEMENTED YET! <enter> to continue");
	  getchar();
	  is_error = 1;
	  goto prompt_again;
  }
  else if (str[0] == '\n')
	  goto loop_read;	/* re-read && refresh */
  else if (!strncmp(str, "se", 2)) {
	  sscanf(&str[2], "%d", &data);
	  if (!WITHIN_RANGE(F_INP_CH, data, L_INP_CH)) {
		  printf("External Start (%d) is out-of-range [%d - %d]!\n",
			 data, F_INP_CH, L_INP_CH);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;
	  }
	  if ( (coco = ctc_setInputChan(handle, chan, data)) ) {
		  printf("Failed to set external start! (cc = %d)\n", coco);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;

	  }
  }
  else if (!strncmp(str, "s1", 2)) {
	  sscanf(&str[2], "%d", &data);
	  if (!WITHIN_RANGE(F_CNTR_CLK, data, L_CNTR_CLK)) {
		  printf("Cntr1 clock (%d) is out-of-range [%d - %d]!\n",
			 data, F_CNTR_CLK, L_CNTR_CLK);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;
	  }
	  if ( (coco = ctc_setClock1(handle, chan, data)) ) {
		  printf("Failed to set cntr1 clock! (cc = %d)\n", coco);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;

	  }
  }
  else if (!strncmp(str, "s2", 2)) {
	  sscanf(&str[2], "%d", &data);
	  if (!WITHIN_RANGE(F_CNTR_CLK, data, L_CNTR_CLK)) {
		  printf("Cntr1 clock (%d) is out-of-range [%d - %d]!\n",
			 data, F_CNTR_CLK, L_CNTR_CLK);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;
	  }
	  if ( (coco = ctc_setClock2(handle, chan, data)) ) {
		  printf("Failed to set cntr1 clock! (cc = %d)\n", coco);
		  printf("\n<enter> to continue");
		  getchar();
		  is_error = 1;
		  goto prompt_again;
	  }
  }
  else {
	  is_error = 1;
	  goto prompt_again;
  }
  goto loop_read;

  return(OK); /* 0 */
}

/**
 * @brief r/w hw memory at all possible addresses
 *
 * @param handle -- DAL handle
 * @param lun    -- Logical Unit Number
 *
 * <long-description>
 *
 * @return <ReturnValue>
 */
static int run_test_bench(HANDLE handle, int lun)
{
	//int rc, data;
	//_dal_h *ditp = (_dal_h*)container_of((void*)handle, struct _tagDALH, hID);

	ioctl(DaGetNodeFd(handle), CTC_TEST_BENCH, NULL);

#if 0
	//printf("read access @%#x [STATUS]\n", ditp->
	rc = DaGetRegister(handle, STATUS_ID, &data, sizeof(data));
	if (rc < 0)
		printf("");

	for (i = 1; i <= L_OUT_CH; i++) {

		/* 1. ctc_setInputChan() */
		printf("Testing ctc_setInputChannel()...\n");
		for (k = 1; k <= L_INP_CH; k++) {
			rc = ctc_setInputChan(handle, i, k);
			if (rc)
				printf("ctc_setInputChan() failed for input"
				       " channel %d <--> output channel %d\n",
				       i, k);
		}
		printf("Done.\n\n\n");

	}
#endif
	       return 0;
}

/**
 * @brief
 *
 * @param handle - lib handle, ret. by @e DaEnableAccess
 * @param lun    - Logical Unit Number
 *
 * @return
 */
int UserDefinedMenu(HANDLE handle, int lun)
{
  int i, choice, cntr;
  int do_usr_wait = 0, cc;

  for (;;) {
    printf("%sCTC %s Test Program (LUN %d) - User Defined Menu\n",
	   ClearScreen, (lun < 0)?"Simulator":"Driver", abs(lun));
    for (i = 0; i < screenWidth(); i++)
      printf("-");

    printf("\n\n");
    printf("01 -- Channel[1] configuration [get/set]\n");
    printf("02 -- Channel[2] configuration [get/set]\n");
    printf("03 -- Channel[3] configuration [get/set]\n");
    printf("04 -- Channel[4] configuration [get/set]\n");
    printf("05 -- Channel[5] configuration [get/set]\n");
    printf("06 -- Channel[6] configuration [get/set]\n");
    printf("07 -- Channel[7] configuration [get/set]\n");
    printf("08 -- Channel[8] configuration [get/set]\n");
    printf("09 -- Chan status (reset reg) and inp chan select\n");
    printf("10 -- Config register of all output channels [1-8]\n");
    printf("11 -- Toggle DBG printout flag [current - %s]\n", (ctc_dbgPrintout(0))?"ON":"OFF");
    printf("12 -- Enable output channel && set it's input channel\n");
    printf("13 -- Disable output channel\n");
    printf("14 -- Run a test bench\n");
    printf("15 -- Return to Main Menu\n");

    printf("\n> ");

    scanf("%d", &choice);
    getchar();

    switch (choice) {
    case 15:	/* user wants out */
	    return OK;
    case 14:
	    return run_test_bench(handle, lun);
    case 13:
	    {
		    int outch = get_output_ch(F_OUT_CH);

		    do_usr_wait = 1;

		    if (outch == -1)
			    break;

		    if ( (cc = ctc_disableChannel(handle, outch)) )
			    printf("Failed to disable output"
				   " channel (cc = %d)\n", cc);

		    break;
	    }
    case 12:
	    {
		    int outch, inpch;

		    do_usr_wait = 1;
		    outch = get_output_ch(F_OUT_CH);
		    if (outch == -1)
			    break;

		    inpch = get_input_ch();
		    if (inpch == -1)
			    break;

		    if ( (cc = ctc_enableChannel(handle, outch, inpch)) )
			    printf("Failed to enable output channel"
				   " (cc = %d)\n", cc);
		    break;
	    }
    case 11:
      ctc_dbgPrintout(1);
      break;
    case 10:	/* all channel config */
      {
	ctc_cfg_reg_t *data_p = NULL;
	data_p = ctc_getChanConf(handle, 0/*get all the channels*/);
	if (!IS_CTC_ERR(data_p)) {
	  printf("\n            ch1   ch2   ch3   ch4   ch5   ch6   ch7   ch8  \n          +-----+-----+-----+-----+-----+-----+-----+-----+\next start |");
	  /* external start data */
	  for (cntr = 0; cntr < L_OUT_CH; cntr++)
	    printf(" %2d  |", data_p[cntr].cr_ext_start);

	  printf("\n          +-----+-----+-----+-----+-----+-----+-----+-----+\ncntr1_clk |");

	  /* counter clock #1 */
	  for (cntr = 0; cntr < L_OUT_CH; cntr++)
	    printf(" %2d  |", data_p[cntr].cr_cntr1_clk);

	  printf("\n          +-----+-----+-----+-----+-----+-----+-----+-----+\ncntr2_clk |");

	  /* counter clock #2 */
	  for (cntr = 0; cntr < L_OUT_CH; cntr++)
	    printf(" %2d  |", data_p[cntr].cr_cntr2_clk);

	  printf("\n          +-----+-----+-----+-----+-----+-----+-----+-----+\nmode      |");

	  /* mode */
	  for (cntr = 0; cntr < L_OUT_CH; cntr++)
	    printf(" %2d  |", data_p[cntr].cr_mode);

	  printf("\n          +-----+-----+-----+-----+-----+-----+-----+-----+\ndirection |");


	  /* direction */
	  for (cntr = 0; cntr < L_OUT_CH; cntr++)
	    printf(" %2d  |", data_p[cntr].cr_direction);

	  printf("\n          +-----+-----+-----+-----+-----+-----+-----+-----+\n");
	}
	do_usr_wait = 1;
      }
      break;
    case 9:	/* counter enable register */
      printout_cntr_enable_reg_and_ext_start(handle, lun);
      do_usr_wait = 0;
      break;
    case 1:	/* ch1 */
      printout_conf_reg(handle, lun, 1);
      do_usr_wait = 0;
      break;
    case 2:	/* ch2 */
      printout_conf_reg(handle, lun, 2);
      do_usr_wait = 0;
      break;
    case 3:	/* ch3 */
      printout_conf_reg(handle, lun, 3);
      do_usr_wait = 0;
      break;
    case 4:	/* ch4 */
      printout_conf_reg(handle, lun, 4);
      do_usr_wait = 0;
      break;
    case 5:	/* ch5 */
      printout_conf_reg(handle, lun, 5);
      do_usr_wait = 0;
      break;
    case 6:	/* ch6 */
      printout_conf_reg(handle, lun, 6);
      do_usr_wait = 0;
      break;
    case 7:	/* ch7 */
      printout_conf_reg(handle, lun, 7);
      do_usr_wait = 0;
      break;
    case 8:	/* ch8 */
      printout_conf_reg(handle, lun, 8);
      do_usr_wait = 0;
      break;
    default:
      printf("Please enter a valid menu item.\n\n<enter> to continue");
      getchar();
      break;
    }

    if (do_usr_wait) {
      printf("\n<enter> to continue");
      getchar();
    }
  }
}
