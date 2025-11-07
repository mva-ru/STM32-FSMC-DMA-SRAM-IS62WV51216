#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#include "stm32f4xx_hal.h"
#include "stdbool.h"                      // определяет тип bool
#include "stddef.h"                       // определяет макросы NULL и offsetof, а также типы ptrdiff_t, wchar_t и size_t
#include "stdlib.h"                       // exit, malloc
#include "math.h"

#define PROGMEM

/*
  float  -> 4 байт ->  7 символов (+- 12345.12)
  double -> 8 байт -> 15 символов (+- 1234567890.12345)
*/

#define _1_BYTE_ true
#define _2_BYTE_ false

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define ul  unsigned long

#define s08 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define vs08 volatile s08
#define vs16 volatile s16
#define vs32 volatile s32
#define vs64 volatile s64 

#define cu8  const uint8_t
#define cu16 const uint16_t
#define cu32 const uint32_t
#define cu64 const uint64_t

#define vu8  volatile uint8_t
#define vu16 volatile uint16_t
#define vu32 volatile uint32_t
#define vu64 volatile uint64_t
  
#define vcu8  volatile const uint8_t
#define vcu16 volatile const uint16_t
#define vcu32 volatile const uint32_t
#define vcu64 volatile const uint64_t
  
#define vi  volatile int
#define vl  volatile long
#define vf  volatile float
#define vd  volatile double
#define vb  volatile bool
  
#define vc  volatile char
#define vcc volatile const char
  
volatile typedef enum {
  T_CHAR      = 0x00,                       // символьные
  T_U8        = 0x01,                       // без знаковые                          
  T_U16       = 0x02,                       
  T_U32       = 0x03,
  T_U64       = 0x04,   
  T_S8        = 0x05,                       // со знаком                          
  T_S16       = 0x06,                       
  T_S32       = 0x07,
  T_S64       = 0x08,   
  T_FLOAT     = 0x09,
  T_DOUBLE    = 0x0A,
  T_SBYTE     = 0x0B,                       // структура - объединение битов в байт
  T_SWORD     = 0x0C,                         
  T_SWORD2    = 0x0D,
  T_SFLOAT    = 0x0E,
  T_SDOUBLE   = 0x0F,
  T_M_U8      = 0x10,                       // массив с элементами типа u8                       
  T_M_U16     = 0x11,                       
  T_M_U32     = 0x12,
  T_M_U64     = 0x13,   
  T_M_S8      = 0x14,                                                     
  T_M_S16     = 0x15,                       
  T_M_S32     = 0x16,
  T_M_S64     = 0x17,   
  T_M_FLOAT   = 0x18,
  T_M_DOUBLE  = 0x19,
  
  T_NONE      = 0xFF,                       // не определен
} Enum_Type_Value;                          // тип переменной

volatile typedef enum {
  F_HEX     = 0x00,                                               
  F_BCD     = 0x01,                       
  F_DEC     = 0x02,
  F_CHAR    = 0x03,
} Enum_Format;                              // форматы переменной (для последующего преобразования)

#endif
