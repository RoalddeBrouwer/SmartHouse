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
#define LORAWAN_INTERVAL 20        //ms
#define DASH7_INTERVAL 20          //ms
#define SIZE 256 //FLASH
#define BLOCK_ID 0 //FLASH
#define SPI_WAIT 50 //FLASH

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//8 bit integers: 
uint8_t murata_init = 0;
uint8_t tmpbuf_ble[1];
uint8_t buffer[5];
uint8_t payload[6];
uint8_t payloadLora[2];
uint8_t flag;
uint8_t loraRetries = 0;
uint8_t murata_data_ready = 0;
//64 bit integers: 
uint64_t short_UID;
//Booleans:
_Bool isAsleep = 0;
//floats:
float SHTData[2];
//volatiles:
volatile _Bool sendFlag = 1; //default is Dash7
volatile _Bool bleflag = 0;
volatile _Bool init_flag = 0;
volatile uint8_t ble_counter = 0;
volatile isActiveSending = 0;
volatile modemRebooted = 0;
//Static & externs
static uint8_t flashBuf[SIZE]; //read & write buffer (save some memory)
extern volatile failedMessage = 0; //The 


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
  LSM303AGR_powerDownMagnetometer();
  printWelcome();
  HAL_UART_Receive_IT(&BLE_UART, tmpbuf_ble, sizeof(tmpbuf_ble)); //Ready up a buffer to recieve a bluetooth byte.
  S25FL256_Initialize(&FLASH_SPI); //Initialise the flash
  readOldTemperature();            //Read old temperature from flash

  // Initialise the Modem:
  murata_init = Murata_Initialize(short_UID, 0);

  if (murata_init)
  {
    printf("Murata dualstack module init OK\r\n\r\n");
  }
  /* USER CODE END 2 */

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    IWDG_feed(NULL); //dont forget feeding the watchdog
    //Initial setup to recieve bluetooth setup:
    
    switch (flag)
    {
    case 1:
      handleTemperature(); //flag = temperature
      flag = 0;
      break;
    case 2:
      handleButton1(); //flag = button1
      flag = 0;
      break;
    case 3:
      handleButton2();
       if(buffer[0]-48==9 && buffer[0]-48==9){
          printf("sending mode changed \r\n"); //dont write to flash if the number is 99. 
          readOldTemperature();
        }else
        {
          writeToFlash();
        }
        
      flag = 0;
      break;
    }
     while(isActiveSending){
      IWDG_feed(NULL); //dont forget feeding the watchdog
      if(!sendFlag){ //If using LORA
        loraRetries++;
      }
      //Wacht tot hij klaar is
      if(murata_data_ready) 
      {
      printf("processing murata fifo\r\n");
      murata_data_ready = !Murata_process_fifo(); 
      }
      if(modemRebooted){
        isActiveSending=0;
        modemRebooted=0;
        sendMessage();
      }
      //isActiveSending is set to 0 in murata.c line 62
    }

    if(failedMessage==0){ //This is the IF-statement putting the device to sleep if the transmissions went well.
      isAsleep=1; 
    }

    if(failedMessage==1){ //First message failed, try again.
      sendMessage(); //retry once more.
      isAsleep=0; //We cannot go to sleep yet. 
    }

    if(failedMessage>=2){ //Twice failure, switch sending mode. 

      //Assuming Dash7 is our main method of sending. 
      if(sendFlag==1){ //Too many failed in Dash7
      sendFlag=0; //Lora
      isAsleep=0;
      sendMessage();
      }else{ //We are sending in Lora
        if(loraRetries>=1){ //Too many sent. 
        isAsleep=1; //We tried twice Dash7 and Lora, both failed twice. wake up later and try both again. 
        loraRetries=0;
        sendFlag=1; //Set back to Dash7 
        }else{ //If we haven't tried twice, try lora once more. (second lora message)
       // HAL_Delay(2000);
       // sendMessage();
        }
      }
    }

     if(isAsleep){ //De interrupts zijn handled, je mag gaan slapen
        enterStop(); //We can go to sleep here. 
    }   

    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief The CPU will be put to sleep here. 
  * @warning Only enter once comfirmation of sending is received. 
  */
void enterStop(){
  //In this function we safely enter the stop function. 
  printf("ENTER STOP-MODE \r\n");

  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI); //Hush now... go to sleep 
  HAL_ResumeTick();
  SystemClock_Config(); //BS vermijden met putty
  isAsleep=0;

  printf("EXIT STOP-MODE \r\n");

}

