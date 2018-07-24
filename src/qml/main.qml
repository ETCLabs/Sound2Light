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
import QtQuick.Layouts 1.2

import "style"  // import all files in style dir

// -------------------- This is the main application window. ----------------
ApplicationWindow {
	id: window
	visible: true
    width: minimalMode ? 160 : 1200
    height: minimalMode ? 200 : 800
    minimumWidth:  160
    minimumHeight: 200

	title: qsTr("ETC - Sound2Light")

    // the minimal mode property, as an alias from the bpm settings where it is manipulated
    property alias minimalMode: spectrumWithControls.minimalMode

    Action {
        id: tapAction
        text: "Tap Tempo"
        shortcut: "Return"
        onTriggered: {
            controller.triggerBeat()
        }
    }

    Action {
        id: bpmToggleAction
        text: "Enable BPM Detection"
        shortcut: "Escape"
        onTriggered: {
            controller.setBPMActive(!controller.getBPMActive())
        }
    }

	DarkBlueStripedBackground {}

	// -------------------------- SplitView ---------------------------
	DarkSplitView {
		id: splitView
		anchors.fill: parent
		orientation: Qt.Vertical

		// -------------------------- Top Area with Spectrum ---------------------------
		SpectrumWithControls {
            id: spectrumWithControls
			Layout.fillHeight: true
            Layout.minimumHeight: minimalMode ? 200 : 300
		}

		// -------------------------- Bottom Area with Trigger Settings ---------------------------
		TriggerSettingsArea {
			id: triggerSettingsArea
            visible: minimalMode ? false : true
            Layout.minimumHeight: minimalMode ? 0 : detailsVisible ? 390 : 280
            height: minimalMode ? 0 : 400
		}

	}

}
