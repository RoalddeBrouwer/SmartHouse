#include "LSM303AGRSensor.h"


/**
 * Sets the I2C interface for the LSM303AGR
 * @param hi2c: the I2C interface
 */
void LSM303AGR_setI2CInterface(I2C_HandleTypeDef *hi2c) {

	LSM303AGR_hi2c = hi2c;
}

/**
 * Read a certain register of the LSM303AGR depending on the value of ACC_MAG:
 * 		- if this value is 0: read out a register of the accelerometer
 * 		- if this value is 1: read out a register of the magnetormeter
 * The standard HAL library values are used for the size of the register and time-out.
 * @param LSM303AGR_reg: the register that needs to be read of the LSM303AGR
 * @param LSM303AGR_data: a pointer where the data of the register needs to be stored
 * @param ACC_MAG: depending on the value a value a register of the accelerometer of magnetometer are read
 * @return returns the status of the I2C transfer
 */
HAL_StatusTypeDef LSM303AGR_readRegister(uint8_t LSM303AGR_reg,
		uint8_t LSM303AGR_data, uint8_t ACC_MAG) {

	if (ACC_MAG == 0) {
		HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	} else {
		HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	}

	return HAL_OK;
}

/**
 * Writes a cerain value to a certain register of the LSM303AGR depending on the value of ACC_MAG:
 *   	- if this value is 0: write to a register of the accelerometer
 * 		- if this value is 1: write to a register of the magnetormeter
 * the standard HAL library values are used for the size of the register and time-out.
 * @param LSM303AGR_reg: the register where data needs to be written to
 * @param LSM303AGR_data: a pointer to the data that needs to be written
 * @param ACC_MAG: depending on the value, a write is performed to the accelerometer of magnetometer
 * @return returns the status of the I2C transfer
 */
HAL_StatusTypeDef LSM303AGR_writeRegister(uint8_t LSM303AGR_reg,
		uint8_t LSM303AGR_data, uint8_t ACC_MAG) {

	if (ACC_MAG == 0) {
		HAL_I2C_Mem_Write(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	} else {
		HAL_I2C_Mem_Write(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_setting), HAL_MAX_DELAY);
	}

	return HAL_OK;

}

/**
 * Initialization of the LSM303AGR accelerometer and magnetometer.
 * The accelerometer is initialized with the following values:
 * 		- Powerdown,
 * 		- X, Y and Z-axes enabled
 * 		- BDU enabled (output registers are not updated until the MSB and LSB have been read)
 *
 * The magnetometer is initialized with the following values:
 * 		- Idle mode,
 * 		- Temperature compensation enabled,
 * 		- Low-power mode enabled,
 * 		- Offset cancelation enabled,
 * 		- Low pass filter enabled,
 * 		- BDU enabled
 */
