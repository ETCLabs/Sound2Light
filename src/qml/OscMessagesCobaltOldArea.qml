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

import "oscUtils.js" as Utils

import "style"  // import all files in style dir


// To add a new type of message, create a Settings Component (see below)
// with a form to set up the messages.
// The component must have the two functions restoreFromMessages() and getMessages().
// - restoreFromMessages() tries to refill the form from an already stored message.
// - getMessages() returns an object with information about the setted up messages.



// ----------- Content of the OSC dialog for Cobalt (v7.2) console type -----------
Item {
	anchors.fill: parent

	// ------------ ComboBox and Loader for settings components -------
	Column {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 5
		Item {
			width: parent.width
			height: 30
			DarkComboBox {
				width: 100
				height: 30
				model: ["Master", "Custom", "--- Inactive"]
				onCurrentIndexChanged: updateSettingsArea()
				Component.onCompleted: {
					// This method tries to choose the appropriate form
					// for the OSC messages saved in the TriggerController.

					// try to find a not empty OSC message:
					var currentMessage = triggerController.getOnMessage()
					if (!currentMessage) currentMessage = triggerController.getOffMessage()
					if (!currentMessage) currentMessage = triggerController.getLevelMessage()
					currentMessage = currentMessage.toLowerCase()

					// check which category fits the message:
					if (currentMessage.indexOf("/masters/fader") !== -1) {
						currentIndex = model.indexOf("Master")
					}

					// load correct Settings Component:
					updateSettingsArea()

					// restore information in Component:
					settingsArea.item.restoreFromMessages()
				}
				function updateSettingsArea() {
					// This method loads the correct Settings Component
					// for the chosen message type category in this ComboBox.
					if (currentText === "Master") {
						settingsArea.sourceComponent = cobaltMasterSettings
					} else if (currentText === "Custom") {
						settingsArea.sourceComponent = cobaltCustomSettings
					} else if (currentText === "--- Inactive") {
						settingsArea.sourceComponent = cobaltInactiveSettings
					}
				}
			}  // ComboBox end

			GreyText {
				// Console Label in the top right corner
				text: "Cobalt 7.2"
				anchors.right: parent.right
				font.pointSize: 12
				font.bold: true
			}
		}  // Row end
		Loader {
			// This loader holds the Settings Component for the chosen message type category.
			id: settingsArea
			width: parent.width
			height: parent.height - 30
		}
	}


	// forwards the function call to the currently displayed Settings Component:
	function getMessages() {
		return settingsArea.item.getMessages()
	}





	// ---------------------- Settings Components for each message type ---------------------

	// All Settings COmponents must have the two functions restoreFromMessages() and getMessages().
	// - restoreFromMessages() tries to refill the form from an already stored message.
	// - getMessages() returns an object with information about the setted up messages.

	// ------------------------------ Master
	Component {
		id: cobaltMasterSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Master Number:"
				}
				NumericInput {
					id: masterNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 80
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Mode:"
				}
				DarkComboBox {
					id: masterMode
					height: parent.height
					width: parent.width * 0.5
					model: ["Switch", "Level"]
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: masterMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "On Value:"
				}
				NumericInput {
					id: masterOnValue
					height: parent.height
					width: parent.width * 0.5
					value: 100
					minimumValue: 0
					maximumValue: 100
					suffix: " %"
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: masterMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Off Value:"
				}
				NumericInput {
					id: masterOffValue
					height: parent.height
					width: parent.width * 0.5
					value: 0
					minimumValue: 0
					maximumValue: 100
					suffix: " %"
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				if (triggerController.getLevelMessage() !== "") {
					masterMode.currentIndex = masterMode.model.indexOf("Level")
					masterNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					masterMode.currentIndex = masterMode.model.indexOf("Switch")
					masterNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
					masterOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 1) * 100
					masterOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0) * 100
				}
			}


			function getMessages() {
				var level = masterMode.currentText === "Level";
				var messages = {
					"on": level ? "" : "/masters/fader/" + masterNumber.value + "=" + (masterOnValue.value / 100).toFixed(3),
					"off": level ? "" : "/masters/fader/" + masterNumber.value + "=" + (masterOffValue.value / 100).toFixed(3),
					"level": level ? "/masters/fader/" + masterNumber.value + "=" : "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Master " + masterNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Custom
	Component {
		id: cobaltCustomSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.3
					text: "On Message:"
				}
				DarkTextField {
					id: customOnMessage
					height: parent.height
					width: parent.width * 0.7
					text: triggerController.getOnMessage()
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.3
					text: "Off Message:"
				}
				DarkTextField {
					id: customOffMessage
					height: parent.height
					width: parent.width * 0.7
					text: triggerController.getOffMessage()
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.3
					text: "Level Message:"
				}
				DarkTextField {
					id: customLevelMessage
					height: parent.height
					width: parent.width * 0.7
					text: triggerController.getLevelMessage()
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Level Min Value:"
				}
				NumericInput {
					id: customLevelMinValue
					height: parent.height
					width: parent.width * 0.5
					value: triggerController.getMinLevelValue()
					minimumValue: 0
					maximumValue: 100
					decimals: 2
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Level Max Value:"
				}
				NumericInput {
					id: customLevelMaxValue
					height: parent.height
					width: parent.width * 0.5
					value: triggerController.getMaxLevelValue()
					minimumValue: 0
					maximumValue: 100
					decimals: 2
				}
			}

			function restoreFromMessages() { }

			function getMessages() {
				var messages = {
					"on": customOnMessage.text,
					"off": customOffMessage.text,
					"level": customLevelMessage.text,
					"levelMin": customLevelMinValue.value,
					"levelMax": customLevelMaxValue.value,
					"shortText": "Custom"
				}
				return messages
			}
		}
	}
	// ------------------------------ Inactive
	Component {
		id: cobaltInactiveSettings
		Column {
			anchors.fill: parent
			anchors.margins: 10
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width
					text: "Do not send OSC messages with this trigger."
				}
			}

			function restoreFromMessages() { }

			function getMessages() {
				var messages = {
					"on": "",
					"off": "",
					"level": "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": ""
				}
				return messages
			}
		}
	}
}
