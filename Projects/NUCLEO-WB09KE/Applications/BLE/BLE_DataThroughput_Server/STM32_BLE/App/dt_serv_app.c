/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dt_serv_app.c
  * @author  MCD Application Team
  * @brief   UART service application definition.
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "main.h"
#include "app_common.h"
#include "app_ble.h"
#include "ble.h"
#include "dt_serv_app.h"
#include "dt_serv.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  uint8_t buffer[UART_RX_RING_SIZE];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
  uint8_t pending;
  UART_RX_Callback_t callback;
} UART_RxRing_t;
/* USER CODE END PTD */

typedef struct
{
  uint16_t ConnectionHandle;
  uint8_t NotificationsEnabled;
} DT_SERV_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_CMD_LED_ON  "LED=1"
#define UART_CMD_LED_OFF "LED=0"
#define UART_CMD_MAX_LEN (32U)
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static DT_SERV_APP_Context_t DT_SERV_APP_Context;

/* USER CODE BEGIN PV */
static UART_RxRing_t UART_RxRing;
static uint8_t UART_CmdLine[UART_CMD_MAX_LEN];
static uint16_t UART_CmdLineLen;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void UART_RxRingPush(const uint8_t *data, uint16_t len);
static void UART_ProcessChunk(const uint8_t *data, uint16_t len);
static void UART_CommandAck(uint8_t ok);
static void UART_HandleLineCommand(void);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void DT_SERV_Notification(DT_SERV_NotificationEvt_t *p_Notification)
{
  if (p_Notification == NULL)
  {
    return;
  }

  switch (p_Notification->EvtOpcode)
  {
    case DT_SERV_TX_CHAR_NOTIFY_ENABLED_EVT:
      DT_SERV_APP_Context.NotificationsEnabled = 1U;
      break;

    case DT_SERV_TX_CHAR_NOTIFY_DISABLED_EVT:
      DT_SERV_APP_Context.NotificationsEnabled = 0U;
      break;

    case DT_SERV_RX_CHAR_WRITE_EVT:
      break;

    default:
      break;
  }
}

void DT_SERV_APP_EvtRx(DT_SERV_APP_ConnHandleNotEvt_t *p_Notification)
{
  if (p_Notification == NULL)
  {
    return;
  }

  switch (p_Notification->EvtOpcode)
  {
    case DT_SERV_CONN_HANDLE_EVT:
      DT_SERV_APP_Context.ConnectionHandle = p_Notification->ConnectionHandle;
      break;

    case DT_SERV_DISCON_HANDLE_EVT:
      DT_SERV_APP_Context.ConnectionHandle = 0xFFFFU;
      DT_SERV_APP_Context.NotificationsEnabled = 0U;
      break;

    default:
      break;
  }
}

void DT_SERV_APP_Init(void)
{
  DT_SERV_APP_Context.ConnectionHandle = 0xFFFFU;
  DT_SERV_APP_Context.NotificationsEnabled = 0U;

  UART_RxRing.head = 0U;
  UART_RxRing.tail = 0U;
  UART_RxRing.count = 0U;
  UART_RxRing.pending = 0U;
  UART_RxRing.callback = NULL;
  UART_CmdLineLen = 0U;

  DT_SERV_Init();
}

/* USER CODE BEGIN FD */
void DTS_Button1TriggerReceived(void)
{
}

void DTS_Button2TriggerReceived(void)
{
}

void DTS_Button3TriggerReceived(void)
{
}

void UART_RegisterRxCallback(UART_RX_Callback_t cb)
{
  UART_RxRing.callback = cb;
}

uint8_t UART_RX_Pending(void)
{
  return UART_RxRing.pending;
}

uint16_t UART_RX_Available(void)
{
  return UART_RxRing.count;
}

uint16_t UART_RX_Read(uint8_t *out, uint16_t max_len)
{
  uint16_t read_count = 0U;

  if ((out == NULL) || (max_len == 0U))
  {
    return 0U;
  }

  while ((read_count < max_len) && (UART_RxRing.count > 0U))
  {
    out[read_count] = UART_RxRing.buffer[UART_RxRing.tail];
    UART_RxRing.tail = (uint16_t)((UART_RxRing.tail + 1U) % UART_RX_RING_SIZE);
    UART_RxRing.count--;
    read_count++;
  }

  if (UART_RxRing.count == 0U)
  {
    UART_RxRing.pending = 0U;
  }

  return read_count;
}

