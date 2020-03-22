#include "plugin.hpp"


struct AMDecoder : Module {
	enum ParamIds {
        CH1_ATTACK_PARAM,  // NocturnalWhiteKnob
        CH2_ATTACK_PARAM,  // NocturnalWhiteKnob
        CH1_RELEASE_PARAM,  // NocturnalWhiteKnob
        CH2_RELEASE_PARAM,  // NocturnalWhiteKnob
        CH1_ATTENUVERTER_PARAM,  // NocturnalWhiteKnob
        CH2_ATTENUVERTER_PARAM,  // NocturnalWhiteKnob
        CH1_OFFSET_PARAM,  // NocturnalWhiteKnob
        CH2_OFFSET_PARAM,  // NocturnalWhiteKnob
        CH1_SCALE_PARAM,  // NocturnalWhiteKnob
        CH2_SCALE_PARAM,  // NocturnalWhiteKnob
		NUM_PARAMS
	};
	enum InputIds {
        CH1_SIGNAL_INPUT,  // PJ301MPort
        CH2_SIGNAL_INPUT,  // PJ301MPort
		NUM_INPUTS
	};
	enum OutputIds {
        CH1_CV_OUTPUT,  // PJ301MPort
        CH2_CV_OUTPUT,  // PJ301MPort
        CH1_TRIGGER_OUTPUT,  // PJ301MPort
        CH2_TRIGGER_OUTPUT,  // PJ301MPort
		NUM_OUTPUTS
	};
	enum LightIds {
        CH1_WARN_LIGHT,  // SmallLight<RedLight>
        CH2_WARN_LIGHT,  // SmallLight<RedLight>
        CH1_ACTIVITY_LIGHT,  // SmallLight<GreenLight>
        CH2_ACTIVITY_LIGHT,  // SmallLight<GreenLight>
		NUM_LIGHTS
	};

	float out[2] = {};
	bool gate[2] = {};
	dsp::SchmittTrigger trigger[2];
	dsp::PulseGenerator endOfCyclePulse[2];
	dsp::SchmittTrigger mode_button_trigger;

	AMDecoder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CH1_ATTACK_PARAM, 0.0, 1.0, 0.1, "Attack CH1 envelope follower speed");
		configParam(CH1_RELEASE_PARAM, 0.0, 1.0, 0.3, "Release CH1 envelope follower speed");
		configParam(CH1_ATTENUVERTER_PARAM, -1.0, 1.0, 1.0, "Attenuverter CH1 -1 to +1");
		configParam(CH1_OFFSET_PARAM, -10.0, 10.0, 0.0, "Offset CH1 -10v to +10v");
		configParam(CH1_SCALE_PARAM, 1.0, 3.0, 1.0, "Scale CH1 1x to 3x");
		configParam(CH2_ATTACK_PARAM, 0.0, 1.0, 0.1, "Attack CH2 envelope follower speed");
		configParam(CH2_RELEASE_PARAM, 0.0, 1.0, 0.3, "Release CH2 envelope follower speed");
		configParam(CH2_ATTENUVERTER_PARAM, -1.0, 1.0, 1.0, "Attenuverter CH2 -1 to +1");
		configParam(CH2_OFFSET_PARAM, -10.0, 10.0, 0.0, "Offset CH2 -10v to +10v");
		configParam(CH2_SCALE_PARAM, 1.0, 3.0, 1.0, "Scale CH2 1x to 3x");
	}

	void process(const ProcessArgs& args) override {
		for (int c = 0; c < 2; c++) {
			float in = inputs[CH1_SIGNAL_INPUT + c].getVoltage();
			if (in > 9.99f) {
				lights[CH1_WARN_LIGHT + c].value = 1.0f;	
			}
			else {
				lights[CH1_WARN_LIGHT + c].value = 0.0f;
			}
			float shape = 0.0f;
			float delta = in - out[c];

			// Integrator
			float minTimeRise = paramToMinTime(params[CH1_ATTACK_PARAM + c].getValue()); //Effective range 1e-3 to 1
			float minTimeFall = paramToMinTime(params[CH1_RELEASE_PARAM + c].getValue()); //Effective range 1e-3 to 1

			bool rising = false;
			bool falling = false;

			if (delta > 0) {
				// Rise
				float riseCv = params[CH1_ATTACK_PARAM + c].getValue() / 10.0;
				riseCv = clamp(riseCv, 0.0f, 1.0f);
				float rise = minTimeRise * powf(2.0, riseCv * 10.0);
				out[c] += shapeDelta(delta, rise, shape) / args.sampleRate;
				rising = (in - out[c] > 1e-15); //test values 1e-3,1e-15
				if (!rising) {
					gate[c] = false;
				}
			}
			else if (delta < 0) {
				// Fall
				float fallCv = params[CH1_RELEASE_PARAM + c].getValue() / 10.0;
				fallCv = clamp(fallCv, 0.0f, 1.0f);
				float fall = minTimeFall * powf(2.0, fallCv * 10.0);
				out[c] += shapeDelta(delta, fall, shape) / args.sampleRate;
				falling = (in - out[c] < -1e-3); 
				if (!falling) {
					// End of cycle, check if we should turn the gate back on (cycle mode)
					endOfCyclePulse[c].trigger(1e-15);
				}
			}
			else {
				gate[c] = false;
			}

			if (!rising && !falling) {
				out[c] = in;
			}

			if (inputs[CH1_SIGNAL_INPUT + c].isConnected()) {
				// Clamp at -15V/+15V for safety
				outputs[CH1_CV_OUTPUT + c].setVoltage( clamp((out[c] * params[CH1_ATTENUVERTER_PARAM + c].getValue() * params[CH1_SCALE_PARAM + c].getValue()) + params[CH1_OFFSET_PARAM + c].getValue(), -15.0f, 15.0f));
				if (clamp((out[c] * params[CH1_ATTENUVERTER_PARAM + c].getValue() * params[CH1_SCALE_PARAM + c].getValue()) + params[CH1_OFFSET_PARAM + c].getValue(), -15.0f, 15.0f) >= 2.0f) {
					outputs[CH1_TRIGGER_OUTPUT + c].setVoltage( 10.0f);
				}
				else {
					outputs[CH1_TRIGGER_OUTPUT + c].setVoltage( 0.0f);
				}
			}
			lights[CH1_ACTIVITY_LIGHT + c].value = out[c] / 10.0;

		}  //for (int c = 0; c < 2; c++)
	}

	static float shapeDelta(float delta, float tau, float shape) {
		float lin = sgn(delta) * 10.0f / tau; // * 10
		if (shape < 0.0) {
			float log = sgn(delta) * 40.0f / tau / (fabsf(delta) + 1.0f);
			return crossfade(lin, log, -shape * 0.95f);
		}
		else {
			float exp = M_E * delta / tau;
			return crossfade(lin, exp, shape * 0.90f);
		}
	}

	static float paramToMinTime(float timeInput) {
		timeInput = clamp(timeInput, 0.0f, 1.0f);
		switch ((int) static_cast<int>(timeInput * 5)) { 
			case 0: return 1e-3; break;
			case 1: return crossfade(1e-3, 1e-2, (timeInput * 5.0f) - 1.0f); break;
			case 2: return 1e-2; break;
			case 3: return crossfade(1e-2, 1e-1, (timeInput * 5.0f) - 3.0f); break;
			case 4: return 1e-1; break;
			case 5: return crossfade(1e-1, 1.0f, (timeInput * 5.0f) - 5.0f); break;
			default: return 1.0f; break;
		}
	}

};


