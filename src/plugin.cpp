#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
        p->addModel( modelClockkky );
        p->addModel( modelPlanetz );
        p->addModel( modelMusiFrog );
        p->addModel( modelZefiro );
        p->addModel( modelHoldMeTight );
        p->addModel( modelCyclicCA );
        p->addModel( modelMusiMath );
        
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
