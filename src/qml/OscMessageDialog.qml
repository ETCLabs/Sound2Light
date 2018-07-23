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

// ----------- A dialog to configure the OSC messages of a triggerController ---------
Dialog {
	id: dialog
	title: "OSC Message"

	property QtObject triggerController

	// update consoleType when dialog becomes visible
	// because it could have changed:
	onVisibleChanged: mainArea.updateConsoleType()

	contentItem: Item {
		id: content
		implicitWidth: 400
		implicitHeight: 300

		DarkBlueStripedBackground {}

		Column {
			anchors.fill: parent
			anchors.margins: 10

			// --------------------- Message Type and Settings Area -----------------

			Loader {
				// This loader loads the corresponding QML file for Eos or Cobalt consoles:
				id: mainArea
				active: dialog.visible
				width: parent.width
				height: parent.height - 30
				Component.onCompleted: updateConsoleType()
				property alias triggerController: dialog.triggerController

				// sets the right conent for this loader based on the chosen console type
				// - called when Component is complete and when "visible" of dialog changes
				function updateConsoleType() {
                    var consoleType = controller.getConsoleType();
					var sourceUrl = ""
					if (consoleType === "Eos" || consoleType === "EOS") {
						sourceUrl = "qrc:/qml/OscMessagesEosArea.qml"
					} else if (consoleType === "Cobalt 7.3+") {
						sourceUrl = "qrc:/qml/OscMessagesCobaltArea.qml"
					}  else if (consoleType === "Cobalt 7.2") {
						sourceUrl = "qrc:/qml/OscMessagesCobaltOldArea.qml"
					} else if (consoleType === "ColorSource") {
						sourceUrl = "qrc:/qml/OscMessagesColorSourceArea.qml"
					} else if (consoleType === "Hog 4") {
						sourceUrl = "qrc:/qml/OscMessagesHog4Area.qml"
					}
					if (sourceUrl !== source) {
						source = sourceUrl
					}
				}

				// returns an object containing information about
				// the chosen OSC messages (on, off, level, min, max, shortText)
				// - called when "Test" or "OK" button is pressed
				// - calles getMessages() method of currently displayed Item
				function getMessages() {
					var messages = item.getMessages()
					return messages
				}
			}
			// --------------------- Test and OK Buttons -----------------
			Row {
				width: parent.width
				height: 30
				DarkButton {
					width: parent.width * 0.25
					height: parent.height
					text: "Test"
					onPressedChanged: {
						var messages = mainArea.getMessages()
						if (pressed) {
							if (messages["on"]) {
								controller.sendOscTestMessage(messages["on"])
							} else if (messages["level"]) {
								controller.sendOscTestMessage(messages["level"] + messages["levelMax"])
							}
						} else {
							if (messages["off"]) {
								controller.sendOscTestMessage(messages["off"])
							} else if (messages["level"]) {
								controller.sendOscTestMessage(messages["level"] + messages["levelMin"])
							}
						}
					}
				}
				Item {  // spacer
					width: parent.width * 0.1
					height: parent.height
				}
				DarkButton {
					width: parent.width * 0.25
					height: parent.height
					text: "Cancel"
					onClicked: {
						dialog.close()
						controller.dialogIsClosed(dialog)
					}
				}
				Item {  // spacer
					width: parent.width * 0.1
					height: parent.height
				}
				DarkButton {
					width: parent.width * 0.3
					height: parent.height
					text: "OK"
					onClicked: {
						var messages = mainArea.getMessages()
						triggerController.setOscMessages(messages["on"], messages["off"], messages["level"], messages["levelMin"], messages["levelMax"], messages["shortText"])
						dialog.close()
						controller.dialogIsClosed(dialog)
					}
				}
			}  // buttons end
		}  // column end
	}
}
