Nocturnal Encoder
=================

The Nocturnal Encoder plugin for VCV Rack contains the NE-1 Amplitude Modulation
Decoder module and the NE-2 Amplitude Modulation Encoder module. The Nocturnal
Encoder plugin was written by David Peterson.

This plugin exists to provide a low-cost connection between hardware Eurorack
synthesizers and VCV Rack.

The Nocturnal Encoder plugin is a free, open-source, and released under license.
Part of it is based on code written by Andrew Belt for the VCV Fundamental and
VCV Befaco plugins, which is used in compliance with Andrew’s license. Thanks
very much, Andrew!

![Nocturnal Encoder plugin](https://github.com/djpeterso23662/NocturnalEncoder/blob/master/res/NocturnalEncoder_Family.jpg)

NE-1 Amplitude Modulation Decoder
---------------------------------

The NE-1 Amplitude Modulation Decoder module provides two complete, independent
channels. The NE-1 converts an amplitude-encoded CV signal back into a standard
VCV Rack CV signal. An audio signal enters the module through either of the IN 1
or IN 2 jacks. The signal can be adjusted using the five knobs. A CV signal
exits the module from the OUT 1 and OUT 2 jacks. A gate signal exits the module
from the GATE 1 and GATE 2 jacks. The gate outputs work best when the CV output
is in the range 0 – 10 volts (unipolar).

### IN 1 and IN 2

The IN 1 jack accepts the audio signal input for module’s first channel. The IN
2 jack accepts the audio signal input for the module’s second channel. Each jack
has a warning LED next to it that indicates the signal level is too high (above
10 volts) when lit.

### ATTACK and RELEASE

The ATTACK and RELEASE knobs control the speed of each channel’s envelope
follower. The goal is for the envelope follower to match the original CV without
too much distortion or too much smoothing. The default knob positions are a good
starting point. Right-click (Linux/Windows) or control-click (Mac) the knobs to
return to the default setting.

### ATTEN

The ATTEN knob provides an attenuverter for each channel. The full signal passes
through the attenuverter at the 1 position; the signal is inverted at the -1
knob position. At the 0 position the signal is muted.

### OFFSET

The OFFSET knob provides a voltage offset from -10 volts to +10 volts for each
channel. The offset enables a unipolar signal to be moved down to bipolar range.
For example, a signal oscillating between 0 volts and +5 volts, could be moved
down to the range of -2.5 volts to +2.5 volts to send out a bipolar signal.

### SCALE

The SCALE knob provides voltage amplification from 100% (no amplification) at
the 0 position to 300% at the 3 position. The scale knob can help enlarge a
signal to a range expected by VCV Rack modules.

### OUT 1 and OUT 2

The module outputs a CV signal from each channel from the OUT 1 and OUT 2 jacks.
Use of the VCV Fundamental Scope module is highly recommended for obtaining good
output from the NE-1 module. Each output jack has a signal LED that lights in
proportion to the rise and fall of the channel’s envelope follower.

### GATE 1 and GATE 2

The module outputs a gate signal of +10 volts from each channel from the GATE 1
and GATE 2 jacks. The gate signal is high when the envelope follower has risen
through half of its range.

NE-2 Amplitude Modulation Encoder
---------------------------------

The NE-2 Amplitude Modulation Encoder module provides two complete, independent
channels. The NE-2 converts a VCV Rack CV signal into an audio-rate signal. A CV
signal enters the module through either of the IN 1 or IN 2 jacks. The module
generates a carrier signal of approximately C9. The volume of the carrier signal
is modulated by the input CV signal. The modulated carrier signal exits the
module from the OUT 1 and OUT 2 jacks. The NE-2 expects the input CV to be in
the range of 0 volts to +10 volts (unipolar). A bipolar CV should be offset into
the 0 volts to +10 volts range before sending it into the NE-2.

### IN 1 and IN 2

The IN 1 jack accepts the CV signal input for module’s first channel. The IN 2
jack accepts the CV signal input for the module’s second channel. Each jack has
a warning LED next to it that indicates the signal level is too high (above 10
volts) when lit.

### OUT 1 and OUT 2

The module outputs a modulated audio signal from each channel from the OUT 1 and
OUT 2 jacks. In other words, the loudness of the carrier signal is used to
represent the low-frequency waveform of the input CV. Use of the VCV Fundamental
Scope module is highly recommended for obtaining good output from the NE-2
module.

### CARRIER

The CARRIER knob controls the volume of the carrier signal for both channels. At
the default 100% position the carrier is modulated from 0 volts to +10 volts. At
the 50% position the carrier is modulated from 0 volts to +5 volts. At the 0%
position the carrier is muted. The default 100% carrier setting works well with
VCV Rack. Lower carrier settings enable the NE-2 to emulate other amplitude
encoding hardware or software.

Compatibility
-------------

The Nocturnal Encoder plugin modules can connect VCV Rack to hardware Eurorack
synthesizers using either AC-connected or DC-connected audio interfaces.
Hardware audio interfaces or virtual audio connections can also be used to
connect VCV Rack to software synths that use CV’s with the Nocturnal Encoder
plugin.

I used a [StarTech.com \$35 7.1 USB audio
interface](https://www.startech.com/Cards-Adapters/Sound/USB-Audio/USB-7-Channel-Audio-Adapter-with-SPDIF~ICUSBAUDIO7D)
for testing Nocturnal Encoders on Windows. The StarTech does only AC, not DC,
coupling. The encoders only use audio range, so they work with the StarTech 7.1
or any other inexpensive audio interface.

Using my [MOTU 828
Mk1](http://www.motu.com/newsitems/828-makes-recording-technology-history) to
connect the ports, I successfully sent CVs from VCV Rack NS-2 Encoder into
Cubase 8, routed to through the [Silent Way CV
Input](http://www.expert-sleepers.co.uk/silentway.html) effect plugin set to
ES-2 Decoder Mode, then into the [Softube Modular
FX](https://www.softube.com/modular) VST. I also sent CVs from Softube Modular
(instrument VST) to VCV Rack using the NS-1 Decoder. At first, I thought it was
not going to work, but then I found that the levels were WAY too high and I was
banging against the ceiling. With tons of attenuation and low gain, it worked.

There are several ways to do encode/decode on the Eurorack hardware side.
Modular hardware encoding can be done with an [Expert Sleepers
ES-2-2](http://www.expert-sleepers.co.uk/es2.html) or
[Disting](http://www.expert-sleepers.co.uk/disting.html) in ES-2 emulation mode,
or you can roll-your-own by using a VCA as an encoder, and a Sine VCO for the
carrier. Modular hardware decoding can be done with a Doepfer A-119 Ext In or
other envelope follower, with a slew limiter, or even with a [DIY
cable](http://www.expert-sleepers.co.uk/siwaacencoder.html) that has a diode and
a capacitor soldered on the jack (see ES-1 page for diagram). I have not tested
with an [Expert Sleepers ES-1](http://www.expert-sleepers.co.uk/es1.html) or
Disting in ES-1 emulation mode, but they should work as well.

Changes
-------

### Version 0.6.1

Initial public release through the [VCV Rack
Plugins](https://vcvrack.com/plugins.html) page.

### Version 1.0.0

Update to VCV Rack 1.0.0 APIs.  Public release through the [VCV Rack
Plugins](https://vcvrack.com/plugins.html) page.

### Version 2.0.0

Update to VCV Rack 2.0.0 APIs.  Public release through the [VCV Rack
Plugins](https://vcvrack.com/plugins.html) page.
