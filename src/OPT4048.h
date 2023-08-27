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

#ifndef OPT4048_H
#define OPT4048_H

#include <Arduino.h>

typedef enum {
	RESULT_CH0_1		= 0x00,
	RESULT_CH0_2		= 0x01,
	RESULT_CH1_1		= 0x02,
	RESULT_CH1_2		= 0x03,
	RESULT_CH2_1		= 0x04,
	RESULT_CH2_2		= 0x05,
	RESULT_CH3_1		= 0x06,
	RESULT_CH3_2		= 0x07,
	THRESHOLD_LOW		= 0x08,
	THRESHOLD_HIGH		= 0x09,
	CONFIG_A			= 0x0A,
	CONFIG_B			= 0x0B,
	DEVICE_ID			= 0x11,
} OPT4048_Commands;


typedef enum {
	NO_ERROR = 0,
	TIMEOUT_ERROR = -100,

	// Wire I2C translated error codes
	WIRE_I2C_DATA_TOO_LOG = -10,
	WIRE_I2C_RECEIVED_NACK_ON_ADDRESS = -20,
	WIRE_I2C_RECEIVED_NACK_ON_DATA = -30,
	WIRE_I2C_UNKNOW_ERROR = -40
} OPT4048_ErrorCode;

typedef enum {
	CHANNEL_0 = 0x00,
	CHANNEL_1 = 0x02,
	CHANNEL_2 = 0x04,
	CHANNEL_3 = 0x06
} OPT4048_Channel;

typedef union {
	struct __attribute__((__packed__)) {
		uint8_t FaultCount : 2;
		uint8_t Polarity : 1;
		uint8_t Latch : 1;
		uint8_t OpMode : 2;
		uint8_t ConversionTime : 4;
		uint8_t RangeNumber : 4;
		uint8_t FixedZero : 1;
		uint8_t QuickWake : 1;
	};
	uint16_t rawData;
} OPT4048_ConfigA;

typedef union {
	struct __attribute__((__packed__)) {
		uint8_t I2CBurst : 1;
		uint8_t FixedZero : 1;
		uint8_t IntConfig : 2;
		uint8_t IntDirection : 1;
		uint8_t ThresholdChannel : 2;
		uint8_t RangeNumber : 4;
		uint16_t Fixed128 : 9;
	};
	uint16_t rawData;
} OPT4048_ConfigB;

typedef union {
	uint16_t rawData;
	struct __attribute__((__packed__)) {
		uint32_t Mantissa : 20;
		uint8_t Exponent : 4;
	};
} OPT4048_ER20;

typedef union {
	uint16_t rawData;
	struct __attribute__((__packed__)) {
		uint32_t Mantissa : 12;
		uint8_t Exponent : 4;
	};
} OPT4048_ER12;

struct OPT4048_RESULT {
	OPT4048_ER20 rawResult;
	OPT4048_ErrorCode error;
};

struct OPT4048_THRESHOLD {
	OPT4048_ER12 rawResult;
	OPT4048_ErrorCode error;
};

struct OPT4048_REGISTER {
	uint16_t value;
	OPT4048_ErrorCode error;
};

class OPT4048 {
public:
	OPT4048();

	OPT4048_ErrorCode begin(uint8_t address);

	uint16_t readDeviceID();

	OPT4048_RESULT readResult(OPT4048_Channel channel);
	OPT4048_THRESHOLD readHighLimit();
	OPT4048_ErrorCode writeHighLimit(OPT4048_ER12 threshold);
	OPT4048_THRESHOLD readLowLimit();
	OPT4048_ErrorCode writeLowLimit(OPT4048_ER12 threshold);
	
	OPT4048_ConfigA readConfigA();
	OPT4048_ErrorCode writeConfigA(OPT4048_ConfigA config);

	OPT4048_ConfigB readConfigB();
	OPT4048_ErrorCode writeConfigB(OPT4048_ConfigB config);

private:
	uint8_t _address;

	OPT4048_ErrorCode writeData(OPT4048_Commands command);
	OPT4048_ErrorCode readData(uint16_t* data);

	OPT4048_REGISTER readRegister(OPT4048_Commands command);
	OPT4048_THRESHOLD readThresholdRegister(OPT4048_Commands command);
	OPT4048_RESULT returnResultError(OPT4048_ErrorCode error);
	OPT4048_THRESHOLD returnThresholdError(OPT4048_ErrorCode error);

};


#endif 