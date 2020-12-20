/**********************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
**********************************************************************/


#include "bmi160.h"


const struct BMI160::AccConfig BMI160::DEFAULT_ACC_CONFIG = {SENS_2G,
                                                             ACC_US_OFF,
                                                             ACC_BWP_2,
                                                             ACC_ODR_8};

const struct BMI160::GyroConfig BMI160::DEFAULT_GYRO_CONFIG = {DPS_2000,
                                                               GYRO_BWP_2,
                                                               GYRO_ODR_8};


//*****************************************************************************
int32_t BMI160::setSensorPowerMode(Sensors sensor, PowerModes pwrMode)
{
    int32_t rtnVal = -1;

    switch(sensor)
    {
        case MAG:
            rtnVal = writeRegister(CMD, (MAG_SET_PMU_MODE | pwrMode));
        break;

        case GYRO:
            rtnVal = writeRegister(CMD, (GYR_SET_PMU_MODE | pwrMode));
        break;

        case ACC:
            rtnVal = writeRegister(CMD, (ACC_SET_PMU_MODE | pwrMode));
        break;

        default:
            rtnVal = -1;
        break;
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::setSensorConfig(const AccConfig &config)
{
    uint8_t data[2];

    data[0] = ((config.us << ACC_US_POS) | (config.bwp << ACC_BWP_POS) |
               (config.odr << ACC_ODR_POS));
    data[1] = config.range;

    return writeBlock(ACC_CONF, ACC_RANGE, data);
}


//*****************************************************************************
int32_t BMI160::setSensorConfig(const GyroConfig &config)
{
    uint8_t data[2];

    data[0] = ((config.bwp << GYRO_BWP_POS) | (config.odr << GYRO_ODR_POS));
    data[1] = config.range;

    return writeBlock(GYR_CONF, GYR_RANGE, data);
}


//*****************************************************************************
int32_t BMI160::getSensorConfig(AccConfig &config)
{
    uint8_t data[2];
    int32_t rtnVal = readBlock(ACC_CONF, ACC_RANGE, data);

    if(rtnVal == RTN_NO_ERROR)
    {
        config.range = static_cast<BMI160::AccRange>(
        (data[1] & ACC_RANGE_MASK));
        config.us = static_cast<BMI160::AccUnderSampling>(
        ((data[0] & ACC_US_MASK) >> ACC_US_POS));
        config.bwp = static_cast<BMI160::AccBandWidthParam>(
        ((data[0] & ACC_BWP_MASK) >> ACC_BWP_POS));
        config.odr = static_cast<BMI160::AccOutputDataRate>(
        ((data[0] & ACC_ODR_MASK) >> ACC_ODR_POS));
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorConfig(GyroConfig &config)
{
    uint8_t data[2];
    int32_t rtnVal = readBlock(GYR_CONF, GYR_RANGE, data);

    if(rtnVal == RTN_NO_ERROR)
    {
        config.range = static_cast<BMI160::GyroRange>(
        (data[1] & GYRO_RANGE_MASK));
        config.bwp = static_cast<BMI160::GyroBandWidthParam>(
        ((data[0] & GYRO_BWP_MASK) >> GYRO_BWP_POS));
        config.odr = static_cast<BMI160::GyroOutputDataRate>(
        ((data[0] & GYRO_ODR_MASK) >> GYRO_ODR_POS));
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorAxis(SensorAxis axis, AxisData &data, AccRange range)
{
    uint8_t localData[2];
    int32_t rtnVal;

    switch(axis)
    {
        case X_AXIS:
            rtnVal = readBlock(DATA_14, DATA_15, localData);
        break;

        case Y_AXIS:
            rtnVal = readBlock(DATA_16, DATA_17, localData);
        break;

        case Z_AXIS:
            rtnVal = readBlock(DATA_18, DATA_19, localData);
        break;

        default:
            rtnVal = -1;
        break;
    }

    if(rtnVal == RTN_NO_ERROR)
    {
        data.raw = ((localData[1] << 8) | localData[0]);
        switch(range)
        {
            case SENS_2G:
                data.scaled = (data.raw/SENS_2G_LSB_PER_G);
            break;

            case SENS_4G:
                data.scaled = (data.raw/SENS_4G_LSB_PER_G);
            break;

            case SENS_8G:
                data.scaled = (data.raw/SENS_8G_LSB_PER_G);
            break;

            case SENS_16G:
                data.scaled = (data.raw/SENS_16G_LSB_PER_G);
            break;
        }
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorAxis(SensorAxis axis, AxisData &data, GyroRange range)
{
    uint8_t localData[2];
    int32_t rtnVal;

    switch(axis)
    {
        case X_AXIS:
            rtnVal = readBlock(DATA_8, DATA_9, localData);
        break;

        case Y_AXIS:
            rtnVal = readBlock(DATA_10, DATA_11, localData);
        break;

        case Z_AXIS:
            rtnVal = readBlock(DATA_12, DATA_13, localData);
        break;

        default:
            rtnVal = -1;
        break;
    }

    if(rtnVal == RTN_NO_ERROR)
    {
        data.raw = ((localData[1] << 8) | localData[0]);
        switch(range)
        {
            case DPS_2000:
                data.scaled = (data.raw/SENS_2000_DPS_LSB_PER_DPS);
            break;

            case DPS_1000:
                data.scaled = (data.raw/SENS_1000_DPS_LSB_PER_DPS);
            break;

            case DPS_500:
                data.scaled = (data.raw/SENS_500_DPS_LSB_PER_DPS);
            break;

            case DPS_250:
                data.scaled = (data.raw/SENS_250_DPS_LSB_PER_DPS);
            break;

            case DPS_125:
                data.scaled = (data.raw/SENS_125_DPS_LSB_PER_DPS);
            break;
        }
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorXYZ(SensorData &data, AccRange range)
{
    uint8_t localData[6];
    int32_t rtnVal;

	if (m_use_irq == true && bmi160_irq_asserted == false)
		return -1;

	rtnVal = readBlock(DATA_14, DATA_19, localData);
	bmi160_irq_asserted = false;
    if(rtnVal == RTN_NO_ERROR)
    {
        data.xAxis.raw = ((localData[1] << 8) | localData[0]);
        data.yAxis.raw = ((localData[3] << 8) | localData[2]);
        data.zAxis.raw = ((localData[5] << 8) | localData[4]);

        switch(range)
        {
            case SENS_2G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_2G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_2G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_2G_LSB_PER_G);
            break;

            case SENS_4G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_4G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_4G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_4G_LSB_PER_G);
            break;

            case SENS_8G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_8G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_8G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_8G_LSB_PER_G);
            break;

            case SENS_16G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_16G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_16G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_16G_LSB_PER_G);
            break;
        }
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorXYZ(SensorData &data, GyroRange range)
{
    uint8_t localData[6];
    int32_t rtnVal = readBlock(DATA_8, DATA_13, localData);

    if(rtnVal == RTN_NO_ERROR)
    {
        data.xAxis.raw = ((localData[1] << 8) | localData[0]);
        data.yAxis.raw = ((localData[3] << 8) | localData[2]);
        data.zAxis.raw = ((localData[5] << 8) | localData[4]);

        switch(range)
        {
            case DPS_2000:
                data.xAxis.scaled = (data.xAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
            break;

            case DPS_1000:
                data.xAxis.scaled = (data.xAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
            break;

            case DPS_500:
                data.xAxis.scaled = (data.xAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_500_DPS_LSB_PER_DPS);
            break;

            case DPS_250:
                data.xAxis.scaled = (data.xAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_250_DPS_LSB_PER_DPS);
            break;

            case DPS_125:
                data.xAxis.scaled = (data.xAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_125_DPS_LSB_PER_DPS);
            break;
        }
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorXYZandSensorTime(SensorData &data,
                                          SensorTime &sensorTime,
                                          AccRange range)
{
    uint8_t localData[9];
    int32_t rtnVal = readBlock(DATA_14, SENSORTIME_2, localData);
    if(rtnVal == RTN_NO_ERROR)
    {
        data.xAxis.raw = ((localData[1] << 8) | localData[0]);
        data.yAxis.raw = ((localData[3] << 8) | localData[2]);
        data.zAxis.raw = ((localData[5] << 8) | localData[4]);

        switch(range)
        {
            case SENS_2G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_2G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_2G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_2G_LSB_PER_G);
            break;

            case SENS_4G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_4G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_4G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_4G_LSB_PER_G);
            break;

            case SENS_8G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_8G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_8G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_8G_LSB_PER_G);
            break;

            case SENS_16G:
                data.xAxis.scaled = (data.xAxis.raw/SENS_16G_LSB_PER_G);
                data.yAxis.scaled = (data.yAxis.raw/SENS_16G_LSB_PER_G);
                data.zAxis.scaled = (data.zAxis.raw/SENS_16G_LSB_PER_G);
            break;
        }

        sensorTime.raw = ((localData[8] << 16) | (localData[7] << 8) |
                           localData[6]);
        sensorTime.seconds = (sensorTime.raw * SENSOR_TIME_LSB);
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getSensorXYZandSensorTime(SensorData &data,
                                          SensorTime &sensorTime,
                                          GyroRange range)
{
    uint8_t localData[16];
    int32_t rtnVal = readBlock(DATA_8, SENSORTIME_2, localData);
    if(rtnVal == RTN_NO_ERROR)
    {
        data.xAxis.raw = ((localData[1] << 8) | localData[0]);
        data.yAxis.raw = ((localData[3] << 8) | localData[2]);
        data.zAxis.raw = ((localData[5] << 8) | localData[4]);

        switch(range)
        {
            case DPS_2000:
                data.xAxis.scaled = (data.xAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
            break;

            case DPS_1000:
                data.xAxis.scaled = (data.xAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
            break;

            case DPS_500:
                data.xAxis.scaled = (data.xAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_500_DPS_LSB_PER_DPS);
            break;

            case DPS_250:
                data.xAxis.scaled = (data.xAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_250_DPS_LSB_PER_DPS);
            break;

            case DPS_125:
                data.xAxis.scaled = (data.xAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                data.yAxis.scaled = (data.yAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                data.zAxis.scaled = (data.zAxis.raw/SENS_125_DPS_LSB_PER_DPS);
            break;
        }

        sensorTime.raw = ((localData[14] << 16) | (localData[13] << 8) |
                           localData[12]);
        sensorTime.seconds = (sensorTime.raw * SENSOR_TIME_LSB);
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getGyroAccXYZandSensorTime(SensorData &accData,
                                           SensorData &gyroData,
                                           SensorTime &sensorTime,
                                           AccRange accRange,
                                           GyroRange gyroRange)
{
    uint8_t localData[16];
    int32_t rtnVal = readBlock(DATA_8, SENSORTIME_2, localData);
    if(rtnVal == RTN_NO_ERROR)
    {
        gyroData.xAxis.raw = ((localData[1] << 8) | localData[0]);
        gyroData.yAxis.raw = ((localData[3] << 8) | localData[2]);
        gyroData.zAxis.raw = ((localData[5] << 8) | localData[4]);

        accData.xAxis.raw = ((localData[7] << 8) | localData[6]);
        accData.yAxis.raw = ((localData[9] << 8) | localData[8]);
        accData.zAxis.raw = ((localData[11] << 8) | localData[10]);

        switch(gyroRange)
        {
            case DPS_2000:
                gyroData.xAxis.scaled = (gyroData.xAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                gyroData.yAxis.scaled = (gyroData.yAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
                gyroData.zAxis.scaled = (gyroData.zAxis.raw/SENS_2000_DPS_LSB_PER_DPS);
            break;

            case DPS_1000:
                gyroData.xAxis.scaled = (gyroData.xAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                gyroData.yAxis.scaled = (gyroData.yAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
                gyroData.zAxis.scaled = (gyroData.zAxis.raw/SENS_1000_DPS_LSB_PER_DPS);
            break;

            case DPS_500:
                gyroData.xAxis.scaled = (gyroData.xAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                gyroData.yAxis.scaled = (gyroData.yAxis.raw/SENS_500_DPS_LSB_PER_DPS);
                gyroData.zAxis.scaled = (gyroData.zAxis.raw/SENS_500_DPS_LSB_PER_DPS);
            break;

            case DPS_250:
                gyroData.xAxis.scaled = (gyroData.xAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                gyroData.yAxis.scaled = (gyroData.yAxis.raw/SENS_250_DPS_LSB_PER_DPS);
                gyroData.zAxis.scaled = (gyroData.zAxis.raw/SENS_250_DPS_LSB_PER_DPS);
            break;

            case DPS_125:
                gyroData.xAxis.scaled = (gyroData.xAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                gyroData.yAxis.scaled = (gyroData.yAxis.raw/SENS_125_DPS_LSB_PER_DPS);
                gyroData.zAxis.scaled = (gyroData.zAxis.raw/SENS_125_DPS_LSB_PER_DPS);
            break;
        }

        switch(accRange)
        {
            case SENS_2G:
                accData.xAxis.scaled = (accData.xAxis.raw/SENS_2G_LSB_PER_G);
                accData.yAxis.scaled = (accData.yAxis.raw/SENS_2G_LSB_PER_G);
                accData.zAxis.scaled = (accData.zAxis.raw/SENS_2G_LSB_PER_G);
            break;

            case SENS_4G:
                accData.xAxis.scaled = (accData.xAxis.raw/SENS_4G_LSB_PER_G);
                accData.yAxis.scaled = (accData.yAxis.raw/SENS_4G_LSB_PER_G);
                accData.zAxis.scaled = (accData.zAxis.raw/SENS_4G_LSB_PER_G);
            break;

            case SENS_8G:
                accData.xAxis.scaled = (accData.xAxis.raw/SENS_8G_LSB_PER_G);
                accData.yAxis.scaled = (accData.yAxis.raw/SENS_8G_LSB_PER_G);
                accData.zAxis.scaled = (accData.zAxis.raw/SENS_8G_LSB_PER_G);
            break;

            case SENS_16G:
                accData.xAxis.scaled = (accData.xAxis.raw/SENS_16G_LSB_PER_G);
                accData.yAxis.scaled = (accData.yAxis.raw/SENS_16G_LSB_PER_G);
                accData.zAxis.scaled = (accData.zAxis.raw/SENS_16G_LSB_PER_G);
            break;
        }

        sensorTime.raw = ((localData[14] << 16) | (localData[13] << 8) |
                           localData[12]);
        sensorTime.seconds = (sensorTime.raw * SENSOR_TIME_LSB);
    }

    return rtnVal;
}

int32_t BMI160::setSampleRate(int sample_rate)
{
	int sr_reg_val = -1;
	int i;
	const uint16_t odr_table[][2] = {
	    {25, GYRO_ODR_6}, ///<25Hz
        {50, GYRO_ODR_7}, ///<50Hz
        {100, GYRO_ODR_8}, ///<100Hz
        {200, GYRO_ODR_9}, ///<200Hz
        {400, GYRO_ODR_10}, ///<400Hz
        {800, GYRO_ODR_11}, ///<800Hz
        {1600, GYRO_ODR_12}, ///<1600Hz
        {3200, GYRO_ODR_13}, ///<3200Hz
	};

	int num_sr = sizeof(odr_table)/sizeof(odr_table[0]);
	for (i = 0; i < num_sr; i++) {
		if (sample_rate == odr_table[i][0]) {
			sr_reg_val = odr_table[i][1];
			break;
		}
	}

	if (sr_reg_val == -1)
		return -2;

	AccConfig accConfigRead;
	if (getSensorConfig(accConfigRead) == BMI160::RTN_NO_ERROR) {
	accConfigRead.odr = (AccOutputDataRate)sr_reg_val;
		return setSensorConfig(accConfigRead) == BMI160::RTN_NO_ERROR ? 0 : -1;
	} else
		return -1;
}


//*****************************************************************************
int32_t BMI160::getSensorTime(SensorTime &sensorTime)
{
    uint8_t localData[3];
    int32_t rtnVal = readBlock(SENSORTIME_0, SENSORTIME_2, localData);

    if(rtnVal == RTN_NO_ERROR)
    {
        sensorTime.raw = ((localData[2] << 16) | (localData[1] << 8) |
                           localData[0]);
        sensorTime.seconds = (sensorTime.raw * SENSOR_TIME_LSB);
    }

    return rtnVal;
}


//*****************************************************************************
int32_t BMI160::getTemperature(float *temp)
{
    uint8_t data[2];
    uint16_t rawTemp;

    int32_t rtnVal = readBlock(TEMPERATURE_0, TEMPERATURE_1, data);
    if(rtnVal == RTN_NO_ERROR)
    {
        rawTemp = ((data[1] << 8) | data[0]);
        if(rawTemp & 0x8000)
        {
            *temp = (23.0F - ((0x10000 - rawTemp)/512.0F));
        }
        else
        {
            *temp = ((rawTemp/512.0F) + 23.0F);
        }
    }

    return rtnVal;
}

//***********************************************************************************
int32_t BMI160::BMI160_DefaultInitalize(){

		//soft reset the accelerometer
		writeRegister(CMD ,SOFT_RESET);
		wait(0.1);

	    //Power up sensors in normal mode
	    if(setSensorPowerMode(BMI160::GYRO, BMI160::SUSPEND) != BMI160::RTN_NO_ERROR){
	        printf("Failed to set gyroscope power mode\n");
	    }

	    wait(0.1);

	    if(setSensorPowerMode(BMI160::ACC, BMI160::NORMAL) != BMI160::RTN_NO_ERROR){
	        printf("Failed to set accelerometer power mode\n");
	    }
	    wait(0.1);

	    BMI160::AccConfig accConfig;
	    BMI160::AccConfig accConfigRead;
	    accConfig.range = BMI160::SENS_2G;
	    accConfig.us = BMI160::ACC_US_OFF;
	    accConfig.bwp = BMI160::ACC_BWP_2;
	    accConfig.odr = BMI160::ACC_ODR_6;
	    if(setSensorConfig(accConfig) == BMI160::RTN_NO_ERROR)
	    {
	        if(getSensorConfig(accConfigRead) == BMI160::RTN_NO_ERROR)
	        {
	            if((accConfig.range != accConfigRead.range) ||
	                    (accConfig.us != accConfigRead.us) ||
	                    (accConfig.bwp != accConfigRead.bwp) ||
	                    (accConfig.odr != accConfigRead.odr))
	            {
	                printf("ACC read data desn't equal set data\n\n");
	                printf("ACC Set Range = %d\n", accConfig.range);
	                printf("ACC Set UnderSampling = %d\n", accConfig.us);
	                printf("ACC Set BandWidthParam = %d\n", accConfig.bwp);
	                printf("ACC Set OutputDataRate = %d\n\n", accConfig.odr);
	                printf("ACC Read Range = %d\n", accConfigRead.range);
	                printf("ACC Read UnderSampling = %d\n", accConfigRead.us);
	                printf("ACC Read BandWidthParam = %d\n", accConfigRead.bwp);
	                printf("ACC Read OutputDataRate = %d\n\n", accConfigRead.odr);
	            }

	        }
	        else
	        {
	             printf("Failed to read back accelerometer configuration\n");
	        }
	    }
	    else
	    {
	        printf("Failed to set accelerometer configuration\n");
	    }
	    return 0;
}

//***********************************************************************************
int32_t BMI160::enable_data_ready_interrupt() {
	uint8_t data = 0;
	uint8_t temp = 0;
	int32_t result;

	result = readRegister(INT_EN_1, &data);
	temp = data & ~0x10;
	data = temp | ((1 << 4) & 0x10);
	/* Writing data to INT ENABLE 1 Address */
	result |= writeRegister(INT_EN_1, data);

	// configure in_out ctrl
	//bmi160_get_regs(BMI160_INT_OUT_CTRL_ADDR, &data, 1, dev);
	result |= readRegister(INT_OUT_CTRL, &data);
	data = 0x09;
	result |= writeRegister(INT_OUT_CTRL,data);

	//config int latch
	//bmi160_get_regs(BMI160_INT_LATCH_ADDR, &data, 1, dev);
	result |= readRegister(INT_LATCH, &data);
	data = 0x0F;
	result |= writeRegister(INT_LATCH, data);

	//bmi160_get_regs(BMI160_INT_MAP_1_ADDR, &data, 1, dev);
	result |= readRegister(INT_MAP_1, &data);
	data = 0x80;
	result |= writeRegister(INT_MAP_1, data);

	if(result != 0){
		printf("BMI160::%s failed.\r\n", __func__);
		return -1;
	}

	m_bmi160_irq->disable_irq();
	m_bmi160_irq->mode(PullUp);
	m_bmi160_irq->fall(this, &BMI160::irq_handler);
	m_bmi160_irq->enable_irq();
	return 0;
}

void BMI160::irq_handler() {
	bmi160_irq_asserted = true;
}

int32_t BMI160::reset() {
	if (m_use_irq)
		m_bmi160_irq->disable_irq();
	bmi160_irq_asserted = false;
	writeRegister(CMD, SOFT_RESET);
	return 0;
}
