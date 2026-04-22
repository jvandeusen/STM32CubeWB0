/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dt_serv.h
  * @author  MCD Application Team
  * @brief   Header for dt_serv.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DT_SERV_H
#define DT_SERV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ble_status.h"
/* USER CODE BEGIN Includes */
#include "uart_service_uuid.h"

/* USER CODE END Includes */

/* Exported defines ----------------------------------------------------------*/
/* USER CODE BEGIN ED */
#define DATA_NOTIFICATION_MAX_PACKET_SIZE (244U)
/* USER CODE END ED */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  DT_SERV_TX_CHAR,
  DT_SERV_RX_CHAR,

  /* USER CODE BEGIN Service1_CharOpcode_t */

  /* USER CODE END Service1_CharOpcode_t */

  DT_SERV_CHAROPCODE_LAST
} DT_SERV_CharOpcode_t;

typedef enum
{
  DT_SERV_TX_CHAR_NOTIFY_ENABLED_EVT,
  DT_SERV_TX_CHAR_NOTIFY_DISABLED_EVT,
  DT_SERV_RX_CHAR_WRITE_EVT,

  /* USER CODE BEGIN Service1_OpcodeEvt_t */

  /* USER CODE END Service1_OpcodeEvt_t */

  DT_SERV_BOOT_REQUEST_EVT
} DT_SERV_OpcodeEvt_t;

typedef struct
{
  uint8_t *p_Payload;
  uint16_t Length;

  /* USER CODE BEGIN Service1_Data_t */

  /* USER CODE END Service1_Data_t */

} DT_SERV_Data_t;

typedef struct
{
  DT_SERV_OpcodeEvt_t       EvtOpcode;
  DT_SERV_Data_t             DataTransfered;
  uint16_t                ConnectionHandle;
  uint16_t                AttributeHandle;
  uint8_t                 ServiceInstance;

  /* USER CODE BEGIN Service1_NotificationEvt_t */

  /* USER CODE END Service1_NotificationEvt_t */

} DT_SERV_NotificationEvt_t;

/* USER CODE BEGIN ET */
typedef enum
{
  DT_SERV_CHAR_HANDLE_TX = DT_SERV_GATT_ID_UART_TX_CHAR,
  DT_SERV_CHAR_HANDLE_RX = DT_SERV_GATT_ID_UART_RX_CHAR
} DT_SERV_CharHandleId_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ------------------------------------------------------- */
void DT_SERV_Init(void);
void DT_SERV_Notification(DT_SERV_NotificationEvt_t *p_Notification);
tBleStatus DT_SERV_UpdateValue(DT_SERV_CharOpcode_t CharOpcode, DT_SERV_Data_t *pData);
tBleStatus DT_SERV_NotifyValue(DT_SERV_CharOpcode_t CharOpcode, DT_SERV_Data_t *pData, uint16_t ConnectionHandle);
/* USER CODE BEGIN EF */
#define UART_RX_RING_SIZE (512U)

typedef void (*UART_RX_Callback_t)(void);

void UART_RegisterRxCallback(UART_RX_Callback_t cb);
uint8_t UART_RX_Pending(void);
uint16_t UART_RX_Available(void);
uint16_t UART_RX_Read(uint8_t *out, uint16_t max_len);
tBleStatus UART_TX_Notify(uint8_t *data, uint16_t len);

/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*DT_SERV_H */
