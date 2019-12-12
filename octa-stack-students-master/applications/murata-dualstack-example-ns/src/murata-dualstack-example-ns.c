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
#include "murata-dualstack-example-ns.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LSM303AGRSensor.h"
#include "sht31.h"
#include <stdio.h>
#include "murata.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IWDG_INTERVAL           5    //seconds
#define LORAWAN_INTERVAL        60   //seconds
#define DASH7_INTERVAL          20  //seconds
#define MODULE_CHECK_INTERVAL   3600 //seconds
#define SIZE 256
#define BLOCK_ID 0
#define SPI_WAIT 50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t LoRaWAN_Counter = 0;
uint16_t DASH7_Counter = 0;
uint8_t murata_init = 0;
uint64_t short_UID;
uint8_t murata_data_ready = 0;
uint8_t Data[20];
float SHTData[2];
volatile _Bool temperatureflag=0; 
volatile _Bool buttonFlag=0;
volatile _Bool sendFlag=0;
volatile _Bool bleflag=0;
volatile _Bool init_flag=0;
_Bool isAsleep=0;
volatile uint8_t ble_counter=0;
uint8_t tmpbuf_ble[1];
uint8_t buffer[5]; 
uint8_t payload[6];
uint8_t flag;


static uint8_t flashBuf[SIZE]; //read & write buffer (save some memory) 
extern volatile failedMessage=0;
volatile isActiveSending=0;
volatile uint8_t murataSucces=0;
volatile isCommandActive=0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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

  /* Initialize the platform, OCTA in this case */
  Initialize_Platform();
  /* USER CODE BEGIN 2 */

  // Get Unique ID of octa
  short_UID = get_UID();

  // Print Welcome Message & set all the sensors
  LSM303AGR_setI2CInterface(&common_I2C);
  setI2CInterface_SHT31(&common_I2C);
  SHT31_begin(); 
  LSM303AGR_init();
  printWelcome();
  HAL_UART_Receive_IT(&BLE_UART,tmpbuf_ble, sizeof(tmpbuf_ble));
  S25FL256_Initialize(&FLASH_SPI); //Initialise the flash
  readOldTemperature(); //Read old temperature from flash

  // LORAWAN
  murata_init = Murata_Initialize(short_UID, 0);

  if (murata_init)
  {
    printf("Murata dualstack module init OK\r\n\r\n");
  }
  /* USER CODE END 2 */

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);

  /* Infinite loop */
  uint8_t counter = 0;
  uint8_t use_lora = 1;
  /* USER CODE BEGIN WHILE */
  temperatureflag=1;  //measure the temperature once and blink red led. 

  while (1)
  { 

    //Initial setup to recieve bluetooth setup:
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin); //As long as setup is running: Blue led is on! 
     while(!init_flag)
    {
      //Wait till the bluetooth config is completed. 
      if(bleflag==1){ //We received an interrupt from the BLE. 
      bleflag=0;
      ble_callback();
      }
      isAsleep = 1; //You may go to sleep after handling the setup 
    } 
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin); //As long as setup is running: Blue led is on! 
    temp_hum_measurement(); //measure the temperature of the room
    //Dash7_send(NULL); //Send the desired temperature + measured temperature! 
    //Interrupt flag handler 
    switch (flag)
    {
      case 1 : handleTemperature(); //flag = temperature
        flag = 0;
        isAsleep = 1; //Temperatuur meting is gebeurt, je mag gaan slapen
        break;
      case 2: handleButton(); //flag = button1
        flag = 0;
        sendMessage();
        isAsleep = 1; //Button afhandeling is gebeurt, je mag gaan slapen
        break;  
      case 3: writeToFlash();
        flag=0;
        isAsleep =1;
        init_flag =1; 
        break; 
    }

     while(isActiveSending){
      IWDG_feed(NULL);
      if(!sendFlag){
      HAL_Delay(LORAWAN_INTERVAL);
      }
      //Wacht tot hij klaar is
      if(murata_data_ready)
      {
      printf("processing murata fifo\r\n");
      murata_data_ready = !Murata_process_fifo();
      }
    }
    if(murata_data_ready)
    {
      printf("processing murata fifo\r\n");
      murata_data_ready = !Murata_process_fifo();
    } 

    if(failedMessage>0){ //Dash7 or Lora 
      sendFlag=0; //Lora
    }else{
      murataSucces++;
      sendFlag=1; //Dash //VERGEET DIT NIET AAN TE PASSEN !!!
    }

     if(isAsleep){ //De interrupts zijn handled, je mag gaan slapen
        if(!isCommandActive){ //Data versturen is gebeurt, je mag gaan slapen
        enterStop();
        }
    }   
    
    // SEND 5 D7 messages, every 10 sec.
    // Afterwards, send 3 LoRaWAN messages, every minute
    /* if(DASH7_Counter<5)
    {
      if(counter==DASH7_INTERVAL)
      {
        Dash7_send(NULL);
        counter = 0;
      }
    }
    else
    { 
      if(LoRaWAN_Counter == 0)
        Murata_LoRaWAN_Join();
      if(LoRaWAN_Counter<3)
      {
        if (counter == LORAWAN_INTERVAL)
        {
          LoRaWAN_send(NULL);
          counter = 0;
        }
      }
      if(LoRaWAN_Counter == 3)
      {
        //reset counters to restart flow
        DASH7_Counter = 0;
        LoRaWAN_Counter = 0;
      }
    } */    

    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

void enterStop(){
  //In this function we safely enter the stop function. 
  printf("Going to sleep c: \r\n");

  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); //Hush now... go to sleep 
  HAL_ResumeTick();
  SystemClock_Config(); //BS vermijden met putty
  isAsleep=0;

  printf("Waking up :'c \r\n");

}