void bluetoothSetup(){
  HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin); //As long as setup is running: Blue led is on!
  printf("Send the new desired temperature or 99 to change sending mode \r\n");
    while (!init_flag)
    {
      IWDG_feed(NULL);
      //Wait till the bluetooth config is completed.
      if (bleflag == 1)
      { //We received an interrupt from the BLE.
        bleflag = 0;
        ble_callback();
      }
    }
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin); //As long as setup is running: Blue led is on!
  printf("Receiving complete! \r\n");
}

/**
  * @brief This is used to call in other functions to send a message. 
  *        It automatically decides to send over Dash or Lora
  * @attention It will send the currently available buffer with it.
  * @see LoRaWAN_send(void const *argument) Dash7_send(void const *argument)
  */
void sendMessage(){
  if(sendFlag){
    Dash7_send(NULL);
  }
  else
  {
    LoRaWAN_send(NULL);
  }
}

/**
  * @brief This function will measure the temperate, send the temperature and comfirm with leds. 
  * @see LoRaWAN_send(void const *argument) Dash7_send(void const *argument)
  */
void handleTemperature(){ 
      temp_hum_measurement();
      sendMessage();
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(1000);
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
}

/**
  * @brief "Template" function used for a routine after pressing button 1.  
  *        Currently used to build a database. 
  */
void handleButton1(){
      //Routine for fingerprinting
      handleTemperature();
      //The device will be able to sleep AFTER completion of Bluetooth communication
}

/**
  * @brief This function is called after pressing button 2. This allows the user to change the desired temperature.
  */
void handleButton2(){
      //We will allow the user to set a new desired temperature after pressing this button
      init_flag = 0; 
      bluetoothSetup();
}

/**
  * @brief This function does a simple measurement and prints data to console.
  * @see  print_temp_hum()
  */	
void temp_hum_measurement(void){
  SHT31_get_temp_hum(SHTData);
  print_temp_hum();
}

/**
  * @brief This function will gather the temperature and humidity data and print it to the console.  
  */	
void print_temp_hum(void){
  printf("\r\n");
  printf("Temperature: %.2f °C  \r\n", SHTData[0]);
  printf("\r\n");
}

/**
  * @brief This function will gather the temperature and humidity data from the flash memory.
  */	
void readOldTemperature(){
  S25FL256_open(BLOCK_ID);
  S25FL256_read((uint8_t *)flashBuf, SIZE);
  printf("Previous desired temperature: %d%d\r\n",flashBuf[0],flashBuf[1]); //Print the previously desired temperatures. 
  buffer[0]=flashBuf[0]+48;
  buffer[1]=flashBuf[1]+48;
}

/**
  * @brief This function will send the data over LoraWan. 
  */	
void LoRaWAN_send(void const *argument)
{
  if (murata_init)
  {
    isActiveSending = 1;
    //Load the BLE data into the payload
    payloadLora[0]=buffer[0]-48;
    payloadLora[1]=buffer[1]-48;
    if(!Murata_LoRaWAN_Send((uint8_t *)payloadLora, sizeof(payloadLora)))
    {
      murata_init++;
      if(murata_init == 10)
        murata_init = 0;
    }
    else
    {
      murata_init = 1;
    }
  }
  else{
    printf("murata not initialized, not sending\r\n"); 
  }
}
/**
  * @brief This function will send the data over dash7. 
  */	
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
  }
  else{
    printf("murata not initialized, not sending\r\n");
  }
}

/**
  * @brief Callback function for UART RX. 
  */	
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &P1_UART)
  {
    Murata_rxCallback();
    murata_data_ready = 1;
  }
  if(huart == &BLE_UART)
  {
    bleflag=1; //flag for ble
  }
}

/**
  * @brief This function will overwrite the existing bluetooth data in flash memory. 
  */	
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

      printf("Write to flash complete.\r\n");
}


void ble_callback(){
  
    if(tmpbuf_ble[0]!=0){
    buffer[ble_counter]=tmpbuf_ble[0];
    HAL_UART_Receive_IT(&BLE_UART,tmpbuf_ble, sizeof(tmpbuf_ble));
    printf("Callback succesfully handled, received: %d\r\n",buffer[ble_counter]-48);
    ble_counter++;
    if(ble_counter==2){
      ble_counter=0; //If we recieved two bytes, we can set it back to zero. 
      init_flag=1;
      if(buffer[0]-48==9 && buffer[0]-48==9){
        sendFlag=!sendFlag;
      }
    } 
  }
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
 /*  printf("octa ID: ");
  for (const char* p = UIDString; *p; ++p)
    {
        printf("%02x", *p);
    }
  printf("\r\n\r\n"); */ //No need to print it out. Still in here incase we add new devices. 
  HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
  HAL_Delay(2000);
  HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
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

  if(GPIO_Pin==OCTA_BTN2_Pin){
  flag = 3;
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