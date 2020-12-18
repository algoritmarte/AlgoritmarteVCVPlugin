#include "plugin.hpp"

#define MAXSTEPS 16
#define NUMSCALES 4

#define LAKESIZE 16

#define MODE_PROG 1
#define MODE_RUN  0

#define MINOFFSET 21
#define MAXOFFSET 96
#define DEF_OFFSET 60

#define MAXNOTE 108
#define MINNOTE 21

#define DONTUSEDBG
#define NO_INTERNAL_CLOCK

struct MusiFrog : Module {
	enum ParamIds {
#ifdef USEDBG              
		DBG_PARAM,
#endif                
		HOLD_PARAM,
		JUMP00_PARAM,
		JUMP01_PARAM,
		JUMP02_PARAM,
		JUMP03_PARAM,
		JUMP04_PARAM,
		JUMP05_PARAM,
		JUMP06_PARAM,
		JUMP07_PARAM,
		JUMP08_PARAM,
		JUMP09_PARAM,
		JUMP10_PARAM,
		JUMP11_PARAM,
		JUMP12_PARAM,
		JUMP13_PARAM,
		JUMP14_PARAM,
		JUMP15_PARAM,
		MODE_PARAM,
		OFFSET_PARAM,
		RST_PARAM,
		SCALE_PARAM,
		STEPS_PARAM,
		TRIG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RST_INPUT,
		SCALE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		TRIG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		HOLD_LIGHT,
		LEDRST_LIGHT,
		STEP00_LIGHT,
		STEP01_LIGHT,
		STEP02_LIGHT,
		STEP03_LIGHT,
		STEP04_LIGHT,
		STEP05_LIGHT,
		STEP06_LIGHT,
		STEP07_LIGHT,
		STEP08_LIGHT,
		STEP09_LIGHT,
		STEP10_LIGHT,
		STEP11_LIGHT,
		STEP12_LIGHT,
		STEP13_LIGHT,
		STEP14_LIGHT,
		STEP15_LIGHT,
		TRIG_LIGHT,
		NUM_LIGHTS
	};

        
    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger holdTrigger;
    dsp::SchmittTrigger dbgTrigger;
    dsp::PulseGenerator outpulse;
    dsp::SchmittTrigger gatemodeTrigger;

    int scalelist[NUMSCALES][ MAXSTEPS ] = {
        {
             0,  2,  4,  5,  7,  9, 11,
            12, 14, 16, 17, 19, 21, 23,
            24, 26
        },
        {
                                 9, 11,
            12, 14, 16, 17, 19, 21, 23,
            24, 26, 28, 29, 31, 33, 35
        },
        {
             0,  2,  4,      7,  9, 
            12, 14, 16,     19, 16, 14,
            12,  9,  7,      4,  2
        },
        {
                     4,  5,  7,  9, 11,
            12, 14, 16, 17, 16, 14, 12,
            11,  9,  7,  5 
        },
    };
    
    
    int buf[MAXSTEPS] = {};
    int lakerun[ LAKESIZE ] = {};
    int lake[ LAKESIZE ] = {};
    int currscale = 0;
    int frogpos = 0;
    int lakesize = 7;
    bool holdsame = false;
    int mode = 0;
    float phase = 0;
    int curroctave = 0;
    int bufsteps = 0;

    
    int lastnote = -2;
    int currnote = 0;
    int numsteps = 7;
    bool bufready = false;
    bool stepsready = false;
    bool mustreset = false;
    
    bool afterrand = false;
    bool gatemode = false;
    
