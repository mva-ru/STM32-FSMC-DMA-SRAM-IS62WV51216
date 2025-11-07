#include "hal_is62wv51216.h"
#include <string.h>

/********************************************************************************************************************
***************************************  Глобальные переменные внутри модуля   **************************************
********************************************************************************************************************/
DMA_HandleTypeDef _hdma_fsmc;
IS62WV51216_s_obj _drv[IS62WV51216_N_DRIVERS];
//------------------------------------------------------------------------------------------------------------------

/********************************************************************************************************************
**************************************  Прототипы всех функций и методов модуля   ***********************************
********************************************************************************************************************/
bool IS62WV51216_Init(u8 n_drv, SRAM_HandleTypeDef* hsram, bool fl_dma);  // Инициализация драйвера
void IS62WV51216_Handler(void);                                           // Обработчик данных модуля (положить в main)
void IS62WV51216_Handler_Tm(void);                                        // Обработчик данных модуля в прерывании таймера (положить в таймер 1мс)

#if IS62WV51216_DMA_ON
bool IS62WV51216_DMA_Init(u8 n_drv, SRAM_HandleTypeDef* hsram);           // Инициализация DMA для работы по шине FSMC с SRAM
void IS62WV51216_DMA_Transfer_Complete(DMA_HandleTypeDef *hdma);          // DMA успешно завершил работу
void IS62WV51216_DMA_Transfer_Error(DMA_HandleTypeDef *hdma);             // DMA не смог завершить работу, возникла ошибка

bool IS62WV51216_IsValid_DMA_Stream_MemToMem(DMA_Stream_TypeDef* stream); // Функция для проверки валидности DMA потока для memory-to-memory
IRQn_Type IS62WV51216_Get_DMA_Stream_IRQ(DMA_Stream_TypeDef* stream);     // Функция для получения IRQ номера по потоку DMA
#endif

bool IS62WV51216_Rw_Byte(u8 n_drv, u32 adr, u8* data, bool fl_rw, 
                         bool fl_sb);                                     // Операция чтения\записи байта по заданному адресу (без LB и UB)
bool IS62WV51216_Rw_Data(u8 n_drv, u32 adr, u16* data, u32 size, 
                         bool fl_rw, bool fl_dma);                        // Операция чтения\записи массива данных по заданному адресу

bool IS62WV51216_Check_Connection(u8 n_drv);                              // Проверить на связи ли микросхема памяти

IS62WV51216_s_obj* IS62WV51216_Get_Obj_Link(void);                        // Получить ссылку на объект управления драйвером
//------------------------------------------------------------------------------------------------------------------

/********************************************************************************************************************
******************************************  Описание функций и методов модуля  **************************************
********************************************************************************************************************/
/**
  * @brief         Инициализация драйвера
  * @param  n_drv: Номер объекта управления драйвером
  * @param  hsram: Ссылка на объект управления шиной драйвера
  * @param fl_dma: Флаг - инициализировать DMA
  * @retval  bool: Результат операции (true - успешно)
  */
