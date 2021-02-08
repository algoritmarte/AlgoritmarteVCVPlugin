
#include "CyclicCA.hpp"

struct CyclicCA : Module {
	enum ParamIds {
		BLOCKSIZE_PARAM,
		CASIZE_PARAM,
		COLSAMPLE_PARAM,
		NUMSTATES_PARAM,
		RANDBLOCK_PARAM,
		RANDRULE_PARAM,
		RST_PARAM,
		SPEED_PARAM,
		XNEIGH0_PARAM,
		XNEIGH1_PARAM,
		XNEIGH2_PARAM,
		XNEIGH3_PARAM,
		YSHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		COLCH_INPUT,
		COLCL_INPUT,
		COLCS_INPUT,
		COLSAMPLE_INPUT,
		RANDBLOCK_INPUT,
		RANDRULE_INPUT,
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CyclicCA() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BLOCKSIZE_PARAM, MINBLOCKSIZE, MAXBLOCKSIZE, DEFBLOCKSIZE, "Random block size");
		configParam(CASIZE_PARAM, MINCELLSIZE, MAXCELLSIZE, DEFCELLSIZE, "Cell size");
		configParam(COLSAMPLE_PARAM, 0.f, 1.f, 0.f, "Sample color");
		configParam(NUMSTATES_PARAM, MINNUMSTATES, MAXNUMSTATES, DEFNUMSTATES, "Number of states");
		configParam(RANDBLOCK_PARAM, 0.f, 1.f, 0.f, "Create random block");
		configParam(RANDRULE_PARAM, 0.f, 1.f, 0.f, "Random rule");
		configParam(RST_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(SPEED_PARAM, MINSPEED, MAXSPEED, DEFSPEED, "Speed");
		configParam(XNEIGH0_PARAM, MINNEIGH, MAXNEIGH, DEFNEIGH0, "First neighbor");
		configParam(XNEIGH1_PARAM, MINNEIGH, MAXNEIGH, DEFNEIGH1, "Second neighbor");
		configParam(XNEIGH2_PARAM, MINNEIGH, MAXNEIGH, DEFNEIGH2, "Third neighbor");
		configParam(XNEIGH3_PARAM, MINNEIGH, MAXNEIGH, DEFNEIGH3, "Fourth neighbor");
		configParam(YSHAPE_PARAM, 0.f, 1.f, 0.f, "Shape (diamond/square)");
                
                cca.initCA();
	}
        
        CCA_t cca = {};
        
        dsp::SchmittTrigger resetTrigger;
        dsp::SchmittTrigger colSampleTrigger;
        dsp::SchmittTrigger randBlockTrigger;
        dsp::SchmittTrigger randRuleTrigger;
        
	void process(const ProcessArgs& args) override {
            
            if (resetTrigger.process(params[RST_PARAM].getValue() + inputs[RST_INPUT].getVoltage())) {
                cca.sigReset = true;
            }            
            
            if (randBlockTrigger.process(params[RANDBLOCK_PARAM].getValue() + inputs[RANDBLOCK_INPUT].getVoltage())) {
                cca.sigRandBlock = true;
            }            
            
            if (randRuleTrigger.process(params[RANDRULE_PARAM].getValue() + inputs[RANDRULE_INPUT].getVoltage())) {
                cca.sigRandRule = true;
            }            
            
            int ns = (int)clamp( params[ NUMSTATES_PARAM ].getValue() , MINNUMSTATES, MAXNUMSTATES );
            cca.numstates = ns;
                        
            int z = (int)clamp( params[ CASIZE_PARAM ].getValue() , MINCELLSIZE, MAXCELLSIZE );
            if ( z != cca.cellsize ) {
                cca.newsize = z;
                cca.sigChangeSize = true;
            } 
            
            cca.blockshape = (int)params[ YSHAPE_PARAM ].getValue();
            cca.blocksize = (int)clamp( params[ BLOCKSIZE_PARAM ].getValue() , MINBLOCKSIZE, MAXBLOCKSIZE );
            cca.caspeed = (int)clamp( params[ SPEED_PARAM ].getValue() , MINSPEED, MAXSPEED );
            
            
            if (colSampleTrigger.process(params[COLSAMPLE_PARAM].getValue() + inputs[COLSAMPLE_INPUT].getVoltage())) {
                
                float h, s, l;
                if (  inputs[COLCH_INPUT ].isConnected() ) {
                    h = inputs[COLCH_INPUT ].getVoltage();
                    h = abs(h) / 10.0;
                    if ( h < 0 ) h = 0;
                    if ( h > 1 ) h = 1;
                } else {
                    h = random::uniform();
                }
                if (  inputs[COLCS_INPUT ].isConnected() ) {
                    s = inputs[COLCS_INPUT ].getVoltage();
                    s = abs(s) / 10.0;
                    if ( s < 0 ) s = 0;
                    if ( s > 1 ) s = 1;
                } else {
                    s = random::uniform();
                }
                if (  inputs[COLCL_INPUT ].isConnected() ) {
                    l = inputs[COLCL_INPUT ].getVoltage();
                    l = abs(l) / 10.0;
                    if ( l < 0 ) l = 0;
                    if ( l > 1 ) l = 1;
                } else {
                    l = random::uniform();
                }
                cca.pushColor( h, s, l );
            }


            outputs[ OUT1_OUTPUT ].setVoltage( (cca.out1 * 4) - 2 ); 
            outputs[ OUT2_OUTPUT ].setVoltage( (cca.out2 * 4) - 2 ); 

	}
        
