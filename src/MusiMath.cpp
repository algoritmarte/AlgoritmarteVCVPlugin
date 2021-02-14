#include "plugin.hpp"

#define NUMNOTES 12
#define NUMBITS  10

#define MINMODE 1.f
#define MAXMODE 3.f

#define MINOFFSET 0.f
#define MAXOFFSET 24.f
#define DEFOFFSET 0.f

#define MINBASE 1.f
#define MAXBASE 10.f
#define DEFBASE 2.f

#define MINLEN 1.f
#define MAXLEN 24.f
#define DEFLEN 8.f


#define NODBG

struct MusiMath : Module {
	enum ParamIds {
		ADDA0_PARAM,
		ADDA1_PARAM,
		ADDB0_PARAM,
		ADDB1_PARAM,
		BASE_PARAM,
		FMIRROR_PARAM,
		LEN_PARAM,
		MODE_PARAM,
		OFFSET_PARAM,
		PROB_PARAM,
		RST_PARAM,
		START0_PARAM,
		START1_PARAM,
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
		CLOCK_INPUT,
		OFFSET_INPUT,
		PROB_INPUT,
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
#ifdef SHOWDBG            
		DBG_OUTPUT,
#endif
		GATE_OUTPUT,
		MAIN_OUTPUT,
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
		RST_LIGHT,
		NUM_LIGHTS
	};