bool IS62WV51216_Init(u8 n_drv, SRAM_HandleTypeDef* hsram, bool fl_dma){
  _drv[n_drv].state = IS62WV51216_STATE_INIT;
  
#if IS62WV51216_CHECK_FUNCTION  
  if(n_drv >= IS62WV51216_N_DRIVERS){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - ERROR: n_drv >= IS62WV51216_N_DRIVERS\n", n_drv);
    goto error;
  }
  if(!hsram){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - ERROR: adr >= IS62WV51216_SIZE_BYTES\n", n_drv);
    goto error;
  };
#endif
  
  _drv[n_drv].hsram = hsram;
  _drv[n_drv].hsram->hdma = &_hdma_fsmc;
  
#if IS62WV51216_FSMC_BIT_DETH == IS62WV51216_FSMC_DATA_8_BIT
  _drv[n_drv].bus_size = IS62WV51216_FSMC_BUS_SIZE_8;
  _drv[n_drv].p_memory = (__IO u8*)(IS62WV51216_FSMC_MEMORY_BASE | (IS62WV51216_FSMC_BANK_SIZE*(IS62WV51216_FSMC_NUM_NE - 1))); // -1 для приведения к индексу (0-3)
#else
  _drv[n_drv].bus_size = IS62WV51216_FSMC_BUS_SIZE_16;
  _drv[n_drv].p_memory = (__IO u16*)(IS62WV51216_FSMC_MEMORY_BASE | (IS62WV51216_FSMC_BANK_SIZE*(IS62WV51216_FSMC_NUM_NE - 1))); // -1 для приведения к индексу (0-3)
#endif
    
  _drv[n_drv].fl_connect = false;
  _drv[n_drv].fl_check_con = false;
  
  _drv[n_drv].adr = 0;
  _drv[n_drv].data = 0; 
  _drv[n_drv].cnt_err_dma = 0;
  _drv[n_drv].cnt_err_fn_data = 0;
  _drv[n_drv].cnt_err_fsmc_idle = 0;  
  _drv[n_drv].cnt_tm_connect = 0;
  
#if IS62WV51216_BUFFER
  for(u32 i = 0; i < IS62WV51216_BUFFER_SIZE; i++)
    _drv[n_drv].buf[i] = 0;
#endif

  if(fl_dma){
#if IS62WV51216_DMA_ON  
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - DMA ON\n", n_drv);
    IS62WV51216_DMA_Init(n_drv, hsram);
#endif
  }else
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - DMA OFF\n", n_drv);
  
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - Size RAM: %lu Bytes\n",  n_drv, (long)IS62WV51216_SIZE_BYTES);
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - Size RAM: %lu KBytes\n", n_drv, (long)IS62WV51216_SIZE_KBYTES);
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - Size RAM: %lu HWords\n", n_drv, (long)IS62WV51216_SIZE_HALF_WORDS);
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Init() - OK\n", n_drv);
  IS62WV51216_Check_Connection(n_drv);
  return true;

#if IS62WV51216_CHECK_FUNCTION
error:
  _drv[n_drv].cnt_err_fn_data++;
  return false;
#endif
}

#if IS62WV51216_DMA_ON
/**
  * @brief         Инициализация DMA для работы по шине FSMC с SRAM
  * @param  n_drv: Номер объекта управления драйвером
  * @param  hsram: Ссылка на объект управления шиной драйвера
  * @retval  bool: Результат операции (true - успешно)
  */
bool IS62WV51216_DMA_Init(u8 n_drv, SRAM_HandleTypeDef* hsram)
{
  IRQn_Type irq_number;
  
#if IS62WV51216_CHECK_FUNCTION
  if(n_drv >= IS62WV51216_N_DRIVERS){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - ERROR: n_drv >= IS62WV51216_N_DRIVERS\n", n_drv);
    goto error;
  }
  if(!hsram || !hsram->hdma){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - ERROR: !hsram || !hsram->hdma\n", n_drv);
    goto error;
  }
#endif
  // Проверяем, что выбранный DMA поток поддерживает memory-to-memory
  if(!IS62WV51216_IsValid_DMA_Stream_MemToMem(IS62WV51216_DMA_STREAM)){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - ERROR: invalid DMA stream mem-to-mem\n", n_drv);
    goto error;
  }// Включение тактирования DMA
  __HAL_RCC_DMA2_CLK_ENABLE();
  
  // Настройка DMA для Memory-to-Memory
  hsram->hdma->Instance = IS62WV51216_DMA_STREAM;     // любой свободный
  hsram->hdma->Init.Channel = DMA_CHANNEL_0;          // для memory-to-memory, обычно channel 0
  hsram->hdma->Init.Direction = DMA_MEMORY_TO_MEMORY;
  hsram->hdma->Init.PeriphInc = DMA_PINC_ENABLE;
  hsram->hdma->Init.MemInc = DMA_MINC_ENABLE;
  
#if IS62WV51216_FSMC_BIT_DETH == IS62WV51216_FSMC_DATA_8_BIT  
  hsram->hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hsram->hdma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
#else
  hsram->hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hsram->hdma->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
#endif  
  
  hsram->hdma->Init.Mode = DMA_NORMAL;
  hsram->hdma->Init.Priority = DMA_PRIORITY_HIGH;
  hsram->hdma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hsram->hdma->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  // для совместимости с FSMC
  hsram->hdma->Init.MemBurst = DMA_MBURST_SINGLE;
  hsram->hdma->Init.PeriphBurst = DMA_PBURST_SINGLE;
  //------------------------------------------------
  if(HAL_DMA_Init(hsram->hdma) != HAL_OK){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - ERROR: DMA_Init()\n", n_drv);
    return false;
  }
  if(HAL_DMA_RegisterCallback(hsram->hdma, HAL_DMA_XFER_CPLT_CB_ID, IS62WV51216_DMA_Transfer_Complete) != HAL_OK ||
     HAL_DMA_RegisterCallback(hsram->hdma, HAL_DMA_XFER_ERROR_CB_ID, IS62WV51216_DMA_Transfer_Error) != HAL_OK){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - ERROR: DMA_RegisterCallback()", n_drv);
    return false;
  }// Настройка прерываний для конкретного потока
  irq_number = IS62WV51216_Get_DMA_Stream_IRQ(IS62WV51216_DMA_STREAM);
  HAL_NVIC_SetPriority(irq_number, 0, 0);
  HAL_NVIC_EnableIRQ(irq_number); 
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Init() - OK\n", n_drv);
  return true;
  
#if IS62WV51216_CHECK_FUNCTION
error:
  _drv[n_drv].cnt_err_fn_data++;
  return false;
#endif
}

