/*
V2.0

  Copyright (c) 15/04/2018

    By Nitrof

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/  



#ifndef IOctrl_H
#define IOctrl_H

#include "Arduino.h"

#include <ArduinoJson.h>
#include <TimeOut.h>

const int bitScale = 12;
#define floatToFixed(x)(x*(long)(1<<bitScale))
#define fixedToInt(x)(x/(1<<bitScale))

class RTDinChannels {
	private:

	public:
	  RTDinChannels() {
	    Ainput = 0;
	    offset = 0;
		}
		void channelSet ( char* name, byte pin, float inOffset = 0);
		byte AinputPin;
		float Ainput;
		int offset = 0;
		char channelName[20];
		JsonObject& backupInput(JsonBuffer& jsonBuffer);
		void restoreInput(JsonObject& json) ; 
};

class SSRoutput : public TimeOut {
	public:

	  	SSRoutput();
	  	void OutChannels(byte ouputPin, float Sp, float Smm, bool ChSwitch);
		void ssrOut(RTDinChannels & input);
		void dutyCycle(unsigned int dutyCycle);
	  	unsigned long Aoutput = 0;
	  	float sp;
	  	bool channelSwitch;
	  	bool permRun = false;
	  	void smm(float Smm);
		JsonObject& backupSSROutput(JsonBuffer& jsonBuffer);
		void restoreSSROutput(JsonObject& output);
		void toggleSwitch();
	private:
		void TO_callbackCaller();
		float _smm;
	  	byte ssrPin;
		unsigned int _dutyCycle = 10000; //time of the loop in ms
  		bool state = false;
};

class Regulator {
	public:
		Regulator();
		Regulator(byte K, float vs);
		unsigned int regLoop(RTDinChannels & input, SSRoutput & output);
		void gain(byte K);
		void sustain(float vs);
	private:
		byte _K;
		float _vs;	
};

#endif