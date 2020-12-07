#include "plugin.hpp"

#define MINRAY         1
#define MAXRAY        30
#define MINSPEED    -180
#define MAXSPEED     180
#define MINALPHA    -180
#define MAXALPHA     180
#define MAXPLANETS     8
#define MINSEL         1
#define MAXSEL         5

#define MINSCALE   0
#define MAXSCALE   3

#define PARPLANETS 5 

#define PALPHA0    0
#define PRAY       1
#define PSPEED     2
#define PCX        3
#define PCY        4
#define PALPHA     5
# define MY_PI           3.14159265358979323846 

#define DEFSEL1    3
#define DEFSEL2    5

#define noUSEDBG   1

struct Planet {
    float alpha0;
    float ray;
    float speed;
    float cx;
    float cy;
    float alpha;
};
typedef struct Planet TPlanet;


struct Planetz : Module {
	enum ParamIds {
#ifdef USEDBG            
		DBG_PARAM,
#endif                
		P1ALPHA_PARAM,
		P1RAY_PARAM,
		P1SPEED_PARAM,
		P2ALPHA_PARAM,
		P2RAY_PARAM,
		P2SPEED_PARAM,
		P3ALPHA_PARAM,
		P3RAY_PARAM,
		P3SPEED_PARAM,
		P4ALPHA_PARAM,
		P4RAY_PARAM,
		P4SPEED_PARAM,
		P5ALPHA_PARAM,
		P5RAY_PARAM,
		P5SPEED_PARAM,
		RST_PARAM,
		SCALEX_PARAM,
		SCALEY_PARAM,
		SELP1_PARAM,
		SELP2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTP1X_OUTPUT,
		OUTP1Y_OUTPUT,
		OUTP2X_OUTPUT,
		OUTP2Y_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
        
        dsp::SchmittTrigger dbgTrigger;
        dsp::SchmittTrigger rstTrigger;
        
        TPlanet planets[MAXPLANETS] = {};
        int numplanets = 6; // 6 including the central "star"
        int p1, p2;
        int currstep;
        int animevery = 441 ;
        float slowdown = 1.0 / 100.0;
        bool firstrun = true;
        float scalex = 1;
        float scaley = 1;
        
        int irand( int imin, int imax) {
            return rand() % (imax - imin + 1) + imin;
        }
        
        /*
        void onRandomize() override {
            
            for ( int i = 1; i < numplanets; i++ ) {
                params[ P1RAY_PARAM + (i-1) * 3 ].setValue( irand(MINRAY,MAXRAY) );
                params[ P1ALPHA_PARAM + (i-1) * 3 ].setValue( irand(MINALPHA,MAXALPHA) );
                params[ P1SPEED_PARAM + (i-1) * 3 ].setValue( irand(MINSPEED,MAXSPEED) );                
            }
            params[ SELP1_PARAM ].setValue( p1 );
            params[ SELP2_PARAM ].setValue( p2 );
            params[ SCALEX_PARAM ].setValue( scalex );
            params[ SCALEY_PARAM ].setValue( scaley );
            update_planets( true );
        } 
        */         
        
        void dbg() {
            INFO ( "p1=%d p2=%d currstep=%d", p1, p2, currstep );
            for (int i = 0; i <numplanets; i++ ) {
                INFO ( "planet %d: alpha0=%f ray=%f speed=%f pos=(%f,%f) alpha=%f", i, planets[i].alpha0, planets[i].ray, planets[i].speed,
                        planets[i].cx, planets[i].cy, planets[i].alpha );
                
            }
        }
        
        /**
         * RST it!
         */
        void rst() {
            currstep = 0;
            update_planets( true );
        }
        
        /**
         * Update planets from parameters
         */
        void update_planets( bool rstalpha ) {
            for (int i = 0; i < numplanets; i++) {
                
                if ( i == 0) {
                    planets[i].cx = 0;
                    planets[i].cy = 0;
                    planets[i].alpha = 0;
                    planets[i].speed = 0;
                } else {
                    planets[i].ray = params[ P1RAY_PARAM + (i-1) * 3 ].getValue();
                    planets[i].alpha0 = params[ P1ALPHA_PARAM + (i-1) * 3 ].getValue();
                    if ( rstalpha) planets[i].alpha = planets[i].alpha0;
                    planets[i].speed = params[ P1SPEED_PARAM + (i-1) * 3 ].getValue();
                    
                    planets[i].cx = planets[i-1].cx + planets[i].ray * std::cos( planets[i].alpha * MY_PI / 180.0 );
                    planets[i].cy = planets[i-1].cy + planets[i].ray * std::sin( planets[i].alpha * MY_PI / 180.0 );
                }
                
            }
            p1 = ( (int)params[ SELP1_PARAM ].getValue() ) % numplanets;
            p2 = ( (int)params[ SELP2_PARAM ].getValue() ) % numplanets;            
        }
        
        /**
         * Move the planets
         */
        void anim_planets() {
            for (int i = 0; i < numplanets; i++) {
                planets[i].alpha += planets[i].speed * slowdown;                
            }
        }
        

	Planetz() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
#ifdef USEDBG                
		configParam(DBG_PARAM, 0.f, 1.f, 0.f, "Debug");
#endif                
		configParam(P1ALPHA_PARAM, MINALPHA, MAXALPHA, 0.f, "Planet 1 angle");
		configParam(P1RAY_PARAM, MINRAY, MAXRAY, 16.f, "Planet 1 ray");
		configParam(P1SPEED_PARAM, MINSPEED, MAXSPEED, 10.f, "Planet 1 speed");
                