        void updateNeighbors( bool gui2ca ) {
            for (int i = 0; i < 4; i++) {
                int n = (int)clamp( params[ XNEIGH0_PARAM + i ].getValue() , MINNEIGH, MAXNEIGH );
                if ( gui2ca ) {
                    int x = n % 7;
                    int y = n / 7;
                    cca.carules[i][0] = x - 3;
                    cca.carules[i][1] = y - 3;
                } else {
                    int n2 = (cca.carules[i][0]+3)+(cca.carules[i][1]+3)*7;
                    if ( n2 != n ) params[ XNEIGH0_PARAM + i ].setValue( n2 );                
                }
            }
        }
        
        
};

struct CyclicCADisplay : LightWidget {
    CyclicCA *module;
    
    int img = -1;
    //float initX = 0;
    //float initY = 0;
    //float dragX = 0;
    //float dragY = 0;
    int vidframe = 0;
    
    int imgbuf[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {};

    CyclicCADisplay() {}
    
    unsigned char* pimg;

    void onButton(const event::Button &e) override {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            e.consume(this);
            //initX = e.pos.x;
            //initY = e.pos.y;

        }
    }

    void onDragStart(const event::DragStart &e) override {
        //dragX = APP->scene->rack->mousePos.x;
        //dragY = APP->scene->rack->mousePos.y;
    }

    void onDragMove(const event::DragMove &e) override {
        //float newDragX = APP->scene->rack->mousePos.x;
        //float newDragY = APP->scene->rack->mousePos.y;
    }

    void updateBuf() {
        int cf = module->cca.caframe;
        for (int y = 0; y < DISPLAY_HEIGHT; y++ ) {
            int cay = y / module->cca.cellsize;
            for (int x = 0; x < DISPLAY_WIDTH; x++ ) {
                int cax = x / module->cca.cellsize;
                int c = module->cca.cagrid[cf][cax][cay];
                imgbuf[ x + y*DISPLAY_HEIGHT ] = module->cca.cacolscurr[ c ];
            }
        }
    }

    void drawLight(const DrawArgs &args) override {
        if (module == NULL) {
            // background
            nvgFillColor(args.vg, nvgRGB(0, 0, 0));
            nvgBeginPath(args.vg);
            nvgRect(args.vg, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT );
            nvgFill(args.vg);
            return;
        }
        NVGcontext* vg = args.vg;

        
        //for (int x = 0; x < )
        bool rendernew = false;
        
        int sp = MAXSPEED + 1 - module->cca.caspeed;
        //if ( sp < 1 ) sp = 1;
        if ( (vidframe++ % sp ) == 0 ) {
            rendernew = true;            
            module->updateNeighbors( true );
            module->cca.procCAParams();            
            module->updateNeighbors( false );
            if (sp <= MAXSPEED) {
                module->cca.evolveCA();
            }
        }
        
        
        pimg = (unsigned char*)(& imgbuf);
        if ( img == -1 ) {
            updateBuf();
            img = nvgCreateImageRGBA(vg, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, pimg );

        } else {
            if ( rendernew ) {
                updateBuf();
                nvgUpdateImage(vg, img, pimg );
            }
        }
        int x = 0;
        int y = 0;
        int w = DISPLAY_WIDTH;
        int h = DISPLAY_HEIGHT;
        nvgBeginPath(vg);
        NVGpaint imgPaint = nvgImagePattern(vg, x, y, w,h, 0, img, 1.0f);
        nvgRect(vg, x, y, w, h);
        nvgFillPaint(vg, imgPaint);
        nvgFill(vg);        
        
    }
};

struct RoundSmallBlackKnobZnap : RoundKnob {
	RoundSmallBlackKnobZnap() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
                snap = true;
	}
};

struct CyclicCAWidget : ModuleWidget {
	CyclicCAWidget(CyclicCA* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CyclicCA.svg")));
                
        CyclicCADisplay *display = new CyclicCADisplay();
        display->module = module;
        display->box.pos = mm2px(Vec(16.0, 4.0));
        display->box.size = Vec(350, 350 );
        addChild(display);
                

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(152.0, 48.0)), module, CyclicCA::BLOCKSIZE_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(140.0, 31.0)), module, CyclicCA::CASIZE_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 86.0)), module, CyclicCA::COLSAMPLE_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(145.0, 13.0)), module, CyclicCA::NUMSTATES_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 64.0)), module, CyclicCA::RANDBLOCK_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 43.0)), module, CyclicCA::RANDRULE_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.0, 22.0)), module, CyclicCA::RST_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(152.0, 31.0)), module, CyclicCA::SPEED_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(140.0, 65.0)), module, CyclicCA::XNEIGH0_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(152.0, 65.0)), module, CyclicCA::XNEIGH1_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(140.0, 77.0)), module, CyclicCA::XNEIGH2_PARAM));
		addParam(createParam<RoundSmallBlackKnobZnap>(mm2px(Vec(152.0, 77.0)), module, CyclicCA::XNEIGH3_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(140.0, 49.0)), module, CyclicCA::YSHAPE_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 91.0)), module, CyclicCA::COLCH_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 113.0)), module, CyclicCA::COLCL_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 102.0)), module, CyclicCA::COLCS_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 73.0)), module, CyclicCA::COLSAMPLE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 51.0)), module, CyclicCA::RANDBLOCK_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 30.0)), module, CyclicCA::RANDRULE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.0, 9.0)), module, CyclicCA::RST_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(145.762, 95.0)), module, CyclicCA::OUT1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(145.762, 111.0)), module, CyclicCA::OUT2_OUTPUT));

		// mm2px(Vec(120.0, 120.0))
		//addChild(createWidget<Widget>(mm2px(Vec(26.0, 4.0))));
	}
};


Model* modelCyclicCA = createModel<CyclicCA, CyclicCAWidget>("CyclicCA");