void LSM303AGR_init() {

/* 	//ga naar de application node op de site en daar staat alle registers die je moet instellen voor interrupts

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, 0x2F, 0);  //enable/turn on xyz, ODR = 25Hz (gegokt, normal mode)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG2, 0x00 , 0); //highpass filter disable (got undesired effects otherwise)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG3, 0x40 , 0); //interrupt to int1 and choose AOI. je hebt ook DataReady en dan kun je pollen naar accelerometer bv om te zien of data klaarstaat, maar gebruiken wij niet.

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG4, 0x00 , 0); //FS=2G, wil zeggen laagste treshold, alles voor derest disablen. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, 0x00 , 0); //no latching, alles disablen

	LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_THS, 0x50 , 0);	//set treshold to 250mg, is voor hoe hard je stoot. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_DUR, 0x3F , 0);  //duration=0, duration van de interrupt. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_CFG, 0x0A , 0);  //INT1_CFG_A enable xh & yH interrupt
 */

 //Click interrupt! 
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, 0x1F, 0);  //enable/turn on xyz, ODR = 1Hz (LP Mode)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG2, 0x00 , 0); //highpass filter disable (got undesired effects otherwise)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG3, 0x80 , 0); //We use a click interrupt because we want to generate an interrupt when the person stops walking. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG4, 0x00 , 0); //FS=2G, wil zeggen laagste treshold, alles voor derest disablen. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, 0x00 , 0); //no latching, alles disablen

	LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_CFG_A, 0x15 , 0);  //Enable all single click interrupts! 

	//LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_SRC_A, 0x10 , 0);  //enable interrupt Sclick

	LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_THS_A, 0x07, 0);  //set an appriorate treshold over which we have to go for a certain time limit! 

	LSM303AGR_writeRegister(LSM303AGR_ACC_TIME_LIMIT_A, 0x7F , 0);  //Time limit = value*1/ODR (ODR = 1).. 127*0.1= 127s 

	LSM303AGR_writeRegister(LSM303AGR_ACC_TIME_LATENCY_A, 0x00, 0); //Time latency on zero  

	/* //click double interrupt

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, 0x1F, 0);  //enable/turn on xyz, ODR = 10Hz (normal mode), zodanig dat time limit groot genoeg kan worden gezet (zie verder)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG2, 0x00 , 0); //highpass filter disable (got undesired effects otherwise)

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG3, 0x80 , 0); //put de click interrupt on int line 1

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG4, 0x00 , 0); //FS=2G, wil zeggen laagste treshold, alles voor derest disablen. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, 0x00 , 0); //no latching, alles disablen

	LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_CFG_A, 0x02 , 0); //enablen double click on x-axis

	//LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_SRC_A, 0x20 , 0); //double click enable

	LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_THS_A, 0x20 , 0);	//set treshold  

	LSM303AGR_writeRegister(LSM303AGR_ACC_TIME_LATENCY_A, 0x00 , 0);  //set how long the interrupt lasts, we set minimum duration. 

	LSM303AGR_writeRegister(LSM303AGR_ACC_TIME_LIMIT_A, 0x7F , 0);  	 //time limiit = value * 1/ODR so maximum time limit = 127*0.1 = +/- 10 s = time to do rep

	LSM303AGR_writeRegister(LSM303AGR_ACC_CLICK_WINDOW, 0x0A , 0);  	 //max time in between 2 clicks. If the second click is later than the time window the interrupt won't be generated.  */
}

/**
 * Resets the LSM303AGR accelerometer
 */
void LSM303AGR_ACC_reset() {
	LSM303AGR_setting = LSM303AGR_ACC_BOOT;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, LSM303AGR_setting, 0);
}

/**
 * Resets the LSM303AGR magnetometer
 */
void LSM303AGR_MAG_reset() {

	LSM303AGR_setting = LSM303AGR_MAG_SOFT_RST;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);

}

/**
 * Put the accelerometer in power-down mode
 */
void LSM303AGR_powerDownAccelerometer() {

	LSM303AGR_setting = LSM303AGR_ACC_ODR_POWERDOWN | LSM303AGR_ACC_X_EN
			| LSM303AGR_ACC_Y_EN | LSM303AGR_ACC_Z_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, LSM303AGR_setting, 0);

}

/**
 * Put the magnetometer in idle mode
 */
void LSM303AGR_powerDownMagnetometer() {

	LSM303AGR_setting = LSM303AGR_MAG_COMP_TEMP_EN | LSM303AGR_MAG_LP_EN
			| LSM303AGR_MAG_ODR_10HZ | LSM303AGR_MAG_MD_IDLE;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);
}

/**
 * Wake up the accelerometer from powerdown mode
 */
void LSM303AGR_wakeUpAccelerometer() {

	LSM303AGR_setting = LSM303AGR_ACC_ODR_10HZ | LSM303AGR_ACC_X_EN
			| LSM303AGR_ACC_Y_EN | LSM303AGR_ACC_Z_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, LSM303AGR_setting, 0);

}

/**
 * Wake up the magnetometer from idle mode
 */
void LSM303AGR_wakeUpMagnetometer() {

	LSM303AGR_setting = LSM303AGR_MAG_COMP_TEMP_EN | LSM303AGR_MAG_LP_EN
			| LSM303AGR_MAG_ODR_10HZ | LSM303AGR_MAG_MD_CONT;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);

}

