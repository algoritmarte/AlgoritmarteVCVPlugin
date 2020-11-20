#include "plugin.hpp"

#define DEF_TEMPO 1.f


struct Clockkky : Module {
	enum ParamIds {
		CLOCK_PARAM,
		MODE_PARAM,
		RST_PARAM,
		RUN_PARAM,
		STEP00_PARAM,
		STEP01_PARAM,
		STEP02_PARAM,
		STEP03_PARAM,
		STEP04_PARAM,
		STEP05_PARAM,
		STEP06_PARAM,
		STEP07_PARAM,
		STEP08_PARAM,
		STEP09_PARAM,
		STEP10_PARAM,
		STEP11_PARAM,
		STEP12_PARAM,
		STEP13_PARAM,
		STEP14_PARAM,
		STEP15_PARAM,
		STEP16_PARAM,
		STEP17_PARAM,
		STEP18_PARAM,
		STEP19_PARAM,
		STEP20_PARAM,
		STEP21_PARAM,
		STEP22_PARAM,
		STEP23_PARAM,
		TRACK1STEPS_PARAM,
		TRACK2STEPS_PARAM,
		TRACK3STEPS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXTCLOCK_INPUT,
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		TRACK1_OUTPUT,
		TRACK2_OUTPUT,
		TRACK3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEDRST_LIGHT,
		LEDRUN_LIGHT,
		LEDSTEP00_LIGHT,
		LEDSTEP01_LIGHT,
		LEDSTEP02_LIGHT,
		LEDSTEP03_LIGHT,
		LEDSTEP04_LIGHT,
		LEDSTEP05_LIGHT,
		LEDSTEP06_LIGHT,
		LEDSTEP07_LIGHT,
		LEDSTEP08_LIGHT,
		LEDSTEP09_LIGHT,
		LEDSTEP10_LIGHT,
		LEDSTEP11_LIGHT,
		LEDSTEP12_LIGHT,
		LEDSTEP13_LIGHT,
		LEDSTEP14_LIGHT,
		LEDSTEP15_LIGHT,
		LEDSTEP16_LIGHT,
		LEDSTEP17_LIGHT,
		LEDSTEP18_LIGHT,
		LEDSTEP19_LIGHT,
		LEDSTEP20_LIGHT,
		LEDSTEP21_LIGHT,
		LEDSTEP22_LIGHT,
		LEDSTEP23_LIGHT,
		NUM_LIGHTS
	};

        
        static const int MODE_8STEPS = 0;
        static const int MODE_16STEPS = 1;
                
        static const int NUMTRACKS = 3;
        static const int MAXTRACKSTEPS = 8;
        static const int TOTSTEPS = NUMTRACKS * MAXTRACKSTEPS;
        
        int mode = MODE_8STEPS;
        

	bool running = true;
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger runningTrigger;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger stepsTriggers[ 24 ];
	/** Phase of internal LFO */
	float phase = 0.f;
	
	bool steps[ 24 ] = {};
        
        int numSteps[ 3 ];
        int index[ 3 ];
        
	Clockkky() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CLOCK_PARAM, -2.f, 6.f, DEF_TEMPO, "Clock tempo", " bpm", 2.f, 60.f);
		configParam(MODE_PARAM, 0.f, 1.f, 0.f, "Mode");
		configParam(RST_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");
		configParam(STEP00_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP01_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP02_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP03_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP04_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP05_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP06_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP07_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP08_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP09_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP17_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP18_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP19_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP20_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP21_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP22_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEP23_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRACK1STEPS_PARAM, 0.f, 8.f, 8.f, "Track 1 steps");
		configParam(TRACK2STEPS_PARAM, 0.f, 8.f, 8.f, "Track 2 steps");
		configParam(TRACK3STEPS_PARAM, 0.f, 8.f, 8.f, "Track 3 steps");
                
                onReset();
    }
        
    /**
     * Standard reset
     */

    void onReset() override {
        phase = 0.f;
        for (int i = 0; i < TOTSTEPS; i++) {
            if (i < MAXTRACKSTEPS) {
                steps[i] = ((i % 4) == 0);
                continue;
            }
            if (i < MAXTRACKSTEPS * 2) {
                steps[i] = ((i % 2) == 1);
                continue;
            }
            steps[i] = 1;
        }
        for (int i = 0; i < NUMTRACKS; i++) {
            params[TRACK1STEPS_PARAM+i].setValue( 8.f );   
        }
        params[CLOCK_PARAM].setValue( DEF_TEMPO );
        params[MODE_PARAM].setValue( 0 );
    }

