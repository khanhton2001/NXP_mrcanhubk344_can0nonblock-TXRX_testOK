/*!
** Copyright 2019 NXP
** @file main.c
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


/* Including necessary configuration files. */
#include "Mcal.h"
#include "Clock_Ip.h"
#include "FlexCAN_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Emios_Pwm_Ip.h"
#include "Emios_Pwm_Ip_Sa_PBcfg.h"
#include "Emios_Mcl_Ip.h"
#include "Dma_Ip.h"
#include "Dma_Ip_Sa_PBcfg.h"

#define MSG_ID 0xC0FFEEu
#define RX_MB_IDX 9U   /* MB 0..7 are taken by the legacy RX FIFO + filters in FlexCAN_Config0 */
#define TX_MB_IDX 8U

/* ---- CAN RX test (polling) ------------------------------------------------------------
 * Two receive mailboxes, exact ID match (mask left at the driver default = all bits compared):
 *   MB 9  : STANDARD  ID 0x123
 *   MB 10 : EXTENDED  ID 0x18DAF100
 * From the PC (Waveshare USB-CAN) send exactly these IDs, 1 Mbit/s, any 8 data bytes.
 * Watch in the debugger:  canRxCount, canRxLast, canRxLog[]                               */
#define RX_STD_MB_IDX   9U
#define RX_EXT_MB_IDX   10U
#define RX_STD_ID       0x123UL
#define RX_EXT_ID       0x18DAF100UL
#define RX_LOG_SIZE     16U

/* 1 = accept ALL IDs (mask = 0, "don't care" on every bit): MB 9 gets every standard frame,
 *     MB 10 gets every extended frame.  RX_STD_ID / RX_EXT_ID are then ignored.
 * 0 = accept only RX_STD_ID / RX_EXT_ID (exact match).                                    */
#define CAN_RX_ACCEPT_ALL   1

typedef struct
{
	uint32 seq;        /* running frame number            */
	uint32 id;         /* 11-bit or 29-bit ID             */
	uint8  ext;        /* 1 = extended, 0 = standard      */
	uint8  dlc;        /* number of data bytes            */
	uint8  data[8];
	uint32 timestamp;  /* FlexCAN free-running timer      */
} CanRxRecord;

volatile CanRxRecord canRxLog[RX_LOG_SIZE];   /* ring buffer, newest at (canRxCount-1) % RX_LOG_SIZE */
volatile CanRxRecord canRxLast;               /* last frame received                                 */
volatile uint32      canRxCount = 0U;         /* total frames received                               */
volatile uint32      canRxStdCount = 0U, canRxExtCount = 0U;
static Flexcan_Ip_MsgBuffType rxMsgStd, rxMsgExt;   /* driver fills these on reception */
Flexcan_Ip_DataInfoType rx_info_std = { .msg_id_type = FLEXCAN_MSG_ID_STD, .data_length = 8u, .is_polling = TRUE, .is_remote = FALSE };
Flexcan_Ip_DataInfoType rx_info_ext = { .msg_id_type = FLEXCAN_MSG_ID_EXT, .data_length = 8u, .is_polling = TRUE, .is_remote = FALSE };
volatile int exit_code = 0;
/* User includes */
uint8 dummyData[8] = {1,2,3,4,5,6,7};
Flexcan_Ip_DataInfoType tx_info = {
		.msg_id_type = FLEXCAN_MSG_ID_STD,
		.data_length = 8u,
		.is_polling = TRUE,
		.is_remote = FALSE
	};
Flexcan_Ip_DataInfoType rx_info = {
		.msg_id_type = FLEXCAN_MSG_ID_STD,
		.data_length = 8u,
		.is_polling = TRUE,
		.is_remote = FALSE
	};
/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
extern void CAN0_ORED_0_31_MB_IRQHandler(void);
uint8 sendData[8]={0,0,0,0,0,0,0,0};