/**
 * Reads the output registers of the accelerometer
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_ACC_readAccelerationData(int32_t *pData) {
	//accRegisterData[6];
	//accRawData[3];

	//magRegisterData[6];
	//magRawData[3];
	//LSM303AGR_ACC_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,	LSM303AGR_ACC_MULTI_READ, I2C_MEMADD_SIZE_8BIT, accRegisterData,sizeof(accRegisterData), HAL_MAX_DELAY);

	accRawData[0] = (accRegisterData[1] << 8)	| accRegisterData[0];
	accRawData[1] = (accRegisterData[3] << 8)| accRegisterData[2];
	accRawData[2] = (accRegisterData[5] << 8)| accRegisterData[4];

	/* Apply proper shift and sensitivity */
	// Normal mode 10-bit, shift = 6 and FS = 2
	pData[0] = (int32_t) (((accRawData[0] >> 6) * 3900 + 500) / 1000);
	pData[1] = (int32_t) (((accRawData[1] >> 6) * 3900 + 500) / 1000);
	pData[2] = (int32_t) (((accRawData[2] >> 6) * 3900 + 500) / 1000);

}

/**
 * Reads the output registers of the magnetometer and applies the sensitivity
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_MAG_readMagneticData(int32_t *pData) {
	//magRegisterData[6];
		//magRawData[3];
		//LSM303AGR_ACC_TEMP_DATA rawData;

	//LSM303AGR_MAG_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
	LSM303AGR_MAG_MULTI_READ, I2C_MEMADD_SIZE_8BIT, magRegisterData,
			sizeof(magRegisterData), HAL_MAX_DELAY);

	magRawData[0] = (magRegisterData[1] << 8)
			| magRegisterData[0];
	magRawData[1] = (magRegisterData[3] << 8)
			| magRegisterData[2];
	magRawData[2] = (magRegisterData[5] << 8)
			| magRegisterData[4];

	/* Calculate the data. */
	pData[0] = (int32_t) (magRawData[0] * 1.5f);
	pData[1] = (int32_t) (magRawData[1] * 1.5f);
	pData[2] = (int32_t) (magRawData[2] * 1.5f);

}

/**
 * Read the output registers of the magnetometer
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_MAG_readMagneticRawData(uint16_t *pData) {
	//magRegisterData[6];
			//magRawData[3];
	//LSM303AGR_MAG_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
	LSM303AGR_MAG_MULTI_READ, I2C_MEMADD_SIZE_8BIT, magRegisterData,
			sizeof(magRegisterData), HAL_MAX_DELAY);

	pData[0] = (magRegisterData[1] << 8)
			| magRegisterData[0];
	pData[1] = (magRegisterData[3] << 8)
			| magRegisterData[2];
	pData[2] = (magRegisterData[5] << 8)
			| magRegisterData[4];


}

void LSM303AGR_ACC_readAccelerationRawData(uint16_t *pData) {
	//accRegisterData[6];
	//accRawData[3];

	//magRegisterData[6];
	//magRawData[3];
	//LSM303AGR_ACC_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,	LSM303AGR_ACC_MULTI_READ, I2C_MEMADD_SIZE_8BIT, accRegisterData,sizeof(accRegisterData), HAL_MAX_DELAY);

	pData[0] = (accRegisterData[1] << 8)	| accRegisterData[0];
	pData[1] = (accRegisterData[3] << 8)| accRegisterData[2];
	pData[2] = (accRegisterData[5] << 8)| accRegisterData[4];

	/* Apply proper shift and sensitivity */
	// Normal mode 10-bit, shift = 6 and FS = 2
	//pData[0] = (int32_t) (((accRawData[0] >> 6) * 3900 + 500) / 1000);
	//pData[1] = (int32_t) (((accRawData[1] >> 6) * 3900 + 500) / 1000);
	//pData[2] = (int32_t) (((accRawData[2] >> 6) * 3900 + 500) / 1000);

}
