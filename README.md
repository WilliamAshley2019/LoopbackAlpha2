********** IT IS ESSENTIAL YOU READ HOW TO USE THIS AS IT IS NOT NORMAL *******************
This version requires you to load FL Studio or whatever DAW run as administrator  because it uses Windows Global Memory AFAIK only accessable when you run a program in administrator mode - another version of this plugin is planned but I've been too lazy to build / test it so far. This is a highly 
experimental plugin - the buffer stuff needs to be fine tuned likely to be very usable.... this is not a production ready plugin!!!! BE ADVISED!!!!
--------------------------------------------------------------------------------------------------
Copyright (c) 2026 William Ashley d/b/a William Ashley Music ( http://WilliamAshley.music )
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License  (v3) 

This program is distributed in the hope that it will be useful to other audio programmers and music makers in their own plugin designs.
There is no WARRANTY expressed or implied including for MERCHANTABILITY or FITNESS FOR ANY PURPOSE. 
See the GNU General Public License for more details.

Attributtion is requested where possible if you use or modify any of the source,
Notice of use is requested so I can familiarize myself with how the code has been adapted for personal interest.
contact@WilliamAshley.music   
-----------------------------------------------------------------------------------------------------
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![JUCE](https://img.shields.io/badge/Built%20with-JUCE%208.0.12-blue)](https://juce.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20-lightgrey)]()
[![Format](https://img.shields.io/badge/Format-VST3%20%7C%20-orange)]()

I uploaded a release build so I think it will be usable without visual studio ms runtimes. It "should" run now on its own as long as you run fl studio as administrator "run as administrator" so the program has access to windows shared global memory. This was my lazy approach to bypassing fl studio's internal audio buffer that would prevent linking back audio to itself (runaway feedback creation in mixer)

Basic Plugin - one is built as an effect the other as instrument. Common manufacturer ID and Different Plugin ID was used intially due to earlier attempt to use shared instance memory but it didn't work so windows shared memory was used instead.

HOW IT WORKS

Load the Send Effect VST3 plugin to a mixer track - this was made for FL Studio due to FL Studio blocking some types of loopback routing.

Load the Return Vsti plugin in fl studio into a channel rack channel (or possibly patcher havn't tested it). It may work in other DAWs but is untested in them.

Once loaded whatever audio is reaching the Send Effect point will be relayed to the Return Plugin you can then assign a mixer track to that generator to send it to a different route so it essentially works like a patch cable.

It was built with a buffer and in this version some adjustment on the delay is provided via a knob. The first version had very easy to get runnaway feedback this version seems to be much more tame as the goal was to partially make the feedback manageable.


I need to test to see how it behaves with a dynamic feedback rate knob, the big issue is that the feedback needs a good control system because even with a 3 second delay the feedback can build rapidly.

It was built for windows using windows shared memory - the source probably will not work on Mac I haven't tested or built this for Mac OS.

Future add ons I was going to do but chose sleep instead - perhaps some type of limiter or soft clipper it is suggested to load some type of clipper or limiter in chain be preared to control the feedback you can add effects in the signal chain. A knob for a feedback reduction or shaping algorithm might also be useful. I am also going to look into building a return effect I thought the generator would be best suited because I thought it would work where as I wasn't sure if the effect would work as a generator in fl studio due to possible return path audio blocks coded into fl studio within mixer channel routing. If using something like senderella you can insert the channel it is assigned to. A delay knob to adjust the buffer to less than 3 seconds or more was not added to safety considerations while testing.
I have started to add some feedback reduction aspects but not sure what direction that will go fully yet the idea is to allow feedback shaping.
VST3 license https://steinbergmedia.github.io/vst3_dev_portal/pages/VST+3+Licensing/VST3+License

This version is much more smoothed and less grainy due to different handlign of the sampling and removal of the DC offset (may not be reflected in the vst yet as it hasn't been built with the
removed dc offset. 


Also it not only works on the same so routing loopback on the same mixer track, but also a child to parent via the generator with senderella at any insert point.

Future to add - clear loopback buffer button needed. 
Exactly how to handle the buffer is not fully decided on however the next version is set to include.

Common Buffer Management Scenarios:

Clear Buffer - Wipe everything, start fresh (useful after experimenting)  DONE
Fade Out - Gradually reduce buffer content (smooth stop for feedback loops) Basic implementation may require multiple button presses this likely needs to be refined more as it doesn't cause full fadeout during playback.
Freeze - Stop writing but keep playing what's there (capture a loop)  (Stop does seem to work but it keeps the buffer going but does not build the buffer)
Auto-clear on Stop - Clear when transport playback stops  (NOT DONE likely due to isplaying not being fully implemented)

Edit 2025-11-29 I have moved the old version of LoopbackAlpha2 to the Old folder. Changes were adding some buffer control.  However there does appear to be more common clicking in the current version that I don't remember from the last one however a little more buffer control. Future plans to potentially ad some way of controlling the buffer types via midi as currently 
plugin is more of a real time thing and not so sure if button presses will be automable without midi triggers via piano rolli in the generator. The major processor issue ATM is removing 
the clicking likely due to something with the samples being non smoothed need to look into it more
Since people may have different uses - however the question still remains how to specifically implement it for rendered playback as live looping is a different issue from automated looping where reseting isn't an option I may need to add midi triggering so midi notes can control the buffer modes.



TO DO: adjust the cycle length knob to add to change the time it takes to cycle the loopback.
So the loopback cycle length knob/setting will be provided in the next version LoopbackAlpha3. It still needs to be tested.  Hopefully tomorrow.  