/**
  * @brief          Функция для проверки валидности DMA потока для memory-to-memory
  * @param stream*: Ссылка на структуру с регистрами для управления DMA контроллером
  * @retval         Результат операции (true - успешно)
  */
bool IS62WV51216_IsValid_DMA_Stream_MemToMem(DMA_Stream_TypeDef* stream)
{
  // Проверка что поток поддерживает memory-to-memory
  // Для DMA2: Stream 0-7, но не все поддерживают M2M
  return (stream == DMA2_Stream0) || (stream == DMA2_Stream1) || 
         (stream == DMA2_Stream2) || (stream == DMA2_Stream3) ||
         (stream == DMA2_Stream4) || (stream == DMA2_Stream5) ||
         (stream == DMA2_Stream6) || (stream == DMA2_Stream7);
}

/**
  * @brief          Функция для получения IRQ номера по потоку DMA
  * @param stream*: Ссылка на структуру с регистрами для управления DMA контроллером
  * @retval         Результат операции (true - успешно)
  */
IRQn_Type IS62WV51216_Get_DMA_Stream_IRQ(DMA_Stream_TypeDef* stream)
{
  if(stream == DMA2_Stream0) return DMA2_Stream0_IRQn;
  if(stream == DMA2_Stream1) return DMA2_Stream1_IRQn;
  if(stream == DMA2_Stream2) return DMA2_Stream2_IRQn;
  if(stream == DMA2_Stream3) return DMA2_Stream3_IRQn;
  if(stream == DMA2_Stream4) return DMA2_Stream4_IRQn;
  if(stream == DMA2_Stream5) return DMA2_Stream5_IRQn;
  if(stream == DMA2_Stream6) return DMA2_Stream6_IRQn;
  if(stream == DMA2_Stream7) return DMA2_Stream7_IRQn;
  return (IRQn_Type)0;
}
#endif

/**
  * @brief Обработчик данных модуля (положить в main)
  */
void IS62WV51216_Handler(void)
{
  int n_drv = 0; 
  
  while(n_drv < IS62WV51216_N_DRIVERS){
    if(_drv[n_drv].fl_check_con){
      IS62WV51216_Check_Connection(n_drv);
      _drv[n_drv].fl_check_con = false;
    }
    n_drv++;
  }
}

/**
  * @brief Обработчик данных модуля в прерывании таймера (положить в таймер 1мс)
  */
void IS62WV51216_Handler_Tm(void)
{
  int n_drv = 0; 
  
  while(n_drv < IS62WV51216_N_DRIVERS){
    _drv[n_drv].cnt_tm_connect++;
    
    if(_drv[n_drv].cnt_tm_connect == IS62WV51216_TIMOUT_CONNECT){
      _drv[n_drv].cnt_tm_connect = 0;
      _drv[n_drv].fl_check_con = true;
    }
    n_drv++;
  }
}

#if IS62WV51216_DMA_ON
/**
  * @brief        DMA успешно завершил работу
  * @param  hdma: Ссылка на объект управления DMA
  */
