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


// ----------- Content of the OSC dialog for EOS console type -----------
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
				model: ["Channel", "Group", "Macro", "Submaster", "Bump Sub", "Cue", "Fader", "Custom", "--- Inactive"]
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
					if (currentMessage.indexOf("/eos/chan") !== -1) {
						currentIndex = model.indexOf("Channel")
					} else if (currentMessage.indexOf("/group/") !== -1) {
						currentIndex = model.indexOf("Group")
					} else if (currentMessage.indexOf("/macro/") !== -1) {
						currentIndex = model.indexOf("Macro")
					} else if (currentMessage.indexOf("/sub/") !== -1 && currentMessage.search("fire") === -1) {
						currentIndex = model.indexOf("Submaster")
					} else if (currentMessage.indexOf("/sub/") !== -1 && currentMessage.search("fire") !== -1) {
						currentIndex = model.indexOf("Bump Sub")
					} else if (currentMessage.indexOf("/cue/") !== -1) {
						currentIndex = model.indexOf("Cue")
					} else if (currentMessage.indexOf("/fader/") !== -1) {
						currentIndex = model.indexOf("Fader")
					}

					// load correct Settings Component:
					updateSettingsArea()

					// restore information in Component:
					settingsArea.item.restoreFromMessages()
				}
				function updateSettingsArea() {
					// This method loads the correct Settings Component
					// for the chosen message type category in this ComboBox.
					if (currentText === "Channel") {
						settingsArea.sourceComponent = eosChannelSettings
					} else if (currentText === "Group") {
						settingsArea.sourceComponent = eosGroupSettings
					} else if (currentText === "Macro") {
						settingsArea.sourceComponent = eosMacroSettings
					} else if (currentText === "Submaster") {
						settingsArea.sourceComponent = eosSubmasterSettings
					} else if (currentText === "Bump Sub") {
						settingsArea.sourceComponent = eosBumpSubSettings
					} else if (currentText === "Cue") {
						settingsArea.sourceComponent = eosCueSettings
					} else if (currentText === "Fader") {
						settingsArea.sourceComponent = eosFaderSettings
					} else if (currentText === "Custom") {
						settingsArea.sourceComponent = eosCustomSettings
					} else if (currentText === "--- Inactive") {
						settingsArea.sourceComponent = eosInactiveSettings
					}
				}
			}  // ComboBox end

			GreyText {
				// Console Label in the top right corner
				text: "Eos"
				anchors.right: parent.right
				font.pointSize: 12
				font.bold: true
			}
		}
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

	// ------------------------------ Channel
	Component {
		id: eosChannelSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Channel Number:"
				}
				NumericInput {
					id: channelNumber
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
					id: channelMode
					height: parent.height
					width: parent.width * 0.5
					model: ["Switch", "Level"]
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: channelMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "On Value:"
				}
				NumericInput {
					id: channelOnValue
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
				visible: channelMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Off Value:"
				}
				NumericInput {
					id: channelOffValue
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
					channelMode.currentIndex = channelMode.model.indexOf("Level")
					channelNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					channelMode.currentIndex = channelMode.model.indexOf("Switch")
					channelNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
					channelOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 100)
					channelOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0)
				}
			}


			function getMessages() {
				var level = channelMode.currentText === "Level";
				var messages = {
					"on": level ? "" : "/eos/user/<USER>/chan/" + channelNumber.value + "=" + channelOnValue.value,
					"off": level ? "" : "/eos/user/<USER>/chan/" + channelNumber.value + "=" + channelOffValue.value,
					"level": level ? "/eos/user/<USER>/chan/" + channelNumber.value + "=" : "",
					"levelMin": 0,
					"levelMax": 100,
					"shortText": "Channel " + channelNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Group
	Component {
		id: eosGroupSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Group Number:"
				}
				NumericInput {
					id: groupNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
                    minimumValue: 0.001
                    maximumValue: 9999.999
                    decimals: 3
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
					id: groupMode
					height: parent.height
					width: parent.width * 0.5
					model: ["Switch", "Level"]
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: groupMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "On Value:"
				}
				NumericInput {
					id: groupOnValue
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
				visible: groupMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Off Value:"
				}
				NumericInput {
					id: groupOffValue
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
					groupMode.currentIndex = groupMode.model.indexOf("Level")
					groupNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					groupMode.currentIndex = groupMode.model.indexOf("Switch")
					groupNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
					groupOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 100)
					groupOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0)
				}
			}

			function getMessages() {
				var level = groupMode.currentText === "Level";
				var messages = {
					"on": level ? "" : "/eos/user/<USER>/group/" + groupNumber.value + "=" + groupOnValue.value,
					"off": level ? "" : "/eos/user/<USER>/group/" + groupNumber.value + "=" + groupOffValue.value,
					"level": level ? "/eos/user/<USER>/group/" + groupNumber.value + "=" : "",
					"levelMin": 0,
					"levelMax": 100,
					"shortText": "Group " + groupNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Macro
	Component {
		id: eosMacroSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.4
					text: "On Macro:"
				}
				DarkCheckBox {
					id: onMacroCheckbox
					height: parent.height
					width: parent.width * 0.1
					checked: true
				}
				NumericInput {
					id: onMacroNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 999
					enabled: onMacroCheckbox.checked
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.4
					text: "Off Macro:"
				}
				DarkCheckBox {
					id: offMacroCheckbox
					height: parent.height
					width: parent.width * 0.1
					checked: false
				}
				NumericInput {
					id: offMacroNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 999
					enabled: offMacroCheckbox.checked
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				if (triggerController.getOnMessage() !== "") {
					onMacroCheckbox.checked = true
					onMacroNumber.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 1)
				}
				if (triggerController.getOffMessage() !== "") {
					offMacroCheckbox.checked = true
					if (triggerController.getOnMessage() === "") onMacroCheckbox.checked = false
					offMacroNumber.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 1)
				}
			}

			function getMessages() {
				var messages = {
					"on": onMacroCheckbox.checked ? "/eos/user/<USER>/macro/fire=" + onMacroNumber.value : "",
					"off": offMacroCheckbox.checked? "/eos/user/<USER>/macro/fire=" + offMacroNumber.value : "",
					"level": "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Macro " + (onMacroCheckbox.checked ? onMacroNumber.value : offMacroNumber.value)
				}
				return messages
			}
		}
	}
	// ------------------------------ Submaster
	Component {
		id: eosSubmasterSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Submaster Number:"
				}
				NumericInput {
					id: submasterNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 999
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
					id: submasterMode
					height: parent.height
					width: parent.width * 0.5
					model: ["Switch", "Level"]
				}
			}
			Row {
				height: 30
				width: parent.width
				visible: submasterMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "On Value:"
				}
				NumericInput {
					id: submasterOnValue
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
				visible: submasterMode.currentText === "Switch"
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Off Value:"
				}
				NumericInput {
					id: submasterOffValue
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
					submasterMode.currentIndex = submasterMode.model.indexOf("Level")
					submasterNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					submasterMode.currentIndex = submasterMode.model.indexOf("Switch")
					submasterNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
					submasterOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 100) * 100
					submasterOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0) * 100
				}
			}


			function getMessages() {
				var level = submasterMode.currentText === "Level";
				var messages = {
					"on": level ? "" : "/eos/user/<USER>/sub/" + submasterNumber.value + "=" + (submasterOnValue.value / 100),
					"off": level ? "" : "/eos/user/<USER>/sub/" + submasterNumber.value + "=" + (submasterOffValue.value / 100),
					"level": level ? "/eos/user/<USER>/sub/" + submasterNumber.value + "=" : "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Sub " + submasterNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Bump Sub
	Component {
		id: eosBumpSubSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Submaster:"
				}
				NumericInput {
					id: bumpSubNumber
					height: parent.height
					width: parent.width * 0.5
					value: 1
					minimumValue: 1
					maximumValue: 999
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				bumpSubNumber.value = Utils.partAsInt(triggerController.getOnMessage(), 5, 1)
			}

			function getMessages() {
				var messages = {
					"on": "/eos/user/<USER>/sub/" + bumpSubNumber.value + "/fire=1.0",
					"off": "/eos/user/<USER>/sub/" + bumpSubNumber.value + "/fire=0.0",
					"level": "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Bump " + bumpSubNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Cue
	Component {
		id: eosCueSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Run Cue when activated:"
				}
				DarkCheckBox {
					id: onCueCheckbox
					height: parent.height
					width: parent.width * 0.5
					checked: true
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "List:"
					enabled: onCueCheckbox.enabled
				}
				NumericInput {
					id: onCueList
					height: parent.height
					width: parent.width * 0.25
					value: 1
					minimumValue: 1
					maximumValue: 999
					enabled: onCueCheckbox.checked
				}
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "Number:"
					enabled: onCueCheckbox.enabled
				}
				DarkTextField {
					id: onCueNumber
					height: parent.height
					width: parent.width * 0.25
					enabled: onCueCheckbox.checked
					text: "1"
					placeholderText: "?"
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.5
					text: "Run Cue when released:"
				}
				DarkCheckBox {
					id: offCueCheckbox
					height: parent.height
					width: parent.width * 0.5
					checked: false
				}
			}
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "List:"
					enabled: offCueCheckbox.enabled
				}
				NumericInput {
					id: offCueList
					height: parent.height
					width: parent.width * 0.25
					value: 1
					minimumValue: 1
					maximumValue: 999
					enabled: offCueCheckbox.checked
				}
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "Number:"
					enabled: offCueCheckbox.enabled
				}
				DarkTextField {
					id: offCueNumber
					height: parent.height
					width: parent.width * 0.25
					enabled: offCueCheckbox.checked
					text: "1"
					placeholderText: "?"
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				if (triggerController.getOnMessage() !== "") {
                    onCueCheckbox.checked = true
                    onCueList.value = Utils.partAsInt(triggerController.getOnMessage(), 5, 1)
                    onCueNumber.text = Utils.partAsString(triggerController.getOnMessage(), 6, "1")
				}
				if (triggerController.getOffMessage() !== "") {
					offCueCheckbox.checked = true
					if (triggerController.getOnMessage() === "") onCueCheckbox.checked = false
					offCueList.value = Utils.partAsInt(triggerController.getOffMessage(), 5, 1)
					offCueNumber.text = Utils.partAsString(triggerController.getOffMessage(), 6, "1")
				}
			}

			function getMessages() {
				var onNumber = onCueNumber.text ? onCueNumber.text : "1"
				var offNumber = offCueNumber.text ? offCueNumber.text : "1"
				var messages = {
					"on": onCueCheckbox.checked ? "/eos/user/<USER>/cue/" + onCueList.value + "/" + onNumber + "/fire" : "",
					"off": offCueCheckbox.checked ? "/eos/user/<USER>/cue/" + offCueList.value + "/" + offNumber + "/fire" : "",
					"level": "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Cue " + (onCueCheckbox.checked ? onCueNumber.text : offCueNumber.text)
				}
				return messages
			}
		}
	}// ------------------------------ Fader
	Component {
		id: eosFaderSettings
		Column {
			anchors.fill: parent
			spacing: 5
			Row {
				height: 30
				width: parent.width
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "Bank:"
				}
				NumericInput {
					id: faderBank
					height: parent.height
					width: parent.width * 0.25
					value: 1
					minimumValue: 1
					maximumValue: 999
				}
				CenterLabel {
					height: parent.height
					width: parent.width * 0.25
					text: "Fader:"
				}
				NumericInput {
					id: faderNumber
					height: parent.height
					width: parent.width * 0.25
					value: 1
					minimumValue: 1
					maximumValue: 999
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
					value: 100
					minimumValue: 0
					maximumValue: 100
					suffix: " %"
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
					maximumValue: 100
					suffix: " %"
				}
			}

			function restoreFromMessages() {
				// restore values from current messages:
				if (triggerController.getLevelMessage() !== "") {
					faderMode.currentIndex = faderMode.model.indexOf("Level")
					faderBank.value = Utils.partAsInt(triggerController.getLevelMessage(), 5, 1)
					faderNumber.value = Utils.lastPartAsInt(triggerController.getLevelMessage(), 1)
				} else {
					faderMode.currentIndex = faderMode.model.indexOf("Switch")
					faderBank.value = Utils.partAsInt(triggerController.getOnMessage(), 5, 1)
					faderNumber.value = Utils.lastPartAsInt(triggerController.getOnMessage(), 1)
					faderOnValue.value = Utils.argumentAsFloat(triggerController.getOnMessage(), 100) * 100
					faderOffValue.value = Utils.argumentAsFloat(triggerController.getOffMessage(), 0) * 100
				}
			}


			function getMessages() {
				var level = faderMode.currentText === "Level";
				var messages = {
					"on": level ? "" : "/eos/user/<USER>/fader/" + faderBank.value + "/" + faderNumber.value + "=" + (faderOnValue.value / 100),
					"off": level ? "" : "/eos/user/<USER>/fader/" + faderBank.value + "/" + faderNumber.value + "=" + (faderOffValue.value / 100),
					"level": level ? "/eos/user/<USER>/fader/" + faderBank.value + "/" + faderNumber.value + "=" : "",
					"levelMin": 0,
					"levelMax": 1,
					"shortText": "Fader " + faderBank.value + "/" + faderNumber.value
				}
				return messages
			}
		}
	}
	// ------------------------------ Custom
	Component {
		id: eosCustomSettings
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
				// add "=" to the end of the level message if it is not there:
				var level = customLevelMessage.text
				if (level.lastIndexOf('=', level.length - 1) !== level.length - 1) {
					level = level + "="
				}

				var messages = {
					"on": customOnMessage.text,
					"off": customOffMessage.text,
					"level": level,
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
		id: eosInactiveSettings
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