/**
     * Save data
     * @return 
     */
    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        // running
        json_object_set_new(rootJ, "running", json_boolean(running));

        // gates
        json_t* stepsJ = json_array();
        for (int i = 0; i < TOTSTEPS; i++) {
            json_array_insert_new(stepsJ, i, json_integer((int) steps[i]));
        }
        json_object_set_new(rootJ, "steps", stepsJ);
        
        // mode
        json_object_set_new(rootJ, "mode", json_boolean(mode != 0) );
        
        // number of steps
        json_t* numstepsJ = json_array();
        for (int i = 0; i < 3; i++) {
            json_array_insert_new(numstepsJ, i, json_integer((int)( numSteps[i]  )  ));            
        }
        json_object_set_new(rootJ, "numsteps", numstepsJ);

        return rootJ;
    }

    /**
     * Load data
     * @param rootJ
     */
    void dataFromJson(json_t* rootJ) override {
        // running
        json_t* runningJ = json_object_get(rootJ, "running");
        if (runningJ)
            running = json_is_true(runningJ);

        // gates
        json_t* stepsJ = json_object_get(rootJ, "steps");
        if (stepsJ) {
            for (int i = 0; i < TOTSTEPS; i++) {
                json_t* stepJ = json_array_get(stepsJ, i);
                if (stepJ)
                    steps[i] = !!json_integer_value(stepJ);
            }
        }
        
        // mode
        json_t* modeJ = json_object_get( rootJ, "mode");
        if ( modeJ ) {
            int n = json_integer_value( modeJ );
            mode = n? 1 : 0;
            params[MODE_PARAM].setValue( mode );
        } 
        
        // number of steps
        json_t* numstepsJ = json_object_get(rootJ, "numsteps");
        if (numstepsJ) {
            for (int i = 0; i < NUMTRACKS; i++) {
                json_t* numstepJ = json_array_get(numstepsJ, i);
                if (numstepJ) {
                    int n = json_integer_value( numstepJ );
                    if ( n < 0 || n > 8 ) {
                        n = 8;
                    }
                    numSteps[i] = n;
                    params[TRACK1STEPS_PARAM + i].setValue( n );
                }
            }
        }        
            
        
    }
    

    /**
     * Reset (resete phase and indexes)
     */
    void rstTick() {
        phase = 0;
        index[0] = 0;
        if (mode == 0) {
            index[1] = 0;
            index[2] = 0;
        } else {
            index[1] = -1;
            index[2] = -1;
        }
    }

    /**
     * Called at every clock tick
     */
    void clockTick() {
        phase = 0;
        bool skip[3] = {};
        for (int i = 0; i < 3; i++) {
            if (!skip[i] && index[i] >= 0) {
                index[i]++;
                if (index[i] >= numSteps[i]) {
                    if (mode == 0) {
                        index[i] = 0;
                    } else {
                        skip[ (i + 1) % 3 ] = true;
                        skip[ (i + 2) % 3 ] = true;
                        index[i] = -1;

                        if (numSteps[(i + 1) % 3] > 0) {
                            index[(i + 1) % 3] = 0;
                        } else {
                            if (numSteps[(i + 2) % 3] > 0) {
                                index[(i + 2) % 3] = 0;
                            } else {
                                index[(i + 3) % 3] = 0;
                            }
                        }
                    }
                }
            }
        }
    }
           
    /**
     * Let's process it ...
     */
    void process(const ProcessArgs& args) override {

        // Run
        if (runningTrigger.process(params[RUN_PARAM].getValue())) {
            if ( running ) {
                running = false;
            } else {
                phase = 0;
                running = true;
            }
        }

        // Mode
        int newmode = (params[MODE_PARAM].getValue() == 0.f) ? MODE_8STEPS : MODE_16STEPS;
        if ( newmode != mode ) {
            mode = newmode;
            rstTick();
        }

        // Number of steps
        for (int i = 0; i < NUMTRACKS; i++) {

            numSteps[i] = (int) clamp(std::round(params[TRACK1STEPS_PARAM + i].getValue()), 0.f, 1.f * MAXTRACKSTEPS);

        }

        bool gateIn = false;
        if (running) {
            if (inputs[EXTCLOCK_INPUT].isConnected()) {
                // External clock
                if (clockTrigger.process(inputs[EXTCLOCK_INPUT].getVoltage())) {
                    clockTick();
                }
                gateIn = clockTrigger.isHigh();
            } else {
                // Internal clock
                float clockTime = std::pow(2.f, params[CLOCK_PARAM].getValue() /* + inputs[CLOCK_INPUT].getVoltage() */ );
                phase += clockTime * args.sampleTime;
                if (phase >= 1.f) {
                    clockTick();
                }
                gateIn = (phase < 0.5f);
            }
        }

        // Reset
        if (resetTrigger.process(params[RST_PARAM].getValue() + inputs[RST_INPUT].getVoltage())) {
            rstTick();
        }

       
        // Steps
        bool found[3] = {};
        for (int i = 0; i < TOTSTEPS; i++) {
            if (stepsTriggers[i].process(params[STEP00_PARAM + i].getValue())) {
                steps[i] = !steps[i];
            }
            //			outputs[GATE_OUTPUT + i].setVoltage((running && gateIn && i == index && gates[i]) ? 10.f : 0.f);

            bool matchIndex = false;
            
            bool m1 = ( index[0] >= 0 && index[0] == i && numSteps[0] > 0 );
            bool m2 = ( index[1] >= 0 && (index[1]+MAXTRACKSTEPS) == i && numSteps[1] > 0 );
            bool m3 = ( index[2] >= 0 && (index[2]+MAXTRACKSTEPS*2) == i  && numSteps[2] > 0);
            matchIndex = m1 || m2 || m3;
            found[0] = found[0] || m1;
            found[1] = found[1] || m2;
            found[2] = found[2] || m3;
           

            if ( mode == 0 ) {
                if ( m1 ) outputs[TRACK1_OUTPUT + 0].setVoltage((running && gateIn && steps[i]) ? 10.f : 0.f);
                if ( m2 ) outputs[TRACK1_OUTPUT + 1].setVoltage((running && gateIn && steps[i]) ? 10.f : 0.f);
                if ( m3 ) outputs[TRACK1_OUTPUT + 2].setVoltage((running && gateIn && steps[i]) ? 10.f : 0.f);
            } else {
                if ( matchIndex ) outputs[TRACK1_OUTPUT + 0].setVoltage((running && gateIn && steps[i]) ? 10.f : 0.f);
                outputs[TRACK1_OUTPUT + 1].setVoltage( 0.f );
                outputs[TRACK1_OUTPUT + 2].setVoltage( 0.f );
            }               
                
            lights[LEDSTEP00_LIGHT + i].setSmoothBrightness((gateIn && matchIndex) ? (steps[i] ? 1.f : 0.33) : (steps[i] ? 0.66 : 0.0), args.sampleTime);            
        }
        // if steps are set to zero
        if ( mode == 0 ) {
            for (int i = 0; i < 3; i++) {
                if ( ! found[i] ) outputs[TRACK1_OUTPUT + i].setVoltage(0.f);
            }
        } else {
            if ( ! found[0] && ! found[1] && ! found[2] ) {
                outputs[TRACK1_OUTPUT].setVoltage(0.f);
                outputs[TRACK1_OUTPUT+1].setVoltage(0.f);
                outputs[TRACK1_OUTPUT+2].setVoltage(0.f);
            }
        }
        
        
        


        
        outputs[CLOCK_OUTPUT].setVoltage(gateIn ? 10.f : 0.f);

        //lights[LEDCLOCK_LIGHT].setSmoothBrightness(gateIn, args.sampleTime);
        lights[LEDRST_LIGHT].setSmoothBrightness(resetTrigger.isHigh(), args.sampleTime);

        lights[LEDRUN_LIGHT].value = (running);


    }
};


