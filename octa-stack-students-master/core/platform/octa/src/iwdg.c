#include "iwdg.h"

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
void OCTA_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */
/*   FLASH_OBProgramInitTypeDef pOBInit;
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); // Clear the FLASH's pending flags.
  HAL_FLASH_OB_Unlock();
  HAL_FLASHEx_OBGetConfig(&pOBInit); // Get the Option bytes configuration.
  if(pOBInit.OptionType==OPTIONBYTE_USER && pOBInit.USERType == OB_USER_IWDG_STOP && pOBInit.USERConfig == OB_IWDG_STOP_FREEZE){
    printf("Watchdog already disabled\r\n");
  }
  else{
    printf("Watchdog disabled\r\n");
    pOBInit.OptionType = OPTIONBYTE_USER;
    pOBInit.USERType = OB_USER_IWDG_STOP;
    pOBInit.USERConfig = OB_IWDG_STOP_FREEZE;
    HAL_FLASHEx_OBProgram(&pOBInit);
  }
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();
  HAL_FLASH_OB_Launch(); */  
  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095; 
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN IWDG_Init 2 */

  //ENABLE FREEZE IN STOP MODE
  /* USER CODE END IWDG_Init 2 */

}

void IWDG_feed(void const *argument)
{
  WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
}