#include "plugin.hpp"

#define NUMNOTES 12
#define NUMHOLDS 3

#define MODE_PROP    2
#define MODE_NEAREST 1
#define MODE_CLAMP   0

#define NOTEST

struct HoldMeTight : Module {
	enum ParamIds {
		QUANT0_PARAM,
		QUANT1_PARAM,
		QUANT2_PARAM,
		QUANTMODE_PARAM,
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
		NUM_PARAMS
	};
	enum InputIds {
		HOLD0_INPUT,
		HOLD1_INPUT,
		HOLD2_INPUT,
		IN0_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT0_OUTPUT,
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		PULSE0_OUTPUT,
		PULSE1_OUTPUT,
		PULSE2_OUTPUT,
#ifdef TEST                
		TEST_OUTPUT,
		TEST2_OUTPUT,
		TEST3_OUTPUT,
		TEST4_OUTPUT,
		TEST5_OUTPUT,
		TEST6_OUTPUT,
#endif
		NUM_OUTPUTS
	};
	enum LightIds {
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
		QUANT0_LIGHT,
		QUANT1_LIGHT,
		QUANT2_LIGHT,
		NUM_LIGHTS
	};

	HoldMeTight() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(QUANT0_PARAM, 0.f, 1.f, 0.f, "Quantize");
		configParam(QUANT1_PARAM, 0.f, 1.f, 0.f, "Quantize");
		configParam(QUANT2_PARAM, 0.f, 1.f, 0.f, "Quantize");
		configParam(QUANTMODE_PARAM, 0.f, 2.f, 1.f * MODE_PROP, "Quantize mode (clamp, nearest, proportional)");
		configParam(STEP00_PARAM, 0.f, 1.f, 0.f, "Note C");
		configParam(STEP01_PARAM, 0.f, 1.f, 0.f, "Note C#");
		configParam(STEP02_PARAM, 0.f, 1.f, 0.f, "Note D");
		configParam(STEP03_PARAM, 0.f, 1.f, 0.f, "Note Eb");
		configParam(STEP04_PARAM, 0.f, 1.f, 0.f, "Note E");
		configParam(STEP05_PARAM, 0.f, 1.f, 0.f, "Note F");
		configParam(STEP06_PARAM, 0.f, 1.f, 0.f, "Note F#");
		configParam(STEP07_PARAM, 0.f, 1.f, 0.f, "Note G");
		configParam(STEP08_PARAM, 0.f, 1.f, 0.f, "Note Ab");
		configParam(STEP09_PARAM, 0.f, 1.f, 0.f, "Note A");
		configParam(STEP10_PARAM, 0.f, 1.f, 0.f, "Note Bb");
		configParam(STEP11_PARAM, 0.f, 1.f, 0.f, "Note B");
                