void setupCan(void)
{

	//uint8 sendData[8]={0,0,0,0,0,0,0,0};
	/* Filling of the frame's payload for transmission */
	//Siul2_Dio_Ip_WritePin(PTB_L_HALF, 12, 0U);
	Siul2_Dio_Ip_WritePin(STB_PORT, STB_PIN, 1U);
	Siul2_Dio_Ip_WritePin(EN_PORT, EN_PIN, 1U);

	FlexCAN_Ip_Init(INST_FLEXCAN_0, &FlexCAN_State0, &FlexCAN_Config0);

#if CAN_RX_ACCEPT_ALL
	/* Masks can only be changed while the module is stopped/frozen: after Init, BEFORE SetStartMode.
	 * Mask bit 1 = "this ID bit must match", 0 = "don't care". The driver default after Init is all ones. */
	(void)FlexCAN_Ip_SetRxMaskType(INST_FLEXCAN_0, FLEXCAN_RX_MASK_GLOBAL);
	(void)FlexCAN_Ip_SetRxMbGlobalMask(INST_FLEXCAN_0, 0U);
	(void)FlexCAN_Ip_SetRxIndividualMask(INST_FLEXCAN_0, RX_STD_MB_IDX, 0U);   /* harmless, in case individual masks are on */
	(void)FlexCAN_Ip_SetRxIndividualMask(INST_FLEXCAN_0, RX_EXT_MB_IDX, 0U);
#endif
	FlexCAN_Ip_SetStartMode(INST_FLEXCAN_0);
	tx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
	tx_info.data_length=8;
	FlexCAN_Ip_SendBlocking(INST_FLEXCAN_0, TX_MB_IDX, &tx_info, 0x555, sendData, 1000);
	uint32_t ID_Stinger = 0x18DA00F1;
	uint32_t PAYLOAD_FEC[2];
	uint8_t COMMAND_BYTE_FEC = 0x10;
	uint8_t DLC_FEC = 6;
	uint8_t Filter_Element_Index_0 = 0;
	uint8_t Filter_Element_Type = 1;
	uint8_t Extended_Filter_Config = 3; /* Both TWL and BBL */
	uint32_t ID_TX = 0xC0FFEE;

	/* Formation of the payload */
	uint32_t Filter_Element_Contents =  (Extended_Filter_Config << 29) | ID_TX ;

	PAYLOAD_FEC[0] = (COMMAND_BYTE_FEC << 24) | (Filter_Element_Index_0 << 16) |
					 (Filter_Element_Type << 15) | (Filter_Element_Contents >> 16);

	PAYLOAD_FEC[1] = (Filter_Element_Contents & 0xFFFF) << 16;
	tx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
	tx_info.data_length=DLC_FEC;
	sendData[3]=(PAYLOAD_FEC[0] & 0x000000FF);
	sendData[2]=(PAYLOAD_FEC[0] & 0x0000FF00) >> 8;
	sendData[1]=(PAYLOAD_FEC[0] & 0x00FF0000) >> 16;
	sendData[0]=(PAYLOAD_FEC[0] & 0xFF000000) >> 24;
	sendData[7]=(PAYLOAD_FEC[1] & 0x000000FF);
	sendData[6]=(PAYLOAD_FEC[1] & 0x0000FF00) >> 8;
	sendData[5]=(PAYLOAD_FEC[1] & 0x00FF0000) >> 16;
	sendData[4]=(PAYLOAD_FEC[1] & 0xFF000000) >> 24;
	FlexCAN_Ip_SendBlocking(INST_FLEXCAN_0, TX_MB_IDX, &tx_info, ID_Stinger, sendData, 1000);

	PAYLOAD_FEC[0] = 0x71020304;
	PAYLOAD_FEC[1] = 0x05060708;
	sendData[3]=(PAYLOAD_FEC[0] & 0x000000FF);
	sendData[2]=(PAYLOAD_FEC[0] & 0x0000FF00) >> 8;
	sendData[1]=(PAYLOAD_FEC[0] & 0x00FF0000) >> 16;
	sendData[0]=(PAYLOAD_FEC[0] & 0xFF000000) >> 24;
	sendData[7]=(PAYLOAD_FEC[1] & 0x000000FF);
	sendData[6]=(PAYLOAD_FEC[1] & 0x0000FF00) >> 8;
	sendData[5]=(PAYLOAD_FEC[1] & 0x00FF0000) >> 16;
	sendData[4]=(PAYLOAD_FEC[1] & 0xFF000000) >> 24;
	tx_info.data_length=8;
	FlexCAN_Ip_SendBlocking(INST_FLEXCAN_0, TX_MB_IDX, &tx_info, ID_Stinger, sendData, 1000);
	//Siul2_Dio_Ip_WritePin(PTB_L_HALF, 12, 1U);

	/* RX mailboxes: configure the ID, then arm them (polling) */
	(void)FlexCAN_Ip_ConfigRxMb(INST_FLEXCAN_0, RX_STD_MB_IDX, &rx_info_std, RX_STD_ID);
	(void)FlexCAN_Ip_ConfigRxMb(INST_FLEXCAN_0, RX_EXT_MB_IDX, &rx_info_ext, RX_EXT_ID);
	(void)FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_STD_MB_IDX, &rxMsgStd, TRUE);
	(void)FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_EXT_MB_IDX, &rxMsgExt, TRUE);
}

