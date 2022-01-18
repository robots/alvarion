# various infomration from reverseengineering


la  contain traces of control signal from internal unit to the transverter. Captured using OLS - https://www.lxtreme.nl/ols/

la2 is python program to gather different bitstreams from the captured data. Used to extract all possible commands.


## PLL ranges
|        |           |
|--------|-----------|
|MAIN pll|1849 - 2117|
|+2.2pF  |1769 - 2054|
|RX pll  |1027 - 1176|
|TX pll  | 916 - 1076|



IF 440 Mhz CW (unit present), signal at 437.907Mhz

## IF calculations for HAM band

3400 - 1575(rx if filter) = 1825

3400 - 1472(tx if filter) = 1928


1575 - 440(if) = 1135

1472 - 440(if) = 1032


# Below is Reverse engineering work

## pinout of the 20x2 pin connector near CPLD

#### outer edge
|   |                                |
|---|--------------------------------|
|1  |                                |
|2  |                                |
|3  |                                |
|4  | IF to TX switch                |
|5  | IF to RX switch                |
|6  |                                |
|7  | RXPLL AUX vco enable (440Mhz)  |
|8  |                                |
|9  |                                |
|10 |                                |
|11 |                                |
|12 |                                |
|13 | 500Mhz pll data                | 
|14 | 500Mhz pll clk                 |
|15 | 500Mhz pll RX le               |
|16 | 500Mhz pll TX le               |
|17 | 1.9GHZ PLL le                  |
|18 | 1.9GHZ PLL data                |
|19 | 1.9GHZ PLL clk                 |
|20 |

#### inner edge
|   |                                |
|---|--------------------------------|
|1  | bias 1                         |
|2  | bias 2                         |
|3  | bias 3                         |
|4  | bias 4                         |
|5  | vin                            |
|6  | vin                            |
|7  | vin                            |
|8  | vin                            |
|9  | GND                            |
|10 | GND                            |
|11 | GND                            |
|12 | GND                            |
|13 |                                |
|14 |                                |
|15 |                                |
|16 |                                |
|17 |                                |
|18 |                                |
|19 |                                |
|20 |                                |


## watchdog:
WDI - 1Mhz, stops when transmission in progress


## bit sequences (initial analysis):
timing ~1Mhz

unknown:

1 + x*0 x>30

11 + x*0

1101

101 - reset?

101000011

1101000001

111110001111011


111101000010100100011
     
111101000000000000000

#### Write to lmx2326:
11110001 00 000000000000010010010

writes to pll then strobes LE

         00 0000000000000 1 001 0 0 10

testmode, pwrdn mode, testmodes, timeout cntr, CP-tristate = 0

phase detector polarity = 1,

FoLD = 100 - Digital lock detect

powerdown, counterreset = 0


11110001 00 000000000000001010000

writes to pll then strobes LE

         00 0 0000 00000000010100 00

LD precision = 0

testmodes = 0

counter R = 20

11110001 00 100000001111000100001

writes to pll then strobes LE

         00 1 0000000111100 01000 01
GO = 1

counter A = 8

counter B = 60


111000 0000 100000001111001011101 1111111111001111 + x*0

strobes LE, writes data

       0000 1 0000000111100 10111 01

ledbar = 1st ledon

wlnk = off

eth = off


111000 0010 000000000110111000001 0001010010001100 + x*0

strobes LE, writes data

       0010 0 0000000011011 10000 01


#### write to TXPLL (lmx1601):

111100000 1000 101100000001010010

writes to TX pll, strobe LE at end

          1000 1011 000000010100 10

cp word = 1011

main R = 20


111100000 1000 000001000000100011

writes to TX pll, strobe LE at end

          1000 000001000000 1000 11

main A = 8

main B = 64


#### write to RXPLL (lmx1601):

111100001 0000 101100000001010010

writes to RX pll, strobe LE at end

          0000 1011 000000010100 10

cp word = 1011

main R = 20

111100001 0000 000001000110110011

writes to RX pll, strobe LE at end

          0000 000001000110 1100 11

main A = 12

main B = 70


111100001 0000 001000000001010000

writes to RX pll, strobe LE at end

          0000 0010 000000010100 00

FoLD = 0010 - main lock detect

aux R = 20


111100001 0000 000000110111000001

writes to RX pll, strobe LE at end

          0000 000000110111 0000 01
aux A = 0

aux B = 55 (out of specs!)


## Frequencies:
#### TX:

IF = 440Mhz

Fo = N * (Fin / R)

N = (P * B) + A

P = 16

Fo_txpll = 1032Mhz


IF2 = IF + Fo_txpll = 1472Mhz

Fout = IF2 + IF3 = 3.xxxGhz


#### RX:

IF = 440Mhz

Fo = N * (Fin / R)

N = (P * B) + A

P = 16

Fo_rxpll = 1132Mhz

IF2 = IF + Fo_rxpll = 1572Mhz

#### RXaux:
Fo = N * (Fin / R)

N = (P * B) + A

P = 8

Fo_auxrxpll = 440Mhz

#### IFpll:
Fin = 20.0000Mhz

Fif = N * (Fin / R)

N = (P * B) + A

P = 32

Fif = ~1928Mhz