	MusiMath() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ADDA0_PARAM, 0.f, 63.f, 0.f, "Add A Low nibble");
		configParam(ADDA1_PARAM, 0.f, 63.f, 0.f, "Add A High nibble");
		configParam(ADDB0_PARAM, 0.f, 63.f, 0.f, "Add B Low nibble");
		configParam(ADDB1_PARAM, 0.f, 63.f, 0.f, "Add B High nibble");
		configParam(BASE_PARAM, MINBASE, MAXBASE, DEFBASE, "Base");
		configParam(FMIRROR_PARAM, 0.f, 1.f, 0.f, "Mirror scale");
		configParam(LEN_PARAM, MINLEN, MAXLEN, DEFLEN, "Scale length");
		configParam(MODE_PARAM, MINMODE, MAXMODE, 1.f, "Mode");
		configParam(OFFSET_PARAM, MINOFFSET, MAXOFFSET, DEFOFFSET, "Offset");
		configParam(PROB_PARAM, 0.f, 1.f, 0.f, "Probability");
		configParam(RST_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(START0_PARAM, 0.f, 63.f, 0.f, "Start Low nibble");
		configParam(START1_PARAM, 0.f, 63.f, 0.f, "Start High nibble");
		configParam(STEP00_PARAM, 0.f, 1.f, 0.f, "C");
		configParam(STEP01_PARAM, 0.f, 1.f, 0.f, "C#");
		configParam(STEP02_PARAM, 0.f, 1.f, 0.f, "D");
		configParam(STEP03_PARAM, 0.f, 1.f, 0.f, "D#");
		configParam(STEP04_PARAM, 0.f, 1.f, 0.f, "E");
		configParam(STEP05_PARAM, 0.f, 1.f, 0.f, "F");
		configParam(STEP06_PARAM, 0.f, 1.f, 0.f, "F#");
		configParam(STEP07_PARAM, 0.f, 1.f, 0.f, "G");
		configParam(STEP08_PARAM, 0.f, 1.f, 0.f, "G#");
		configParam(STEP09_PARAM, 0.f, 1.f, 0.f, "A");
		configParam(STEP10_PARAM, 0.f, 1.f, 0.f, "A#");
		configParam(STEP11_PARAM, 0.f, 1.f, 0.f, "B");
                
                bitscale[0] = true;
                bitscale[2] = true;
                bitscale[4] = true;
                bitscale[5] = true;
                bitscale[7] = true;
                bitscale[9] = true;
                bitscale[11] = true;
                              
	}
        
        
        dsp::SchmittTrigger clockTrigger;
        dsp::SchmittTrigger resetTrigger;
        dsp::SchmittTrigger scaleTrigger[NUMNOTES];
        
        dsp::PulseGenerator outpulse;
        
        bool bitscale[NUMNOTES] = {};

        
        int scalemap[ NUMNOTES ];
        
        int scalelen = 7;
        
        int offset = 0;
        int bufoffset = 0;
        int curr = 0;
        int oldnote = -1;
        int start = 0;
        int adda = 0;
        int addb = 0;
        int mode = 0;
        float prob = 0;
        int base = 2;
        int len = 8;
        
	void process(const ProcessArgs& args) override {
            
            
            int z = 0;
            for (int i = 0; i < NUMNOTES; i++) {
                if ( scaleTrigger[i].process( params[STEP00_PARAM+i].getValue()) ) {
                    if ( scalelen > 1) {
                        bitscale[i] = ! bitscale[i]; 
                    }
                }
                if ( bitscale[i] ) scalemap[ z++ ] = i;
                lights[ LEDSTEP00_LIGHT + i ].setBrightness( bitscale[i]? 1.0f : 0.f );     
            }
            scalelen = z;
            
            int newstart = (int)(params[ START0_PARAM ].getValue() + 64 * params[ START1_PARAM ].getValue());
            int adda = (int)(params[ ADDA0_PARAM ].getValue() + 64 * params[ ADDA1_PARAM ].getValue());
            int addb = (int)(params[ ADDB0_PARAM ].getValue() + 64 * params[ ADDB1_PARAM ].getValue());
            bool startchanged = newstart != start;
            start = newstart;
                      
            prob = params[ PROB_PARAM ].getValue() + inputs[ PROB_INPUT ].getVoltage() / 10.f;
            if ( prob < 0 ) prob = 0;
            if ( prob > 1 ) prob = 1;
                    
            mode = (int)(clamp( params[ MODE_PARAM ].getValue(), MINMODE, MAXMODE  ));
            base = (int)(clamp( params[ BASE_PARAM ].getValue(), MINBASE, MAXBASE  ));
            len = (int)(clamp( params[ LEN_PARAM ].getValue(), MINLEN, MAXLEN  ));
            
           
            bufoffset = (int)(clamp( params[ OFFSET_PARAM ].getValue() + inputs[ OFFSET_INPUT ].getVoltage(), MINOFFSET, MAXOFFSET  ));
            
            bool fmirr = params[ FMIRROR_PARAM ].getValue() > 0;
            
            bool fplay = false;
            if ( clockTrigger.process(inputs[ CLOCK_INPUT ].getVoltage()) ) {
                float p = random::uniform();
                if ( p >= prob ) {
                    curr = curr + adda;
                } else {
                    curr = curr + addb;
                }
                fplay = true;
                offset = bufoffset;
            }
            if ( startchanged ) curr = start;
            
            // Reset
            if (resetTrigger.process(params[RST_PARAM].getValue() + inputs[RST_INPUT].getVoltage())) {
                oldnote = -1;
                curr = start;
            }  
            
            int v = 0;
            if ( base == 1) {
                v = curr;                
            } else {
                int c = curr;
                while ( c != 0 ) {
                    if ( (c % base) == 1 ) v++;   
                    c /= base;
                }
            }
            
            v += offset;
            if ( v < 0 ) v = 0;
            
            if ( fmirr && len > 2 ) {
                int x = len + (len - 2);
                v = v % x;
                if ( v >= len ) {
                    v = len  - (v - len) - 2;
                }
            }
            
            if ( scalelen < 1 ) scalelen = 1;
            
            int inote = v % len;
            int ioct = inote / scalelen;
            inote = inote % scalelen;
            
            int note = scalemap[ inote ] + 12 * ioct;
            
            bool ftrig = true;
            if ( mode == 2 ) {
                if ( note == oldnote ) ftrig = false;                
            }
            if ( mode == 3 ) {
                if ( note != oldnote ) ftrig = false;                
            }
            if ( fplay && ftrig ) {
                outpulse.trigger(1e-3f);
            }
            oldnote = note;                
          
                      
            //outputs[ GATE_OUTPUT ].setVoltage( 1.0 * offset / 10.0 );            
            //outputs[ GATE_OUTPUT ].setVoltage( inputs[ OFFSET_INPUT ].getVoltage() / 10.f );
#ifdef SHOWDBG             
            outputs[ DBG_OUTPUT ].setVoltage( 1.0 * curr / 10.0 );
#endif            
            outputs[ MAIN_OUTPUT ].setVoltage( 1.0 * note / 12.0 );
            
            outputs[ GATE_OUTPUT ].setVoltage( outpulse.process(args.sampleTime) ? 0.0f : 10.f);
            
            lights[ RST_LIGHT ].setSmoothBrightness(resetTrigger.isHigh(), args.sampleTime);
            
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
            json_array_insert_new(notesJ, i, json_integer((int) bitscale[i]));
        }
        json_object_set_new(rootJ, "notes", notesJ);       
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
                    bitscale[i] = !!json_integer_value(noteJ);
            }
        }
    }        
         
};

