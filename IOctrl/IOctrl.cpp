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

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include "IOctrl.h"
#include <ArduinoJson.h>

void RTDinChannels::channelSet ( char* name, byte pin, bool switchCh, float inOffset) { //setup temp input
    strcpy(channelName, name);
    AinputPin = pin;
    Ainput = 0;
    offset = inOffset;
    RTDSwitch = switchCh;
  }

JsonObject& RTDinChannels::backupInput(JsonBuffer& jsonBuffer){
	JsonObject& input = jsonBuffer.createObject();
	input["name"] = channelName;
	input["offset"] = offset;
	return input;
}

void RTDinChannels::restoreInput(JsonObject& input){
	offset = input["offset"];
	strcpy(channelName, input["name"]);
}

Regulator::Regulator(){
	extern byte K; //make variable accible outside lib
	extern float vs;
	_K = K;
	_vs = vs;
}

Regulator::Regulator(byte K, float vs){
	_K = K;
	_vs = vs;
}

void Regulator::gain(byte K){
_K = K;;
}

void Regulator::sustain(float vs){
	_vs = vs;
}


byte Regulator::regLoop(RTDinChannels* input, SSRoutput* output){
 	Regulator();
	int Aoutput = 0; //output regulation variable in %
	if (output->channelSwitch) {
    	float er = output->sp - input->Ainput; // calculate the error of process
    	Aoutput = (er + _vs) * _K; //calculate the output power
    	Aoutput = constrain(Aoutput, 0, 100); //constrain the output in 0-100 % range
	} else Aoutput = 0;
	return Aoutput;
}

SSRoutput::SSRoutput() { //struct initiation function
	Aoutput = 0;
	sp = 5;
	channelSwitch = false;
	permRun = false;
	_smm = 0;
}

void SSRoutput::OutChannels(byte ouputPin, float Sp, float Smm, bool ChSwitch) { //initiate struct
    ssrPin = ouputPin;
    sp = Sp;
    channelSwitch = ChSwitch;
    Aoutput = 0;
    permRun = false;
    pinMode(ssrPin, OUTPUT);
    _smm = Smm;
}

JsonObject& SSRoutput::backupSSROutput(JsonBuffer& jsonBuffer){
	JsonObject& output = jsonBuffer.createObject();
	output["setPoint"] = sp;
	output["status"] = channelSwitch;
	return output;
}

void SSRoutput::restoreSSROutput(JsonObject& output){
	sp = output["setPoint"];
	channelSwitch = output["status"];
}

void SSRoutput::dutyCycle(unsigned int dutyCycle){
	_dutyCycle = dutyCycle;
}

void SSRoutput::smm(float Smm){
	_smm = Smm;
}

void SSRoutput::ssrOut(RTDinChannels* input){
	Regulator reg;
	if (input->RTDSwitch) Aoutput = reg.regLoop(input, this);
	else Aoutput = 0;

	int cycleOn = (_dutyCycle / 100 * Aoutput); //calculate the time off
	int cycleOff = (_dutyCycle - cycleOn);        //calculate the time on
	
	if(!channelSwitch | !input->RTDSwitch) permRun = false;

	if (input->RTDSwitch && (Aoutput >= _smm)) { //check if the error is bigger then the treshold, if yes set permisssion to true
	permRun = true;
	cycleIn = millis() + cycleOn;
	}
	extern float vs;
	if (permRun && (input->Ainput >= (sp + vs))) { //check if the error is enough small, if yes set permisssion to false
	permRun = false;
	}

	if (permRun) { //output running if permisssion true
	if ((ssrPin != HIGH) && (millis() > cycleOut)){
		digitalWrite(ssrPin, HIGH);
		cycleOut = millis() + cycleOff;
		}
	else if ((ssrPin != LOW) && (millis() > cycleIn)){
		digitalWrite(ssrPin, LOW);
		cycleIn = millis() + cycleOn;
		}
	} else {
	digitalWrite(ssrPin, LOW); //non running loop
	cycleIn = 0;
	cycleOut = 0;
	}
}

