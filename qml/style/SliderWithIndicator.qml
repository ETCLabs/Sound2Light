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

// ------------- Dark styled Slider with optional level indiactor -----------------
Slider {
	property real indicator: 0.0
	property bool showIndicator: true
	property color handleActiveColor: "#B5B7BA"
	orientation: Qt.Vertical
	minimumValue: 0.0
	maximumValue: 1.0
	style: SliderStyle {
		groove: Rectangle {
			implicitWidth: 200
			implicitHeight: 8
			color: "#333333"
			radius: 8
			Rectangle {
				implicitWidth: showIndicator ? parent.width * indicator : styleData.handlePosition
				implicitHeight: 8
				color: control.enabled ? "#B5B7BA" : "#777"
				radius: 8
			}
		}
		handle: Rectangle {
			anchors.centerIn: parent
			color: (!showIndicator || indicator >= control.value) ? handleActiveColor : "#555"
			border.color: "gray"
			border.width: 2
			implicitWidth: 18
			implicitHeight: 18
			radius: 9
			visible: control.enabled
		}
	}
}
