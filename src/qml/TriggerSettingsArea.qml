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

// -------------- Bottom Area with Trigger Settings -------------------

Row {
	id: triggerSettings
	signal showSettingsDialog
	signal showAboutDialog
	signal showLoadPresetDialog
	signal showSavePresetAsDialog
	property alias detailsVisible: legend.detailsVisible
	// ------------------------------------ Legend -------------------
	TriggerSettingsLegend {
		id: legend
		width: parent.width / 7
		height: parent.height
	}
	// ------------------------------------ Bass -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "Bass"
		triggerController: bassController
		color: Qt.rgba(1, 0.1, 0.1, 1)
	}
	// ------------------------------------ LoMid -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "LoMid"
		triggerController: loMidController
		color: Qt.rgba(1, 0.8, 0.1, 1)
	}
	// ------------------------------------ HiMid -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "HiMid"
		triggerController: hiMidController
		color: Qt.rgba(0.1, 1.0, 0.1, 1)
	}
	// ------------------------------------ High -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "High"
		triggerController: highController
		color: Qt.rgba(0.1, 0.2, 1, 1)
	}

	// ------------------------------------ Level -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "Level"
		triggerController: envelopeController
		color: Qt.rgba(0, 0, 0, 0)
		isBandpass: false
	}
	// ------------------------------------ Silence -------------------
	TriggerSettings {
		detailsVisible: legend.detailsVisible
		width: parent.width / 7
		height: parent.height
		triggerName: "Silence"
		triggerController: silenceController
		color: Qt.rgba(0, 0, 0, 0)
		isBandpass: false
	}
}