void sendMessage(){
  if(sendFlag){
    Dash7_send(NULL);
  }
  else
  {
    LoRaWAN_send(NULL);
  }
  isAsleep=1;
}

void handleTemperature(){ 
      temp_hum_measurement();
      sendMessage();
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(1000);
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
}

void handleButton(){
      //We will allow the user to set a new desired temperature after pressing this button
      /* init_flag = 0; 
      sendMessage();
 */
      //Routine for fingerprinting
       temp_hum_measurement();
      Dash7_send(NULL);
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(1000);
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin); 

      //The device will be able to sleep AFTER completion of Bluetooth communication
}

	
void temp_hum_measurement(void){
  SHT31_get_temp_hum(SHTData);
  print_temp_hum();
}

void print_temp_hum(void){
  printf("\r\n");
  printf("Temperature: %.2f °C  \r\n", SHTData[0]);
  printf("Humidity: %.2f %% \r\n", SHTData[1]);
  printf("\r\n");
}

void readOldTemperature(){
  S25FL256_open(BLOCK_ID);
  S25FL256_read((uint8_t *)flashBuf, SIZE);
  printf("Previous desired temperature: %d%d\r\n",flashBuf[0],flashBuf[1]); //Print the previously desired temperatures. 

}

void LoRaWAN_send(void const *argument)
{
  if (murata_init)
  {
    isActiveSending = 1;
    //Load the BLE data into the payload
    payload[0]=buffer[0];
    payload[1]=buffer[1];

    //Load the temperature data into the payload.
    uint8_t* byte = (uint8_t*)&SHTData[0]; //We make a pointer to look at the floating point number
    for(int i=2; i<sizeof(payload); i++){
      payload[i]=byte[i-2]; //load the payload from place 2 to size. 
    }

    if(!Murata_LoRaWAN_Send((uint8_t *)payload, sizeof(payload)))
    {
      murata_init++;
      if(murata_init == 10)
        murata_init == 0;
    }
    else
    {
      murata_init = 1;
    }
    LoRaWAN_Counter++;
  }
  else{
    printf("murata not initialized, not sending\r\n"); 
  }
}

void Dash7_send(void const *argument)
{
  if (murata_init)
  {
    isActiveSending = 1;
    //Load the BLE data into the payload
    payload[0]=buffer[0];
    payload[1]=buffer[1];

    //Load the temperature data into the payload.
    uint8_t* byte = (uint8_t*)&SHTData[0]; //We make a pointer to look at the floating point number
    for(int i=2; i<sizeof(payload); i++){
      payload[i]=byte[i-2]; //load the payload from place 2 to size. 
    }

    if(!Murata_Dash7_Send((uint8_t *)&payload, sizeof(payload)))
    {
      murata_init++;
      if(murata_init == 10)
        murata_init == 0;
    }
    else
    {
      murata_init = 1;
    }
    DASH7_Counter++;
  }
  else{
    printf("murata not initialized, not sending\r\n");
  }
}

// UART RX CALLBACK
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &P1_UART)
  {
    Murata_rxCallback();
    murata_data_ready = 1;
  }
  if(huart == &BLE_UART)
  {
    bleflag=1;
  }
}

void writeToFlash(){
        //Write it to flash: 
        
        //First we will delete the whole block: 
      S25FL256_eraseSectorFromBlock(0); // first we delete block nr 0 

      //fill the flash array to write data to block 0 
      flashBuf[0] = buffer[0]-48;
      flashBuf[1] = buffer[1]-48;
        for (int n = 0; n < SIZE; n++)
        {
          flashBuf[n+2] = 0; //fill the rest of the 256bytes with 0's 
        }

      S25FL256_open(BLOCK_ID);

      S25FL256_write((uint8_t *)flashBuf, SIZE); //Write the flashBuf to memory
        //If you don't write the full 256 the memory doesnt get flushed ! 

      while (S25FL256_isWriteInProgress())
      {
        HAL_Delay(SPI_WAIT);
      }
}


void ble_callback(){

    if(tmpbuf_ble[0]!=0){
    buffer[ble_counter]=tmpbuf_ble[0];
    HAL_UART_Receive_IT(&BLE_UART,tmpbuf_ble, sizeof(tmpbuf_ble));
    printf("Callback succesfully handled %d\r\n",buffer[ble_counter]);
    ble_counter++;
    if(ble_counter==2){
      ble_counter=0; //If we recieved two bytes, we can set it back to zero. 
      init_flag=1; //Initialise is complete. 
      flag=3;
      }
    } 
    //Write to flash flag
    //Bluetooth call back complete so we will write to flash
}

void vApplicationIdleHook(){
  #if LOW_POWER
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
  #endif
}

void printWelcome(void)
{
  printf("\r\n");
  printf("*****************************************\r\n");
  printf("---------Welcome to Smarthouse!----------\r\n");
  printf("*****************************************\r\n");
  printf("\r\n");
  char UIDString[sizeof(short_UID)];
  memcpy(UIDString, &short_UID, sizeof(short_UID));
  printf("octa ID: ");
  for (const char* p = UIDString; *p; ++p)
    {
        printf("%02x", *p);
    }
  printf("\r\n\r\n");
  HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
  HAL_Delay(2000);
  HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
}

/* USER CODE END 4 */
	//hier de callback die zul das willen opgeroepen als de EXTI15_10 interrupt opvangt. 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin==GPIO_Pin_13) {
  flag = 1; 
  // we work with a flag so as to make sure that we don't stay in the callback for too long. 
  // the flag will call the measurement function 
  } 
  if(GPIO_Pin==OCTA_BTN1_Pin){
  flag = 2;
  }
}



/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
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