void IS62WV51216_DMA_Transfer_Complete(DMA_HandleTypeDef *hdma)
{
  int n_drv = 0; 
  
  while(n_drv < IS62WV51216_N_DRIVERS){
    if(_drv[n_drv].hsram->hdma == hdma)
    {   
#if IS62WV51216_SOFTWARE_LB_UB
    IS62WV51216_BOTH_BYTES_OFF();
#else   
      CLEAR_BIT(_drv[n_drv].hsram->Instance->BTCR[_drv[n_drv].hsram->Init.NSBank], FSMC_WRITE_OPERATION_ENABLE);
#endif    
      _drv[n_drv].hsram->State = HAL_SRAM_STATE_READY;
      IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Transfer_Complete() - OK\n", n_drv);
      break;
    }
    n_drv++;
  }   
}

/**
  * @brief        DMA не смог завершить работу, возникла ошибка
  * @param  hdma: Ссылка на объект управления DMA
  */
void IS62WV51216_DMA_Transfer_Error(DMA_HandleTypeDef *hdma)
{
  int n_drv = 0; 
  
  while(n_drv < IS62WV51216_N_DRIVERS){
    if(_drv[n_drv].hsram->hdma == hdma)
    {
#if IS62WV51216_SOFTWARE_LB_UB
      IS62WV51216_BOTH_BYTES_OFF();
#else   
      CLEAR_BIT(_drv[n_drv].hsram->Instance->BTCR[_drv[n_drv].hsram->Init.NSBank], FSMC_WRITE_OPERATION_ENABLE);
#endif  
      _drv[n_drv].cnt_err_dma++;
      _drv[n_drv].hsram->State = HAL_SRAM_STATE_READY;
      IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->DMA_Transfer_Error() - ERROR\n", n_drv);
      break;
    }
    n_drv++;
  }
}
#endif  

/**
  * @brief         Операция чтения\записи байта по заданному адресу 
                  (если контакты LB и UB, не используются)
  * @param  n_drv: Номер объекта управления драйвером
  * @param    adr: Адрес ячейки памяти для чтения (макс. значение IS62WV51216_SIZE_BYTES)
  * @param  data*: Байт данных для чтения\записи
  * @param  fl_rw: Флаг - операция с памятью, чтения\запись данных 
                   IS62WV51216_READ\IS62WV51216_WRITE
  * @param  fl_sb: Выбор байта памяти для чтения\запись (если мин. размер ячйеки памяти 16 бит)
                   IS62WV51216_HI_BYTE\IS62WV51216_LO_BYTE
  * @retval  bool: Результат операции (true - успешно)
  */
