#include "NocturnalEncoder.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "NocturnalEncoder";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/djpeterso23662/nocturnalencoder";

	p->addModel(modelAMDecoder);
	p->addModel(modelAMEncoder);
}
