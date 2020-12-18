# AlgoritmarteVCVPlugin
Algoritmarte VCV Rack Modules

- [Clockkky](#clockkky)
- [Planetz](#planetz)
- [MusiFrog](#musifrog)


## <a name="clockkky"></a>Clockkky

Clockkky is a standard clock generator with three integrated 8 steps gate sequencers.

- **CLK** is the clock output, the tempo (BPM) can be changed with the top-central knob; you can also use an external clock (**EXT-CLK** input)
- the **RUN** button starts/stops the clock

- On the bottom there are three independent *tracks* **T1**, **T2** and **T3** with 8 *steps* each. Press the switches to activate/deactivate the steps. At each clock tick, the *current step* of each track is incremented and if it is active a pulse is generated in the corresponding output **T1**, **T2** or **T3**. 

- The number of steps of each track can be changed using the three central knobs (values from 0 to 8).

- In the default *mode* **3x8** the three tracks are independent, but you can also join them and get a 24 steps sequence using the **1x24** mode. In this mode only output **T1** is active.

- The **RST** button (or the associated input) can be used to restart the sequencer


![Clockkky](thumbClockkky.png)

## <a name="planetz"></a>Planetz

**Planetz**, is a *Random sequence generator* which is based on five planets rotating around each other and around a central star.

The distance between a planet and the previous one is controlled by the **RAY** parameter. Its initial angle is controlled by the **ANGLE** parameter ; and the planet speed is controlled by the **SPEED** parameter (the rotating speed is in degrees per second).

You can select two planets (using the two bottom left knobs) and send their coordinates to outputs **X1,Y1** and **X2, Y2**.

Using the **SCALE XY** knobs the output values can be scaled.

The **RST** input and button can be used to restart the sequence (to use it in a deterministic way).

Interesting **generative* results can be achieved connecting one of the four coordinates (or a combination of them) to one or more oscillators (to the  1V/octave input but also to the FM input) or to one or more quantizers in which similar scales are selected.

Load the ``planetz_example.vcv`` in the example folder for a simple demo.

![Clockkky](thumbPlanetz.png)


## <a name="musifrog"></a>MusiFrog

The MusiFrog is a pseudo-random (deterministic) quantized sequence generator.

There are 16 _stones_, each stone has a *jump value*. Each stone also corresponds to a *note* from the current selected 16 notes *scale*. Initially the frog is on stone 1; suppose that it contains the jump value X. When an pulse is received on the **CLK** input, the first note of the scale is played, then the frog jumps forward at stone N = 1+X and the jump value of stone 1 is increased by 1. At the next pulse the N-th note of the scale is played, the frog jumps at stone N'=N+Y (where Y is the current jump value of stone N), and the jump value of stone N is increased by 1. The process is repeated at each impulse. When the frog jumps off the last stone it "re-enters" at the beginning of the sequence.

The algorithm is described in detail at:

[https://www.algoritmarte.com/musifrog/](https://www.algoritmarte.com/musifrog/)    

### Inputs/Outputs/Parameters

- the **CLK** input must be conncted to an external clock generator; the frog will make a jump at each pulse;

- the **OUT** output is the quantized pitch of the current note (1 Volt/octave);
- the **TRIG** output is triggered at every jump (note);

- the **STEPS** knob can be used to set the number of stones (1 to 16);
- the 16 **JUMP SEQUENCE** knobs can be used to set the _initial jump values_; 
- the **SCALE** knob can be used to select one of the 4 scales available. There is also an associated input to change the scale using a signal (the voltage is summed to the knob value: 0Volt=ScaleKnob, 1Volt=ScaleKnob+1, 2Volt=ScaleKnob+2,...). The 4 default scales are: 1= C ionian, 2= A aeolian, 3=pentatonic up/down, 4=E phrygian up/down. 

- the **OFFSET** knob can be used to control the base note of the scale;
- when the **Hold Same** switch is active, if the frog jump on a note that is equal to the last note played, then the trigger signal is not generated;

- the **RST** switch and the associated input can be used to reset the sequence: the frog is positioned on the first stone and all stones are filled with the initial *jump values* (those specified by the 16 knobs).

It is also possible to edit/change the scales by putting the bottom right switch on **PRG**. In this mode the 16 notes of the scale are played sequentially (no jumps) using an internal clock at 120bpm; and the notes can be changed in this way:

- the STEPS knob selects the octave (max 3 octaves)
- the 16 JUMP knobs selects the pitch:
  - 0 = silence (when the frog jump to a stone corresponding to a _silence_, no note is played). The four default scales don't contain silences.
  - 1 = C
  - 2 = C# / Db
  - 3 = D
  - 4 = D# / Eb
  - 5 = E
  - ...
  - 10 = A# / Bb
  - 11 = B

!!! Notice that the notes of the scale (and the octave) are not changed unless the corresponding knob is moved !!!

After editing the scale(s) you can return to **RUN** mode and re-adjust the *jump values*.

Load the ``musifrog_example.vcv`` in the example folder for a simple demo.

![MusiFrog](thumbMusiFrog.png)
