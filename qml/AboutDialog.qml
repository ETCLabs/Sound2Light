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
import QtQuick.Dialogs 1.2

import "style"  // import all files in style dir

// ---------------------------- About Dialog ----------------------
Dialog {
	id: dialog
	title: "About"

	contentItem: Item {
		implicitWidth: 600
		implicitHeight: 400

		DarkBlueStripedBackground {}

		// ------------------------ Top Area with Text and Logo ----------------
		Column {
			anchors.margins: 20
			anchors.fill: parent

			Row {
				width: parent.width
				height: parent.height - 40

				// ------------------------ Left Area with Text --------------------
				Column {
					id: textArea
					height: parent.height
					width: parent.width - logoArea.width

					GreyText {
						width: parent.width
						height: 40
						font.pointSize: 14
						text: "Sound2Light"
					}
					GreyText {
						width: parent.width
						height: 30
						font.pointSize: 10
						text: "Version: " + controller.getVersionString()
					}
				}

				// ------------------------- ETC Logo on the right ---------------------
				Item {
					id: logoArea
					height: parent.height
					width: 200
					Image {
						source: "qrc:/images/icons/etclogo.png"
						width: 200
						height: 200
						anchors.centerIn: parent
						MouseArea {
							anchors.fill: parent
							onClicked: Qt.openUrlExternally("http://www.etcconnect.com")
						}
					}
				}
			}

			// --------------- Bottom Area with OK Button and Copyright notes -------------
			Row {
				id: bottomRow
				width: parent.width
				height: 40
				GreyText {
					height: parent.height
					width: parent.width * 0.4
					text: "www.etcconnect.com"
					font.underline: true
					verticalAlignment: Text.AlignBottom
					horizontalAlignment: Text.AlignHCenter
					MouseArea {
						anchors.fill: parent
						onClicked: Qt.openUrlExternally("http://www.etcconnect.com")
					}
				}
				DarkButton {
					height: parent.height
					width: parent.width * 0.2
					text: "OK"
					onClicked: {
						dialog.close()
						controller.dialogIsClosed(dialog)
					}
				}
				GreyText {
					height: parent.height
					width: parent.width * 0.4
					text: "© ETC GmbH"
					verticalAlignment: Text.AlignBottom
					horizontalAlignment: Text.AlignHCenter
				}

			}
		}
	}
}
