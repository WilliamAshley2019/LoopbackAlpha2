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
Exactly how to handle the buffer is not fully decided on however the next version is set to included
Common Buffer Management Scenarios:

Clear Buffer - Wipe everything, start fresh (useful after experimenting)
Fade Out - Gradually reduce buffer content (smooth stop for feedback loops)
Freeze - Stop writing but keep playing what's there (capture a loop)
Auto-clear on Stop - Clear when transport stops (clean slate each playback)
