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

// ------------- Dark styled Button -----------------
Button {
	property bool highlighted: false
	property color highlightColor: "lightgreen"
	style: ButtonStyle {
		background: Rectangle {
			anchors.fill: parent
			anchors.margins: 2
			radius: 12
			border.width: 2
			border.color: highlighted ? highlightColor : "#888"
			gradient: Gradient {
				GradientStop { position: 0 ; color: control.pressed ? "#444" : "#333" }
				GradientStop { position: 1 ; color: control.pressed ? "#555" : "#444" }
			}
		}
		label: GreyText {
			text: control.text
			font.pointSize: 10
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			elide: Text.ElideRight
		}
	}
}
