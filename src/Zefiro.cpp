
#include "Zefiro.hpp"

#define TRIG_KEYB   2
#define TRIG_PULSER 1
#define TRIG_CLOCK  0
#define TRIG_SEQ    0

#define ENV_SUSTAINED    2
#define ENV_TRANSIENT    1
#define ENV_SELF         0

#define PULSE_SYNC       2
#define PULSE_FREE       1
#define PULSE_CLOCK      0

#define DEF_ATCK         0.5f
#define DEF_SUST         0.5f
#define DEF_DECA         0.5f

#define GATE_VCA         0
#define GATE_COMBO       1
#define GATE_LOPASS      2

#define SRC_GATE1        0
#define SRC_MOD          1
#define SRC_PREAMP       2

#define PREAMPSRC_NOISE        0
#define PREAMPSRC_FEEDBACK     1
#define PREAMPSRC_EXT          2

#define PULSETRIG_SEQ    0
#define PULSETRIG_CLOCK  1
#define PULSETRIG_KEYB   2

#define PULSEMODE_SELF   0
#define PULSEMODE_TRIG   1


#define MODTYPE_FM       0
#define MODTYPE_AM       1
#define MODTYPE_OFF      2

#define FREQ_LO 0
#define FREQ_HI 1

#define MINPITCH -54.f
// 0=C4 12=C5 24=C6 33=A6
#define MAXPITCH  33.f

#define DEFPITCH 0.f
#define DEFFREQ -5.f

#define MAXGAIN          2.f

#define NOTEST

const float MIN_TIME = 1e-3f;
const float MAX_TIME = 5.f;
const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

struct Zefiro : Module {
	enum ParamIds {
		CHA_PARAM,
		CHB_PARAM,
		CLOCK_PARAM,
		//DBG_PARAM,
		ENV0ATCK_PARAM,
		ENV1SUST_PARAM,
		ENV2DECA_PARAM,
		ENVMODE_PARAM,
		ENVTRIG_PARAM,
		FREQ_PARAM,
		FREQFINE_PARAM,
		FREQKEY_PARAM,
		FREQLOHI_PARAM,
		FREQMOD_PARAM,
		FREQMODTYPE_PARAM,
		FREQQUANT_PARAM,
		FREQWAVE_PARAM,
		GATE1MODE_PARAM,
		GATE2MODE_PARAM,
		GATE2SRC_PARAM,
		LEV1_PARAM,
		LEV1MOD_PARAM,
		LEV2_PARAM,
		LEV2MOD_PARAM,
		MOD_PARAM,
		MODMOD_PARAM,
		NOISETRIG_PARAM,
		PITCH_PARAM,
		PITCHFINE_PARAM,
		PITCHKEY_PARAM,
		PITCHMOD_PARAM,
		PITCHPOL_PARAM,
		PITCHQUANT_PARAM,
		PREAMPSRC_PARAM,
		PULSEMOD_PARAM,
		PULSEMODE_PARAM,
		PULSEPERIOD_PARAM,
		PULSETRIG_PARAM,
		REVERBFBACK_PARAM,
		REVERBMIX_PARAM,
		REVERBTIME_PARAM,
		STEPFREQ0_PARAM,
		STEPFREQ1_PARAM,
		STEPFREQ2_PARAM,
		STEPFREQ3_PARAM,
		STEPFREQ4_PARAM,
		STEPNUM_PARAM,
		STEPSWITCH0_PARAM,
		STEPSWITCH1_PARAM,
		STEPSWITCH2_PARAM,
		STEPSWITCH3_PARAM,
		STEPSWITCH4_PARAM,
		STEPTRIG_PARAM,
		TIMBRE_PARAM,
		TIMBREMIX_PARAM,
		TIMBREMOD_PARAM,
		TIMBREWAVE_PARAM,
		VOL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXT_INPUT,
		EXTCLK_INPUT,
		FREQMOD_INPUT,
		INVERTER_INPUT,
		KEYB_INPUT,
		KEYBGATE_INPUT,
		LEV1MOD_INPUT,
		LEV2MOD_INPUT,
		MODMOD_INPUT,
		PITCHMOD_INPUT,
		PULSEMOD_INPUT,
		REVERBTIMEMOD_INPUT,
		TIMBREMOD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENV_OUTPUT,
		INVERTER_OUTPUT,
		MOD_OUTPUT,
		NOISE_OUTPUT,
		NOISE2_OUTPUT,
		OUT_OUTPUT,
		PULSE_OUTPUT,
		SEQ_OUTPUT,
#ifdef USETEST                
		TEST_OUTPUT,
		TEST2_OUTPUT,
#endif                
		//TEST3_OUTPUT,
		//TEST4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_LIGHT,
		ENV_LIGHT,
		GATE1_LIGHT,
		GATE2_LIGHT,
		NOISE_LIGHT,
		PULSE_LIGHT,
		STEP0_LIGHT,
		STEP1_LIGHT,
		STEP2_LIGHT,
		STEP3_LIGHT,
		STEP4_LIGHT,
		NUM_LIGHTS
	};