struct ClockkkyWidget : ModuleWidget {
	ClockkkyWidget(Clockkky* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Clockkky.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoundBlackKnob>(mm2px(Vec(14.0, 9.0)), module, Clockkky::CLOCK_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(2.442, 53.879)), module, Clockkky::MODE_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(6.5, 24.912)), module, Clockkky::RST_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(19.0, 25.0)), module, Clockkky::RUN_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 76.0)), module, Clockkky::STEP00_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 76.0)), module, Clockkky::STEP01_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 76.0)), module, Clockkky::STEP02_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 76.0)), module, Clockkky::STEP03_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 84.0)), module, Clockkky::STEP04_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 84.058)), module, Clockkky::STEP05_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 84.058)), module, Clockkky::STEP06_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 84.058)), module, Clockkky::STEP07_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 93.0)), module, Clockkky::STEP08_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 93.117)), module, Clockkky::STEP09_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 93.117)), module, Clockkky::STEP10_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 93.117)), module, Clockkky::STEP11_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 101.0)), module, Clockkky::STEP12_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 101.0)), module, Clockkky::STEP13_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 101.0)), module, Clockkky::STEP14_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 101.0)), module, Clockkky::STEP15_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 110.058)), module, Clockkky::STEP16_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 110.0)), module, Clockkky::STEP17_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 110.0)), module, Clockkky::STEP18_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 110.0)), module, Clockkky::STEP19_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 118.058)), module, Clockkky::STEP20_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(16.0, 118.058)), module, Clockkky::STEP21_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(24.0, 118.058)), module, Clockkky::STEP22_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(32.0, 118.058)), module, Clockkky::STEP23_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 31.0)), module, Clockkky::TRACK1STEPS_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 45.0)), module, Clockkky::TRACK2STEPS_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(14.0, 59.0)), module, Clockkky::TRACK3STEPS_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(2.0, 10.0)), module, Clockkky::EXTCLOCK_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(2.0, 32.0)), module, Clockkky::RST_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(29.059, 9.955)), module, Clockkky::CLOCK_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(29.0, 32.0)), module, Clockkky::TRACK1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(29.058, 45.883)), module, Clockkky::TRACK2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(29.0, 60.0)), module, Clockkky::TRACK3_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.5, 24.912)), module, Clockkky::LEDRST_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(19.0, 25.0)), module, Clockkky::LEDRUN_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 76.0)), module, Clockkky::LEDSTEP00_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 76.0)), module, Clockkky::LEDSTEP01_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 76.0)), module, Clockkky::LEDSTEP02_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 76.0)), module, Clockkky::LEDSTEP03_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 84.058)), module, Clockkky::LEDSTEP04_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 84.058)), module, Clockkky::LEDSTEP05_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 84.058)), module, Clockkky::LEDSTEP06_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 84.058)), module, Clockkky::LEDSTEP07_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 93.117)), module, Clockkky::LEDSTEP08_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 93.117)), module, Clockkky::LEDSTEP09_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 93.117)), module, Clockkky::LEDSTEP10_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 93.117)), module, Clockkky::LEDSTEP11_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 101.0)), module, Clockkky::LEDSTEP12_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 101.0)), module, Clockkky::LEDSTEP13_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 101.0)), module, Clockkky::LEDSTEP14_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 101.0)), module, Clockkky::LEDSTEP15_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 110.0)), module, Clockkky::LEDSTEP16_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 110.0)), module, Clockkky::LEDSTEP17_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 110.0)), module, Clockkky::LEDSTEP18_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 110.0)), module, Clockkky::LEDSTEP19_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(8.0, 118.058)), module, Clockkky::LEDSTEP20_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.0, 118.058)), module, Clockkky::LEDSTEP21_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.0, 118.058)), module, Clockkky::LEDSTEP22_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(32.0, 118.058)), module, Clockkky::LEDSTEP23_LIGHT));
	}
};


Model* modelClockkky = createModel<Clockkky, ClockkkyWidget>("Clockkky");
