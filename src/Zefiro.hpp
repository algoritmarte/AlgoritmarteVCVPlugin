/* 
 * File:   Zefiro.hpp
 * Author: marziodebiasi
 *
 * Created on 30 dicembre 2020, 12.03
 */
#include "plugin.hpp"

#ifndef ZEFIRO_HPP
#define ZEFIRO_HPP



#define UNIPEAK          10.0f
#define BIPPEAK          5.0f

#define WTLEN 88200
#define FWTLEN 88200.f

#define WTSIN 0
#define WTTRI 1
#define WTSQR 2
#define WTSAW 3

#define MAXOSCFREQ 5000
#define MAXOUTFREQ 5000

#define DELAYBUFLEN 88200

#define REVERBTIMEMIN 5.f
#define REVERBTIMEMAX 1500.f
#define REVERBTIMEDEF 200.f

#define WETATT 5

float pitchToFreq( float pitch ); 
float paramToFreq( float param );

/**
 * Simple ADSR
 */
struct WestADSR {
    float output = 0;
    int phase = 0;
    float target_value = UNIPEAK;
    float attack_time = 500;
    float decay_time = 500;
    float sustain_perc = 1;
    float sustain_time = 500;
    float release_time = 500;
    float phasetime = 0;
    bool auto_sustain = true;
    bool ended = false;
    
    WestADSR() {
    }
    
    void trigger() {
        phase = 1;
        if ( target_value > 0) {
            phasetime =  attack_time * output / target_value;
        } else { 
            phasetime = 0;
        }
        ended = false;
    }
    
    void process( float deltams, bool gate, bool autoretrig ) {
        ended = false;
        attack_time = abs( attack_time );
        decay_time = abs( decay_time );
        if ( sustain_perc < 0 ) sustain_perc = 0;
        if ( sustain_perc > 1 ) sustain_perc = 1;
        release_time = abs( release_time );
        
        if ( phase == 0 ) {
            phasetime = 0;
            output = 0;
            return;
        }
        
        phasetime += deltams;
        
        if ( phase == 1 ) { // attack
            if ( phasetime > attack_time || attack_time == 0 ) {
                phase++;
                phasetime = phasetime - attack_time;
            } else {
                output = target_value * ( phasetime / attack_time ) ;
            }            
        }
        if ( phase == 2 ) { // decay
            if ( phasetime > decay_time || decay_time == 0 ) {
                phase++;
                phasetime = phasetime - decay_time;
            } else {
                output = target_value - target_value * ( 1 - sustain_perc ) * ( phasetime / decay_time );
            }                        
        }        
        if ( phase == 3 ) { // sustain            
            if ( auto_sustain ) {
                if ( gate ) {
                    phasetime = 0;
                    //output = target_value * sustain_perc;
                } else {
                    if ( phasetime > sustain_time || sustain_time == 0 ) {
                        phase++;
                        phasetime = phasetime - sustain_time;
                    } else {
                        //output = target_value * sustain_perc;
                    }
                }
            } else {
                if ( ! gate ) {
                    phase++;
                    phasetime = 0;                    
                } else {
                    //output = target_value * sustain_perc;                    
                }                
            }            
        }            
        if ( phase == 4 ) { // release
            if ( phasetime > release_time || release_time == 0 ) {
                phasetime = 0; //release_time - phasetime;
                output = 0;
                ended = true;
                if ( autoretrig ) {
                    phase = 1;                                        
                } else {
                    phase = 0;
                }
            } else {
                output = ( release_time == 0 )? 0 : ( target_value * sustain_perc * ( 1  -  ( phasetime / release_time )) );
            }                                    
        }
        if ( output < 0 ) output = 0;
        if ( output > target_value ) output = target_value;
    }    
};
typedef struct WestADSR WestADSR_t;

/**
 * Struct for longer sliders ...
 */       
struct LEDSliderGreenLong : app::SvgSlider {
	LEDSliderGreenLong() {
		maxHandlePos = app::mm2px(math::Vec(0.738, 0.738).plus(math::Vec(2, 0)));
		minHandlePos = app::mm2px(math::Vec(0.738, 22.078 * 1.6).plus(math::Vec(2, 0)));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/LEDSliderLong.svg")));            
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderGreenHandle.svg")));
	}
};

/**
 * Oscillator based on a prebuilt wave table ...
 */
struct WestWave {
    float wt[ WTLEN ] = {};
    int wavetype = 0;
    float freq = 20;
    float output = 0;
    float wtindex = 0;
    float wtamp = 5.f;
    
    WestWave() {

    }
  