bool IS62WV51216_Rw_Byte(u8 n_drv, u32 adr, u8* data, bool fl_rw, bool fl_sb)
{
#if IS62WV51216_DEBUG
  static char s_rw[5], s_sb[2];
#endif
  
#if IS62WV51216_DEBUG
  if(fl_rw == IS62WV51216_READ)
    strcpy(s_rw, "Read ");
  else
    strcpy(s_rw, "Write");
  
  if(fl_sb == IS62WV51216_LO_BYTE)
    strcpy(s_sb, "Lo");
  else
    strcpy(s_sb, "Hi");
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Software_%s_Byte_%s(adr:0x%02X) - START\n", n_drv, s_rw, s_sb, adr);
#endif
  
#if IS62WV51216_FSMC_BIT_DETH == IS62WV51216_FSMC_DATA_16_BIT
  
#if !IS62WV51216_SOFTWARE_LB_UB
  if(fl_rw == IS62WV51216_WRITE){
    _drv[n_drv].data = 0;
    
    if(IS62WV51216_Rw_Data(n_drv, adr, (u16*)&_drv[n_drv].data, 1, IS62WV51216_READ, IS62WV51216_SET_DMA_OFF)){
      if(fl_sb == IS62WV51216_LO_BYTE){     
        _drv[n_drv].data &= 0xff00;
        _drv[n_drv].data |= *data;
      }else{
        _drv[n_drv].data &= 0x00ff;
        _drv[n_drv].data |= (*data)<<8;     
      }
      if(IS62WV51216_Rw_Data(n_drv, adr, (u16*)&_drv[n_drv].data, 1, IS62WV51216_WRITE, IS62WV51216_SET_DMA_OFF))
        goto ok;
    }
  }else{ // fl_rw == IS62WV51216_READ
    if(IS62WV51216_Rw_Data(n_drv, adr, (u16*)&_drv[n_drv].data, 1, fl_rw, IS62WV51216_SET_DMA_OFF)){
      if(fl_sb == IS62WV51216_LO_BYTE)
        *data = _drv[n_drv].data;
      else
        *data = _drv[n_drv].data>>8;
      
      goto ok;
    }
  }
#else  // #if !IS62WV51216_SOFTWARE_LB_UB
  __IO u16* link = &_drv[n_drv].p_memory[adr];
  
  if(fl_rw == IS62WV51216_WRITE){
    _drv[n_drv].data = 0;
    
    if(fl_sb == IS62WV51216_LO_BYTE)
      *link = *data;    
    else
      *link = (*data)<<8;
    
    IS62WV51216_BOTH_BYTES_ON();
    goto ok;
  }else{ // fl_rw == IS62WV51216_READ
    if(fl_sb == IS62WV51216_LO_BYTE)
      *data = *link;
    else
      *data = *link>>8;
    
    goto ok;
  }
#endif // #if !IS62WV51216_SOFTWARE_LB_UB
  
#else  // #if IS62WV51216_FSMC_BIT_DETH == IS62WV51216_FSMC_DATA_16_BIT

  // код для 8 битной шины
  
#endif // #if IS62WV51216_FSMC_BIT_DETH == IS62WV51216_FSMC_DATA_16_BIT
  
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Software_%s_Byte_%s(adr:0x%02X) - ERROR\n", n_drv, s_rw, s_sb, adr);
  return false;
  
  ok:
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Software_%s_Byte_%s(adr:0x%02X) - OK\n", n_drv, s_rw, s_sb, adr);
  return true;
}

/**
  * @brief         Операция чтения\записи массива данных по заданному адресу
                  (аппаратно используются контакты LB = 0 и UB = 0)
  * @param  n_drv: Номер объекта управления драйвером
  * @param    adr: Адрес ячейки памяти для чтения (макс. значение IS62WV51216_SIZE_BYTES)
  * @param  data*: Массив для чтения\записи данных
  * @param   size: Размер массива данных (data*) в полусловах (16 бит)
  * @param  fl_rw: Флаг - операция с памятью, чтения\запись данных 
                   IS62WV51216_READ\IS62WV51216_WRITE
  * @param fl_dma: Флаг - использовать DMA (true - да)
  * @retval  bool: Результат операции (true - успешно)
  */