struct AMDecoderWidget : ModuleWidget {
	AMDecoderWidget(AMDecoder* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AMDecoder.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<NocturnalWhiteKnob>(Vec(14.1188581306313,89.5001245921275), module, AMDecoder::CH1_ATTACK_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(60.7393632849054,89.5001245921275), module, AMDecoder::CH1_RELEASE_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(37.7493086960543,148.999281247235), module, AMDecoder::CH1_ATTENUVERTER_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(37.7493086960543,205.811125742502), module, AMDecoder::CH1_OFFSET_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(37.7493086960543,262.497754488161), module, AMDecoder::CH1_SCALE_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(174.116838729031,89.5001245921275), module, AMDecoder::CH2_ATTACK_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(220.737349552596,89.5001245921275), module, AMDecoder::CH2_RELEASE_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(197.747291940123,148.999281247235), module, AMDecoder::CH2_ATTENUVERTER_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(197.747291940123,205.811125742502), module, AMDecoder::CH2_OFFSET_PARAM));
		addParam(createParam<NocturnalWhiteKnob>(Vec(197.747291940123,262.497754488161), module, AMDecoder::CH2_SCALE_PARAM));

		addInput(createInput<PJ301MPort>(Vec(42.7493069952669,48.2498954110223), module, AMDecoder::CH1_SIGNAL_INPUT));
		addInput(createInput<PJ301MPort>(Vec(202.747297042486,48.2498954110223), module, AMDecoder::CH2_SIGNAL_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(19.1119398943237,326.249714976374), module, AMDecoder::CH1_CV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(65.7393608282124,326.249714976374), module, AMDecoder::CH1_TRIGGER_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(179.109923516345,326.249714976374), module, AMDecoder::CH2_CV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(225.737354654959,326.249714976374), module, AMDecoder::CH2_TRIGGER_OUTPUT));

		addChild(createLight<SmallLight<RedLight>>(Vec(73.0822610782017,55.2668945545436), module, AMDecoder::CH1_WARN_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(Vec(26.9521125915227,310.388715721433), module, AMDecoder::CH1_ACTIVITY_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(233.080243566365,55.2668945545436), module, AMDecoder::CH2_WARN_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(Vec(186.950096213544,310.388715721433), module, AMDecoder::CH2_ACTIVITY_LIGHT));
	}
};

Model* modelAMDecoder = createModel<AMDecoder, AMDecoderWidget>("AMDecoder");