#ifndef _CTC_DEFINITIONS_H_INCLUDE_
#define _CTC_DEFINITIONS_H_INCLUDE_

#define F_OUT_CH 1	//!< first output channel
#define L_OUT_CH 8	//!< last output channel

#define F_INP_CH 1	//!< first external trigger channel
#define L_INP_CH 40	//!< last external trigger channel

#define F_CNTR_CLK 1 //!< first clock counter
#define L_CNTR_CLK 6 //!< last clock counter

#define REGS_PER_CHAN 6	/* each channel is this register long */

#define CHECK_OUT_CHAN(ch)			\
({						\
  __label__ found;				\
  int _rid = -1;				\
  if (!WITHIN_RANGE(0, ch, L_OUT_CH))		\
    goto found; /* out of range */		\
  else						\
    switch (ch) {				\
    case 0:					\
      _rid = ALL_CHANNELS_ID;			\
      break;					\
    case F_OUT_CH:				\
      _rid = CHANNEL_1_ID;			\
      break;					\
    case 2:					\
      _rid = CHANNEL_2_ID;			\
      break;					\
    case 3:					\
      _rid = CHANNEL_3_ID;			\
      break;					\
    case 4:					\
      _rid = CHANNEL_4_ID;			\
      break;					\
    case 5:					\
      _rid = CHANNEL_5_ID;			\
      break;					\
    case 6:					\
      _rid = CHANNEL_6_ID;			\
      break;					\
    case 7:					\
      _rid = CHANNEL_7_ID;			\
      break;					\
    case L_OUT_CH:				\
      _rid = CHANNEL_8_ID;			\
      break;					\
    }						\
  found:					\
  _rid;						\
})

/*
   'Channel Configuration register' layout:

   bits [29 -> 24] - External start (i.e. input channel) select [1 - 40]
   bits [22 -> 20] - Clock1 select
   bits [18 -> 16] - Clock2 select
   bit 1           - Direction (Up - 1, Down - 0)
   bit 0           - Mode (Normal - 0, Up/Down - 1)
   XX              - not used

bit  31 30 29 ....... 24 23 22 ..... 20 19 18 ..... 16 15..2   1       0
    |--|--|-------------|--|-----------|--|-----------|-----|----|---------|
    |XX|XX|Ext Start sel|XX|cntr1 clock|XX|cntr2 clock||XXXX|Mode|Direction|
    |--|--|-------------|--|-----------|--|-----------|-----|----|---------|


  Note on Clock1 and Clock2 values:
  0 - clock1
  1 - clock2
  2 - clock3 etc...
  I.e. we always have some clock set (clock1 by default as it's value is 0)

  The same for the External Start Channel Select:
  0  - Channel 1
  1  - Channel 2
  2  - Channel 3
  ...
  39 - Channel 40
  I.e. we always have some External Start selected (Ext Start 1 by default as
  it's value is 0)

 */

typedef struct _CTC_config_reg {
  int cr_ext_start;	/*  */
  int cr_cntr1_clk;	/*  */
  int cr_cntr2_clk;	/*  */
  int cr_mode;		/*  */
  int cr_direction;	/*  */
} ctc_cfg_reg_t;

#define CHCFG_ESC_MASK 0x3F000000L
#define CHCFG_C1S_MASK 0x700000L
#define CHCFG_C2S_MASK 0x70000L
#define CHCFG_MOD_MASK 0x2L
#define CHCFG_DIR_MASK 0x1L

#define CHCFG_ESC_SHIFT 24
#define CHCFG_C1S_SHIFT 20
#define CHCFG_C2S_SHIFT 16
#define CHCFG_MOD_SHIFT 1
#define CHCFG_DIR_SHIFT 0

#define CHCFG_ESC_SIGBITS_MASK 0x3F
#define CHCFG_C1S_SIGBITS_MASK 0x7
#define CHCFG_C2S_SIGBITS_MASK 0x7
#define CHCFG_MOD_SIGBITS_MASK 0x2
#define CHCFG_DIR_SIGBITS_MASK 0x1

#endif /* _CTC_DEFINITIONS_H_INCLUDE_ */