bool IS62WV51216_Rw_Data(u8 n_drv, u32 adr, u16* data, u32 size, bool fl_rw, bool fl_dma)
{
#if IS62WV51216_DEBUG
  static char s_rw[5];
#endif
  
#if IS62WV51216_CHECK_FUNCTION
  if(n_drv >= IS62WV51216_N_DRIVERS){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data(0x%02X) - ERROR: n_drv >= IS62WV51216_N_DRIVERS\n", n_drv, adr);
    goto error;
  }
  if(adr >= IS62WV51216_SIZE_HALF_WORDS || 
    ((adr + size) > IS62WV51216_SIZE_HALF_WORDS)){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data(0x%02X) - ERROR: adr || size\n", n_drv, adr);
    goto error;   
  };
  if(!data){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data(0x%02X) - ERROR: data* == NULL", n_drv, adr);
    goto error;
  }
#endif
  if(_drv[n_drv].hsram->State != HAL_SRAM_STATE_READY){
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data(0x%02X) - ERROR: State != HAL_SRAM_STATE_READY\n", n_drv, adr);
    return false;   
  };
#if IS62WV51216_DEBUG
  if(fl_rw == IS62WV51216_READ)
    strcpy(s_rw, "Read ");
  else
    strcpy(s_rw, "Write");
#endif
  _drv[n_drv].hsram->State = HAL_SRAM_STATE_BUSY;
  SET_BIT(_drv[n_drv].hsram->Instance->BTCR[_drv[n_drv].hsram->Init.NSBank], FSMC_WRITE_OPERATION_ENABLE);
  
#if IS62WV51216_SOFTWARE_LB_UB
  IS62WV51216_BOTH_BYTES_ON();
#endif  
  
  if(!fl_dma){
    __IO u16* link = &_drv[n_drv].p_memory[adr]; 
    
    if(fl_rw == IS62WV51216_WRITE)
      while(size-- > 0) *link++ = *data++;
    else // fl_rw == IS62WV51216_READ
      while(size-- > 0) *data++ = *link++;
    
#if IS62WV51216_SOFTWARE_LB_UB
    IS62WV51216_BOTH_BYTES_OFF();
#endif
    
    CLEAR_BIT(_drv[n_drv].hsram->Instance->BTCR[_drv[n_drv].hsram->Init.NSBank], FSMC_WRITE_OPERATION_ENABLE);
    _drv[n_drv].hsram->State = HAL_SRAM_STATE_READY;
    IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data_%s(adr:0x%02X) - OK\n", n_drv, s_rw, adr);
  }
#if IS62WV51216_DMA_ON
  else{
    HAL_StatusTypeDef dma_result;
    
    if(fl_rw == IS62WV51216_WRITE)
      dma_result = HAL_DMA_Start_IT(_drv[n_drv].hsram->hdma, (uint32_t)data, (uint32_t)&((u16*)_drv[n_drv].p_memory)[adr], size);
    else // fl_rw == IS62WV51216_READ
      dma_result = HAL_DMA_Start_IT(_drv[n_drv].hsram->hdma, (uint32_t)&((u16*)_drv[n_drv].p_memory)[adr], (uint32_t)data, size);
    
#if IS62WV51216_DMA_POLLING
    dma_result = HAL_DMA_PollForTransfer(_drv[n_drv].hsram->hdma, HAL_DMA_FULL_TRANSFER, IS62WV51216_DMA_POLLING_TMO);
#endif    
    if(dma_result == HAL_OK){
      IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data_%s(adr:0x%02X) - DMA->Idle\n", n_drv, s_rw, adr);
    }else{
      IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->RW_Data_%s(adr:0x%02X) - DMA->ERROR: Start_IT()\n", n_drv, s_rw, adr);
      return false;
    }
  }
#endif
  return true;
  
#if IS62WV51216_CHECK_FUNCTION
error:
  _drv[n_drv].cnt_err_fn_data++;
  return false;
#endif
}

/**
  * @brief         Проверить на связи ли микросхема памяти
  * @param  n_drv: Номер объекта управления драйвером
  * @retval  bool: Результат операции (true - успешно)
  */
bool IS62WV51216_Check_Connection(u8 n_drv)
{
  u8 n_once = 3;
  
  while(n_once > 0){
    _drv[n_drv].data = IS62WV51216_TEST_CONNECTION_DATA;
    
    if(IS62WV51216_Rw_Data(n_drv, IS62WV51216_TEST_CONNECTION_ADR, (u16*)&_drv[n_drv].data, 1, IS62WV51216_WRITE, IS62WV51216_SET_DMA_OFF)){
      _drv[n_drv].data = 0;
      
      if(IS62WV51216_Rw_Data(n_drv, IS62WV51216_TEST_CONNECTION_ADR, (u16*)&_drv[n_drv].data, 1, IS62WV51216_READ, IS62WV51216_SET_DMA_OFF))
        if(_drv[n_drv].data == IS62WV51216_TEST_CONNECTION_DATA){
          IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Read_Check_Connection() - CONNECTED\n", n_drv);
          _drv[n_drv].fl_connect = true;
          return true;
        }
      }
    n_once--;
  }
  IS62WV51216_DEBUG_PRINTF("IS62WV51216(%02d)->Read_Check_Connection() - DISCONNECTED\n", n_drv);
  _drv[n_drv].fl_connect = false;
  return false;
}

/**
  * @brief  Получить ссылку на объект управления драйвером
  * @retval Возвращает ссылку на первую микросхему SRAM в массиве
  */
IS62WV51216_s_obj* IS62WV51216_Get_Obj_Link(void)
{
  return (IS62WV51216_s_obj*)&_drv[0];
}
//------------------------------------------------------------------------------------------------------------------