		configParam(P2ALPHA_PARAM, MINALPHA, MAXALPHA, 30.f, "Planet 2 angle");
		configParam(P2RAY_PARAM, MINRAY, MAXRAY, 8.f, "Planet 2 ray");
		configParam(P2SPEED_PARAM, MINSPEED, MAXSPEED, -20.f, "Planet 2 speed");
                
		configParam(P3ALPHA_PARAM, MINALPHA, MAXALPHA, 90.f, "Planet 3 angle");
		configParam(P3RAY_PARAM, MINRAY, MAXRAY, 4.f, "Planet 3 ray");
		configParam(P3SPEED_PARAM, MINSPEED, MAXSPEED, 15.f, "Planet 3 speed");
                
		configParam(P4ALPHA_PARAM, MINALPHA, MAXALPHA, 60.f, "Planet 4 angle");
		configParam(P4RAY_PARAM, MINRAY, MAXRAY, 2.f, "Planet 4 ray");
		configParam(P4SPEED_PARAM, MINSPEED, MAXSPEED, 20.f, "Planet 4 speed");
                
		configParam(P5ALPHA_PARAM, MINALPHA, MAXALPHA, -60.f, "Planet 5 angle");
		configParam(P5RAY_PARAM, MINRAY, MAXRAY, 10.f, "Planet 5 ray");
		configParam(P5SPEED_PARAM, MINSPEED, MAXSPEED, -5.f, "Planet 5 speed");
		
                configParam(RST_PARAM, 0.f, 1.f, 0.f, "Reset");
                
		configParam(SCALEX_PARAM, MINSCALE, MAXSCALE, 1.f, "X scale");
		configParam(SCALEY_PARAM, MINSCALE, MAXSCALE, 1.f, "Y scale");
                
		configParam(SELP1_PARAM, MINSEL, MAXSEL, DEFSEL1, "Out planet 1");
		configParam(SELP2_PARAM, MINSEL, MAXSEL, DEFSEL2, "Out planet 2");
                
                update_planets( true );
	}

        
        
	void process(const ProcessArgs& args) override {
                      
            animevery = (int)(1.0 * args.sampleRate / 100);
            
            if (rstTrigger.process(params[RST_PARAM].getValue() + inputs[RST_INPUT].getVoltage() )) {
                rst();
            }
            
            if ( firstrun ) {
                firstrun = false;
                update_planets( true );
            }
            
            if ( currstep >= animevery  ) {
                currstep = 0;
                anim_planets();
                update_planets(false);
            }
            currstep++;

            p1 = ( (int)params[ SELP1_PARAM ].getValue() ) % numplanets;
            p2 = ( (int)params[ SELP2_PARAM ].getValue() ) % numplanets;
            
            scalex = params[ SCALEX_PARAM ].getValue();
            scaley = params[ SCALEY_PARAM ].getValue();
            
            outputs[ OUTP1X_OUTPUT ].setVoltage( scalex * planets[p1].cx / 10.0 );
            outputs[ OUTP1Y_OUTPUT ].setVoltage( scaley * planets[p1].cy / 10.0 );
            outputs[ OUTP2X_OUTPUT ].setVoltage( scalex * planets[p2].cx / 10.0 );
            outputs[ OUTP2Y_OUTPUT ].setVoltage( scaley * planets[p2].cy / 10.0 );
#ifdef USEDBG            
            // Dbg
            if (dbgTrigger.process(params[DBG_PARAM].getValue())) {
                dbg();
            }
#endif            
	}

};


struct PlanetzWidget : ModuleWidget {
	PlanetzWidget(Planetz* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Planetz.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
#ifdef USEDBG
		addParam(createParamCentered<LEDButton>(mm2px(Vec(17.908, 7.728)), module, Planetz::DBG_PARAM));
#endif
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 32.0)), module, Planetz::P1ALPHA_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 32.0)), module, Planetz::P1RAY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(27.0, 32.0)), module, Planetz::P1SPEED_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 44.0)), module, Planetz::P2ALPHA_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 44.0)), module, Planetz::P2RAY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(27.0, 44.0)), module, Planetz::P2SPEED_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 56.0)), module, Planetz::P3ALPHA_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 56.0)), module, Planetz::P3RAY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(27.0, 56.0)), module, Planetz::P3SPEED_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 68.0)), module, Planetz::P4ALPHA_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 68.0)), module, Planetz::P4RAY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(27.0, 68.0)), module, Planetz::P4SPEED_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 80.0)), module, Planetz::P5ALPHA_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 80.0)), module, Planetz::P5RAY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(27.0, 80.0)), module, Planetz::P5SPEED_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(6.0, 22.0)), module, Planetz::RST_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(14.0, 7.0)), module, Planetz::SCALEX_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(27.0, 7.0)), module, Planetz::SCALEY_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 93.879)), module, Planetz::SELP1_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(1.0, 109.35)), module, Planetz::SELP2_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(2.0, 8.0)), module, Planetz::RST_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.287, 95.879)), module, Planetz::OUTP1X_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(28.288, 95.879)), module, Planetz::OUTP1Y_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.287, 110.821)), module, Planetz::OUTP2X_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(28.288, 110.821)), module, Planetz::OUTP2Y_OUTPUT));
	}
};


Model* modelPlanetz = createModel<Planetz, PlanetzWidget>("Planetz");