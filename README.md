# Sostenuto
A Midi-Effect Plugin which implements the Sostenuto Pedal.

NOTE:
	This plugin probably only works for Logic Pro X right now. At least this is what is was tested on.
	I don't know all too much about plugins, I just saw a fixable problem and read into the IPLUG Framework.

Install:

1. Download [Sostenuto.zip](https://drive.google.com/uc?export=download&id=1-2_gufnrwh-ln6FxafyyanDqDR5zFj8J)
2. Unzip the file
3. Drag the .component file into /Users/_your_name_/Library/Audio/Plug-Ins/Components
4. Enjoy!

How it works:

	1. Load a Midi instrument of your choosing. (Let's say the Steinway Grand Piano in Logic)
	2. Above the Instrumen (Sampler) you can add Midi Effects.
	3. Choose: Audio Unit -> justus -> Sostenuto
	4. There are three main components to this plugin:
		-> The Midi Display
		-> The Pedal Value
		-> The Keyboard
	
	The Midi display shows you every Midi input that is sent through the Plugin.
	This can be useful to get your pedal value (which should be 64 for a sustain pedal).
	
	The pedal value is used to catch the Midi value which is used to engage the Sostenuto pedal.
	The value must be a Midi-Control-Value though which is the kind of value that should be sent by a pedal.
	
	The Keyboard is just a visual effect that can be used to confirm that the Plugin works.

	Note:
		If the Plugin is engaged and the pedal value is set to 64 your Sustain Pedal will only act as a Sostenuto Pedal.
		This should be obvious though.

	
Future Work:
	The plugin works for me and this is all I wanted. If there are visible bugs or easy extensions, I'll get back to work.
	Or just feel free to fork the repo and continue the project.

So long!