/* Copy a received frame into the debug log */
static void Can_RxLog(const Flexcan_Ip_MsgBuffType *m, uint8 ext)
{
	uint32 slot = canRxCount % RX_LOG_SIZE;
	canRxLog[slot].seq       = canRxCount + 1U;
	canRxLog[slot].id        = m->msgId;
	canRxLog[slot].ext       = ext;
	canRxLog[slot].dlc       = m->dataLen;
	for (uint8 i = 0U; i < 8U; i++) { canRxLog[slot].data[i] = m->data[i]; }
	canRxLog[slot].timestamp = m->time_stamp;
	canRxLast = canRxLog[slot];
	canRxCount++;
}

/* Polling RX service - call often from the main loop.
 * MainFunctionRead() checks the mailbox flag and, if a frame arrived, the driver copies it into rxMsgXxx
 * and sets the mailbox state to IDLE. We log it and re-arm the mailbox with FlexCAN_Ip_Receive().      */
static void Can_RxService(void)
{
	FlexCAN_Ip_MainFunctionRead(INST_FLEXCAN_0, RX_STD_MB_IDX);
	if (FlexCAN_Ip_GetTransferStatus(INST_FLEXCAN_0, RX_STD_MB_IDX) == FLEXCAN_STATUS_SUCCESS)
	{
		Can_RxLog(&rxMsgStd, 0U);
		canRxStdCount++;
		(void)FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_STD_MB_IDX, &rxMsgStd, TRUE);
	}

	FlexCAN_Ip_MainFunctionRead(INST_FLEXCAN_0, RX_EXT_MB_IDX);
	if (FlexCAN_Ip_GetTransferStatus(INST_FLEXCAN_0, RX_EXT_MB_IDX) == FLEXCAN_STATUS_SUCCESS)
	{
		Can_RxLog(&rxMsgExt, 1U);
		canRxExtCount++;
		(void)FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_EXT_MB_IDX, &rxMsgExt, TRUE);
	}
}


/* =====================================================================================
 * WS2812 on eMIOS_0 CH1 (PTB13, header P8A pin 6, GND pin 4) driven by eDMA.
 *   eMIOS_0 CH1 OPWFMB, period 100 ticks of 12.5 ns = 1.25 us (800 kHz)
 *   bit 0 = 33 ticks high, bit 1 = 66 ticks high, reset = zero duty for WS_RESET_SLOTS periods
 *   Every period the eMIOS CH1 flag raises a DMA request, the eDMA copies the NEXT duty value
 *   (uint32) into UC[1].A (A is buffered, applied at the next period start).
 * ===================================================================================== */
