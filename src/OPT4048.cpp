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
	ADC_to_XYZ[0][0] = 2.34892992e-4;
}

OPT4048_ErrorCode OPT4048::begin(uint8_t address) 
{
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

OPT4048_ErrorCode OPT4048::readConfig(OPT4048_ConfigA& config)
{
	OPT4048_ErrorCode error = writeData(CONFIG_A);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return error;
}

OPT4048_ErrorCode OPT4048::readConfig(OPT4048_ConfigB& config)
{
	OPT4048_ErrorCode error = writeData(CONFIG_B);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return error;
}

OPT4048_ErrorCode OPT4048::writeConfig(OPT4048_ConfigA config) 
{
	config.FixedZero = 0;
	Wire.beginTransmission(_address);
	Wire.write(CONFIG_A);
	Wire.write(config.rawData >> 8);
	Wire.write(config.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_ErrorCode OPT4048::writeConfig(OPT4048_ConfigB config) 
{
	config.FixedZero = 0;
	config.Fixed128 = 128;
	Wire.beginTransmission(_address);
	Wire.write(CONFIG_B);
	Wire.write(config.rawData >> 8);
	Wire.write(config.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_THRESHOLD OPT4048::readLimit(OPT4048_Commands command)
{
	OPT4048_THRESHOLD limit;
	limit.error = writeData(command);
	if (limit.error == NO_ERROR)
		limit.error = readData(&limit.rawResult.rawData);
	return limit;
}

OPT4048_ErrorCode OPT4048::writeLimit(OPT4048_Commands command, OPT4048_ER12 threshold)
{
	Wire.beginTransmission(_address);
	Wire.write(command);
	Wire.write(threshold.rawData >> 8);
	Wire.write(threshold.rawData & 0x00FF);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission());
}

OPT4048_RESULT OPT4048::readChannel(OPT4048_Channel channel) 
{
	OPT4048_THRESHOLD register1 = readLimit((OPT4048_Commands)channel);
	if (register1.error == NO_ERROR)
	{
		uint8_t exponent = register1.rawResult.Exponent;
		uint32_t mantissa = register1.rawResult.Mantissa << 8;
		OPT4048_REGISTER register2 = readRegister((OPT4048_Commands)(channel + 1));
		if (register2.error == NO_ERROR)
		{
			OPT4048_RESULT result;
			result.rawResult.Mantissa = mantissa + (register2.value >> 8);
			result.rawResult.Exponent = exponent;
			result.error = NO_ERROR;
			return result;
		}
		else
		{
			return returnResultError(register2.error);
		}
	}
	else
	{
		return returnResultError(register1.error);
	}
}

OPT4048_THRESHOLD OPT4048::readHighLimit() 
{
	return readLimit(THRESHOLD_HIGH);
}

OPT4048_ErrorCode OPT4048::writeHighLimit(OPT4048_ER12 threshold)
{
	return writeLimit(THRESHOLD_HIGH, threshold);
}

OPT4048_THRESHOLD OPT4048::readLowLimit() 
{
	return readLimit(THRESHOLD_LOW);
}

OPT4048_ErrorCode OPT4048::writeLowLimit(OPT4048_ER12 threshold)
{
	return writeLimit(THRESHOLD_LOW, threshold);
}

OPT4048_REGISTER OPT4048::readRegister(OPT4048_Commands command) 
{
	OPT4048_REGISTER result;
	result.error = writeData(command);
	if (result.error == NO_ERROR) 
	{
		result.error = readData(&result.value);
	}
	return result;
}

OPT4048_ErrorCode OPT4048::writeData(OPT4048_Commands command)
{
	Wire.beginTransmission(_address);
	Wire.write(command);
	return (OPT4048_ErrorCode)(-10 * Wire.endTransmission(true));
}

OPT4048_ErrorCode OPT4048::readADC(OPT4048_ADC& values)
{
	OPT4048_RESULT result = readChannel(CHANNEL_0);
	if (result.error == NO_ERROR)
	{
		values.channel_0 = result.GetADCValue();
		result = readChannel(CHANNEL_1);
		if (result.error == NO_ERROR)
		{
			values.channel_1 = result.GetADCValue();
			result = readChannel(CHANNEL_2);
			if (result.error == NO_ERROR)
			{
				values.channel_2 = result.GetADCValue();
				result = readChannel(CHANNEL_3);
				if (result.error == NO_ERROR)
				{
					values.channel_3 = result.GetADCValue();
				}
			}
		}
	}

	return result.error;
}

OPT4048_ErrorCode OPT4048::readXYZ(OPT4048_XYZ& values)
{
	// This is from the datasheet
	float ADC_to_XYZ[4][4] = { { 2.34892992e-4, -1.89652390e-5,  1.20811684e-5,       0 },
							   { 4.07467441e-5,  1.98958202e-4, -1.58848115e-5, 2.15e-3 },
							   { 9.28619404e-5, -1.69739553e-5,  6.74021520e-4,       0 },
							   {             0,              0,              0,       0 }
	};
	OPT4048_ADC adc;
	OPT4048_ErrorCode err = readADC(adc);

	if (err == NO_ERROR)
	{
		values.X   = adc.channel_0 * ADC_to_XYZ[0][0] + adc.channel_1 * ADC_to_XYZ[1][0] + adc.channel_2 * ADC_to_XYZ[2][0] + adc.channel_3 * ADC_to_XYZ[3][0];
		values.Y   = adc.channel_0 * ADC_to_XYZ[0][1] + adc.channel_1 * ADC_to_XYZ[1][1] + adc.channel_2 * ADC_to_XYZ[2][1] + adc.channel_3 * ADC_to_XYZ[3][1];
		values.Z   = adc.channel_0 * ADC_to_XYZ[0][2] + adc.channel_1 * ADC_to_XYZ[1][2] + adc.channel_2 * ADC_to_XYZ[2][2] + adc.channel_3 * ADC_to_XYZ[3][2];
		values.Lux = adc.channel_0 * ADC_to_XYZ[0][3] + adc.channel_1 * ADC_to_XYZ[1][3] + adc.channel_2 * ADC_to_XYZ[2][3] + adc.channel_3 * ADC_to_XYZ[3][3];

	}

	return err;
}

OPT4048_ErrorCode OPT4048::readCIE(OPT4048_CIE& values)
{
	OPT4048_XYZ result;

	OPT4048_ErrorCode err = readXYZ(result);

	if (err == NO_ERROR)
	{
		float denominator = result.X + result.Y + result.Z;
		values.X = result.X / denominator;
		values.Y = result.Y / denominator;
	}

	return err;
}

OPT4048_ErrorCode OPT4048::readLux(float& lux)
{
	OPT4048_RESULT result = readChannel(CHANNEL_1);

	if (result.error == NO_ERROR)
	{
		lux = result.GetADCValue() * 2.15e-3;
	}

	return result.error;

}

OPT4048_ErrorCode OPT4048::readRGB(OPT4048_RGB& values)
{
	// M^-1 for sRGB @ D65 from www.brucelindbloom.com
	float XYZ_to_RGB[3][3] = { {  3.2404542, -1.5371385, -0.4985314 },
							   { -0.9692660,  1.8760108,  0.0415560 },
							   {  0.0556434, -0.2040259,  1.0572252 }
	};
	OPT4048_XYZ xyz;
	OPT4048_ErrorCode err = readXYZ(xyz);

	if (err == NO_ERROR)
	{
		// XYZ of D65 Illuminant
		xyz.X /= 95.0500;
		xyz.Y /= 100.0000;
		xyz.Z /= 108.9000;

		values.R = xyz.X * XYZ_to_RGB[0][0] + xyz.Y * XYZ_to_RGB[0][1] + xyz.Z * XYZ_to_RGB[0][2];
		values.G = xyz.X * XYZ_to_RGB[1][0] + xyz.Y * XYZ_to_RGB[1][1] + xyz.Z * XYZ_to_RGB[1][2];
		values.B = xyz.X * XYZ_to_RGB[2][0] + xyz.Y * XYZ_to_RGB[2][1] + xyz.Z * XYZ_to_RGB[2][2];

		// sRGB Companding function from www.brucelindbloom.com
		if (values.R <= 0.0031308)
			values.R *= 12.92;
		else
			values.R = pow(1.055 * values.R, 1 / 2.4) - 0.055;

		if (values.G <= 0.0031308)
			values.G *= 12.92;
		else
			values.G = pow(1.055 * values.G, 1 / 2.4) - 0.055;

		if (values.B <= 0.0031308)
			values.B *= 12.92;
		else
			values.B = pow(1.055 * values.B, 1 / 2.4) - 0.055;

		values.R = min(1.0, max(0.0, values.R));
		values.G = min(1.0, max(0.0, values.G));
		values.B = min(1.0, max(0.0, values.B));
	}
	return err;
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