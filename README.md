# MARKNES

Yet another NES emulator written in C++.

Ever since my childhood years, I've always been fascinated with the NES console. It's always been my interest to learn and know the wonderful engineering of how this piece of hardware works. I started this project some time in the first quarter of 2020, working only on my weekends; the global pandemic also helped me stay on track with this project. See [Code Development](#code-development) section of this page for some details in my implementation.


## Screenshots
![supermario](https://raw.github.com/mark-unat/marknes/main/res/supermario.png)
![contra](https://raw.github.com/mark-unat/marknes/main/res/contra.png)
![battlecity](https://raw.github.com/mark-unat/marknes/main/res/battlecity.png)
![excitebike](https://raw.github.com/mark-unat/marknes/main/res/excitebike.png)
![kungfu](https://raw.github.com/mark-unat/marknes/main/res/kungfu.png)
![megaman](https://raw.github.com/mark-unat/marknes/main/res/megaman.png)


## Dependencies

**Graphics:** Using OpenGL

    sudo apt-get install freeglut3-dev

**Audio:** Using OpenAL

    sudo apt-get install libopenal-dev

**NOTE**: I've only tried building this code on a Linux platform.


## Building

From the root directory:

    make marknes


## Usage

    marknes romfile.nes

Example: `marknes supermario.nes`


## Controls

NES USB Joysticks are supported.
Keyboard controls are mapped below (but only Player 1 was mapped):

| Nintendo              | Emulator    |
| --------------------- | ----------- |
| Up, Down, Left, Right | Arrow Keys  |
| Start                 | L           |
| Select                | K           |
| A                     | O           |
| B                     | P           |


## Supported Mappers

The following mappers are supported:

* NROM  (Mapper 000)
* UxROM (Mapper 002)

Looking to support more mappers in the future.


## Code Development

The CPU bus and memory were first implemented, followed by emulating the NES CPU (MOS 6502 8-bit processor) with its official opcodes and address modes. Also ported the infamous CPU hardware bug in absolute indirect addressing, where it cannot correctly read two bytes from a certain address (of the form 0x--FF). This module has also been been tested successfully with a NES CPU Test ROM found in https://www.qmtpro.com/~nes/misc/nestest.txt.

The NES PPU, or Picture Processing Unit was the next one I've implemented and this is the most (subjectively) complicated module among the rest to implement (at least for me). This is Ricoh's 2C02 processor running at 5.37MHz (NTSC), 3 times faster than CPU, more like a graphics card nowadays. The rendering of a frame is a tricky part to emulate where you have to follow the frame timing diagram or else you will be seeing corrupt artifacts from the screen, with pixels coming from the pattern/name table. This module also takes care of the Sprite Zero hit detection and Sprite overflow detection (where I also ported the hardware bug of checking the remaining primary OAM buffer in a weird way). Lastly, I've used OpenGL/GLUT library to render graphics from PPU to the screen.

Adding NES controller is fairly straight forward, it's just a matter of mapping button inputs from either joystick/keyboard to the Controller1/2's address (0x4016/0x4017). I've used the Linux joystick device (/dev/input/jsX) to interface with the NES Joystick.

As of the time of this writing, I've only implemented two Mappers (000/NROM and 002/UxROM), and this is also fairly easy to implement as this is an all or nothing implementation, either it works to support that game or it won't, so it would be easy and fast to test. Looking forward to support more mappers in order to support more complex games, I'm looking at you MMC3 since I want the SMB3 game.

Next big piece of the puzzle to implement is what adds life to the game, the Audio Processing Unit or APU. This is Ricoh's 2A03 processor, running half the speed of the CPU. At first I was having a hard time to implement this as the audio processing/playing (happening in real-time) needs to be in-sync on every APU tick (which needs to be cycle accurate). Since my NES graphics (PPU tick) is driven from the refresh rate of the OpenGL library, which every CPU/APU ticks were also derived from, I ended up not using the sound timing given from every APU tick, instead I have separate thread that continuously poll audio samples from the APU with the given time (based on sampling rate), and from there I can manually generate the waveform based on the current frequency/period and amplitude. I've used OpenAL for my audio library. The APU has five (5) channels: 2 pulse waves (which is more like the melody line), triangle wave (usually the bass part of the song), noise wave (can be used for percussion), and the Delta Modulation Channel (or DMC) which I think plays a sampled audio like a voice or some unique sound. As to date, I have only implemented 3 channels which are the pulse waves and triangle wave. Looking forward to implement noise and DMC channels in the future.

Lastly, big thanks to the NES Wiki Page: https://wiki.nesdev.com for an amazing NES reference guide. As well as to the following repositories for my code references: https://github.com/OneLoneCoder/olcNES and https://github.com/kanathan/plainNES. I couldn't have completed this project without those valuable references.


## Support

For any questions/bugs/support, please contact me at markunat@gmail.com.
