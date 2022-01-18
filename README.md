# alvarion su control program

This project aims to repurpose Alvarion Breezenet units as transvertor for HAM usage.

It contains information how I reverse engineered the control protocol for the CPLD. What needs to be changed is in the annotated image in alvarion directory.


Control program is written for Bluepill (ORIGINAL!!!) STM32 processor. Magic with 2 timers and DMA is used to generate and modulate 18MHz signal.


THIS IS WORK IN PROGRESS. There is currently ONE - signle unit that has been repurposed.

## what you will need


* Soldering iron and preheat plate - the PCB is VERY thick and very good heat conductor.
* smd soldering skills
* 2.2pF 0603 NPO capacitor
* 3400MHz bandpass filter for TX part (i sourced some from ebay)
* RF relay - i used one with latching electronics inside - controlled by the existing RX/TX signals
* RF cabling for the relay.

* bluepill with stm32f1, the original one. I mean it! This software uses some parts of stm32 chip that do not work on clones!
* signal triplexer - to combine DC + 440MHz IF + control signal (i will publish my schematic/board design soon)

* patience! - much of it :)


## Rework

1. Open IMGP4630-annotated.png image and zoom in at the center, there is the MAIN PLL uncovered, you need to add the 2.2pF capacitor where marked.

2. Middle bottom is the TX filter you need to replace.

3. Remove the antenna diplexer and replace it with relay 

I have replaced cable in the antenna with SMA connectored one, and router the antenna directly to the RF relay.
Control signals can be taken near the CPLD - look at the image. These signals control the RF switch near IF input MCX connector.
