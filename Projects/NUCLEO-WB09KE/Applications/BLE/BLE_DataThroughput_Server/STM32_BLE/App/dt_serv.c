/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dt_serv.c
  * @author  MCD Application Team
  * @brief   UART custom service definition.
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
#include <app_common.h>
#include "ble.h"
#include "dt_serv.h"
#include "dt_serv_app.h"
#include "ble_evt.h"

/* USER CODE BEGIN Includes */
#include "uart_service_uuid.h"

/* USER CODE END Includes */

typedef struct {
  uint16_t GattDbHandle[DT_SERV_GATT_ID_COUNT];
} DT_SERV_Context_t;

#define CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET        2U
#define CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET             1U
#define UART_TX_CHAR_SIZE                                 244U
#define UART_RX_CHAR_SIZE                                 244U

/* USER CODE BEGIN PM */

/* USER CODE END PM */

static DT_SERV_Context_t DT_SERV_Context;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* USER CODE BEGIN UUID */

/* USER CODE END UUID */

BLE_GATT_SRV_CCCD_DECLARE(uart_tx_char, CFG_BLE_NUM_RADIO_TASKS, BLE_GATT_SRV_CCCD_PERM_DEFAULT,
                          BLE_GATT_SRV_OP_MODIFIED_EVT_ENABLE_FLAG);

static uint8_t uart_rx_val_buffer[UART_RX_CHAR_SIZE];

static ble_gatt_val_buffer_def_t uart_rx_val_buffer_def = {
  .op_flags = BLE_GATT_SRV_OP_MODIFIED_EVT_ENABLE_FLAG,
  .val_len = UART_RX_CHAR_SIZE,
  .buffer_len = sizeof(uart_rx_val_buffer),
  .buffer_p = uart_rx_val_buffer
};

static const ble_gatt_chr_def_t dt_serv_chars[] = {
  {
    .properties = BLE_GATT_SRV_CHAR_PROP_NOTIFY,
    .permissions = BLE_GATT_SRV_PERM_NONE,
    .min_key_size = 0x10,
    .uuid = BLE_UUID_INIT_128(UART_TX_CHAR_UUID_128),
    .descrs = {
      .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(uart_tx_char),
      .descr_count = 1U,
    },
  },
  {
    .properties = BLE_GATT_SRV_CHAR_PROP_WRITE | BLE_GATT_SRV_CHAR_PROP_WRITE_NO_RESP,
    .permissions = BLE_GATT_SRV_PERM_NONE,
    .min_key_size = 0x10,
    .uuid = BLE_UUID_INIT_128(UART_RX_CHAR_UUID_128),
    .val_buffer_p = &uart_rx_val_buffer_def,
  },
};

static const ble_gatt_srv_def_t dt_serv_service = {
  .type = BLE_GATT_SRV_PRIMARY_SRV_TYPE,
  .uuid = BLE_UUID_INIT_128(UART_SERVICE_UUID_128),
  .chrs = {
    .chrs_p = (ble_gatt_chr_def_t *)dt_serv_chars,
    .chr_count = 2U,
  },
};

