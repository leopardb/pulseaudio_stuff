#pulseaudio_stuff
================

some scripts and stuff


## get-sink-input-index

Outputs the index of a sink-input to the standard output from a property and value. Useful for changing the sink an application is outputing its sound to within a script. You can then use "pactl move-sink-input".

I might change it in the future to outputing directly the value (instead of printf-ing it) but i'm not sure right now what happens when several sink-inputs (a.k.a "playback streams") do hold the same value for the searched property...


usage: ./get-sink-input-index property value
  property: pulseaudio sink-input property (ex:application.name)
  value: the value for the wanted property (ex: "Quod Libet")

ex: **get-sink-input-index application.name "Quod Libet"**