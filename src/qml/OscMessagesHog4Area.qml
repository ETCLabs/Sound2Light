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



// ----------- Content of the OSC dialog for ColorSource console type -----------
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
				width: 130
				height: 30
				model: ["Fader", "Cue List", "Scene", "Macro", "Custom", "--- Inactive"]
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
					if (currentMessage.indexOf("/hog/hardware/fader") !== -1) {
						currentIndex = model.indexOf("Fader")
					} else if (currentMessage.indexOf("/hog/playback/go") !== -1 && Utils.argumentAsInt(currentMessage, 0) === 0) {
						currentIndex = model.indexOf("Cue List")
					} else if (currentMessage.indexOf("/hog/playback/go") !== -1 && Utils.argumentAsInt(currentMessage, 0) === 1) {
						currentIndex = model.indexOf("Scene")
					} else if (currentMessage.indexOf("/hog/playback/go") !== -1 && Utils.argumentAsInt(currentMessage, 0) === 2) {
						currentIndex = model.indexOf("Macro")
					}

					// load correct Settings Component:
					updateSettingsArea()

					// restore information in Component:
					settingsArea.item.restoreFromMessages()
				}
				function updateSettingsArea() {
					// This method loads the correct Settings Component
					// for the chosen message type category in this ComboBox.
					if (currentText === "Fader") {
						settingsArea.sourceComponent = hog4FaderSettings
					} else if (currentText === "Cue List") {
						settingsArea.sourceComponent = hog4CueListSettings
					} else if (currentText === "Scene") {
						settingsArea.sourceComponent = hog4SceneSettings
					} else if (currentText === "Macro") {
						settingsArea.sourceComponent = hog4MacroSettings
					} else if (currentText === "Custom") {
						settingsArea.sourceComponent = customSettings
					} else if (currentText === "--- Inactive") {
						settingsArea.sourceComponent = inactiveSettings
					}
				}
			}  // ComboBox end

			GreyText {
				// Console Label in the top right corner
				text: "Hog 4"
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

	// All Settings Components must have the two functions restoreFromMessages() and getMessages().
	// - restoreFromMessages() tries to refill the form from an already stored message.
	// - getMessages() returns an object with information about the setted up messages.

	// ------------------------------ Fader
	Component {
		id: hog4FaderSettings
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
					id: cueListNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 9999
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
					id: faderMode
					height: parent.height
					width: parent.width * 0.5
					model: ["Switch", "Level"]
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: faderMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "On Value:"
				}
				NumericInput {
					id: faderOnValue
					height: parent.height
					width: parent.width * 0.5
                    value: 255
					minimumValue: 0
                    maximumValue: 255
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: faderMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Off Value:"
				}
				NumericInput {
					id: faderOffValue
					height: parent.height
					width: parent.width * 0.5
					value: 0
					minimumValue: 0
                    maximumValue: 255
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				if (triggerController.getLevelMessage() !== "") {
					faderMode.currentIndex = faderMode.model.indexOf("Level")
					cueListNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					faderMode.currentIndex = faderMode.model.indexOf("Switch")
					cueListNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
                    faderOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 255)
					faderOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0)
				}
			}


			function getMessages() {
				var level = faderMode.currentText === "Level";
                var messages = {
                    "on": level ? "" : "/hog/hardware/fader/" + cueListNumber.value + "=" + faderOnValue.value,
                    "off": level ? "" : "/hog/hardware/fader/" + cueListNumber.value + "=" + faderOffValue.value,
                    "level": level ? "/hog/hardware/fader/" + cueListNumber.value + "=" : "",
                    "levelMin": 0,
                    "levelMax": 255,
                    "shortText": "Fader " + cueListNumber.value
                }
				return messages
			}
		}
	}
	// ------------------------------ Cue List
	Component {
		id: hog4CueListSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Cue List:"
				}
				NumericInput {
					id: cueListNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 9999
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Release on off:"
				}
				DarkCheckBox {
					id: releaseOnOffCheckbox
					height: parent.height
					width: parent.width * 0.5
					checked: true
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				cueListNumber.value = Utils.argumentAsInt(triggerController.getOnMessage(), 1, 1)
				releaseOnOffCheckbox.checked = triggerController.getOffMessage() !== ""
			}

			function getMessages() {
				var messages = {
					"on": "/hog/playback/go=0," + cueListNumber.value,
					"off": releaseOnOffCheckbox.checked ? "/hog/playback/release=0," + cueListNumber.value : "",
                    "level": "",
                    "levelMin": 0,
                    "levelMax": 1,
                    "shortText": "Cue List " + cueListNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Scene
	Component {
		id: hog4SceneSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Scene:"
				}
				NumericInput {
					id: sceneNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 9999
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Release on off:"
				}
				DarkCheckBox {
					id: releaseOnOffSceneCheckbox
					height: parent.height
					width: parent.width * 0.5
					checked: true
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				sceneNumber.value = Utils.argumentAsInt(triggerController.getOnMessage(), 1, 1)
				releaseOnOffSceneCheckbox.checked = triggerController.getOffMessage() !== ""
			}

			function getMessages() {
				var messages = {
					"on": "/hog/playback/go=1," + sceneNumber.value,
					"off": releaseOnOffSceneCheckbox.checked ? "/hog/playback/release=1," + sceneNumber.value : "",
                    "level": "",
                    "levelMin": 0,
                    "levelMax": 1,
                    "shortText": "Scene " + sceneNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Macro
	Component {
		id: hog4MacroSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Macro:"
				}
				NumericInput {
					id: macroNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 9999
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Release on off:"
				}
				DarkCheckBox {
					id: releaseOnOffMacroCheckbox
					height: parent.height
					width: parent.width * 0.5
					checked: true
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				macroNumber.value = Utils.argumentAsInt(triggerController.getOnMessage(), 1, 1)
				releaseOnOffMacroCheckbox.checked = triggerController.getOffMessage() !== ""
			}

			function getMessages() {
				var messages = {
					"on": "/hog/playback/go=2," + macroNumber.value,
					"off": releaseOnOffMacroCheckbox.checked ? "/hog/playback/release=2," + macroNumber.value : "",
                    "level": "",
                    "levelMin": 0,
                    "levelMax": 1,
                    "shortText": "Macro " + macroNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Custom
	Component {
		id: customSettings
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
                    maximumValue: 255
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
                    maximumValue: 255
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
		id: inactiveSettings
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