    void setPitch(float pitch) {
	freq = dsp::FREQ_C4 * pow(2, pitch );                
        if ( freq > MAXOUTFREQ ) freq = MAXOUTFREQ;
    }
        
    void prepare( int awavetype ) {
        output = 0;
        float f;
        int off;
        wavetype = awavetype;
        switch ( wavetype ) {
            
            case WTSIN :               
                off = WTLEN / 2;
                for (int i = 0; i < off; i++) {
                    wt[i] = std::sin(1.0 * i * (2 * M_PI) / WTLEN);
                    wt[i+off] = 0 - wt[i];
                };
                break;
                
            case WTTRI : 
                off = WTLEN / 4;
                f = off;
                for (int i = 0; i < off; i++) {
                    float f2 = 1.0 * i / f;
                    wt[i] = f2;
                    wt[i+off] = 1.0 - f2;
                    wt[i+2*off] =  - f2;
                    wt[i+3*off] = -1.0 + f2;
                };
                break;
                
            case WTSAW : 
                off = WTLEN / 2;
                f = off;
                for (int i = 0; i < off; i++) {
                    float f2 = 1.0 * i / f;
                    wt[i] = f2;
                    wt[i+off] = -1.0 + f2;
                };
                break;                

            case WTSQR : 
                off = WTLEN / 2;
                f = off;
                for (int i = 0; i < off; i++) {
                    wt[i] = 1.0;
                    wt[i+off] = -1.0;
                };
                break;                                
        }        
    }
    
    
    void process( float deltasec ) {
            if ( wtindex < 0 || wtindex >= WTLEN ) wtindex = 0;
          
            output = wt[ (int)(round(wtindex)) ] * wtamp;
            float delta =  deltasec * FWTLEN / ( 1.0 / freq );
            
            wtindex += delta;
            if ( wtindex >= WTLEN ) wtindex -= WTLEN;        
    }
    
};
typedef struct WestWave WestWave_t;


/**
 * Filter ... yet unusable ... using the dsp::RCFilter
 */
struct WestFilter {
// trash ... poor filter 
    float output = 0;
    float freq  = 0;
    float alpha = 0;
    float test;    
    void setFreq( float afreq, float deltas ) {
        if ( freq != afreq ) {
            freq = afreq;
            test = deltas; 
            alpha = 1.0 - exp ( - freq * deltas  ); //(freq / 2093.00 )    ) ; // 1046.50
        } 
    }    
    void process( float input ) {
        if (freq <= 2) input = 0;
        output += alpha * (input - output );
    }
};
typedef struct WestFilter WestFilter_t;


/**
 * Simple delay ... should be a spring reverb
 */
struct WestDelay {
    
    float buf[ DELAYBUFLEN ] = {};
    
    int bufindex = 0;
    float wet = 0;
    float output = 0;
    float delaytime = 0;
    int delayoff = 0;
    float lastwet[ WETATT ] = {};
    
    
    void setTempo( float amillis, int samplerate ) {
        if ( amillis != delaytime ) {
            delaytime = amillis;            
            if ( delaytime < REVERBTIMEMIN ) delaytime = REVERBTIMEMIN;
            delayoff = (int)(round( (samplerate * delaytime / 1000.) ));
            if ( delayoff < 0 ) delayoff = 0;
            if ( delayoff >= DELAYBUFLEN ) delayoff = DELAYBUFLEN - 1;
        }        
    }
    
    void process( float value, float fback, float mix ) {
        float twet = buf[ ( bufindex + DELAYBUFLEN - delayoff ) % DELAYBUFLEN ];
        wet = twet;
        for ( int i = 0; i < WETATT; i++ ) {
            wet += lastwet[i];
        }
        wet /= 6.0;
        buf[ bufindex ] =  value + wet * fback;
        for ( int i = 0; i < WETATT-1; i++ ) {
            lastwet[i] = lastwet[i+1];
        }
        lastwet[ WETATT - 1 ] = buf[ bufindex ];
        output = value * ( 1 - mix ) + wet  * mix;
        bufindex = (bufindex + 1) % DELAYBUFLEN ;
    }
       
};


struct WestBuf {
    float queue[8] = {};
    float output = 0;
    
    void process( float input, int queuelen ) {
        queuelen = clamp( queuelen, 1, 7 );
        queue[ queuelen ] = input;
        output = input;
        for (int i = 0; i < queuelen; i++) {
            output += queue[i];
            queue[i] = queue[i+1];
        }
        output /= queuelen + 1;
    }
};



#endif /* ZEFIRO_HPP */
