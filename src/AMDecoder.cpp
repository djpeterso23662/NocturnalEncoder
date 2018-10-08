#include "NocturnalEncoder.hpp"
#include "dsp/digital.hpp"


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
	SchmittTrigger trigger[2]; 
	PulseGenerator endOfCyclePulse[2]; 

    AMDecoder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

};

static float shapeDelta(float delta, float tau, float shape) {
	float lin = sgn(delta) * 10.0f / tau; 
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

void AMDecoder::step() {

	for (int c = 0; c < 2; c++) {  
		float in = inputs[CH1_SIGNAL_INPUT + c].value;
		if (in > 9.99f) {
			lights[CH1_WARN_LIGHT + c].value = 1.0f;	
		}
		else {
			lights[CH1_WARN_LIGHT + c].value = 0.0f;
		}
		float shape = 0.0f;
		float delta = in - out[c];

		// Integrator
		float minTimeRise = paramToMinTime(params[CH1_ATTACK_PARAM + c].value); //Effective range 1e-3 to 1
		float minTimeFall = paramToMinTime(params[CH1_RELEASE_PARAM + c].value); //Effective range 1e-3 to 1

		bool rising = false;
		bool falling = false;

		if (delta > 0) {
			// Rise
			float riseCv = params[CH1_ATTACK_PARAM + c].value / 10.0;
			riseCv = clamp(riseCv, 0.0f, 1.0f);
			float rise = minTimeRise * powf(2.0, riseCv * 10.0);
			out[c] += shapeDelta(delta, rise, shape) / engineGetSampleRate();
			rising = (in - out[c] > 1e-15); //test values 1e-3,1e-15
			if (!rising) {
				gate[c] = false;
			}
		}
		else if (delta < 0) {
			// Fall
			float fallCv = params[CH1_RELEASE_PARAM + c].value / 10.0;
			fallCv = clamp(fallCv, 0.0f, 1.0f);
			float fall = minTimeFall * powf(2.0, fallCv * 10.0);
			out[c] += shapeDelta(delta, fall, shape) / engineGetSampleRate();
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

		if (inputs[CH1_SIGNAL_INPUT + c].active) {
			// Clamp at -15V/+15V for safety
			outputs[CH1_CV_OUTPUT + c].value =  clamp((out[c] * params[CH1_ATTENUVERTER_PARAM + c].value * params[CH1_SCALE_PARAM + c].value) + params[CH1_OFFSET_PARAM + c].value, -15.0f, 15.0f);
			if (clamp((out[c] * params[CH1_ATTENUVERTER_PARAM + c].value * params[CH1_SCALE_PARAM + c].value) + params[CH1_OFFSET_PARAM + c].value, -15.0f, 15.0f) >= 2.0f) {
				outputs[CH1_TRIGGER_OUTPUT + c].value =  10.0f;
			}
			else {
				outputs[CH1_TRIGGER_OUTPUT + c].value =  0.0f;
			}
		}
		lights[CH1_ACTIVITY_LIGHT + c].value = out[c] / 10.0;

	}  //for (int c = 0; c < 2; c++)
}

struct AMDecoderWidget : ModuleWidget {
	AMDecoderWidget(AMDecoder *module) : ModuleWidget(module) {
		box.size = Vec(18 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);  // RACK_GRID_WIDTH = 15px, RACK_GRID_HEIGHT = 380px

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/AMDecoder.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(14.1188581306313,89.5001245921275), module, AMDecoder::CH1_ATTACK_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(60.7393632849054,89.5001245921275), module, AMDecoder::CH1_RELEASE_PARAM, 0.0, 1.0, 0.3));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(37.7493086960543,148.999281247235), module, AMDecoder::CH1_ATTENUVERTER_PARAM, -1.0, 1.0, 1.0));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(37.7493086960543,205.811125742502), module, AMDecoder::CH1_OFFSET_PARAM, -10.0, 10.0, 0.0));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(37.7493086960543,262.497754488161), module, AMDecoder::CH1_SCALE_PARAM, 1.0, 3.0, 1.0));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(174.116838729031,89.5001245921275), module, AMDecoder::CH2_ATTACK_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(220.737349552596,89.5001245921275), module, AMDecoder::CH2_RELEASE_PARAM, 0.0, 1.0, 0.3));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(197.747291940123,148.999281247235), module, AMDecoder::CH2_ATTENUVERTER_PARAM, -1.0, 1.0, 1.0));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(197.747291940123,205.811125742502), module, AMDecoder::CH2_OFFSET_PARAM, -10.0, 10.0, 0.0));
		addParam(ParamWidget::create<NocturnalWhiteKnob>(Vec(197.747291940123,262.497754488161), module, AMDecoder::CH2_SCALE_PARAM, 1.0, 3.0, 1.0));

		addInput(Port::create<PJ301MPort>(Vec(42.7493069952669,48.2498954110223), Port::INPUT, module, AMDecoder::CH1_SIGNAL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(202.747297042486,48.2498954110223), Port::INPUT, module, AMDecoder::CH2_SIGNAL_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(19.1119398943237,326.249714976374), Port::OUTPUT, module, AMDecoder::CH1_CV_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(65.7393608282124,326.249714976374), Port::OUTPUT, module, AMDecoder::CH1_TRIGGER_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(179.109923516345,326.249714976374), Port::OUTPUT, module, AMDecoder::CH2_CV_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(225.737354654959,326.249714976374), Port::OUTPUT, module, AMDecoder::CH2_TRIGGER_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(73.0822610782017,55.2668945545436), module, AMDecoder::CH1_WARN_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(26.9521125915227,310.388715721433), module, AMDecoder::CH1_ACTIVITY_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(233.080243566365,55.2668945545436), module, AMDecoder::CH2_WARN_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(186.950096213544,310.388715721433), module, AMDecoder::CH2_ACTIVITY_LIGHT));
	}
};

Model *modelAMDecoder = Model::create<AMDecoder, AMDecoderWidget>("Nocturnal Encoder", "AMDecoder", "NE-1 Decoder", ATTENUATOR_TAG, SLEW_LIMITER_TAG, ENVELOPE_FOLLOWER_TAG, DUAL_TAG);