tBleStatus UART_TX_Notify(uint8_t *data, uint16_t len)
{
  DT_SERV_Data_t tx_data;

  if ((data == NULL) || (len == 0U))
  {
    return BLE_STATUS_INVALID_PARAMS;
  }

  if (DT_SERV_APP_Context.ConnectionHandle == 0xFFFFU)
  {
    return BLE_STATUS_FAILED;
  }

  if (DT_SERV_APP_Context.NotificationsEnabled == 0U)
  {
    return BLE_STATUS_FAILED;
  }

  if (len > DATA_NOTIFICATION_MAX_PACKET_SIZE)
  {
    len = DATA_NOTIFICATION_MAX_PACKET_SIZE;
  }

  tx_data.p_Payload = data;
  tx_data.Length = len;

  return DT_SERV_NotifyValue(DT_SERV_TX_CHAR, &tx_data, DT_SERV_APP_Context.ConnectionHandle);
}

void Resume_Notification(void)
{
}

void UART_RX_WriteHandler(const uint8_t *data, uint16_t len)
{
  if ((data == NULL) || (len == 0U))
  {
    return;
  }

  UART_RxRingPush(data, len);
}

void UART_ProcessPendingRx(void)
{
  uint8_t chunk[64];
  uint16_t read_len;

  while (UART_RX_Available() > 0U)
  {
    read_len = UART_RX_Read(chunk, (uint16_t)sizeof(chunk));
    if (read_len == 0U)
    {
      break;
    }

    UART_ProcessChunk(chunk, read_len);
  }
}
/* USER CODE END FD */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */
static void UART_RxRingPush(const uint8_t *data, uint16_t len)
{
  uint16_t i;

  if ((data == NULL) || (len == 0U))
  {
    return;
  }

  for (i = 0U; i < len; i++)
  {
    if (UART_RxRing.count == UART_RX_RING_SIZE)
    {
      UART_RxRing.tail = (uint16_t)((UART_RxRing.tail + 1U) % UART_RX_RING_SIZE);
      UART_RxRing.count--;
    }

    UART_RxRing.buffer[UART_RxRing.head] = data[i];
    UART_RxRing.head = (uint16_t)((UART_RxRing.head + 1U) % UART_RX_RING_SIZE);
    UART_RxRing.count++;
  }

  UART_RxRing.pending = 1U;
  if (UART_RxRing.callback != NULL)
  {
    UART_RxRing.callback();
  }
}

static void UART_ProcessChunk(const uint8_t *data, uint16_t len)
{
  uint16_t i;
  uint8_t had_line_terminator = 0U;
  uint8_t command_in_progress_at_entry = (UART_CmdLineLen > 0U) ? 1U : 0U;

  if ((data == NULL) || (len == 0U))
  {
    return;
  }

  for (i = 0U; i < len; i++)
  {
    uint8_t ch = data[i];

    if (ch == (uint8_t)'\r')
    {
      continue;
    }

    if (ch == (uint8_t)'\n')
    {
      had_line_terminator = 1U;
      UART_HandleLineCommand();
      UART_CmdLineLen = 0U;
      continue;
    }

    if (UART_CmdLineLen < (UART_CMD_MAX_LEN - 1U))
    {
      UART_CmdLine[UART_CmdLineLen++] = ch;
    }
  }

  if ((had_line_terminator == 0U) && (command_in_progress_at_entry == 0U))
  {
    (void)UART_TX_Notify((uint8_t *)data, len);
    UART_CmdLineLen = 0U;
  }
}

static void UART_CommandAck(uint8_t ok)
{
  static uint8_t ack_ok[] = "OK\n";
  static uint8_t ack_err[] = "ERR\n";

  (void)UART_TX_Notify(ok ? ack_ok : ack_err, ok ? (uint16_t)3U : (uint16_t)4U);
}

static void UART_HandleLineCommand(void)
{
  if (UART_CmdLineLen >= UART_CMD_MAX_LEN)
  {
    UART_CmdLineLen = UART_CMD_MAX_LEN - 1U;
  }
  UART_CmdLine[UART_CmdLineLen] = 0U;

  /* USER CODE BEGIN UART_Command_Handling */
  /* Command parse runs in scheduler task context with no blocking operations. */
  if ((UART_CmdLineLen == (uint16_t)(sizeof(UART_CMD_LED_ON) - 1U))
      && (memcmp(UART_CmdLine, UART_CMD_LED_ON, sizeof(UART_CMD_LED_ON) - 1U) == 0))
  {
    HAL_GPIO_WritePin(LD3_GPIO_PORT, LD3_PIN, GPIO_PIN_SET);
    UART_CommandAck(1U);
    return;
  }

  if ((UART_CmdLineLen == (uint16_t)(sizeof(UART_CMD_LED_OFF) - 1U))
      && (memcmp(UART_CmdLine, UART_CMD_LED_OFF, sizeof(UART_CMD_LED_OFF) - 1U) == 0))
  {
    HAL_GPIO_WritePin(LD3_GPIO_PORT, LD3_PIN, GPIO_PIN_RESET);
    UART_CommandAck(1U);
    return;
  }

  UART_CommandAck(0U);
  /* USER CODE END UART_Command_Handling */
}
/* USER CODE END FD_LOCAL_FUNCTIONS */
