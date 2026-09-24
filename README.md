# Logic Analyzer Tester

Raspberry Pi Pico based pattern generator for testing up to 24 channel logic analyzers.

## Channel map

| Ch | GPIO | Pin | Fan freq | Ch | GPIO | Pin | Fan freq |
|---:|:----:|----:|---------:|---:|:----:|----:|---------:|
| 1 | GP0 | 1 | 5000.00 Hz | 13 | GP12 | 16 | 384.62 Hz |
| 2 | GP1 | 2 | 2500.00 Hz | 14 | GP13 | 17 | 357.14 Hz |
| 3 | GP2 | 4 | 1666.67 Hz | 15 | GP14 | 19 | 333.33 Hz |
| 4 | GP3 | 5 | 1250.00 Hz | 16 | GP15 | 20 | 312.50 Hz |
| 5 | GP4 | 6 | 1000.00 Hz | 17 | GP16 | 21 | 294.12 Hz |
| 6 | GP5 | 7 | 833.33 Hz | 18 | GP17 | 22 | 277.78 Hz |
| 7 | GP6 | 9 | 714.29 Hz | 19 | GP18 | 24 | 263.16 Hz |
| 8 | GP7 | 10 | 625.00 Hz | 20 | GP19 | 25 | 250.00 Hz |
| 9 | GP8 | 11 | 555.56 Hz | 21 | GP20 | 26 | 238.10 Hz |
| 10 | GP9 | 12 | 500.00 Hz | 22 | GP21 | 27 | 227.27 Hz |
| 11 | GP10 | 14 | 454.55 Hz | 23 | GP22 | 29 | 217.39 Hz |
| 12 | GP11 | 15 | 416.67 Hz | 24 | GP26 | 31 | 208.33 Hz |


Set the logic analyzer's input reference to 3.3V and connect its ground to the Pico's GND pins. 

## Patterns

Connect to the Pico's USB serial port at 115200 baud and enter a pattern key.
The selected pattern runs until a different pattern key is entered.

| Key | Pattern | Rate | Post samples |
|:---:|---------|-----:|-------------:|
| 1 | all low | 10 kHz | 5,000 |
| 2 | all high | 10 kHz | 5,000 |
| 3 | all hi-Z | 10 kHz | 5,000 |
| 4 | walking ones | 200 kHz | 10,000 |
| 5 | frequency fan | 100 kHz | 20,000 |
| 6 | auto, 1-5 at 2 s each | | |
| ? | help and frequency table | | |

**1, all low.** All channels are driven low. A channel that reads high is stuck high or shorted to a rail.

**2, all high.** All channels are driven high. A channel that reads low is stuck low or disconnected.

**3, all hi-Z.** All pins are released to inputs and no channel is driven. Channels are held low by the analyzer's input pull-downs.

**4, walking ones.** All pins are released, then each channel is driven high for 1 ms, from CH1 to CH24 taking 24 ms per pass.
Exactly one channel is high at any time; two channels high together indicate a short between them. 
An incorrect order indicates swapped wiring.
Idle channels are released and not driven low, so a bridged input follows the driven channel with no contention between outputs. 


**5, frequency fan.** Channel n toggles every n intervals, with a 100 µs timebase and f=5000/n Hz.
Each channel carries a unique frequency, so misrouted channels can be identified by their captured frequency.
Capture at 100 kHz or above in this mode.

**6, auto.** Cycles patterns 1 to 5 for 2 s each.

**?** Prints the command list and frequency table.

Every pattern change emits a sync pulse; all channels are high for 500 µs and then low.
Set the trigger on a rising edge of CH1, arm the logic analyzer capture and then enter the pattern key.