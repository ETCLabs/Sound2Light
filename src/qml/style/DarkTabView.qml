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

// ------------- Dark styled TabView -----------------
TabView {
	style: TabViewStyle {
		frameOverlap: 1
		tab: Rectangle {
			color: styleData.selected ? "#1C2C40" :"#333333"
			border.color:  "#B5B7BA"
			implicitWidth: Math.max(text.width + 10, 80)
			implicitHeight: 30
			radius: 2
			Text {
				id: text
				anchors.centerIn: parent
				text: styleData.title
				color: "#B5B7BA"
				font.pointSize: 10
			}
		}
		frame: Rectangle {
			color: "transparent"
			border.width: 1
			border.color: "#B5B7BA"
		}
	}

	// returns the currently displayed item
	function getCurrentTabContent() {
		var tab = contentItem.children[currentIndex]
		return tab.item
	}
}