    MusiFrog() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
#ifdef USEDBG        
		configParam(DBG_PARAM, 0.f, 1.f, 0.f, "");
#endif                
        configParam(HOLD_PARAM, 0.f, 1.f, 0.f, "Hold same notes");
        for (int i = 0; i < 16; i++) {
            configParam(JUMP00_PARAM + i, 0.f, 15.f, 1, string::f("Jump %d", i + 1));
        }
        configParam(MODE_PARAM, 0.f, 1.f, 0.f, "Mode RUN/PROG");
	configParam(OFFSET_PARAM, MINOFFSET, MAXOFFSET, DEF_OFFSET, "Note offset"); // A0 to C7, def C4
        configParam(RST_PARAM, 0.f, 1.f, 0.f, "Reset");
        configParam(SCALE_PARAM, 1.f, NUMSCALES, 1, "Scale");
        configParam(STEPS_PARAM, 1.f, 16.f, 7.f, "Number of stepf");
		configParam(TRIG_PARAM, 0.f, 1.f, 0.f, "0ff=Trigger On=Gate");
        mode = MODE_RUN;
        holdsame = false;
        for (int i = 0; i < MAXSTEPS; i++) {
            lakerun[i] = lake[i] = 1;
        }
        rstTick();
    }
    
    
    /**
     * It's reset time
     */
    void rstTick() {
        bufready = false;
        stepsready = false;
        phase = 0;
        frogpos = 0;
        lastnote = -1;
        for (int i = 0; i < LAKESIZE; i++) {
            lakerun[i] = lake[i];
        }
    }
    
    /**
     * Frog Jump !!! ... ( RIP Eddie)
     */
    void jump() {
        
    }

    int irand(int imin, int imax) {
        return rand() % (imax - imin + 1) + imin;
    }
    
    void onRandomize() override {
        for (int i = 0; i < NUMSCALES; i++) {
            for (int j = 0; j < MAXSTEPS; j++) {
                scalelist[i][j] = irand( 0, 20) - 1; // silence + C1 .... G2
            }
        }
        afterrand = true;
        mustreset = true;
    }
        
    void dbg() {
        std::string inf;
        for (int i = 0; i < MAXSTEPS; i++) {
            inf = inf + std::to_string( scalelist[currscale][i] ) + " ";
        }
        const char *s = inf.c_str();
        INFO ( "currscale=%d currnote=%d numsteps=%d scale=%s", currscale, currnote, numsteps, s );        
    }    
    
    /**
     * It's time to ....
     */
    void clockTick() {
        
        bool skipjump = false;
        // delayed reset ... in sync
        if ( mustreset ) {
            mustreset = false;
            rstTick();
            skipjump = true;
        }
        
        phase = 0;
        int v = 0;
        if ( mode == MODE_PROG ) {
            if (! skipjump) frogpos = (frogpos + 1) % LAKESIZE;
            v = scalelist[ currscale ][frogpos];
            if ( v >= 0) currnote = v;            
        } else {            
            int p = frogpos;
            if (! skipjump) {
                p = ( frogpos + lakerun[frogpos] ) % numsteps;
                lakerun[ frogpos ]++;
            }
            frogpos = p; 
            v = scalelist[ currscale ][ frogpos ];
            if (v >= 0) currnote = v;
        }
        if (v != lastnote || ! holdsame || mode == MODE_PROG ) {
            if ( v >= 0 ) {
                outpulse.trigger(1e-3f);
            }
        }
        lastnote = v;
    }

    /**
     * Main process loop
     */
    void process(const ProcessArgs& args) override {
        
        // after randomize switch back to MODE_RUN
        if ( afterrand ) {
            params[MODE_PARAM].setValue( MODE_RUN );
            afterrand = false;
        }

        // Reset
        if (resetTrigger.process(params[RST_PARAM].getValue() + inputs[RST_INPUT].getVoltage())) {
            mustreset = true;
        }
        
        // Mode
        int newmode = (params[MODE_PARAM].getValue() == 0.f) ? MODE_RUN : MODE_PROG;
        if ( newmode != mode ) {
            mustreset = true;
            mode = newmode;
        }        
        
        // holdsame
        if ( holdTrigger.process( params[HOLD_PARAM].getValue() ) ) {
                holdsame = ! holdsame;
        }
        lights[ HOLD_LIGHT ].setBrightness( ( holdsame )? 1 : 0 );
        
        // Number of steps
        numsteps = (int) clamp( params[STEPS_PARAM].getValue(), 1.0, 1.0*LAKESIZE );
        
        if ( stepsready ) {
            if ( numsteps != bufsteps ) {
                curroctave = (numsteps - 1) % 3; // allow only 3 octaves
            }
        }
        bufsteps = numsteps;
        stepsready = true;
        
        // scale
        currscale =  (int)params[SCALE_PARAM].getValue() - 1;
        if ( inputs[SCALE_INPUT].isConnected() && mode == MODE_RUN ) {
            int scaleoff = (int) std::round( inputs[SCALE_INPUT].getVoltage() );
            if ( scaleoff < -10) scaleoff = -10;
            if ( scaleoff > 10 ) scaleoff = 10;
            currscale = (currscale + scaleoff + (NUMSCALES*10) );
            if ( currscale < 0) currscale = 0;
        }
        currscale = currscale % NUMSCALES;      

        //bool gateIn;
        if (inputs[CLOCK_INPUT].isConnected() && mode != MODE_PROG ) {
            // External clock
            if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
                clockTick();
            }
            //gateIn = clockTrigger.isHigh();
        } else {
            if ( mode == MODE_PROG ) {           
                // Internal clock (120 bpm)
                float clockTime = std::pow(2.f, 1 /* + inputs[CLOCK_INPUT].getVoltage() */);
                phase += clockTime * args.sampleTime;
                if (phase >= 1.f) {
                    clockTick();
                }
            }
            //gateIn = (phase < 0.5f);            
        }
        
       
        // jumps
        for (int i = 0; i < MAXSTEPS; i++) {            
            int n = (int)( params[JUMP00_PARAM + i].getValue() );
            if ( mode == MODE_RUN ) {                
                lake[ i ] = n;                
            } else {   
                if ( bufready && buf[i] != n  ) {
                    scalelist[ currscale ][i] = ( n == 0)? -1 : ( (n-1) + 12*curroctave );                     
                } 
            }
            buf[i] = n;            
        }
        bufready = true;
        
        // light array
        for (int i = 0; i < MAXSTEPS; i++) {
            lights[ STEP00_LIGHT + i].setSmoothBrightness( (i == frogpos)? 1 : 0, args.sampleTime);
        }
        
        // trigger mode
        if ( gatemodeTrigger.process( params[TRIG_PARAM].getValue() ) ) {
                gatemode = ! gatemode;                
        }        
        lights[ TRIG_LIGHT].setBrightness( gatemode? 1 : 0 );
        
        
        int offset = (int) clamp(std::round(params[OFFSET_PARAM].getValue()), 1.0f*MINOFFSET, 1.0f*MAXOFFSET  );
        
        int note0 = offset + currnote;
        if (note0 < MINNOTE ) note0 = MINNOTE;        
        if (note0 > MAXNOTE ) note0 = MAXNOTE;
        
        note0 -= 60;
        
        float freq = (int)(note0 / 12) + (1.0 * (note0 % 12) / 12.0 );
        if ( freq > 10 ) freq = 10;
        if ( freq < -10 ) freq = -10;
        outputs[OUT_OUTPUT].setVoltage( freq  );
        
        if ( gatemode ) {
            outputs[TRIG_OUTPUT].setVoltage( outpulse.process(args.sampleTime)? 0.0f : 10.f );            
        } else {
            outputs[TRIG_OUTPUT].setVoltage( outpulse.process(args.sampleTime)? 10.0f : 0.f );            
        }
        