	Zefiro() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CHA_PARAM, 0.f, MAXGAIN, 1.f, "Channel A gain");
		configParam(CHB_PARAM, 0.f, MAXGAIN, 1.f, "Channel B gain");
		configParam(CLOCK_PARAM, -2.f, 6.f, 1.f, "Internal clock tempo", " bpm", 2.f, 60.f);
		//configParam(DBG_PARAM, 0.f, 1.f, 0.f, "Dbg");
		configParam(ENV0ATCK_PARAM, 0.f, 1.f, DEF_ATCK, "Attack time", "ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(ENV1SUST_PARAM, 0.f, 1.f, DEF_SUST, "Sustain time", "ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(ENV2DECA_PARAM, 0.f, 1.f, DEF_DECA,"Decay time", "ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(ENVMODE_PARAM, 0.f, 2.f, ENV_SUSTAINED * 1.f, "Envelope mode (Self/Transient/Sustained)");
		configParam(ENVTRIG_PARAM, 0.f, 2.f, TRIG_KEYB * 1.f, "Envelope trigger source (Sequencer/Pulser/Keyb)");
		configParam(FREQ_PARAM, MINPITCH , MAXPITCH, DEFFREQ, "Modulator frequency" , "Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(FREQFINE_PARAM, -1.f, 1.f, 0.f, "Modulator fine tune");
		configParam(FREQKEY_PARAM, 0.f, 1.f, 1.f, "Modulator keyboard tracking");
		configParam(FREQLOHI_PARAM, 0.f, 1.f, FREQ_HI, "Modulator LO/HI frequency");
		configParam(FREQMOD_PARAM, 0.f, UNIPEAK, 0.f, "Modulator frequency modulation amount");
		configParam(FREQMODTYPE_PARAM, 0.f, 2.f, MODTYPE_AM, "Modulator type (FM/AM/OFF)");
		configParam(FREQQUANT_PARAM, 0.f, 1.f, 1.f, "Modulator quantize");
		configParam(FREQWAVE_PARAM, 0.f, 2.f, 0.f, "Modulator wave type (Triangle/Square/Saw)");
		configParam(GATE1MODE_PARAM, 0.f, 2.f, GATE_COMBO, "Gate 1 mode (VCA/Combo/LoPass");
		configParam(GATE2MODE_PARAM, 0.f, 2.f, GATE_COMBO, "Gate 2 mode (VCA/Combo/LoPass");
		configParam(GATE2SRC_PARAM, 0.f, 2.f, SRC_MOD, "Gate 2 source (Gate1/Modulator/Preamp)");
		configParam(LEV1_PARAM, 0.f, UNIPEAK, 8.f, "Gate 1 level");
		configParam(LEV1MOD_PARAM, 0.f, UNIPEAK, 0.f, "Gate 1 modulation");
		configParam(LEV2_PARAM, 0.f, UNIPEAK, 6.f, "Gate 2 level");
		configParam(LEV2MOD_PARAM, 0.f, UNIPEAK, 0.f, "Gate 2 modulation");
		configParam(MOD_PARAM, 0.f, UNIPEAK, 0.f, "Moduation amount");
		configParam(MODMOD_PARAM, 0.f, UNIPEAK, 0.f, "Modulation of modulation");
		configParam(NOISETRIG_PARAM, 0.f, 2.f, TRIG_KEYB, "Noise hold trigger source (Sequencer/Pulser/Keyb)");
		configParam(PITCH_PARAM, MINPITCH, MAXPITCH, DEFPITCH, "Pitch frequency" , "Hz",dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(PITCHFINE_PARAM, -1.f, 1.f, 0.f, "Pitch fine tune");
		configParam(PITCHKEY_PARAM, 0.f, 1.f, 1.f, "Pitch keyboard tracking");
		configParam(PITCHMOD_PARAM, 0.f, UNIPEAK, 0.f, "Pitch modulation");
		configParam(PITCHPOL_PARAM, 0.f, 1.f, 0.f, "Pitch modulation polarity");
		configParam(PITCHQUANT_PARAM, 0.f, 1.f, 1.f, "Pitch quantize");
		configParam(PREAMPSRC_PARAM, 0.f, 2.f, 0.f, "Preamp source (Noise/Feedback/Ext");
		configParam(PULSEMOD_PARAM, 0.f, UNIPEAK, 0.f, "Period modulation");
		configParam(PULSEMODE_PARAM, 0.f, 1.f, PULSEMODE_TRIG, "Pulser mode (Triggered/Self)");
		configParam(PULSEPERIOD_PARAM, 0.f, 1.f, 0.648f, "Period time", "ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(PULSETRIG_PARAM, 0.f, 2.f, PULSETRIG_KEYB, "Pulser trigger source (Sequencer/Clock/Keyb)");
		configParam(REVERBFBACK_PARAM, 0.f, 100.f, 50.f, "Rerverb feedback", "%");
		configParam(REVERBMIX_PARAM, 0.f, 100.f, 50.f, "Reverb dry/wet", "%");
		configParam(REVERBTIME_PARAM, REVERBTIMEMIN, REVERBTIMEMAX, REVERBTIMEDEF, "Reverb time", "ms");
		configParam(STEPFREQ0_PARAM, 0.f, 10.f, 0.f, "Step 1 voltage", "V" );
		configParam(STEPFREQ1_PARAM, 0.f, 10.f, 1.f, "Step 2 voltage", "V" );
		configParam(STEPFREQ2_PARAM, 0.f, 10.f, 2.f, "Step 3 voltage", "V" );
		configParam(STEPFREQ3_PARAM, 0.f, 10.f, 3.f, "Step 4 voltage", "V" );
		configParam(STEPFREQ4_PARAM, 0.f, 10.f, 4.f, "Step 5 voltage", "V" );
		configParam(STEPNUM_PARAM, 0.f, 2.f, 2.f, "Number of steps (3/4/5)", "", 0, 1, 3 );
		configParam(STEPSWITCH0_PARAM, 0.f, 1.f, 1.f, "Step 1 on/off" );
		configParam(STEPSWITCH1_PARAM, 0.f, 1.f, 1.f, "Step 2 on/off" );
		configParam(STEPSWITCH2_PARAM, 0.f, 1.f, 0.f, "Step 3 on/off" );
		configParam(STEPSWITCH3_PARAM, 0.f, 1.f, 1.f, "Step 4 on/off" );
		configParam(STEPSWITCH4_PARAM, 0.f, 1.f, 0.f, "Step 5 on/off" );
		configParam(STEPTRIG_PARAM, 0.f, 2.f, TRIG_CLOCK * 1.f, "Sequencer trigger source (Clock/Pulser/Keyb)");
		configParam(TIMBRE_PARAM, 0.f, UNIPEAK, 0.f, "Timbre");
		configParam(TIMBREMIX_PARAM, 0.f, 1.f, 0.f, "Timbre wave + sine mix");
		configParam(TIMBREMOD_PARAM, 0.f, UNIPEAK, 0.f, "Timbre modulation");
		configParam(TIMBREWAVE_PARAM, 0.f, 2.f, 0.f, "Timbre waveform (Triangle/Square/Saw)");
		configParam(VOL_PARAM, 0.f, MAXGAIN, 1.f, "Volume");
                
                initOsc();
                
	}
        
        dsp::SchmittTrigger dbgTrigger;    
        dsp::ClockDivider lightDivider;
        
    	dsp::SchmittTrigger clockTrigger;         // external clock
        float phase = 0.f;                        // phase of internal clock
        bool fClockPulse = false;                 // clock pulse        
        
        int stepnum_map[3] = { 3,4,5 };           // map for number of steps
        int stepnum = 5;                          // number of steps
        int stepcurr = 0;                         // current step
        int steptrig = TRIG_CLOCK;                // trigger for sequencer ( clock / pulser / keyboard ) 
        bool fSeqPulse = false;                   // sequencer pulse
                
        dsp::SchmittTrigger keybTrigger;          // external keyboard
        float keybGate = 0;
        float keybVolt = 0;                       // keyboard voltage
        bool fKeybPulse = false;                  // keyboard pulse
        
        WestADSR_t envelope;
        int envelope_mode = ENV_SUSTAINED;        // envelope mode (self / transient / sustained )
        int envtrig = TRIG_KEYB;                  // trigger for envelope ( sequencer / pulser / keyboard )
        
               
        WestADSR_t pulser;
        bool fPulserPulse = false;                // pulser pulse
        int pulse_mode = PULSEMODE_TRIG;          // pulser mode ( clock / free / sync )
        int pulsetrig = PULSETRIG_KEYB;           // trigger for pulser ( sequencer, pulser, keyboard )
        
        int noisetrig = TRIG_KEYB;                // trigger for noise ( sequencer / pulser / keyboard )
        float noiseout = 0;
        float noiseout2 = 0;
        
        int freqlohi = 1;
        int freqmodtype = 1;
        WestWave modosc[3] = {};
        float modulator_output = 0;
        float lomodelo = 0;
        float lomodehi = 0;
        WestBuf modbuf;
        
        WestWave osc[4] = {};
        float osc_output = 0;
        
        
        float complexosc_output = 0;
        
        float gate1_output = 0;
        float gate2_output = 0;
        float gate2_preoutput = 0;
        dsp::RCFilter gate1filt;
        dsp::RCFilter gate2filt;
        //WestFilter gate1filt;
        //WestFilter gate2filt;
        
        WestDelay reverb;
        
        float ext_output = 0;
        float feedback_output = 0;
        float noise_output = 0;
        float preamp_output = 0;
        
        float invert_output = 0;
        
        
    /**
     * Preparing the oscillators
     */    
    void initOsc() {
        osc[0].prepare( WTSIN );
        osc[1].prepare( WTTRI );
        osc[2].prepare( WTSQR );
        osc[3].prepare( WTSAW );

        modosc[0].prepare( WTTRI );
        modosc[1].prepare( WTSQR );
        modosc[2].prepare( WTSAW ); 
        
        lomodelo = log2( 0.17 / dsp::FREQ_C4 );
        lomodehi = log2( 55.0 / dsp::FREQ_C4 );
        
        lightDivider.setDivision(16);
    }    
        
    /**
     *
     */    
    void process(const ProcessArgs& args) override {


        // -------------------- KEYBOARD INPUT (1V/OCT and PULSE)
        procKeyb( args );
        
        // -------------------- CLOCK SECTION
        procClock(args);
        
        // -------------------- SEQUENCER SECTION
        procSequencer(args);
        
        
        // -------------------- ENVELOPE SECTION
        // trigger mode for envelope
        envtrig = (int)clamp( params[ ENVTRIG_PARAM ].getValue() , 0.f, 2.f );
        envelope_mode = (int)clamp( params[ ENVMODE_PARAM ].getValue() , 0.f, 2.f );
        bool shotenv = false;
        if ( envelope_mode != ENV_SELF ) {
            switch ( envtrig ) {
                case TRIG_SEQ :
                    if ( fSeqPulse ) {
                        shotenv = true;
                    }
                    break;
                case TRIG_PULSER :
                    if ( fPulserPulse ) {
                        shotenv = true;
                    }
                    break;
                case TRIG_KEYB :
                    if ( fKeybPulse ) {
                        shotenv = true;
                    }
                    break;
            }
        }
        if ( envelope_mode == ENV_SELF && envelope.phase == 0 ) { // self triggering
            shotenv = true;
        }
        if ( shotenv ) {
            envelope.trigger();
        }
        bool fgate = ( envelope_mode == ENV_SUSTAINED) && (keybGate >= 1.0f );
        envelope.attack_time = simd::pow( LAMBDA_BASE, params[ ENV0ATCK_PARAM ].getValue() );
        envelope.decay_time = 0;
        envelope.auto_sustain = true;
        envelope.sustain_perc = 1;
        envelope.sustain_time = simd::pow( LAMBDA_BASE, params[ ENV1SUST_PARAM ].getValue() );
        envelope.release_time = simd::pow( LAMBDA_BASE, params[ ENV2DECA_PARAM ].getValue() );
        envelope.process( 1000.0 / args.sampleRate, fgate, envelope_mode == ENV_SELF );
        outputs[ ENV_OUTPUT ].setVoltage( envelope.output );
        lights[ ENV_LIGHT ].setSmoothBrightness( (envelope.phase == 1)? 1. : 0. , args.sampleTime );
        
        // -------------------- PULSER SECTION
        // trigger mode for pulser
        pulsetrig = (int)clamp( params[ PULSETRIG_PARAM ].getValue() , 0.f, 2.f );
        pulse_mode = (int)clamp( params[ PULSEMODE_PARAM ].getValue() , 0.f, 1.f );
        bool shotpulse = false;
        if ( pulse_mode == PULSEMODE_TRIG ) {
            switch ( pulsetrig ) {
                case PULSETRIG_SEQ :
                    if ( fSeqPulse ) {
                        shotpulse = true;
                    }
                    break;
                case PULSETRIG_CLOCK :
                    if ( fClockPulse ) { //self triggering
                        shotpulse = true;
                    }
                    break;
                case PULSETRIG_KEYB :
                    if ( fKeybPulse ) {
                        shotpulse = true;
                    }
                    break;
            }
        } else {
            if ( pulser.phase == 0 ) { //self triggering PULSEMODE_SELF
                shotpulse = true;
            }            
        }
        if ( shotpulse ) {
            pulser.trigger();
        }
        bool fgate2 = false; //( pulsetrig == TRIG_KEYB ) && (keybGate >= 1.0f ); //abilita sustain anche sul pulser ?!?
        pulser.auto_sustain = true;
        pulser.attack_time = 5;
        pulser.decay_time = 5;
        pulser.sustain_perc = 0.5;
        float sustmod = (params[ PULSEMOD_PARAM ].getValue() / UNIPEAK) * ( inputs[ PULSEMOD_INPUT ].getVoltage() / UNIPEAK ) ;
        float sust = clamp( params[ PULSEPERIOD_PARAM ].getValue() + sustmod, 0.f, 1.f );
        float tmpsust = simd::pow( LAMBDA_BASE, sust  );
//        float tmpsust = sust + 1000. * sustmod * MAX_TIME;  // linear modualtion ?!?
//        if ( tmpsust < 1000 * MIN_TIME ) tmpsust = 1000 * MIN_TIME;
//        if ( tmpsust > 1000 * MAX_TIME ) tmpsust = 1000 * MAX_TIME;        
        pulser.sustain_time = 0;       //  tmpsust * 0.7; 70% to sustain  (personal (not so accurte) heuristic)
        pulser.release_time = tmpsust; //  tmpsust * 0.3; 30% to release
        pulser.process( 1000.0 / args.sampleRate, fgate2, pulse_mode == PULSEMODE_SELF );
        outputs[ PULSE_OUTPUT ].setVoltage( pulser.output );
        fPulserPulse = ( pulser.ended )? true : false;
        lights[ PULSE_LIGHT ].setSmoothBrightness( fPulserPulse ? 1.f : 0.f, args.sampleTime  );
        
//        outputs[ TEST_OUTPUT ].setVoltage( tmpsust );
//        outputs[ TEST2_OUTPUT ].setVoltage( sustmod  );

        
        // -------------------- NOISE SECTION
        procNoise( args );


        // -------------------- MODULATION SECTION
        freqlohi = (int)clamp( params[ FREQLOHI_PARAM ].getValue() , 0.f, 1.f );
        
        float freqParam;
        if ( freqlohi == FREQ_HI ) {
            // hi frequency
            freqParam = params[FREQ_PARAM].getValue() / 12.0f;
            if ( params[ FREQQUANT_PARAM ].getValue() > 0 ) {
                freqParam = ((int)(freqParam * 12.0)) / 12.0; // quantize modulation frequency
            }
            if ( params[ FREQKEY_PARAM ].getValue() > 0 ) {
                freqParam += keybVolt;
            }
            freqParam += dsp::quadraticBipolar(params[FREQFINE_PARAM].getValue()) * 3.f / 12.f;
        } else {
            // lo frequency
            freqParam = (params[FREQ_PARAM].getValue() - MINPITCH) / (MAXPITCH - MINPITCH ); // 0 to 30 Hz
            freqParam = lomodelo + freqParam * ( lomodehi - lomodelo ); // 0.17Hz to 55Hz             
            freqParam += dsp::quadraticBipolar(params[FREQFINE_PARAM].getValue()) * 3.f / 12.f;
            //if (freqParam < 0) freqParam = 0;
        }
        float freqmod = ( params[ FREQMOD_PARAM ].getValue() / UNIPEAK ) * inputs[ FREQMOD_INPUT ].getVoltage();
        
        float freq = freqParam;
        
        freq += ( freqmod / UNIPEAK ) * 5; // 5 octaves
        if ( freq > MAXOSCFREQ ) freq = MAXOSCFREQ;
        
        for (int i = 0; i < 3; i++) {
            modosc[i].setPitch( freq );
            modosc[i].process( args.sampleTime );
        }        
        int freqwave = (int)clamp( params[ FREQWAVE_PARAM ].getValue() , 0.f, 2.f );
        modulator_output = modosc[ freqwave ].output;
        
        // ---- prepare modulation for OSC1
        freqmodtype = (int)clamp( params[ FREQMODTYPE_PARAM ].getValue() , 0.f, 2.f ); // modulation type        
        float modstrenght = params[ MOD_PARAM ].getValue() / UNIPEAK;
        float modmod = ( params[ MODMOD_PARAM ].getValue() / UNIPEAK ) * inputs[ MODMOD_INPUT ].getVoltage();
        modstrenght += modmod / UNIPEAK; // [0..1]
        
        if ( outputs[ MOD_OUTPUT ].isConnected() ) {
            outputs[ MOD_OUTPUT ].setVoltage( modulator_output * modstrenght );
        }
        
        // -------------------- OSCILLATOR SECTION
        float pitchParam = params[PITCH_PARAM].getValue() / 12.0f;
        if ( params[ PITCHQUANT_PARAM ].getValue() > 0 ) {
            pitchParam = ((int)(pitchParam * 12.0)) / 12.0; // quantize modulation frequency
        }
        if ( params[ PITCHKEY_PARAM ].getValue() > 0 ) {
            pitchParam += keybVolt;
        }
        
	pitchParam += dsp::quadraticBipolar(params[PITCHFINE_PARAM].getValue()) * 3.f / 12.f;                
        float pitch = pitchParam;        
        
        float pitchmod = ( params[ PITCHMOD_PARAM ].getValue() / UNIPEAK ) * inputs[ PITCHMOD_INPUT ].getVoltage();
        // ^^^^ should quantize modulation ?!?
        float pol = ( params[ PITCHPOL_PARAM ].getValue() > 0 )? -1.f : 1.f;
        pitch += pol * ( pitchmod / UNIPEAK ) * 5; // 5 octaves ?!?         
        
        if ( freqmodtype == MODTYPE_FM ) {
            modbuf.process( pitch + modstrenght * modulator_output / 10.0, 3 );
            //modbuf.process( pitch * ( 1 + modstrenght * modulator_output / 10.0 ), 4 );            
            pitch = modbuf.output;
            ///pitch += modstrenght * modulator_output ; // modstrenght:[0,1] * modulator_osc:[-5,5 ]         
        }
        
        for (int i = 0; i < 4; i++) {
            osc[i].setPitch( pitch );
            osc[i].process( args.sampleTime );
        }
        // mix sine with choosen wave
        int timbrewave = (int)clamp( params[ TIMBREWAVE_PARAM ].getValue() , 0.f, 2.f );
        float timbremix = params[ TIMBREMIX_PARAM ].getValue();
        osc_output = (1 - timbremix) * osc[0].output + timbremix * osc[timbrewave + 1].output; 
        
        
        // -------------------- WAVEFOLDING
        float timbreParam = params[TIMBRE_PARAM].getValue();
        float timbremod = ( params[TIMBREMOD_PARAM].getValue() / UNIPEAK ) * inputs[ TIMBREMOD_INPUT ].getVoltage();
        timbreParam = (timbreParam + timbremod ) / ( UNIPEAK );  // 0-1               

        float buchlavf = 6.0 / UNIPEAK;
        osc_output *= buchlavf;        
        complexosc_output = waveFolder( osc_output, (1 + 5.0 * 2.50 * timbreParam) / 5.05  );
        complexosc_output /= buchlavf; 
        
        if ( freqmodtype == MODTYPE_AM ) {
            modbuf.process( complexosc_output * (1 + modstrenght * modulator_output / 5.0), 4 );
            complexosc_output = modbuf.output;
            //complexosc_output *= (1 + modstrenght * modulator_output / 5); // + 5V AM modulation
        }        
        
        
        // -------------------- PREAMP SECTION
        int preampsrc = (int)clamp( params[ PREAMPSRC_PARAM ].getValue() , 0.f, 2.f ); 
        switch ( preampsrc ) {
            case PREAMPSRC_NOISE :
                preamp_output = noise_output;
                break;
            case PREAMPSRC_FEEDBACK :
                preamp_output = 0; // feedback_output; // to be implemented ...
                break;
            case PREAMPSRC_EXT :
                preamp_output = 0;
                if ( inputs[ EXT_INPUT ].isConnected() ) {
                    preamp_output = inputs[ EXT_INPUT ].getVoltage();
                }
                break;
        }

        // -------------------- GATE 1 SECTION
        int gate1mode = (int)clamp( params[ GATE1MODE_PARAM ].getValue() , 0.f, 2.f );        
        float lev1 = params[ LEV1_PARAM ].getValue();
        lev1 += ( params[LEV1MOD_PARAM].getValue() / UNIPEAK ) * inputs[ LEV1MOD_INPUT ].getVoltage();
        lev1 = clamp( lev1, 0.f, UNIPEAK );
        lev1 /= UNIPEAK;
        gate1filt.setCutoffFreq( paramToFreq( lev1) / args.sampleRate);
	gate1filt.process( complexosc_output );
        switch ( gate1mode ) {            
            case GATE_LOPASS :
                gate1_output = gate1filt.lowpass();
                break;                
            case GATE_COMBO :
                gate1_output = ((complexosc_output * lev1) + gate1filt.lowpass() ) / 2;                        
                break;            
            default :  // GATE_VCA
                gate1_output = complexosc_output * lev1 ;                        
                break;
        }

        // -------------------- GATE 2 SECTION
        int gate2src = (int)clamp( params[ GATE2SRC_PARAM ].getValue() , 0.f, 2.f );
        switch ( gate2src ) {
            case SRC_GATE1 :
                gate2_preoutput = - gate1_output;
                break;
            case SRC_MOD :
                gate2_preoutput = modulator_output;
                break;
            case SRC_PREAMP :
                gate2_preoutput = preamp_output;
                break;
        }        
        
        int gate2mode = (int)clamp( params[ GATE2MODE_PARAM ].getValue() , 0.f, 2.f );
        float lev2 = params[ LEV2_PARAM ].getValue();
        lev2 += ( params[LEV2MOD_PARAM].getValue() / UNIPEAK ) * inputs[ LEV2MOD_INPUT ].getVoltage();
        lev1 = clamp( lev2, 0.f, UNIPEAK );
        lev2 /= UNIPEAK;
        gate2filt.setCutoffFreq( paramToFreq( lev2) / args.sampleRate);
	gate2filt.process( gate2_preoutput );          
        switch ( gate2mode ) {            
            case GATE_LOPASS :
                gate2_output = gate2filt.lowpass();
                break;                
            case GATE_COMBO :
                gate2_output = ((gate2_preoutput * lev2 ) + gate2filt.lowpass() ) / 2;                        
                break;            
            default :  // GATE_VCA
                gate2_output = gate2_preoutput * lev2;                        
                break;
        }
       
        
        // -------------------- PRE-OUTPUT SECTION
        if (lightDivider.process()) {
            float deltaTime = args.sampleTime * lightDivider.getDivision();
            lights[ GATE1_LIGHT ].setSmoothBrightness( abs( gate1_output ) / BIPPEAK, deltaTime  );
            lights[ GATE2_LIGHT ].setSmoothBrightness( abs( gate2_output ) / BIPPEAK, deltaTime  );
        }

        gate1_output *= params[ CHA_PARAM ].getValue();
        gate2_output *= params[ CHB_PARAM ].getValue();
        float mixed_output = ( gate1_output + gate2_output );
        

        
        // -------------------- REVERB SECTION ...
        float reverbtime = params[ REVERBTIME_PARAM ].getValue();
        float reverbtimemod = inputs[ REVERBTIMEMOD_INPUT ].getVoltage();
        reverbtime += reverbtimemod * 100.0;
        
        float reverbmix = params[ REVERBMIX_PARAM ].getValue() / 100.f;
        float reverbfback = params[ REVERBFBACK_PARAM ].getValue() / 100.f;
        
        reverb.setTempo( reverbtime, args.sampleRate );
        reverb.process( mixed_output, reverbfback, reverbmix );
        
        float reverb_output = reverb.output; 
        
        // feedback
        feedback_output = 0; // future versions ...
        
        
        // -------------------- OUTPUT SECTION        
        outputs[ OUT_OUTPUT ].setVoltage( reverb_output * params[ VOL_PARAM ].getValue() );

        
        // -------------------- INVERT SECTION
        outputs[ INVERTER_OUTPUT ].setVoltage( - inputs[ INVERTER_INPUT ].getVoltage() );

        
        // -------------------- DEBUG SECTION
        
        //if (dbgTrigger.process(params[DBG_PARAM].getValue())) {
        //        
        //}

        
    }
    
    
    
    float waveFolder(float v, float normalize) {
        v *= normalize;
        float sig = (v > 0) ? 1 : -1;
        float absv = abs(v);
        float v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;

        if (absv > 0.6000) v1 = 0.8333 * v - 0.5000 * sig;
        if (absv > 2.9940) v2 = 0.3768 * v - 1.1281 * sig;
        if (absv > 5.4600) v3 = 0.2829 * v - 1.5446 * sig;
        if (absv > 1.8000) v4 = 0.5743 * v - 1.0338 * sig;
        if (absv > 4.0800) v5 = 0.2673 * v - 1.0907 * sig;

        v = -12.000 * v1 - 27.777 * v2 - 21.428 * v3 + 17.647 * v4 + 36.363 * v5 + 5.000 * v;
        //v /= normalize;
        return v;
    }
    
    
    
    /**
     * The sequencer ... five steps to heaven
     */    
    void procNoise(const ProcessArgs& args) {    
        // trigger mode for noise
        noisetrig = (int)clamp( params[ NOISETRIG_PARAM ].getValue() , 0.f, 2.f );
        bool shotnoise = false;
        switch ( noisetrig ) {
            case TRIG_SEQ :
                if ( fSeqPulse ) {
                    shotnoise = true;
                }
                break;
            case TRIG_PULSER :
                if ( fPulserPulse ) {
                    shotnoise = true;
                }
                break;
            case TRIG_KEYB :
                if ( fKeybPulse ) {
                    shotnoise = true;
                }
                break;
        }
        if ( shotnoise ) {
            noiseout = random::uniform() * UNIPEAK;
            noiseout2 = random::uniform() * UNIPEAK;
        }
        noise_output = random::uniform() * UNIPEAK - UNIPEAK/2;
        outputs[ NOISE_OUTPUT ].setVoltage( noiseout );
        outputs[ NOISE2_OUTPUT ].setVoltage( noiseout2 );
        lights[NOISE_LIGHT].setSmoothBrightness( shotnoise, args.sampleTime);
    }
    
    /**
     * The sequencer ... five steps to heaven
     */    
    void procSequencer(const ProcessArgs& args) {
        fSeqPulse = false;
        // number of steps
        stepnum = stepnum_map[ (int) clamp( params[ STEPNUM_PARAM ].getValue(), 0.f, 2.f ) ];
        if ( stepnum < 3 || stepnum > 5 ) stepnum = 5;
        // trigger mode for sequencer
        steptrig = (int)clamp( params[ STEPTRIG_PARAM ].getValue() , 0.f, 2.f );       
        bool incstep = false;
        switch ( steptrig ) {
            case TRIG_CLOCK :
                if ( fClockPulse ) incstep = true;
                break;
            case TRIG_PULSER :
                if ( fPulserPulse ) incstep = true;
                break;
            case TRIG_KEYB :
                if ( fKeybPulse ) incstep = true;
                break;
        }
        // let's advance those wild step ...
        if ( incstep ) {
            stepcurr = ( stepcurr + 1 ) % stepnum;
            if ( params[ STEPSWITCH0_PARAM + stepcurr ].getValue() > 0 ) {
                fSeqPulse = true;
            }
        }
        // set output and leds
        float stepvolt = params[ STEPFREQ0_PARAM + stepcurr ].getValue();
        outputs[ SEQ_OUTPUT ].setVoltage( stepvolt );
        for (int i = 0; i < 5; i++ ) {
            lights[ STEP0_LIGHT + i  ].setBrightness( (i == stepcurr) );
        }        
    }
    
    
    
    /**
     * External and internal clock handling
     */
    void procClock(const ProcessArgs& args) {
        fClockPulse = false;
        if (inputs[EXTCLK_INPUT].isConnected()) {
            // External clock
            if (clockTrigger.process(inputs[EXTCLK_INPUT].getVoltage())) {
                phase = 0;
                fClockPulse = true;
            }
        } else {
            // Internal clock
            float clockTime = std::pow(2.f, params[CLOCK_PARAM].getValue() );
            phase += clockTime * args.sampleTime;
            if (phase >= 1.f) {
                phase = 0;
                fClockPulse = true;
            }
        }
        lights[CLOCK_LIGHT].setSmoothBrightness( fClockPulse, args.sampleTime);        
    }
    
    
    /**
     * Keyboard handling ( voltage and trigger )
     */    
    void procKeyb( const ProcessArgs& args )  {       
        keybVolt = inputs[ KEYB_INPUT ].getVoltage();
        keybGate = inputs[ KEYBGATE_INPUT].getVoltage();
        fKeybPulse = false;
        if ( keybTrigger.process( keybGate )) {
            fKeybPulse = true;
        }
    }   
    
};

/**
 * ... should use taylor exp.
 */
float pitchToFreq( float pitch ) {
	float freq = dsp::FREQ_C4 * pow(2, pitch );                
        return freq;
}

float paramToFreq( float param ) {
	float freq = pow(2, param * 13.0 ); // [0,1] --> [0,8192] Hz
        return freq;
    
}


struct ZefiroWidget : ModuleWidget {
	ZefiroWidget(Zefiro* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Zefiro.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoundBlackKnob>(mm2px(Vec(151.0, 43.0)), module, Zefiro::CHA_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(165.5, 43.0)), module, Zefiro::CHB_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(29.354, 3.5)), module, Zefiro::CLOCK_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(92.0, 24.5)), module, Zefiro::ENV0ATCK_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(107.5, 24.5)), module, Zefiro::ENV1SUST_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(123.0, 24.5)), module, Zefiro::ENV2DECA_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(109.5, 4.942)), module, Zefiro::ENVMODE_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(92.0, 5.0)), module, Zefiro::ENVTRIG_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(28.0, 75.0)), module, Zefiro::FREQ_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(31.5, 59.0)), module, Zefiro::FREQFINE_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(17.0, 44.058)), module, Zefiro::FREQKEY_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(17.0, 60.0)), module, Zefiro::FREQLOHI_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(19.0, 75.0)), module, Zefiro::FREQMOD_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(44.0, 59.058)), module, Zefiro::FREQMODTYPE_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(33.0, 44.0)), module, Zefiro::FREQQUANT_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(44.0, 43.0)), module, Zefiro::FREQWAVE_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(106.0, 43.0)), module, Zefiro::GATE1MODE_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(126.5, 43.058)), module, Zefiro::GATE2MODE_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(106.0, 59.058)), module, Zefiro::GATE2SRC_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(117.0, 75.0)), module, Zefiro::LEV1_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(108.0, 75.0)), module, Zefiro::LEV1MOD_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(138.0, 75.0)), module, Zefiro::LEV2_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(129.0, 75.0)), module, Zefiro::LEV2MOD_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(47.0, 75.0)), module, Zefiro::MOD_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(38.0, 75.0)), module, Zefiro::MODMOD_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(2.0, 86.529)), module, Zefiro::NOISETRIG_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(73.0, 75.0)), module, Zefiro::PITCH_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(76.5, 59.0)), module, Zefiro::PITCHFINE_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(62.0, 44.0)), module, Zefiro::PITCHKEY_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(64.0, 75.0)), module, Zefiro::PITCHMOD_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(62.0, 60.0)), module, Zefiro::PITCHPOL_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(78.0, 44.0)), module, Zefiro::PITCHQUANT_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(132.0, 58.942)), module, Zefiro::PREAMPSRC_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(165.5, 24.5)), module, Zefiro::PULSEMOD_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(145.0, 6.5)), module, Zefiro::PULSEMODE_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(147.0, 24.5)), module, Zefiro::PULSEPERIOD_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(127.471, 5.0)), module, Zefiro::PULSETRIG_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(165.5, 62.0)), module, Zefiro::REVERBFBACK_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(165.5, 75.5)), module, Zefiro::REVERBMIX_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(151.0, 62.0)), module, Zefiro::REVERBTIME_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.0, 17.0)), module, Zefiro::STEPFREQ0_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(31.0, 17.0)), module, Zefiro::STEPFREQ1_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(45.0, 17.0)), module, Zefiro::STEPFREQ2_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(59.0, 17.0)), module, Zefiro::STEPFREQ3_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(73.0, 17.0)), module, Zefiro::STEPFREQ4_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(73.0, 5.0)), module, Zefiro::STEPNUM_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(17.0, 29.0)), module, Zefiro::STEPSWITCH0_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(31.0, 29.0)), module, Zefiro::STEPSWITCH1_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(45.0, 29.0)), module, Zefiro::STEPSWITCH2_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(59.0, 29.0)), module, Zefiro::STEPSWITCH3_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(73.0, 29.0)), module, Zefiro::STEPSWITCH4_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(51.5, 5.0)), module, Zefiro::STEPTRIG_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(92.0, 75.0)), module, Zefiro::TIMBRE_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(92.0, 43.0)), module, Zefiro::TIMBREMIX_PARAM));
		addParam(createParam<LEDSliderGreenLong>(mm2px(Vec(83.0, 75.0)), module, Zefiro::TIMBREMOD_PARAM));
		addParam(createParam<CKSSThree>(mm2px(Vec(89.0, 59.0)), module, Zefiro::TIMBREWAVE_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(151.0, 94.0)), module, Zefiro::VOL_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(137.5, 43.5)), module, Zefiro::EXT_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(18.5, 3.5)), module, Zefiro::EXTCLK_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(18.0, 117.0)), module, Zefiro::FREQMOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(152.0, 113.0)), module, Zefiro::INVERTER_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(2.0, 11.0)), module, Zefiro::KEYB_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(2.0, 26.0)), module, Zefiro::KEYBGATE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(107.0, 117.0)), module, Zefiro::LEV1MOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(128.0, 117.0)), module, Zefiro::LEV2MOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(37.0, 117.0)), module, Zefiro::MODMOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(63.0, 117.0)), module, Zefiro::PITCHMOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(166.5, 13.5)), module, Zefiro::PULSEMOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(152.0, 76.5)), module, Zefiro::REVERBTIMEMOD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(82.0, 117.0)), module, Zefiro::TIMBREMOD_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(2.0, 58.0)), module, Zefiro::ENV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(166.471, 113.0)), module, Zefiro::INVERTER_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(48.0, 117.0)), module, Zefiro::MOD_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(2.0, 103.0)), module, Zefiro::NOISE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(2.0, 113.0)), module, Zefiro::NOISE2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(166.471, 96.0)), module, Zefiro::OUT_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(2.0, 72.942)), module, Zefiro::PULSE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(2.0, 42.942)), module, Zefiro::SEQ_OUTPUT));
#ifdef USETEST                
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(117.0, 117.0)), module, Zefiro::TEST_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(138.0, 117.0)), module, Zefiro::TEST2_OUTPUT));
#endif
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(40.5, 5.0)), module, Zefiro::CLOCK_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(104.0, 22.0)), module, Zefiro::ENV_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.0, 72.5)), module, Zefiro::GATE1_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(129.0, 72.5)), module, Zefiro::GATE2_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.0, 101.0)), module, Zefiro::NOISE_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(144.264, 22.065)), module, Zefiro::PULSE_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.5, 30.0)), module, Zefiro::STEP0_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(39.69, 30.07)), module, Zefiro::STEP1_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(53.5, 30.0)), module, Zefiro::STEP2_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(67.5, 30.0)), module, Zefiro::STEP3_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(81.5, 30.0)), module, Zefiro::STEP4_LIGHT));
	}
};


Model* modelZefiro = createModel<Zefiro, ZefiroWidget>("Zefiro");