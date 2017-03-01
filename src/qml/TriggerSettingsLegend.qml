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

import "style"  // import all files in style dir

// ----------------------- Buttons and Legend displayed left of the TriggerSettings ---------------
Column {
	// the other TriggerSettings show their details depending on this property:
	property bool detailsVisible: true

	// ------------------------- Settings and About Buttons --------------------
	DarkButton {
		width: parent.width
		height: 30
		text: "Settings"
		onClicked: controller.openDialog("qrc:/qml/SettingsDialog.qml")
	}
	DarkButton {
		width: parent.width
		height: 30
		text: "Presets"
		onClicked: controller.openDialog("qrc:/qml/PresetsDialog.qml")
	}
    // ------------------------ OSC Monitor Button -----------------------------
    DarkButton {
        width: parent.width
        height: 30
        text: "OSC Monitor"
        onClicked: controller.openDialog("qrc:/qml/OscLogDialog.qml")
    }
	// ------------------------- Send OSC Checkbox ---------------------
	DarkCheckBox {
		id: sendOscCheckbox
		width: parent.width - 10
		height: 30
		x: 7
		checked: controller.getOscEnabled()
		onClicked: controller.setOscEnabled(checked)
		text: "OSC Output"
		Connections {
			target: controller
			onSettingsChanged: sendOscCheckbox.checked = controller.getOscEnabled()
		}
	}
	DarkCheckBox {
		id: oscInputCheckbox
		width: parent.width - 10
		height: 30
		x: 7
		checked: controller.getOscInputEnabled()
		onClicked: controller.setOscInputEnabled(checked)
		text: "OSC Input"
		Connections {
			target: controller
			onSettingsChanged: oscInputCheckbox.checked = controller.getOscInputEnabled()
		}
    }

	Item {  // spacer
		width: parent.width
        height: parent.height - details.height - 30*7
	}

	// ----------------------- Show / Hide Details Button -----------------
	DarkButton {
		width: parent.width
		height: 30
		text: detailsVisible ? "Hide Details" : "Show Details"
		onClicked: detailsVisible = !detailsVisible
	}

	// ----------------------- Legend Labels ---------------------
	Column {
		id: details
		visible: detailsVisible
		width: parent.width
        height: detailsVisible ? 30*6 : 0  // 6 labels with 30px height each
		CenterLabel {
			text: "Frequency"
		}
		CenterLabel {
			text: "Width [Oct.]"
		}
		CenterLabel {
			text: "On Delay"
		}
		CenterLabel {
			text: "Off Delay"
		}
		CenterLabel {
			text: "Decay"
		}
		CenterLabel {
			text: "OSC Message"
        }
	}
    CenterLabel {
        text: "OSC Mute"
    }
}
