/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "no-scheduler-example.h"
#include "D:\Master\I-IOT_PROJECT\octa-stack-students-master_lorawan\core\drivers\LSM303AGR\inc\LSM303AGRSensor.h"
#include <stdio.h>
#include "D:\Master\I-IOT_PROJECT\octa-stack-students-master_lorawan\core\drivers\SHT31\inc\sht31.h"
#include "murata.h"
//#include "C:\Users\yvesk\Documents\Academiejaar 2019-2020\IOT\Practicum\octa-stack-students-master\applications\lorawan-example\inc\lorawan-example.h"
//#include "C:\Users\yvesk\Documents\Academiejaar 2019-2020\IOT\Practicum\octa-stack-students-master\core\platform\common\inc\platform.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HAL_EXTI_MODULE_ENABLED
#define temp_hum_timer    3
#define LOW_POWER
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
osTimerId temp_hum_timer_id;
float SHTData[2];
volatile _Bool temperatureflag=0; 
uint16_t LoRaWAN_Counter = 0;
uint8_t lora_init = 0;
uint64_t short_UID;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
#if USE_BOOTLOADER
  bootloader_SetVTOR();
#endif

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
 
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize theplatform */
  Initialize_Platform();
  /* USER CODE BEGIN 2 */

  // get Unique ID of Octa
  short_UID = get_UID(); 

  //LORAWAN
  lora_init=Murata_Initialize(short_UID);

  if (lora_init) {
    printf("Lorawan module init ok\r\n\r\n");
  }
  
  LSM303AGR_setI2CInterface(&common_I2C);
  setI2CInterface_SHT31(&common_I2C);
  SHT31_begin(); 
  LSM303AGR_init();
  printWelcome();
  uint8_t data; 

  //ENTER SLEEP MODE ONLY WAKE UP ON INTERRUPT!!

// // TX MUTEX ensuring no transmits are happening at the same time
//   osMutexDef(txMutex);
//   txMutexId = osMutexCreate(osMutex(txMutex));
    
//     // pass processing thread handle to murata driver
//   Murata_SetProcessingThread(murata_rx_processing_handle);
  /* USER CODE END 2 */

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    uint8_t payload=5;
    IWDG_feed(NULL); 
    /* USER CODE END WHILE */
    //LSM303AGR_readRegister(0x31, data, 0);
    //de interrupt zal zorgen dat de flag op 1 staat, dan doen we een measurement van temp
    if (temperatureflag==1) {
      temp_hum_measurement();
    // LoRaWAN_send_self();
      temperatureflag=0; 
      //lora_init=Murata_Initialize(short_UID);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin); 
    }

    printf("RUNNING!!!\r\n");

  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
  /* USER CODE END 3 */
}

void printWelcome(void)
{
  printf("\r\n");
  printf("*****************************************\r\n");
  printf("no scheduler example\r\n");
  printf("*****************************************\r\n");
  printf("\r\n");
  HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
  HAL_Delay(2000);
  HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

void print_temp_hum(void){
  printf("\r\n");
  printf("Temperature: %.2f °C  \r\n", SHTData[0]);
  printf("Humidity: %.2f %% \r\n", SHTData[1]);
  printf("\r\n");
}

void temp_hum_measurement(void){

  SHT31_get_temp_hum(SHTData);
  print_temp_hum();
}


/* USER CODE END 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#if USE_BOOTLOADER
    if(huart == &BLE_UART);
    {
          printf("BLE UART INTERRUPT\r\n");
          bootloader_parse_data();        
    }
  #endif
}

//hier de callback die zul das willen opgeroepen als de EXTI15_10 interrupt opvangt. 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin==GPIO_Pin_13) {
  temperatureflag = 1; 
  printf("IM AWAKE NOW :D \r\n");
  // we work with a flag so as to make sure that we don't stay in the callback for too long. 
  // the flag will call the measurement function 
  } 
}

void LoRaWAN_send_self()
{
  if (lora_init)
  {
    uint8_t loraMessage[5];
    uint8_t i = 0;
    //uint16 counter to uint8 array (little endian)
    //counter (large) type byte

    //first data transfer works, but afterwards it stays in a kind of 'transmitted' loop. Find where we can reset the flag.
    loraMessage[i++] = SHTData[0];
    loraMessage[i++] = SHTData[1];
    loraMessage[i++] = LoRaWAN_Counter >> 8;
   // osMutexWait(txMutexId, osWaitForever);
    if(!Murata_LoRaWAN_Send((uint8_t *)loraMessage, i))
    {
      printf("tis ni gelukt :( ");
      lora_init++;
      if(lora_init == 10)
        lora_init == 0;
    }
    else
    {
      lora_init = 1;
    }
    //BLOCK TX MUTEX FOR 3s
    // osDelay(3000);
    // osMutexRelease(txMutexId);
    LoRaWAN_Counter++;
  }
  else{
    printf("murata not initialized, not sending\r\n");
  }
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

void murata_process_rx_response(void)
{
  uint32_t startProcessing;
  while (1)
  {
    // Wait to be notified that the transmission is complete.  Note the first
    //parameter is pdTRUE, which has the effect of clearing the task's notification
    //value back to 0, making the notification value act like a binary (rather than
    //a counting) semaphore.
    startProcessing = ulTaskNotifyTake(pdTRUE, osWaitForever);
    if (startProcessing == 1)
    {
      // The transmission ended as expected.
      Murata_process_fifo();
    }
    else
    {
    }
    osDelay(1);
  }
  osThreadTerminate(NULL);
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
