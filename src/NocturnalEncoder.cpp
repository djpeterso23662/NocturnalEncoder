#include "NocturnalEncoder.hpp"


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;
	p->addModel(modelAMDecoder);
	p->addModel(modelAMEncoder);
}
