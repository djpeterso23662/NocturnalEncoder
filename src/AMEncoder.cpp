#include "NocturnalEncoder.hpp"


struct AMEncoder : Module {
    enum ParamIds {
        CARRIER_LEVEL_PARAM,  // NocturnalWhiteKnob
        NUM_PARAMS
    };
    enum InputIds {
        CH1_CV_INPUT,  // PJ301MPort
        CH2_CV_INPUT,  // PJ301MPort
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_SIGNAL_OUTPUT,  // PJ301MPort
        CH2_SIGNAL_OUTPUT,  // PJ301MPort
        NUM_OUTPUTS
    };
    enum LightIds {
        CH1_WARN_LIGHT,  // SmallLight<RedLight>
        CH2_WARN_LIGHT,  // SmallLight<RedLight>
        NUM_LIGHTS
    };

	float phase = 0.0;
	float carrier = 0.0;

    AMEncoder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(AMEncoder::CARRIER_LEVEL_PARAM, 0.0, 1.0, 1.0, "");
	}
    void process(const ProcessArgs& args) override;
};

static void stepChannel(float in, Input &lin, Output &out) {
	float v = in;
	if (lin.active)
		v *= clamp(lin.value / 10.0f, 0.0f, 1.0f);
	out.value = v;
}

void AMEncoder::process(const ProcessArgs& args) {

	// Implement a simple sine oscillator for the carrier
	float deltaTime = args.sampleTime;

	// Compute the frequency from the pitch parameter and input
	float pitch = 0.0f;
	pitch = clamp(pitch, -4.0f, 4.0f); 
	// The carrier pitch is around C9 - C#9
	float freq = 8697.36f * powf(2.0f, pitch);

	// Accumulate the phase
	phase += freq * deltaTime;
	if (phase >= 1.0f)
		phase -= 1.0f;

	// Compute the carrier output
	float sine = sinf(2.0f * M_PI * phase);
	carrier = 9.99f * sine * params[CARRIER_LEVEL_PARAM].getValue(); // carrier amplitude 0 - 10.0

	stepChannel(carrier, inputs[CH1_CV_INPUT], outputs[CH1_SIGNAL_OUTPUT]);
	stepChannel(carrier, inputs[CH2_CV_INPUT], outputs[CH2_SIGNAL_OUTPUT]);

	lights[CH1_WARN_LIGHT].value = clamp(inputs[CH1_CV_INPUT].getVoltage() / 10.0f * -1.0f, 0.0f, 1.0f);
	lights[CH2_WARN_LIGHT].value = clamp(inputs[CH2_CV_INPUT].getVoltage() / 10.0f * -1.0f, 0.0f, 1.0f);
}


struct AMEncoderWidget : ModuleWidget {
	AMEncoderWidget(AMEncoder *module) {
		setModule(module);
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);  // RACK_GRID_WIDTH = 15px, RACK_GRID_HEIGHT = 380px

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AMEncoder.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		addParam(createParam<NocturnalWhiteKnob>(Vec(12.25,316.125), module, AMEncoder::CARRIER_LEVEL_PARAM));

		addInput(createInput<PJ301MPort>(Vec(17.125,47.625), module, AMEncoder::CH1_CV_INPUT));
		addInput(createInput<PJ301MPort>(Vec(17.125,93.875), module, AMEncoder::CH2_CV_INPUT));


		addOutput(createOutput<PJ301MPort>(Vec(17.125,218.875), module, AMEncoder::CH1_SIGNAL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(17.125,265.25), module, AMEncoder::CH2_SIGNAL_OUTPUT));

		addChild(createLight<SmallLight<RedLight>>(Vec(47.501,56.226), module, AMEncoder::CH1_WARN_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(47.501,102.467), module, AMEncoder::CH2_WARN_LIGHT));
	}
};

Model *modelAMEncoder = createModel<AMEncoder, AMEncoderWidget>("AMEncoder");