#ifdef USEDBG            
            // Dbg
            if (dbgTrigger.process(params[DBG_PARAM].getValue())) {
                dbg();
            }
#endif         
    }
    
    
    
    /**
     * Save data
     * @return 
     */
    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        // holdsame
        json_object_set_new(rootJ, "holdsame", json_boolean(holdsame) );

        // gatemode
        json_object_set_new(rootJ, "gatemode", json_boolean(gatemode) );

        // scales
        json_t* stepsJ = json_array();
        for (int i = 0; i < NUMSCALES; i++) {
            for (int j = 0; j < MAXSTEPS; j++) {
                json_array_insert_new(stepsJ, i*MAXSTEPS + j, json_integer( (int) scalelist[i][j] ));
            }
        }
        json_object_set_new(rootJ, "scales", stepsJ);
        
        return rootJ;
    }

    /**
     * Load data
     * @param rootJ
     */
    void dataFromJson(json_t* rootJ) override {
        // holdsame
        json_t* holdsameJ = json_object_get(rootJ, "holdsame");
        if (holdsameJ)
            holdsame = json_is_true(holdsameJ);

        // gatemode
        json_t* gatemodeJ = json_object_get(rootJ, "gatemode");
        if (gatemodeJ)
            gatemode = json_is_true(gatemodeJ);
        
        // scales
        json_t* stepsJ = json_object_get(rootJ, "steps");
        if (stepsJ) {
            for (int i = 0; i < NUMSCALES; i++) {
                for (int j = 0; j < MAXSTEPS; j++) {
                    json_t* stepJ = json_array_get(stepsJ, i*MAXSTEPS + j );
                    if (stepJ)
                        scalelist[i][j] = !!json_integer_value(stepJ);
                }
            }
        }
    }
    
    
    
    
};
struct RoundSmallBlackKnobInt : RoundBlackKnob {
	RoundSmallBlackKnobInt() {
        setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
		snap = true;
	}
};