static BLEEVT_EvtAckStatus_t DT_SERV_EventHandler(aci_blecore_event *p_evt)
{
  BLEEVT_EvtAckStatus_t return_value = BLEEVT_NoAck;
  DT_SERV_NotificationEvt_t notification;

  memset(&notification, 0, sizeof(notification));

  switch (p_evt->ecode)
  {
    case ACI_GATT_SRV_ATTRIBUTE_MODIFIED_VSEVT_CODE:
    {
      aci_gatt_srv_attribute_modified_event_rp0 *p_attribute_modified;
      p_attribute_modified = (aci_gatt_srv_attribute_modified_event_rp0 *)p_evt->data;

      notification.ConnectionHandle = p_attribute_modified->Connection_Handle;
      notification.AttributeHandle = p_attribute_modified->Attr_Handle;
      notification.DataTransfered.Length = p_attribute_modified->Attr_Data_Length;
      notification.DataTransfered.p_Payload = p_attribute_modified->Attr_Data;

      if (p_attribute_modified->Attr_Handle == (DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_TX_CHAR] + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))
      {
        return_value = BLEEVT_Ack;
        if (p_attribute_modified->Attr_Data[0] == BLE_GATT_SRV_CCCD_NOTIFICATION)
        {
          notification.EvtOpcode = DT_SERV_TX_CHAR_NOTIFY_ENABLED_EVT;
        }
        else
        {
          notification.EvtOpcode = DT_SERV_TX_CHAR_NOTIFY_DISABLED_EVT;
        }
        DT_SERV_Notification(&notification);
      }
      else if (p_attribute_modified->Attr_Handle == (DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_RX_CHAR] + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
      {
        return_value = BLEEVT_Ack;
        notification.EvtOpcode = DT_SERV_RX_CHAR_WRITE_EVT;
        UART_RX_WriteHandler(p_attribute_modified->Attr_Data, p_attribute_modified->Attr_Data_Length);
        DT_SERV_Notification(&notification);
      }
      break;
    }

    case ACI_GATT_SRV_WRITE_VSEVT_CODE:
    {
      aci_gatt_srv_write_event_rp0 *p_write;
      p_write = (aci_gatt_srv_write_event_rp0 *)p_evt->data;

      if (p_write->Attribute_Handle == (DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_RX_CHAR] + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
      {
        return_value = BLEEVT_Ack;
        notification.ConnectionHandle = p_write->Connection_Handle;
        notification.AttributeHandle = p_write->Attribute_Handle;
        notification.EvtOpcode = DT_SERV_RX_CHAR_WRITE_EVT;
        notification.DataTransfered.Length = p_write->Data_Length;
        notification.DataTransfered.p_Payload = p_write->Data;
        DT_SERV_Notification(&notification);
      }
      break;
    }

    case ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE:
    {
      Resume_Notification();
      break;
    }

    default:
      break;
  }

  return return_value;
}

void DT_SERV_Init(void)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  BLEEVT_RegisterGattEvtHandler(DT_SERV_EventHandler);

  ret = aci_gatt_srv_add_service((ble_gatt_srv_def_t *)&dt_serv_service);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_gatt_srv_add_service UART_SERVICE, error code: 0x%x\n", ret);
  }

  DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_SERVICE] = aci_gatt_srv_get_service_handle((ble_gatt_srv_def_t *)&dt_serv_service);
  DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_TX_CHAR] = aci_gatt_srv_get_char_decl_handle((ble_gatt_chr_def_t *)&dt_serv_chars[0]);
  DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_RX_CHAR] = aci_gatt_srv_get_char_decl_handle((ble_gatt_chr_def_t *)&dt_serv_chars[1]);
}

tBleStatus DT_SERV_UpdateValue(DT_SERV_CharOpcode_t CharOpcode, DT_SERV_Data_t *pData)
{
  tBleStatus ret = BLE_STATUS_SUCCESS;

  if ((CharOpcode == DT_SERV_RX_CHAR) && (pData != NULL) && (pData->p_Payload != NULL) && (pData->Length > 0U))
  {
    uint16_t copy_len = pData->Length;
    if (copy_len > sizeof(uart_rx_val_buffer))
    {
      copy_len = sizeof(uart_rx_val_buffer);
    }
    memcpy(uart_rx_val_buffer, pData->p_Payload, copy_len);
  }

  return ret;
}

tBleStatus DT_SERV_NotifyValue(DT_SERV_CharOpcode_t CharOpcode, DT_SERV_Data_t *pData, uint16_t ConnectionHandle)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  if ((CharOpcode == DT_SERV_TX_CHAR) && (pData != NULL) && (pData->p_Payload != NULL) && (pData->Length > 0U))
  {
    ret = aci_gatt_srv_notify(ConnectionHandle,
                              BLE_GATT_UNENHANCED_ATT_L2CAP_CID,
                              DT_SERV_Context.GattDbHandle[DT_SERV_GATT_ID_UART_TX_CHAR] + 1U,
                              GATT_NOTIFICATION,
                              pData->Length,
                              (uint8_t *)pData->p_Payload);
  }

  return ret;
}
