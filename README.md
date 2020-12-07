# AlgoritmarteVCVPlugin
Algoritmarte VCV Rack Modules

The first module is **Clokkky**, a simple *Clock generator* and *Gate sequencer*

## Clokkky

Clokkky is a standard clock generator with three integrated 8 steps gate sequencers.

- **CLK** is the clock output, the tempo (BPM) can be changed with the top-central knob; you can also use an external clock (**EXT-CLK** input)
- the **RUN** button starts/stops the clock

- On the bottom there are three independent *tracks* **T1**, **T2** and **T3** with 8 *steps* each. Press the switches to activate/deactivate the steps. At each clock tick, the *current step* of each track is incremented and if it is active a pulse is generated in the corresponding output **T1**, **T2** or **T3**. 

- The number of steps of each track can be changed using the three central knobs (values from 0 to 8).

- In the default *mode* **3x8** the three tracks are independent, but you can also join them and get a 24 steps sequence using the **1x24** mode. In this mode only output **T1** is active.

- The **RST** button (or the associated input) can be used to restart the sequencer


![Clockkky](thumbClockkky.png)

## Planetz

The second module is **Planetz**, a *Random sequence generator* which is based on five planets rotating around each other and around a central star.

The distance between a planet and the previous one is controlled by the **RAY** parameter. Its initial angle is controlled by the **ANGLE** parameter ; and the planet speed is controlled by the **SPEED** parameter (the rotating speed is in degrees per second).

You can select two planets (using the two bottom left knobs) and send their coordinates to outputs **X1,Y1** and **X2, Y2**.

Using the **SCALE XY** knobs the output values can be scaled.

The **RST** input and button can be used to restart the sequence (to use it in a deterministic way).

Interesting **generative* results can be achieved connecting one of the four coordinates (or a combination of them) to one or more oscillators (to the  1V/octave input but also to the FM input) or to one or more quantizers in which similar scales are selected.

Load the ``planetz_example.vcv`` in the example folder for a simple demo.

![Clockkky](thumbPlanetz.png)
