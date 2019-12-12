#include "platform.h"
#include "SPG30.h"

struct OCTA_header SPG30_Header;
I2C_HandleTypeDef *hi2cLib; 

uint8_t SPG30_Initialize(void) {
    printf("****_____Initalize SPG30 now!!____****\r\n");
    #ifndef SPG30_CONNECTOR
        printf("No SPG30_CONNECTOR provided in Makefile \r\n");
        return 0;
    #else
        SPG30_Header = platform_getHeader((uint8_t)SPG30_CONNECTOR); //this connector is specified in de makeFile of the application (in this case 1)
        if (!SPG30_Header.active) {
            printf("Invalid SPG30_CONNECTOR provided in Makefile\r\n");
            return 0; 
        }
        else 
            printf("SPG30 sensor on P%d, initializing I2C\r\n", (uint8_t)SPG30_CONNECTOR);
    #endif

    platform_initialize_I2C(SPG30_Header);

    hi2cLib = SPG30_Header.i2cHandle;       //we don't know what this does but included for the moment 

    return 1;

}


uint8_t WRITE_REGISTER_SPG30(uint8_t pData[],uint8_t length)
{
    uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, SPG30_ADDR<<1, pData,length, HAL_MAX_DELAY);
    return status;
}

uint8_t READ_REGISTER_SPG30(uint8_t buf[],uint16_t reg,uint8_t length)
{
    uint8_t status = HAL_I2C_Mem_Read(hi2cLib, SPG30_ADDR<<1, reg, I2C_MEMADD_SIZE_16BIT, buf, length, HAL_MAX_DELAY);
    return status;
}


