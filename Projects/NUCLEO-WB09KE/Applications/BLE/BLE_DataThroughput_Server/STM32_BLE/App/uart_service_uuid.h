/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    uart_service_uuid.h
  * @brief   UUID and GATT DB identifiers for UART custom service.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef UART_SERVICE_UUID_H
#define UART_SERVICE_UUID_H

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN UUID_Constants */
#define UART_SERVICE_UUID_128 0x64,0xA7,0x6B,0x5B,0xD8,0xF5,0x4F,0x23,0xB4,0x11,0x9E,0xA5,0xA0,0x11,0x00,0x01
#define UART_TX_CHAR_UUID_128 0x64,0xA7,0x6B,0x5B,0xD8,0xF5,0x4F,0x23,0xB4,0x11,0x9E,0xA5,0xA0,0x11,0x00,0x02
#define UART_RX_CHAR_UUID_128 0x64,0xA7,0x6B,0x5B,0xD8,0xF5,0x4F,0x23,0xB4,0x11,0x9E,0xA5,0xA0,0x11,0x00,0x03

typedef enum
{
  DT_SERV_GATT_ID_UART_SERVICE = 0U,
  DT_SERV_GATT_ID_UART_TX_CHAR = 1U,
  DT_SERV_GATT_ID_UART_RX_CHAR = 2U,
  DT_SERV_GATT_ID_COUNT
} DT_SERV_GattDbId_t;
/* USER CODE END UUID_Constants */

#ifdef __cplusplus
}
#endif

#endif /* UART_SERVICE_UUID_H */
