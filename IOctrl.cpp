#include "IOctrl.h"

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

unsigned int Regulator::regLoop(RTDinChannels & input, SSRoutput & output){
	long Aoutput = 0; //output regulation variable in ,0%	
	int er = floatToFixed(output.sp) - floatToFixed(input.Ainput); // calculate the error of process
	Aoutput = (er + floatToFixed(_vs)) * _K; //calculate the output power
	Aoutput = constrain(fixedToInt(Aoutput), 0, 100); //constrain the output in 0-100 % range
	return Aoutput;
}

SSRoutput::SSRoutput() : //struct initiation function
	Aoutput(0),
	sp(5),
	channelSwitch(false),
	permRun(false),
	_smm(0)
{}

void SSRoutput::OutChannels(byte ouputPin, float Sp, float Smm, bool ChSwitch) { //initiate struct
    ssrPin = ouputPin;
    sp = Sp;
    channelSwitch = ChSwitch;
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
	}
}

void SSRoutput::ssrOut(RTDinChannels & input){
	//the function will be call each time the input reading will be refresh
	if (!channelSwitch) {
		permRun = false;
		Aoutput = 0;
		return;
	}

	extern float vs;
	if (input.Ainput >= (sp + vs)){ //exit if temp is already ti high
		permRun = false;
		Aoutput = 0; 			
		return;
	}

	Regulator reg;
	Aoutput = reg.regLoop(input, *this);
	if (Aoutput < _smm) {
		permRun = false;
		Aoutput = 0;
		return; //return if treshold is not enough
	}

	if ( !permRun ) {
		permRun = true;	
		TO_callbackCaller(); //start the time loop
	}	
}

void SSRoutput::TO_callbackCaller() { //set ouput on and of by Aoutput versus dutyCycle, and it on timer
	if ( !state ) {
		digitalWrite(ssrPin, HIGH);
		state = true;
		uint16_t timeOn = _dutyCycle * Aoutput / 100; //time that the pin stay on
		timeOut(timeOn, NULL);
		return;
	}
	digitalWrite(ssrPin, LOW);
	state = false;
	if ( !permRun ) return; //break the time loop if no permission
	uint16_t timeOff = _dutyCycle - ( _dutyCycle * Aoutput / 100 );//time that the pin stay off
	timeOut(timeOff, NULL);
}
