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

// --------------------- A dialog to show the OSC log / monitor -------------
Dialog {
	id: root
	title: "OSC Monitor"
	modality: Qt.platform.os == "osx" ? Qt.NonModal : Qt.WindowModal

	contentItem: Item {
		implicitWidth: 700
		implicitHeight: 500

		Component.onCompleted: {
			// initialize checkbox values:
			logIncomingCheckbox.checked = controller.getOscLogIncomingIsEnabled()
			logOutgoingCheckbox.checked = controller.getOscLogOutgoingIsEnabled()
		}

		DarkBlueStripedBackground {}

		Column {
			anchors.margins: 10
			anchors.fill: parent
			spacing: 0

			// ---------------------- Incoming / Outgoing Checkboxes -----------

			Row {
				width: parent.width
				height: 30
				DarkCheckBox {
					id: logIncomingCheckbox
					width: parent.width / 2
					height: 30
					onClicked: controller.enableOscLogging(logIncomingCheckbox.checked, logOutgoingCheckbox.checked)
					text: "Incoming Messages"
					textColor: "#88d"
				}
				DarkCheckBox {
					id: logOutgoingCheckbox
					width: parent.width / 2
					height: 30
					onClicked: controller.enableOscLogging(logIncomingCheckbox.checked, logOutgoingCheckbox.checked)
					text: "Outgoing Messages"
				}
			}

			// --------------- ListView with Log -------------

			Item {
				// this item either displays the listview or a label "No Messages"
				width: parent.width
				height: parent.height - 30 - 40

				ScrollView {
					id: scrollView
					anchors.fill: parent
					visible: logList.count !== 0
					ListView {
						id: logList
						model: controller.getOscLog()
						clip: true
						verticalLayoutDirection: ListView.BottomToTop
						delegate: Text {
							height: contentHeight + 3
							width: parent.width
							wrapMode: Text.Wrap
							text: modelData
							color: (modelData.indexOf("[In]") !== -1) ? "#88d" : "#aaa"
							font.pointSize: 10
						}  // end delegate

						onCountChanged: {
							// emit contentYChanged signal, otherwise scrollbar position will be wrong
							contentYChanged()
						}

						Connections {
							target: controller
							onOscLogChanged: {
								logList.model = controller.getOscLog()
							}
						}
					}  // end ListView
				}  // end ScrollView

				CenterLabel {
					text: "No Messages"
					visible: logList.count === 0
				}
			}

			// -------------------------- Clear and Close Button -------------------------

			Item {  // spacer between Log and Buttons
				width: 1
				height: 10
			}

			Row {
				width: parent.width
				height: 30
				DarkButton {
					height: 30
					width: 100
					text: "Clear"
					onClicked: controller.clearOscLog()
				}

				Item {  // spacer between both buttons
					height: 1
					width: 20
				}

				DarkButton {
					width: parent.width - 120
					height: 30
					text: "Close"
					onClicked: {
						root.close()
						controller.dialogIsClosed(root)
					}
				}
			}
		}
	}
}
