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

// ------------- Button to use in a NumBlock -----------------
Button {
	property bool highlighted: false
	property color highlightColor: "lightgreen"
	property bool bold: true
	style: ButtonStyle {
		background: Rectangle {
			anchors.fill: parent
			border.width: 1
			border.color: highlighted ? highlightColor : "#444"
//			gradient: Gradient {
//				GradientStop { position: 0 ; color: control.pressed ? "#444" : "#333" }
//				GradientStop { position: 1 ; color: control.pressed ? "#555" : "#444" }
//			}
			gradient: Gradient {
				GradientStop { position: 0 ; color: control.pressed ? "#444" : "#3D5970" }
				GradientStop { position: 1 ; color: control.pressed ? "#555" : "#334754" }
			}
		}
		label: GreyText {
			text: control.text
			font.pointSize: 13
			font.bold: control.bold
			color: "#ccc"
			fontSizeMode: Text.Fit
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}
	}

	Rectangle {
		// Trigger Feedback Animation
		// Blinks when trigger gets activated
		anchors.fill: parent
		color: Qt.rgba(1, 1, 1, 0.3)
		opacity: 0
		NumberAnimation on opacity {
			id: triggerFeedback
			running: false
			alwaysRunToEnd: true
			from: 1
			to: 0
			duration: 300
			easing.type: Easing.OutCubic
		}
	}
	onPressedChanged: if (pressed) triggerFeedback.restart()

	function simulatePress() {
		clicked()
		triggerFeedback.restart()
	}
}
