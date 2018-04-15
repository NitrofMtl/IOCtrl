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

void RTDinChannels::channelSet ( char* name, byte pin, float inOffset) { //setup temp input
    strcpy(channelName, name);
    AinputPin = pin;
    Ainput = 0;
    offset = inOffset;
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
	extern byte K; 
	extern float vs;
	_K = K;
	_vs = vs;
}

Regulator::Regulator(byte K, float vs) : _K(K), _vs(vs) {
}

void Regulator::gain(byte K){
_K = K;
}

void Regulator::sustain(float vs){
	_vs = vs;
}

unsigned int Regulator::regLoop(RTDinChannels* input, SSRoutput* output){
	long Aoutput = 0; //output regulation variable in ,0%	
	int er = floatToFixed(output->sp) - floatToFixed(input->Ainput); // calculate the error of process
	Aoutput = (er + floatToFixed(_vs)) * _K; //calculate the output power
	Aoutput = constrain(fixedToInt(Aoutput), 0, 100); //constrain the output in 0-100 % range, 0,1 step
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

void SSRoutput::toggleSwitch() {
	channelSwitch=!channelSwitch;
	if (!channelSwitch) {
		digitalWrite(ssrPin, LOW);
		timeCounter = 0;
		state = false;
	}
}

void SSRoutput::ssrOut(RTDinChannels* input){
	if (!channelSwitch) {
		state = false;
		return;
	}

	Regulator reg;
	extern float vs;

	//run regulator on each check only if output is not already on
	if (!permRun) {
		if (timeCounter < _dutyCycle) {//check output only 10sec interval
			timeCounter++;
			return; 
		}
		timeCounter = 0;
		Aoutput = reg.regLoop(input, this);
		if (Aoutput < _smm) {
			return; //return if treshold is not enough
		}
		
		permRun = true;
		digitalWrite(ssrPin, HIGH);
		cycleIn = Aoutput; //set duty cycle				
		return;
	}

	//if cycle in is 100% restart counter at the end of dutycycle
	if ( (cycleIn == timeCounter) && (timeCounter == _dutyCycle) ) { 
		timeCounter = 0;
		Aoutput = reg.regLoop(input, this);//recalculate needs of output
		cycleIn = Aoutput; //Calculate the time on
		return;
	} 

	if (cycleIn == timeCounter) {	
		digitalWrite(ssrPin, LOW);
		timeCounter++;
		return;
	}

	if (timeCounter == _dutyCycle) {
		timeCounter = 0;
		if (input->Ainput >= (sp + vs)){
			permRun = false; //rester permission of work if error getting to small
			digitalWrite(ssrPin, LOW);
			return;
		}
		Aoutput = reg.regLoop(input, this);//recalculate needs of output
		cycleIn = Aoutput; //Calculate the time on
		digitalWrite(ssrPin, HIGH);
		return;
	}
	timeCounter++;// if no condition incounter, increase counter
}