#define WS_EMIOS_INST     0U
#define WS_EMIOS_CH       1U
#define WS_NUM_LEDS       200U
#define WS_LEVEL          120U      /* brightness of the colour (0..255) */
#define WS_HOLD_LOOPS    20000000UL        /* extra wait after each colour; 0 = send the next colour immediately (~1 loop = 50 ns) */
#define WS_BIT0           33U
#define WS_BIT1           66U
#define WS_RESET_SLOTS    50U                         /* 50 x 1.25 us = 62.5 us low reset, same as the STM32 project */
#define WS_IDLE_HIGH      0                           /* 1 = line idles HIGH between frames like the working device, 0 = idles LOW */
#if WS_IDLE_HIGH
#define WS_IDLE_STATE     EMIOS_PWM_IP_OUTPUT_STATE_HIGH
#else
#define WS_IDLE_STATE     EMIOS_PWM_IP_OUTPUT_STATE_LOW
#endif
#define WS_NBITS          (WS_NUM_LEDS * 24U)
#define WS_BUF_LEN        (WS_NBITS + WS_RESET_SLOTS)

/* eDMA hardware channel used (must match the Dma_Ip logic channel 0 -> hw channel 0 in the config) */
#define WS_DMA_LOGIC_CH   0U
#define WS_DMA_HW_CH      0U

/* DMAMUX0 source for eMIOS_0 "DMA request 1" (= CH1): 13, from S32K3xx_DMAMUX_map.xlsx (sheet DMAMUX0,
 * attached to the S32K3xx RM; S32K344 column = YES). */
#define WS_DMAMUX_SRC_EMIOS0_CH1   13U

#define WS_IRQ_OFF()   __asm volatile ("cpsid i" ::: "memory")
#define WS_IRQ_ON()    __asm volatile ("cpsie i" ::: "memory")

static uint32 wsBuf[WS_BUF_LEN] __attribute__((aligned(32)));  /* element 0 is written by the CPU, 1..end read by DMA */

static void Ws2812_SetColorAll(uint8 r, uint8 g, uint8 b)
{
    uint32 color = ((uint32)g << 16) | ((uint32)r << 8) | (uint32)b;   /* GRB, MSB first */
    uint32 n = 0U;
    for (uint32 led = 0U; led < WS_NUM_LEDS; led++)
    {
        for (int bit = 23; bit >= 0; bit--)
        {
            wsBuf[n++] = (color & (1UL << bit)) ? WS_BIT1 : WS_BIT0;
        }
    }
    for (uint32 i = 0U; i < WS_RESET_SLOTS; i++)
    {
        wsBuf[n++] = 0U;             /* zero duty = line low = reset */
    }
}


/* Cortex-M7 D-cache is write-back: the CPU's wsBuf writes sit in the cache and the eDMA reads stale RAM
 * (the capture shows holes in whole 32-byte = 8-entry blocks). Clean (write back) the buffer to RAM.
 * DCCMVAC register = 0xE000EF68 (clean D-cache line by address to PoC). */
#define WS_SCB_DCCMVAC   (*(volatile uint32 *)0xE000EF68UL)
static void Ws2812_CleanBuffer(void)
{
    uint32 addr = ((uint32)wsBuf) & ~31UL;
    uint32 end  = (uint32)wsBuf + (uint32)sizeof(wsBuf);
    __asm volatile ("dsb 0xF" ::: "memory");
    for (; addr < end; addr += 32U) { WS_SCB_DCCMVAC = addr; }
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");
}

/* Global eMIOS config: normally generated by the Mcl tab (enable "eMIOS common", instance EMIOS_0,
 * Global Time Base enabled, prescaler 1 -> /2, master bus channel 0 free-running).
 * Local copy used here so the project builds without regenerating Mcl. */
