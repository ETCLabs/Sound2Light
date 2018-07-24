// Copyright (c) 2016 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import "style"  // import all files in style dir

Row {

    // A property alias to reach the minimal mode from the bpm settings through to the main.qml
    property alias minimalMode: bpmSettings.minimalMode

    // ------------------------------ BPM Detection ----------------------------------
    BPMSettings {
        id: bpmSettings
        width: minimalMode ? parent.width : parent.width / 7
        height: parent.height
    }

    Column {
        id: graphsColumn
        visible: !minimalMode
        width: parent.width - controls.width - bpmSettings.width
        height: parent.height

        // ------------------------------------ WavePlot ---------------------------------
        Column {
            id: wavePlot
            width: parent.width
            height: controller.waveformVisible ? 80 : 0


            // -------------------------------- Wave Plot --------------------------------
            WavePlot {
                width: parent.width
                height: parent.height - divider.height
            }

            // --------------------------------- Divider ---------------------------------
            Rectangle {
                id: divider
                width: parent.width
                height: 5
                color: "#333333"
            }
        }


        // -------------------------- SpectrumPlot and Status Text -----------------------
        Item {
            width: parent.width
            height: parent.height - wavePlot.height
            SpectrumPlot {
                anchors.fill: parent
            }

            // ------------ Status Text -----------

            Row {
                height: 20
                width: statusText.implicitWidth + rxPortText.implicitWidth + spacing * 3
                spacing: 5
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 5

                Text {
                    id: statusText
                    width: implicitWidth
                    height: implicitHeight
                    color: "#777"
                    font.pointSize: 10
                    Component.onCompleted: updateText()
                    function updateText() {
                        text =  (controller.presetChangedButNotSaved ? "*" : "")
                                + (controller.presetName ? controller.presetName : "No Preset")
                                + "  |  '" + controller.getActiveInputName()
                                + "'  |  " + controller.getOscIpAddress()
                                + (controller.getUseTcp() ? (":" + controller.getOscTcpPort()) : (" Tx:" + controller.getOscUdpTxPort()))
                    }
                    Connections {
                        target: controller
                        onInputChanged: statusText.updateText()
                        onSettingsChanged: statusText.updateText()
                        onPresetNameChanged: statusText.updateText()
                        onPresetChangedButNotSavedChanged: statusText.updateText()
                        onIsConnectedChanged: statusText.updateText()
                        onAddressChanged: statusText.updateText()
                    }
                }

                // ------------ Tx Status LED -----------

                Rectangle {
                    id: connectionLED
                    width: 10
                    height: 10
                    y: 3
                    radius: width / 2
                    color: "#777"

                    // updates the color of the "LED" according to te current connection state
                    function updateColor() {
                        if (controller.isConnected()) {
                            // if TCP is connected or UDP is used, the LED lights green when a packet has been sent:
                            color = packetSentTimer.running ? "lightgreen" : "#777"
                        } else {
                            // if TCP is used but not connected the "LED" lights red
                            color = "red"
                        }
                    }

                    Timer {
                        // this timer is triggered when a packet has been sent
                        // it stops running when there was no packet for 200ms
                        id: packetSentTimer
                        repeat: false
                        interval: 200
                        onRunningChanged: connectionLED.updateColor()
                    }

                    Connections {
                        target: controller
                        // update the LED when the connection state changed:
                        onIsConnectedChanged: connectionLED.updateColor()
                        // trigger the timer when a packet has been sent:
                        onPacketSent: packetSentTimer.restart()
                    }
                }

                // ------------ Rx Port Text -----------

                Text {
                    id: rxPortText
                    width: implicitWidth
                    height: implicitHeight
                    color: "#777"
                    font.pointSize: 10
                    Component.onCompleted: updateText()
                    function updateText() {
                        text =  "Rx: " + (controller.getUseTcp() ? controller.getOscTcpPort() : controller.getOscUdpTxPort())
                    }
                    Connections {
                        target: controller
                        onIsConnectedChanged: rxPortText.updateText()
                        onAddressChanged: rxPortText.updateText()
                    }
                }

                // ------------ Rx Status LED -----------

                Rectangle {
                    id: rxStatusLed
                    width: 10
                    height: 10
                    radius: width / 2
                    y: 3
                    color: messageReceivedTimer.running ? "lightgreen" : "#777"

                    Timer {
                        // this timer is triggered when a packet has been received
                        // it stops running 200ms after that
                        id: messageReceivedTimer
                        repeat: false
                        interval: 200
                        onRunningChanged: connectionLED.updateColor()
                    }

                    Connections {
                        target: controller
                        // trigger the timer when a packet has been received:
                        onMessageReceived: messageReceivedTimer.restart()
                    }
                }
            }  // end Row Status Text
        }


    }



	// -------------------------- Right Area with Sliders and Buttons ----------
	Column {
		id: controls
        visible: !minimalMode
        width: 80 * 2
		height: parent.height

		// ------------------------- Gain and Compressor Sliders ---------------------
		Row {
			width: parent.width
            height: parent.height - 30*3
			// -------------------------- Gain
			Column {
				width: 80
				height: parent.height
				SliderWithIndicator {
					id: fftGainSlider
					width: parent.width
					height: parent.height - 40
					orientation: Qt.Vertical
					minimumValue: 0.5
					maximumValue: 5
					value: controller.getFftGain()
					showIndicator: false
					onValueChanged: if (pressed) controller.setFftGain(value)
					enabled: !controller.agcEnabled
				}
				GreyText {
					text: fftGainSlider.value.toFixed(1) + "x\nGain"
					width: parent.width
					height: 40
					font.pointSize: 10
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
				}
				// update gain value 20 times a second when AGC is enabled:
				Timer {
					interval: 50
					running: controller.agcEnabled
					repeat: true
					onTriggered: fftGainSlider.value = controller.getFftGain()
				}
				Connections {
					target: controller
					onGainChanged: fftGainSlider.value = controller.getFftGain()
				}
			}
			// -------------------------- Compressor
			Column {
				width: 80
				height: parent.height
				SliderWithIndicator {
					id: fftCompressionSlider
					width: parent.width
					height: parent.height - 40
					orientation: Qt.Vertical
					minimumValue: 0.2
					maximumValue: 3
					value: controller.getFftCompression()
					showIndicator: false
					onValueChanged: if (pressed) controller.setFftCompression(value)
				}
				GreyText {
					text: fftCompressionSlider.value.toFixed(1) + "\nCompressor"
					width: parent.width
					height: 40
					font.pointSize: 10
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
				}
				Connections {
					target: controller
					onCompressionChanged: fftCompressionSlider.value = controller.getFftCompression()
				}
			}
		}  // Row Gain and Compressor Sliders end

        // ----------------------------- dB + AGC + LowSolo Checkbox -----------------------
		DarkCheckBox {
			id: agcCheckbox
			width: parent.width - 20
			height: 30
			x: 20
			checked: controller.agcEnabled
			onCheckedChanged: if (checked !== controller.agcEnabled) controller.setAgcEnabled(checked)
			text: "Auto Gain"

			Connections {
				target: controller
				onAgcEnabledChanged: agcCheckbox.checked = controller.agcEnabled
			}
		}
		DarkCheckBox {
			id: dbCheckbox
			width: parent.width - 20
			height: 30
			x: 20
			checked: controller.decibelConversion
			onCheckedChanged: if (checked !== controller.decibelConversion) controller.setDecibelConversion(checked)
			text: "Convert to dB"

			Connections {
				target: controller
				onDecibelConversionChanged: dbCheckbox.checked = controller.decibelConversion
			}
		}
        DarkCheckBox {
            id: lowSoloCheckbox
            width: parent.width - 20
            height: 30
            x: 20
            checked: controller.lowSoloMode
            onCheckedChanged: if (checked !== controller.lowSoloMode) controller.setLowSoloMode(checked)
            text: "Low Solo Mode"

            Connections {
                target: controller
                onLowSoloModeChanged: lowSoloCheckbox.checked = controller.lowSoloMode
            }
        }
	}  // Right area column end
}
