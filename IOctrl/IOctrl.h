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
		char* channelName;
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