static const Emios_Ip_GlobalConfigType ws_emiosGlobalCfg =
{
    FALSE,   /* allowDebugMode */
    0U,      /* clkDivVal: the driver writes this straight into MCR[GPRE], so 0 = /1 -> 160 MHz to the channels
              * (channel prescaler DIV_2 then gives 80 MHz, 1 tick = 12.5 ns). A value of 1 would give /2 -> 40 MHz. */
    TRUE     /* enableGlobalTimeBase */
};

static const Emios_Mcl_Ip_ConfigType ws_emiosMclCfg =
{
    0U,                    /* channelsNumber: no master bus needed, CH1 uses its own internal counter */
    &ws_emiosGlobalCfg,    /* emiosGlobalConfig is a POINTER */
    NULL_PTR               /* masterBusConfig */
};

static void Ws2812_Init(void)
{
    /* 1. eMIOS global: prescaler + global time base (writes MCR). */
    Emios_Mcl_Ip_Init(WS_EMIOS_INST, &ws_emiosMclCfg);

    /* 2. PWM channel 1: OPWFMB, period 100, duty 32 (from Emios_Pwm_Ip_Sa_I0_Ch1) */
    Emios_Pwm_Ip_InitChannel(WS_EMIOS_INST, &Emios_Pwm_Ip_Sa_I0_Ch1);

    /* 3. idle level until a frame is sent */
    Emios_Pwm_Ip_SetOutputState(WS_EMIOS_INST, WS_EMIOS_CH, WS_IDLE_STATE);

    /* 4. eDMA */
    (void)Dma_Ip_Init(&Dma_Ip_Sa_xDmaInitPB);

    /* 5. DMAMUX0: route eMIOS_0 CH1 request to eDMA channel WS_DMA_HW_CH.
     *    CHCFG bytes are lane-swapped: eDMA channel n is at CHCFG[n ^ 3] (NXP forum: channel 0 -> CHCFG[3]).
     *    Value = ENBL (0x80) | source. Check in the debugger that the register changed. */
    IP_DMAMUX_0->CHCFG[WS_DMA_HW_CH ^ 3U] = 0U;
    IP_DMAMUX_0->CHCFG[WS_DMA_HW_CH ^ 3U] = (uint8)(0x80U | (WS_DMAMUX_SRC_EMIOS0_CH1 & 0x3FU));
}

