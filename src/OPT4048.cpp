/*

Arduino library for Texas Instruments OPT4048 Ambient Light Sensor

---

The MIT License (MIT)

Copyright (c) 2023 RetroSpy Technologies

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#include <Wire.h>

#include "OPT4048.h"

OPT4048::OPT4048()
{
}

OPT4048_ErrorCode OPT4048::begin(uint8_t address) 
{
	OPT4048_ErrorCode error = NO_ERROR;
	_address = address;
	Wire.begin();

	return NO_ERROR;
}

uint16_t OPT4048::readDeviceID() 
{
	uint16_t result = 0;
	OPT4048_ErrorCode error = writeData(DEVICE_ID);
	if (error == NO_ERROR)
		error = readData(&result);
	return result;
}

OPT4048_ConfigA OPT4048::readConfigA() 
{
	OPT4048_ConfigA config;
	OPT4048_ErrorCode error = writeData(CONFIG_A);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return config;
}

OPT4048_ConfigB OPT4048::readConfigB() 
{
	OPT4048_ConfigB config;
	OPT4048_ErrorCode error = writeData(CONFIG_B);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return config;
}

OPT4048_ErrorCode OPT4048::writeConfigA(OPT4048_ConfigA config) 
{
	config.FixedZero = 0;
	Wire.beginTransmission(_address);
	Wire.write(CONFIG_A);
	Wire.write(config.rawData >> 8);
	Wire.write(config.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_ErrorCode OPT4048::writeConfigB(OPT4048_ConfigB config) 
{
	config.FixedZero = 0;
	config.Fixed128 = 128;
	Wire.beginTransmission(_address);
	Wire.write(CONFIG_B);
	Wire.write(config.rawData >> 8);
	Wire.write(config.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_RESULT OPT4048::readResult(OPT4048_Channel channel) 
{
	OPT4048_REGISTER registerResponse = readRegister((OPT4048_Commands)channel);
	if (registerResponse.error == NO_ERROR) 
	{
		uint8_t exponent = registerResponse.value >> 12;
		uint32_t mantissa = (uint32_t)registerResponse.value << 8;
		registerResponse = readRegister((OPT4048_Commands)(channel+1));
		if (registerResponse.error == NO_ERROR) 
		{
			OPT4048_RESULT result;
			result.rawResult.Mantissa = mantissa | registerResponse.value >> 8;
			result.rawResult.Exponent = exponent;
			result.error = NO_ERROR;
			return result;
		}	
		else 
		{
			return returnResultError(registerResponse.error);
		}
	}	
	else 
	{
		return returnResultError(registerResponse.error);
	}
}

OPT4048_THRESHOLD OPT4048::readHighLimit() 
{
	return readThresholdRegister(THRESHOLD_HIGH);
}

OPT4048_ErrorCode OPT4048::writeHighLimit(OPT4048_ER12 threshold)
{
	Wire.beginTransmission(_address);
	Wire.write(THRESHOLD_HIGH);
	Wire.write(threshold.rawData >> 8);
	Wire.write(threshold.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());	
}

OPT4048_THRESHOLD OPT4048::readLowLimit() 
{
	return readThresholdRegister(THRESHOLD_LOW);
}

OPT4048_ErrorCode OPT4048::writeLowLimit(OPT4048_ER12 threshold)
{
	Wire.beginTransmission(_address);
	Wire.write(THRESHOLD_LOW);
	Wire.write(threshold.rawData >> 8);
	Wire.write(threshold.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_REGISTER OPT4048::readRegister(OPT4048_Commands command) 
{
	OPT4048_REGISTER result;
	result.error = writeData(command);
	if (result.error == NO_ERROR) 
	{

		result.error = readData(&result.value);

		return result;
	}
	else 
	{
		return result;
	}
}

OPT4048_THRESHOLD OPT4048::readThresholdRegister(OPT4048_Commands command) 
{
	OPT4048_ErrorCode error = writeData(command);
	if (error == NO_ERROR) 
	{
		OPT4048_THRESHOLD result;
		result.error = NO_ERROR;

		OPT4048_ER12 er;
		error = readData(&er.rawData);
		if (error == NO_ERROR) 
		{
			result.rawResult = er;
		}
		else 
		{
			result.error = error;
		}

		return result;
	}
	else 
	{	
		return returnThresholdError(error);
	}
}


OPT4048_ErrorCode OPT4048::writeData(OPT4048_Commands command)
{
	Wire.beginTransmission(_address);
	Wire.write(command);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission(true));
}

OPT4048_ErrorCode OPT4048::readData(uint16_t* data)
{
	uint8_t	buf[2];

	Wire.requestFrom(_address, (uint8_t)2);

	int counter = 0;
	while (Wire.available() < 2)
	{
		counter++;
		delay(10);
		if (counter > 250)
			return TIMEOUT_ERROR;
	}

	Wire.readBytes(buf, 2);
	*data = (buf[0] << 8) | buf[1];

	return NO_ERROR;
}


OPT4048_RESULT OPT4048::returnResultError(OPT4048_ErrorCode error) 
{
	OPT4048_RESULT result;
	result.error = error;
	return result;
}

OPT4048_THRESHOLD OPT4048::returnThresholdError(OPT4048_ErrorCode error) 
{
	OPT4048_THRESHOLD result;
	result.error = error;
	return result;
}