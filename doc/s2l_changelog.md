
# Sound2Light Change Log

### Version 0.0.3.1.0.2 - 24 Mar 2018

- Reenabled maximize button

- Make waveform independent from BPM detection

- Replace "BPM Detection" checkbox with "Auto BPM"

### Version 0.0.3.1.0.1 - 23 Mar 2018

- Added Hog 4 OSC commands (Fader, Cue List, Scene and Macro)

- Fix latency issue (reduced latency from >300ms to 80ms)

### Version 0.0.2.1.0.9 - 1 Mar 2017

- Supports Muting the OSC output of individual channels

### Version 0.0.2.1.0.8 - 1 Mar 2017

- Fixed BPM OSC Messages with a one digit value being interpreted falsely on EOS

- Fixed Manualy entered BPMs not being saved in presets

### Version 0.0.2.1.0.7 - 14 Sep 2016

- Fixed BPM OSC Messages not stopping when turning of OSC Output

- Fixed Manualy entered BPMs not being transmitted to the console

### Version 0.0.2.1.0.6 - 12 Sep 2016

- Added Minimal Mode that reduces the GUI to just BPM

- Added option to enter a BPM using a number keypad

### Version 0.0.2.1.0.5 - 9 Sep 2016

- Added BPM Detection for Cobalt 7.3 and 7.2

- Added tap tempo by clicking on the bpm number to overwrite automatic detection

- Moved OSC Monitor Button out of Settings onto the main GUI

### Version 0.0.2.1.0.4 - 7 Sep 2016

- Fixed Waveform not initially displaying on Windows

- Improved BPM Detection and added "Auto" range

### Version 0.0.2.1.0.3 - 6 Sep 2016

- Saved Waveform display status in presets and autosave

- Added new "Ultra" Option to BPM Ranges

- Fixed A Bug that prevented BPM Detection in very quiet signals

- Made BPM Label indicate its probably outdated value if no 
  tempo could be detected within 5 seconds by turning grey

- Improved BPM Target User Interface

### Version 0.0.2.1.0.2	- 6 Sep 2016

- fixed windows installer issues

### Version 0.0.2.1.0.1	-  2 Sep 2016

- added BPM Detection for EOS


### Version 0.0.1.1.0.16	- 22 Aug 2016

- added Low Solo Mode


### Version 0.0.1.1.0.15	- 29 Apr 2016

- added status LED for incoming messages


### Version 0.0.1.1.0.14	- 28 Apr 2016

- added predefined OSC messages for ColorSource console

- added predefined OSC messages for Cobalt v7.3+

- fixed predefined OSC messages for Cobalt v7.2

- fixed incorrect handling of incoming bundled OSC messages


### Version 0.0.1.1.0.13	- 27 Apr 2016

- renamed "Output Enabled" checkbox to "OSC Output"

- renamed "OSC Input Enabled" checkbox to "OSC Input"

- increased font size in OSC Monitor

- changed color of incoming messages in OSC Monitor

- fixed default values of checkboxes in OSC Monitor

- Renamed "Cobalt" console type to "Cobalt 7.2"


### Version 0.0.1.1.0.12	- 26 Apr 2016

- added checkbox "OSC Input Enabled" to control OSC input

- fixed a bug where the OSC Message Dialog displayed incorrect values


### Version 0.0.1.1.0.11	- 22 Apr 2016

- added Clear button to OSC Log dialog

- fixed modality of dialogs on Mac OS X

- renamed "Send OSC" checkbox to "Output enabled"

- renamed "OSC Log" dialog to "OSC Monitor"


### Version 0.0.1.1.0.10	- 21 Apr 2016

- software can now be controlled by OSC messages

	- Preset selection
	- enable or disable Trigger Output
	- optional Level Feedback

- added OSC Log Dialog

- predefined messages for Eos will now be sent as User 0

	-> won't affect the commandline anymore

- added support for Mac OS X


### Version 0.0.1.1.0.9	- 14 Apr 2016

- changed unit of Bandpass Width to Octaves

- added connection status LED right of status text

- added setting to change OSC TCP port


### Version 0.0.1.1.0.8	- 8 Apr 2016

- replaced SpinBoxes with more touchfriendly Number Pad

- changed application icon

- fixed High DPI scaling (i.e. on Microsoft Surface)


### Version 0.0.1.1.0.7	- 1 Apr 2016

- Bandpass Previews can now be manipulated with pinch gestures

- the currently loaded Preset is now highlighted in the Preset List

- fixed OSC message dialog not showing the currently used settings


### Version 0.0.1.1.0.6	- 31 Mar 2016

- bandpass width can now be changed by dragging it while holding CTRL

- added Fader message type to Eos OSC messages

- added option to send OSC over TCP instead of UDP

- added option to choose between OSC 1.0 or 1.1 packet framing

- installer now optionally creates a dekstop icon


### Version 0.0.1.1.0.5	- 29 Mar 2016

- added preset list dialog with option to remove presets

- improved default parameter settings

- added confirm-dialogs before discarding or deleting changes


### Version 0.0.1.1.0.4	- 24 Mar 2016

- added dialog to save and load presets

- added button to reset parameters to factory defaults


### Version 0.0.1.1.0.3	- 23 Mar 2016

- added Splash Screen

- added Automatic Gain Control (AGC)


### Version 0.0.1.1.0.2	- 22 Mar 2016

- improved FFT analysis (44 samples/sec, 4096 steps)

- added continuous OSC command values


### Version 0.0.1.1.0.1 	- 17 Mar 2016

- added 'About' with Version and Copyright infos

- improved trigger indication

- added dialog box for pre-define Eos Channel and Flash Button OSC messages

- added option to switch off OSC Send
