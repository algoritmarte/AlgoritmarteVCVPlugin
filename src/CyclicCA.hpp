/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CyclicCA.hpp
 * Author: marziodebiasi
 *
 * Created on 29 gennaio 2021, 21.21
 */

#ifndef CYCLICCA_HPP
#define CYCLICCA_HPP

#include "nanovg.h"
#include "plugin.hpp"

#define DISPLAY_WIDTH  360
#define DISPLAY_HEIGHT 360
#define MAX_STATES 32
#define MAX_RULES   8

#define MINDELTA -3
#define MAXDELTA  3

#define DEFNUMRULES 4

#define MINNUMSTATES 2.f
#define MAXNUMSTATES 26.f
#define DEFNUMSTATES 18.f

#define DEFCELLSIZE 2.f
#define MINCELLSIZE 1.f
#define MAXCELLSIZE 6.f

#define MINBLOCKSIZE 1.f
#define MAXBLOCKSIZE 50.f
#define DEFBLOCKSIZE 10.f
#define DEFBLOCKSIZEINT 10

// 21 - speed
#define MINSPEED 0.f
#define MAXSPEED 31.f
#define DEFSPEED  25.f
#define DEFSPEEDINT 25

#define MINNEIGH 0.f
#define MAXNEIGH 48.f
#define DEFNEIGH0 17.f
#define DEFNEIGH1 23.f
#define DEFNEIGH2 25.f
#define DEFNEIGH3 31.f


int irand(int min, int max) {
    return std::rand() % (max + 1 - min) + min;
}

struct CCA {
        int cagrid[2][DISPLAY_WIDTH][DISPLAY_HEIGHT] = {};
        int cellsize = DEFCELLSIZE;
        int cawidth, caheight;
        int caframe = 0;
        int carules[MAX_RULES][2] = {};
        int numstates = DEFNUMSTATES;
        int numrules = DEFNUMRULES; 
        float cacolsfade = 0;
        int cacolscurr[MAX_STATES] = {};
        int cacols[MAX_STATES] = {};
        
        int blocksize = DEFBLOCKSIZEINT;
        int caspeed = DEFSPEEDINT;
        
        int colindex = 0;
        
        bool sigRandRule = false;
        bool sigReset = false;
        bool sigRandBlock = false;
        bool sigChangeSize = false;
        
        int newsize;
        
        int blockshape;
        
        float out1 = 0, out2 = 0;
        
        
        void initCA() {
            cellsize = (int)DEFCELLSIZE;
            cawidth = DISPLAY_WIDTH / cellsize;
            caheight = DISPLAY_HEIGHT / cellsize;
                
            carules[0][0] = 1;
            carules[0][1] = 0;
            carules[1][0] = -1;
            carules[1][1] = 0;
            carules[2][0] = 0;
            carules[2][1] = 1;
            carules[3][0] =  0;
            carules[3][1] = -1;
                          
            numstates = DEFNUMSTATES;
            randomizeCAPalette();
            randomizeCAGrid();
        }
        
        int rgbColor( float h, float s, float l  ) {
            NVGcolor cl = nvgHSL( h,  s,  l);
            int r = round( 255 * cl.r );
            int g = round( 255 * cl.g );
            int b = round( 255 * cl.b );
            return 0xff000000 | ( r << 16 ) | ( g << 8 ) | ( b );
            
        }
        
        void pushColor( float h, float s, float l ) {
            
            cacolscurr[ colindex ] = rgbColor( h, s, l );
            colindex = ( colindex  + 1 ) % numstates;
        }
        
        
        void randomizeCAGrid() {
            for (int x = 0; x < cawidth; x++) {
                for (int y = 0; y < caheight; y++) {
                    cagrid[caframe][x][y] = irand( 0, numstates - 1);
                }                
            }        
        }
        
        void randomizeRule() {
            int n = irand( 0, numrules-1 );
            carules[n][0] = irand( MINDELTA, MAXDELTA );
            carules[n][1] = irand( MINDELTA, MAXDELTA );            
        }
        
        void randomizeCAPalette()  {
            int n = 0;
            while (n < MAX_STATES ) {                
                int z = irand(1,3);
                int c = rgbColor( 1.0 * (std::rand() % 100) / 100.0 , 0.9, 0.5 );
                //0xff000000 | (( irand(0,255)) << 16) | (( irand(0,255)) << 8) | (( irand(0,255)) << 0);
                //c = 0xffff0000; // blue
                for (int i = 0; i < z && n < MAX_STATES; i++ ) {
                    cacols[n] = c;
                    cacolscurr[n] = cacols[n];
                    n++;
                }
            }
        }        
        
        void randomizeBlock() {
            int x0 = irand( 0, cawidth-1 );
            int y0 = irand( 0, caheight-1 );            
            int w = blocksize * caheight / 100;            
            for (int x = -w; x < w; x++) {
                for (int y = -w; y < w; y++) {     
                    bool f = (blockshape == 0)? ( abs(x) + abs(y) < w ) : ( abs(x) < w || abs(y) < w );
                    if ( f ) {                        
                        cagrid[caframe][ (cawidth + x + x0) % cawidth ][ (caheight + y + y0) % caheight  ] = irand(0, numstates - 1);                        
                    }
                }
            }
        }
        
        void procCAParams() {
            
            if ( sigRandRule ) {
                sigRandRule = false;
                randomizeRule();
            }            
            if ( sigReset ) {
                sigReset = false;
                randomizeCAGrid();
            }                  
            if ( sigRandBlock ) {
                sigRandBlock = false;
                randomizeBlock();
            }
            if ( sigChangeSize ) {
                sigChangeSize = false;
                cellsize = newsize;
                cawidth = DISPLAY_WIDTH / cellsize;
                caheight = DISPLAY_HEIGHT / cellsize; 
                randomizeCAGrid();
            }
            
        }
        
        void evolveCA() {
            
            int w1 = cawidth / 3;
            int w2 = 2 * cawidth / 3;
            int strip = caheight / 2;
            float t1 = 0, t2 = 0;
            
            int f2 = (caframe + 1) % 2;
            for (int x = 0; x < cawidth; x++) {
                for (int y = 0; y < caheight; y++) {
                    
                    /*
                    if ( x == 0 || x == cawidth - 1 || y == 0 || y == caheight-1) {
                        cagrid[f2][x][y] = irand( 0, numstates - 1 );
                        continue;
                    }
                     */
                    
                    int v = cagrid[caframe][x][y];
                    int v2 = ( v + 1 ) % numstates;
                    for (int i = 0; i < numrules; i++) {
                        
                        int x0 = ( DISPLAY_WIDTH + x + carules[i][0] ) % DISPLAY_WIDTH; 
                        int y0 = ( DISPLAY_HEIGHT + y + carules[i][1] ) % DISPLAY_HEIGHT;
                        
                        if ( cagrid[caframe][x0][y0] == v2 ) {
                            v = v2;
                            break;
                        }                        
                    }
                    cagrid[f2][x][y] = v;

                    if ( x == w1 && y < strip) t1 += v;
                    if ( x == w2 && y < strip) t2 += v;
                }
            }
            caframe = f2;
            
            out1 = t1 / (strip * numstates);
            out2 = t2 / (strip * numstates);
        }

};
typedef struct CCA CCA_t;





#endif /* CYCLICCA_HPP */

