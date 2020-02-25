#include "rack.hpp"


using namespace rack;


extern Plugin *pluginInstance;

////////////////////
// module widgets
////////////////////

extern Model *modelAMDecoder;

extern Model *modelAMEncoder;

struct NocturnalWhiteKnob : RoundKnob {
	NocturnalWhiteKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NocturnalWhiteKnob.svg")));
	}
};