static void Ws2812_Send(void)
{
    Dma_Ip_LogicChannelTransferListType tl[11];
    Dma_Ip_LogicChannelStatusType st;

    /* DMA moves wsBuf[1..] -> UC[1].A, one 32-bit word per request */
    tl[0].Param  = DMA_IP_CH_SET_SOURCE_ADDRESS;                 tl[0].Value  = (uint32)&wsBuf[1];
    tl[1].Param  = DMA_IP_CH_SET_SOURCE_SIGNED_OFFSET;           tl[1].Value  = 4U;
    tl[2].Param  = DMA_IP_CH_SET_SOURCE_TRANSFER_SIZE;           tl[2].Value  = DMA_IP_TRANSFER_SIZE_4_BYTE;
    tl[3].Param  = DMA_IP_CH_SET_SOURCE_SIGNED_LAST_ADDR_ADJ;    tl[3].Value  = 0U;
    tl[4].Param  = DMA_IP_CH_SET_DESTINATION_ADDRESS;            tl[4].Value  = (uint32)&IP_EMIOS_0->CH.UC[WS_EMIOS_CH].A;
    tl[5].Param  = DMA_IP_CH_SET_DESTINATION_SIGNED_OFFSET;      tl[5].Value  = 0U;
    tl[6].Param  = DMA_IP_CH_SET_DESTINATION_TRANSFER_SIZE;      tl[6].Value  = DMA_IP_TRANSFER_SIZE_4_BYTE;
    tl[7].Param  = DMA_IP_CH_SET_DESTINATION_SIGNED_LAST_ADDR_ADJ; tl[7].Value = 0U;
    tl[8].Param  = DMA_IP_CH_SET_MINORLOOP_SIZE;                 tl[8].Value  = 4U;
    tl[9].Param  = DMA_IP_CH_SET_MAJORLOOP_COUNT;                tl[9].Value  = WS_BUF_LEN - 1U;
    tl[10].Param = DMA_IP_CH_SET_CONTROL_DIS_AUTO_REQUEST;       tl[10].Value = TRUE;   /* clear ERQ when done */

    (void)Dma_Ip_SetLogicChannelCommand(WS_DMA_LOGIC_CH, DMA_IP_CH_CLEAR_DONE);
    (void)Dma_Ip_SetLogicChannelTransferList(WS_DMA_LOGIC_CH, tl, 11U);

    /* first bit by CPU while channel is still in GPIO-low mode */
    IP_EMIOS_0->CH.UC[WS_EMIOS_CH].A = wsBuf[0];
    (void)Dma_Ip_SetLogicChannelCommand(WS_DMA_LOGIC_CH, DMA_IP_CH_SET_HARDWARE_REQUEST);

    /* start PWM, THEN enable the flag -> DMA request (SetOutputState LOW clears FEN, so order matters) */
    Emios_Pwm_Ip_SetOutputToNormal(WS_EMIOS_INST, WS_EMIOS_CH, 0U,
                                   EMIOS_PWM_IP_ACTIVE_HIGH, EMIOS_PWM_IP_MODE_OPWFMB_FLAG);
    Emios_Pwm_Ip_SetFlagRequest(WS_EMIOS_INST, WS_EMIOS_CH, EMIOS_PWM_IP_DMA_REQUEST);

    /* wait for DMA done (all bits + reset slots sent) */
    do { (void)Dma_Ip_GetLogicChannelStatus(WS_DMA_LOGIC_CH, &st); } while (!st.Done && (st.Errors == 0U));

    /* Done = last reset slot has been loaded; let it finish (>= 2 periods), then hold the line LOW */
    for (volatile uint32 d = 0U; d < 100U; d++) { }
    Emios_Pwm_Ip_SetOutputState(WS_EMIOS_INST, WS_EMIOS_CH, EMIOS_PWM_IP_OUTPUT_STATE_LOW);
}

int main(void)
{
	//Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);
	Clock_Ip_Init(&Mcu_aClockConfigPB[0]);
	Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals , g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);


	setupCan();

	/* LED TEST ONLY, same behaviour as the STM32 project:
	 *   data buffer = 200 LEDs x 24 bits (33/66 ticks) + 50 zero slots (62.5 us low reset)
	 *   DMA runs once through the buffer, then the PWM is stopped and the line is held LOW.
	 *   Colour (R,G,B) = (1,1,120), blue dominant, like WS2812_SetColor(&LED_channel_A, 1, 1, 120). */
	Ws2812_Init();                       /* eMIOS CH1 + eDMA, pin idles LOW */

	Ws2812_SetColorAll(1U, 1U, 120U);
	Ws2812_CleanBuffer();                /* D-cache -> RAM so the DMA sees the data (not needed on the STM32) */

	while(1)
	{
		/* NON-BLOCKING equivalent of the SendBlocking() line: same mailbox, same tx_info, same ID, same sendData */
		FlexCAN_Ip_MainFunctionWrite(INST_FLEXCAN_0, TX_MB_IDX);   /* close the previous frame if it was sent */
		(void)FlexCAN_Ip_AbortTransfer(INST_FLEXCAN_0, TX_MB_IDX); /* free the MB if still pending (what the SendBlocking timeout does) */
		(void)FlexCAN_Ip_Send(INST_FLEXCAN_0, TX_MB_IDX, &tx_info, 0x555, sendData);

		Can_RxService();

		Ws2812_Send();                   /* 4800 bits + 50 zero slots, then line LOW */
		Can_RxService();
		for (volatile uint32 d = 0U; d < 2000000UL; d++)       /* ~100 ms, then send the same frame again */
		{
			if ((d & 0x3FFU) == 0U) { Can_RxService(); }       /* keep listening while waiting */
		}
	}
	return 0;
}

/* END main */
