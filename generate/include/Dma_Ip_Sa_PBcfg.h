/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : DMA,CACHE,TRGMUX,LCU,EMIOS,FLEXIO
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 6.0.0
*   Build Version        : S32K3_RTD_6_0_0_QLP04_D2508_ASR_REL_4_7_REV_0000_20250822
*
*   Copyright 2020 - 2025 NXP
*
*
*   NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be 
*   used strictly in accordance with the applicable license terms.  By expressly 
*   accepting such terms or by downloading, installing, activating and/or otherwise 
*   using the software, you are agreeing that you have read, and that you agree to 
*   comply with and are bound by, such license terms.  If you do not agree to be 
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/
#ifndef DMA_IP_SA_PBCFG_H_
#define DMA_IP_SA_PBCFG_H_
    
#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define DMA_IP_SA_PBCFG_VENDOR_ID                     43
#define DMA_IP_SA_PBCFG_AR_RELEASE_MAJOR_VERSION      4
#define DMA_IP_SA_PBCFG_AR_RELEASE_MINOR_VERSION      7
#define DMA_IP_SA_PBCFG_AR_RELEASE_REVISION_VERSION   0
#define DMA_IP_SA_PBCFG_SW_MAJOR_VERSION              6
#define DMA_IP_SA_PBCFG_SW_MINOR_VERSION              0
#define DMA_IP_SA_PBCFG_SW_PATCH_VERSION              0
    

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
*                                             ENUMS
==================================================================================================*/

/*==================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/


/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/


/* Number Of Configured Logic Instances */
#define DMA_IP_SA_NOF_CFG_LOGIC_INSTANCES   ((uint32)1U)

/* Logic Instance 0 */
#define SA_DMA_LOGIC_INST_0   ((uint32)0U)

/* Number Of Configured Logic Channels */
#define DMA_IP_SA_NOF_CFG_LOGIC_CHANNELS  ((uint32)2U)

/* Logic Channel 0 */
#define SA_DMA_LOGIC_CH_0   ((uint8)0U)

/* Logic Channel 1 */
#define SA_DMA_LOGIC_CH_1   ((uint8)1U)

#ifdef __cplusplus
}
#endif

#endif
/*==================================================================================================
 *                                        END OF FILE
==================================================================================================*/

