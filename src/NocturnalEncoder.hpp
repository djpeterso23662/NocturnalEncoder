#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

extern Model *modelAMDecoder;

extern Model *modelAMEncoder;

struct NocturnalWhiteKnob : RoundKnob {
	NocturnalWhiteKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/NocturnalWhiteKnob.svg")));
	}

	void onChange(EventChange &e) override {
		RoundKnob::onChange(e);
	}

	virtual std::string formatCurrentValue() {
		return std::to_string(static_cast<unsigned int>(value));
	}
};

