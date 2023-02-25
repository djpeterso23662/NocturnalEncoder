#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelAMDecoder;
extern Model* modelAMEncoder;

struct NocturnalWhiteKnob : RoundKnob {
	NocturnalWhiteKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NocturnalWhiteKnob.svg")));
	}

	void onChange(const event::Change &e) override {
		RoundKnob::onChange(e);
	}

	virtual std::string formatCurrentValue() {
		ParamQuantity* paramQuantity = getParamQuantity();
		if(paramQuantity != NULL){
			return std::to_string(static_cast<unsigned int>(paramQuantity->getValue()));
		}
		return "";
	}
};