                for (int i = 0; i < NUMNOTES; i++ ){
                    notes[i] = true;
                }
	}

        
    bool notes[ NUMNOTES ] = {};


    dsp::SchmittTrigger notesTriggers[ NUMNOTES ];
    
    dsp::SchmittTrigger holdTrigger[ NUMHOLDS ];
    dsp::SchmittTrigger quantTrigger[ NUMHOLDS ];
    bool quantize[NUMHOLDS] = {};
    float output[NUMHOLDS] = {};
    int quantmode = 0;
    int map[NUMNOTES] = {};
    int nummap = 0;
    
    
   
    dsp::PulseGenerator outpulse[NUMHOLDS];
    

    void rebuildMap() {
        nummap = 0;
        for (int i = 0; i < NUMNOTES; i++) {
            if ( notes[i] ) map[nummap++] = i;
        }
        if ( nummap == 0 ) nummap = 1;
    }
    
    void process(const ProcessArgs& args) override {


        quantmode = (int) (clamp(params[QUANTMODE_PARAM].getValue(), 0.f, 2.f));

        for (int i = 0; i < NUMNOTES; i++) {
            if (notesTriggers[i].process(params[STEP00_PARAM + i].getValue())) {
                bool change = true;
                if (notes[i]) { // forbid switching off all lights
                    int c = 0;
                    for (int j = 0; j < NUMNOTES; j++) {
                        if (notes[j]) c++;
                    }
                    if (c == 1) change = false;
                }
                if (change) {
                    notes[i] = !notes[i];
                }
            }
            lights[ LEDSTEP00_LIGHT + i ].setBrightness(notes[i]);
        }


        for (int i = 0; i < NUMHOLDS; i++) {
            if (quantTrigger[i].process(params[QUANT0_PARAM + i].getValue())) {
                quantize[i] = !quantize[i];
            }
            lights[ QUANT0_LIGHT + i ].setBrightness(quantize[i]);
        }

        for (int i = 0; i < NUMHOLDS; i++) {

            if ( inputs[HOLD0_INPUT + i].isConnected() ) {

                if (holdTrigger[i].process(inputs[HOLD0_INPUT + i].getVoltage())) {

                    float v = inputs[ IN0_INPUT + i ].getVoltage();
                    if ( ! inputs[ IN0_INPUT + i].isConnected() ) {
                        v = random::uniform();
                    }
                    if (quantize[i]) {

                        float v12 = v * 12.0f;
                        int v12int = (int) (round(v12));

                        switch (quantmode) {
                            case MODE_PROP:
                            {
                                // what a weird terrible way of calculating the nearest note on ....
                                int off = 0;
                                if (v12 >= 0) {
                                    while (v12 - 12 >= 0) {
                                        v12 -= 12;
                                        off++;
                                    }
                                } else {
                                    while (v12 < 0) {
                                        v12 += 12;
                                        off--;
                                    }
                                }
                                rebuildMap();
                                // 
                                int n = round(  nummap * v12 / 12.0 );
                                if ( n == nummap ) { n = 0; off++; }
                                v = map[ n % nummap ] + off*12;
                                v /= 12.0;
                            }
                            break;


                            case MODE_CLAMP:
                                while (v12int < 0) v12int += 12;
                                if (notes[ v12int % NUMNOTES ]) {
                                    v = 1.0 * v12int / 12.0;
                                } else {
                                    v = output[i];
                                }
                                break;

                            case MODE_NEAREST:
                            {
                                // what a weird terrible way of calculating the nearest note on ....
                                int off = 0;
                                if (v12 >= 0) {
                                    while (v12 - 12 >= 0) {
                                        v12 -= 12;
                                        off++;
                                    }
                                } else {
                                    while (v12 < 0) {
                                        v12 += 12;
                                        off--;
                                    }
                                }
                                int a = (int) (round(v12));
                                int r = a;
                                int l = a;
                                int z = 0;
                                while (!notes[ r % NUMNOTES ] && z < NUMNOTES * 2) {
                                    r = r + 1;
                                    z++;
                                }
                                z = 0;
                                while (!notes[ (l + NUMNOTES * 2) % NUMNOTES ] && z < NUMNOTES * 2) {
                                    l = l - 1;
                                    z++;
                                }
                                float dr = abs(v12 - r);
                                float dl = abs(v12 - l);
                                if (dr <= dl) {
                                    v = r + off * 12;
                                } else {
                                    v = l + off * 12;
                                }
                                v /= 12.0;

                                //outputs[ TEST_OUTPUT ].setVoltage( v12 );
                                //outputs[ TEST2_OUTPUT ].setVoltage( a );
                                //outputs[ TEST3_OUTPUT ].setVoltage( r );
                                //outputs[ TEST4_OUTPUT ].setVoltage( l );
                                //outputs[ TEST5_OUTPUT ].setVoltage( dr );
                                //outputs[ TEST6_OUTPUT ].setVoltage( dl );
                                break;
                            }


                        }
                    }
                    if (v < -5) v = -5;
                    if (v > 5) v = 5;
                    float prev = output[i];
                    output[i] = v;
                    if (v != prev) {
                        outpulse[i].trigger(1e-3f);
                    }

                }
            }

            outputs[ OUT0_OUTPUT + i].setVoltage(output[i]);

        }

        for (int i = 0; i < NUMHOLDS; i++) {
            outputs[PULSE0_OUTPUT + i ].setVoltage(outpulse[i].process(args.sampleTime) ? 0.0f : 10.f);
            //if ( gatemode ) {
            //    outputs[TRIG_OUTPUT].setVoltage( outpulse.process(args.sampleTime)? 0.0f : 10.f );            
            //} else {
            //    outputs[TRIG_OUTPUT].setVoltage( outpulse.process(args.sampleTime)? 10.0f : 0.f );            
            //}
        }




    }

    
        

    /**
     * Save data
     * @return 
     */
    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        // notes
        json_t* notesJ = json_array();
        for (int i = 0; i < NUMNOTES; i++) {
            json_array_insert_new(notesJ, i, json_integer((int) notes[i]));
        }
        json_object_set_new(rootJ, "notes", notesJ);
        
        // quantize
        json_t* quantJ = json_array();
        for (int i = 0; i < NUMHOLDS; i++) {
            json_array_insert_new(quantJ, i, json_integer((int) quantize[i]));
        }
        json_object_set_new(rootJ, "quantize", quantJ);
        
        return rootJ;
    }

    /**
     * Load data
     * @param rootJ
     */
    void dataFromJson(json_t* rootJ) override {
        // notes
        json_t* notesJ = json_object_get(rootJ, "notes");
        if (notesJ) {
            for (int i = 0; i < NUMNOTES; i++) {
                json_t* noteJ = json_array_get(notesJ, i);
                if (noteJ)
                    notes[i] = !!json_integer_value(noteJ);
            }
        }
        // quantize
        json_t* quantsJ = json_object_get(rootJ, "quantize");
        if (quantsJ) {
            for (int i = 0; i < NUMHOLDS; i++) {
                json_t* quantJ = json_array_get(quantsJ, i);
                if (quantJ)
                    quantize[i] = !!json_integer_value(quantJ);
            }
        }
    }        
        
        
        
};


