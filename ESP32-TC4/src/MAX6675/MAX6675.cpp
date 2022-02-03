#include "MAX6675.h"

MAX6675::MAX6675(int16_t SSPin)
{
	_SSPin = SSPin;
	pinMode(_SSPin, OUTPUT);
	digitalWrite(_SSPin, HIGH);
	SPI.begin();
	_lastCallTime = 0;
}

float MAX6675::readTempC()
{
	if (millis() - _lastCallTime >= MAX6675_READ_PERIOD)
	{
		SPI.beginTransaction(MAX6675_Settings);
		digitalWrite(_SSPin, LOW);
		_incomingMessage = SPI.transfer16(0x00);		
		digitalWrite(_SSPin, HIGH);
		SPI.endTransaction();
		_lastCallTime = millis();
		
		if (_incomingMessage & MAX6675_THERMOCOUPLE_OPEN_BIT)
			return MAX6675_THERMOCOUPLE_OPEN;
		_currentTemp = (_incomingMessage >> 4) * MAX6675_CONVERSION_RATIO;

		return _currentTemp;
	}
	return _currentTemp;
}

float MAX6675::readTempF()
{
	return readTempC() * 9.0/5.0 + 32;
}