struct RoundSmallBlackKnobZnap : RoundKnob {
	RoundSmallBlackKnobZnap() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
                snap = true;
	}
};

struct MusiMathWidget : ModuleWidget {
	MusiMathWidget(MusiMath* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MusiMath.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(29.0, 23.0)), module, MusiMath::ADDA0_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(16.0, 23.0)), module, MusiMath::ADDA1_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(29.0, 39.0)), module, MusiMath::ADDB0_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(16.0, 39.0)), module, MusiMath::ADDB1_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(30.0, 81.0)), module, MusiMath::BASE_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(1.0, 113.0)), module, MusiMath::FMIRROR_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(17.0, 55.0)), module, MusiMath::LEN_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(17.0, 106.0)), module, MusiMath::MODE_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(17.0, 81.0)), module, MusiMath::OFFSET_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(30.0, 55.0)), module, MusiMath::PROB_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 105.0)), module, MusiMath::RST_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(29.0, 7.0)), module, MusiMath::START0_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(16.0, 7.0)), module, MusiMath::START1_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 69.942)), module, MusiMath::STEP00_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 64.942)), module, MusiMath::STEP01_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 59.942)), module, MusiMath::STEP02_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 54.942)), module, MusiMath::STEP03_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 49.942)), module, MusiMath::STEP04_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 40.0)), module, MusiMath::STEP05_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 35.0)), module, MusiMath::STEP06_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 30.0)), module, MusiMath::STEP07_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 25.0)), module, MusiMath::STEP08_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 20.0)), module, MusiMath::STEP09_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.0, 15.0)), module, MusiMath::STEP10_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10.0, 10.0)), module, MusiMath::STEP11_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 81.0)), module, MusiMath::CLOCK_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(17.0, 92.0)), module, MusiMath::OFFSET_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(30.0, 66.0)), module, MusiMath::PROB_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.0, 92.0)), module, MusiMath::RST_INPUT));
#ifdef SHOWDBG 
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17.0, 118.0)), module, MusiMath::DBG_OUTPUT));
#endif                
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(30.0, 96.0)), module, MusiMath::GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(30.0, 109.0)), module, MusiMath::MAIN_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 69.942)), module, MusiMath::LEDSTEP00_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 64.942)), module, MusiMath::LEDSTEP01_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 59.942)), module, MusiMath::LEDSTEP02_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 54.942)), module, MusiMath::LEDSTEP03_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 50.0)), module, MusiMath::LEDSTEP04_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 40.0)), module, MusiMath::LEDSTEP05_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 35.0)), module, MusiMath::LEDSTEP06_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 30.0)), module, MusiMath::LEDSTEP07_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 25.0)), module, MusiMath::LEDSTEP08_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 20.0)), module, MusiMath::LEDSTEP09_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 15.0)), module, MusiMath::LEDSTEP10_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10.0, 10.0)), module, MusiMath::LEDSTEP11_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(5.0, 105.0)), module, MusiMath::RST_LIGHT));
	}
};


Model* modelMusiMath = createModel<MusiMath, MusiMathWidget>("MusiMath");