struct HoldMeTightWidget : ModuleWidget {
	HoldMeTightWidget(HoldMeTight* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HoldMeTight.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<LEDButton>(mm2px(Vec(14.0, 15.942)), module, HoldMeTight::QUANT0_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(14.0, 44.058)), module, HoldMeTight::QUANT1_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(14.0, 69.058)), module, HoldMeTight::QUANT2_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(23.0, 88.942)), module, HoldMeTight::QUANTMODE_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 106.0)), module, HoldMeTight::STEP00_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(12.0, 106.0)), module, HoldMeTight::STEP01_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(19.0, 106.0)), module, HoldMeTight::STEP02_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(26.0, 106.0)), module, HoldMeTight::STEP03_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 112.942)), module, HoldMeTight::STEP04_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(12.0, 113.0)), module, HoldMeTight::STEP05_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(19.0, 113.0)), module, HoldMeTight::STEP06_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(26.0, 113.0)), module, HoldMeTight::STEP07_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 120.0)), module, HoldMeTight::STEP08_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(12.0, 120.0)), module, HoldMeTight::STEP09_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(19.0, 120.0)), module, HoldMeTight::STEP10_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(26.0, 120.0)), module, HoldMeTight::STEP11_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 11.942)), module, HoldMeTight::HOLD0_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 40.0)), module, HoldMeTight::HOLD1_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 65.0)), module, HoldMeTight::HOLD2_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 21.942)), module, HoldMeTight::IN0_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 50.0)), module, HoldMeTight::IN1_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 75.0)), module, HoldMeTight::IN2_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 21.942)), module, HoldMeTight::OUT0_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 50.058)), module, HoldMeTight::OUT1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 75.058)), module, HoldMeTight::OUT2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 11.942)), module, HoldMeTight::PULSE0_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 40.058)), module, HoldMeTight::PULSE1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 65.058)), module, HoldMeTight::PULSE2_OUTPUT));
#ifdef TEST                                
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(8.0, 40.0)), module, HoldMeTight::TEST_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 40.0)), module, HoldMeTight::TEST2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 50.0)), module, HoldMeTight::TEST3_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 60.0)), module, HoldMeTight::TEST4_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 70.0)), module, HoldMeTight::TEST5_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(19.0, 80.0)), module, HoldMeTight::TEST6_OUTPUT));
#endif
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 106.0)), module, HoldMeTight::LEDSTEP00_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12.0, 106.0)), module, HoldMeTight::LEDSTEP01_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(19.0, 106.0)), module, HoldMeTight::LEDSTEP02_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.0, 106.0)), module, HoldMeTight::LEDSTEP03_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 113.0)), module, HoldMeTight::LEDSTEP04_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12.0, 113.0)), module, HoldMeTight::LEDSTEP05_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(19.0, 113.0)), module, HoldMeTight::LEDSTEP06_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.0, 113.0)), module, HoldMeTight::LEDSTEP07_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 120.0)), module, HoldMeTight::LEDSTEP08_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12.0, 120.0)), module, HoldMeTight::LEDSTEP09_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(19.0, 120.0)), module, HoldMeTight::LEDSTEP10_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.0, 120.0)), module, HoldMeTight::LEDSTEP11_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.0, 15.942)), module, HoldMeTight::QUANT0_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.0, 44.058)), module, HoldMeTight::QUANT1_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.0, 69.058)), module, HoldMeTight::QUANT2_LIGHT));
	}
};


Model* modelHoldMeTight = createModel<HoldMeTight, HoldMeTightWidget>("HoldMeTight");