struct MusiFrogWidget : ModuleWidget {
	MusiFrogWidget(MusiFrog* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MusiFrog.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

#ifdef USEDBG  
		addParam(createParamCentered<LEDButton>(mm2px(Vec(37.0, 18.0)), module, MusiFrog::DBG_PARAM));
#endif
		addParam(createParamCentered<LEDButton>(mm2px(Vec(21.0, 30.0)), module, MusiFrog::HOLD_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(1.0, 41.0)), module, MusiFrog::JUMP00_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(11.0, 41.0)), module, MusiFrog::JUMP01_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(21.0, 41.0)), module, MusiFrog::JUMP02_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(31.0, 41.0)), module, MusiFrog::JUMP03_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(1.0, 54.0)), module, MusiFrog::JUMP04_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(11.0, 54.0)), module, MusiFrog::JUMP05_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(21.0, 54.0)), module, MusiFrog::JUMP06_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(31.0, 54.0)), module, MusiFrog::JUMP07_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(1.0, 67.0)), module, MusiFrog::JUMP08_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(11.0, 67.0)), module, MusiFrog::JUMP09_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(21.0, 67.0)), module, MusiFrog::JUMP10_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(31.0, 67.0)), module, MusiFrog::JUMP11_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(1.0, 80.0)), module, MusiFrog::JUMP12_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(11.0, 80.0)), module, MusiFrog::JUMP13_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(21.0, 80.0)), module, MusiFrog::JUMP14_PARAM));
		addParam(createParam<RoundSmallBlackKnobInt>(mm2px(Vec(31.0, 80.0)), module, MusiFrog::JUMP15_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(33.058, 113.0)), module, MusiFrog::MODE_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(28.0, 24.0)), module, MusiFrog::OFFSET_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(30.0, 10.0)), module, MusiFrog::RST_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(2.0, 99.0)), module, MusiFrog::SCALE_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(2.0, 24.0)), module, MusiFrog::STEPS_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(17.0, 116.0)), module, MusiFrog::TRIG_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.117, 9.92)), module, MusiFrog::CLOCK_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(16.0, 10.0)), module, MusiFrog::RST_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.029, 111.971)), module, MusiFrog::SCALE_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(29.0, 100.0)), module, MusiFrog::OUT_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.0, 100.0)), module, MusiFrog::TRIG_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(21.0, 30.0)), module, MusiFrog::HOLD_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(30.0, 10.0)), module, MusiFrog::LEDRST_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 52.0)), module, MusiFrog::STEP00_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.0, 52.0)), module, MusiFrog::STEP01_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.0, 52.0)), module, MusiFrog::STEP02_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(35.0, 52.0)), module, MusiFrog::STEP03_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 65.0)), module, MusiFrog::STEP04_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.0, 65.0)), module, MusiFrog::STEP05_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.0, 65.0)), module, MusiFrog::STEP06_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(35.0, 65.0)), module, MusiFrog::STEP07_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 78.0)), module, MusiFrog::STEP08_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.0, 78.0)), module, MusiFrog::STEP09_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.0, 78.0)), module, MusiFrog::STEP10_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(35.0, 78.0)), module, MusiFrog::STEP11_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 91.0)), module, MusiFrog::STEP12_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.0, 91.0)), module, MusiFrog::STEP13_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.0, 91.0)), module, MusiFrog::STEP14_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(35.0, 91.0)), module, MusiFrog::STEP15_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(17.0, 116.0)), module, MusiFrog::TRIG_LIGHT));
	}
};


Model* modelMusiFrog = createModel<MusiFrog, MusiFrogWidget>("MusiFrog");