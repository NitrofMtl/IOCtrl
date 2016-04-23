/*
  Copyright (c) 28/03/2016

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

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif
#include <ArduinoJson.h>

class RTDinChannels {
	private:

	public:
	  RTDinChannels() {
	    Ainput = 0;
	    offset = 0;
	    RTDSwitch = false;
		}
		void channelSet ( char* name, byte pin, bool switchCh, float inOffset);
		byte AinputPin;
		float Ainput;
		float offset;
		bool RTDSwitch;
		char channelName[20];
		JsonObject& backupInput(JsonBuffer& jsonBuffer);
		void restoreInput(JsonObject& json) ; 
};

class SSRoutput {
	public:

	  	SSRoutput();
	  	void OutChannels(byte ouputPin, float Sp, float Smm, bool ChSwitch);
		void ssrOut(RTDinChannels* input);
		void dutyCycle(unsigned int dutyCycle);
	  	byte Aoutput; //declare struct var
	  	float sp;
	  	bool channelSwitch;
	  	bool permRun;
	  	void smm(float Smm);
		JsonObject& backupSSROutput(JsonBuffer& jsonBuffer);
		void restoreSSROutput(JsonObject& output);
	private:
		float _smm;
	  	byte ssrPin;
		unsigned int _dutyCycle = 10000; //time of the loop
		unsigned int cycleIn = 0;
  		unsigned int cycleOut = 0;
};

class Regulator {
	public:
		Regulator();
		Regulator(byte K, float vs);
		byte regLoop(RTDinChannels* input, SSRoutput* output);
		void gain(byte K);
		void sustain(float vs);
	private:
		byte _K;
		byte _vs;	
	

};




#endif
