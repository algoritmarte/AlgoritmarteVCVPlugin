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
