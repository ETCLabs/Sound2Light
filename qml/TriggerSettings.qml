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

import "style"  // import all files in style dir

// ---------- Contains controls to change the parameters of a BandpassTrigger ------------
// --- Can also be used for an Envelope Trigger
Item {
	id: root
	property string triggerName: ""
	property QtObject triggerController
	property color color: "darkblue"
	property bool detailsVisible: true
	property bool isBandpass: true

	Rectangle {  // colored background
		anchors.fill: parent
		color: root.color
		opacity: 0.2
	}

	Column {
		anchors.fill: parent

		// ------------------ Title with colorchanging Background -----------
		Item {
			width: parent.width
			height: 40

			Rectangle {
				// Green background, visible while trigger is active
				anchors.fill: parent
				visible: triggerController.active
				gradient: Gradient {
					GradientStop { position: 0 ; color: isBandpass ? Qt.lighter(root.color, 1.3) : Qt.rgba(0.3, 0.35, 0.7, 1) }
					GradientStop { position: 1 ; color: isBandpass ? Qt.lighter(root.color, 1.5) : Qt.rgba(0.3, 0.35, 0.9, 1) }
				}
			}

			Rectangle {
				// Trigger Feedback Animation
				// Blinks when trigger gets activated
				anchors.fill: parent
				color: Qt.rgba(1, 1, 1, 0.4)
				opacity: 0
				NumberAnimation on opacity {
					id: triggerFeedback
					running: triggerController.active
					alwaysRunToEnd: true
					from: 1
					to: 0
					duration: 400
					easing.type: Easing.OutCubic
				}
			}

			GreyText {
				anchors.fill: parent
				text: triggerName
				font.pointSize: 12
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				color: triggerController.active ? "#333" : "#b5b7ba"
			}
		}

		// ---------------------------- Threshold ----------------------
		SliderWithIndicator {
			id: thresholdSlider
			width: parent.width
			height: parent.height - 40 - details.height
			value: triggerController.threshold
			onValueChanged: if (triggerController.threshold !== value) triggerController.threshold = value

			Timer {
				interval: 25; running: true; repeat: true
				onTriggered: thresholdSlider.indicator = triggerController.getCurrentLevel()
			}
		}
		Column {
			id: details
			visible: detailsVisible
			width: parent.width
			height: detailsVisible ? 30*6 : 0

			Column {  // ------------------ Frequency and Width - only visible if this is a Bandpass ---------
				width: parent.width
				height: 60
				// ---------------------------- Frequency ----------------------
				NumericInput {
					width: parent.width
					height: 30
					minimumValue: 20
					maximumValue: 22050
					stepSize: 20
					suffix: " Hz"
					value: isBandpass ? triggerController.midFreq : 100
					onValueChanged: if (triggerController.midFreq !== value) triggerController.midFreq = value
					visible: isBandpass
				}
				// ---------------------------- Width ----------------------
				NumericInput {
					width: parent.width
					height: 30
					minimumValue: 0.3
					maximumValue: 10
					decimals: 1
					stepSize: 0.1
					value: isBandpass ? triggerController.width * 10 : 0.5
					onValueChanged: if (Math.abs(triggerController.width - value / 10) > 0.01) triggerController.width = value / 10
					visible: isBandpass
				}
			}


			// ---------------------------- On Delay ----------------------
			NumericInput {
				width: parent.width
				height: 30
				minimumValue: 0.0
				maximumValue: 20
				stepSize: 0.1
				decimals: 2
				suffix: " s"
				value: triggerController.onDelay
				onValueChanged: if (triggerController.onDelay !== value) triggerController.setOnDelay(value)
			}
			// ---------------------------- Off Delay ----------------------
			NumericInput {
				width: parent.width
				height: 30
				minimumValue: 0.0
				maximumValue: 20
				stepSize: 0.1
				decimals: 2
				suffix: " s"
				value: triggerController.offDelay
				onValueChanged: if (triggerController.offDelay !== value) triggerController.setOffDelay(value)
			}
			// ---------------------------- Max Hold / Decay ----------------------
			NumericInput {
				width: parent.width
				height: 30
				minimumValue: 0.0
				maximumValue: 20
				stepSize: 0.1
				decimals: 2
				suffix: " s"
				textReplacement: (value <= 0.0) ? "Off" : ""
				value: triggerController.maxHold
				onValueChanged: if (triggerController.maxHold !== value) triggerController.setMaxHold(value)
			}
			// ---------------------------- OSC Settings ----------------------
			DarkButton {
				width: parent.width
				height: 30
				text: (triggerController.oscLabelText !== "") ? triggerController.oscLabelText : "---"
				onClicked: controller.openDialog("qrc:/qml/OscMessageDialog.qml", "triggerController", root.triggerController)
			}
		}  // details Column
	}  // Column
}
