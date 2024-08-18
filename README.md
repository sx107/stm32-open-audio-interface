# Stm32 open audio interface
 Open source UAC2 audio card

This is an attempt at making an open source audio interface on stm32h750. Currently using es8388 cheap chinese codec, better codecs, as well as multiple input/output channels are in plans.
Currently supporting only one output channel.

Current problems: It does not work (stable enough to work without glitches) on "busy" usb hubs and on linux/android/mac devices.