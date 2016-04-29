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
import QtQuick.Window 2.2

import "style"  // Used to import styled NumBlockButton

// -------------- A QML Component For Numeric Input -------------
// - Can be used as a drop-in replacement for SpinBox

Item {
	id: root

	// ------------------------- Public Properties ------------------
	property real value: 0.0
	property int decimals: 0
	property real minimumValue: 0.0
	property real maximumValue: 100.0
	property real stepSize: 0.1  // not used, only for compatibility with SpinBox
	property string suffix: ""
	property string prefix: ""
	property string textReplacement: ""  // if set it replaces the displayed number and pre- and suffix


	// ------------------------- Private Properties ------------------
	QtObject {
		id: priv
		property bool justClosed: false
	}


	// ------------------------ Label + Background -------------------

	Text {
		id: displayedText
		anchors.fill: parent
		anchors.rightMargin: 10
		font.pointSize: 10
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignRight
		color: root.enabled ? "#b5b7ba" : "#555"

		text: textReplacement ? textReplacement : prefix + value.toFixed(decimals) + suffix
	}

	// background and border:
	Rectangle {
		anchors.fill: parent
		z: -1
		color: "#333333"
		border.width: 1
		border.color: numBlockWindow.visible ? "lightblue" : "#444"
	}


	// ------------------------- Setter --------------------------

	function setValueFromString(str) {
		var val = parseFloat(str)
		if (isNaN(val)) val = 0.0
		val = Math.max(minimumValue, Math.min(val, maximumValue))
		val = Math.round(val * Math.pow(10, decimals)) / Math.pow(10, decimals)
		controller.setPropertyWithoutChangingBindings(this, "value", val)
	}


	// ------------------------ NumBlock Handling ----------------------

	MouseArea {
		anchors.fill: parent

		onClicked: {
			if (!root.enabled) return
			if (priv.justClosed) return

			// ------------------ Set Preferred Local Coordinates -------------------
			// preffered horizontal position is centered above the middle of the NumericInput:
			var preferredLocalX = (root.width / 2) - (numBlockWidth / 2)
			// if NumericInput is in the lower half of the window, show the input above
			var prefferedLocalY = -numBlockHeight
			// if NumericInput is in the upper half of the window, show the input below
			if (root.mapToItem(null, 0, 0).y < (Window.height / 2)) {
				prefferedLocalY = root.height
			}

			// ---------- Translate Local Coordinates to Screen Coordiantes -----------
			var windowCoords = root.mapToItem(null, preferredLocalX, prefferedLocalY)
			var windowGeometry = controller.getWindowGeometryOfItem(root)
			var screenX = windowGeometry.x + windowCoords.x
			var screenY = windowGeometry.y + windowCoords.y


			// --------------- Check bounds to be visible ----------------
			screenX = Math.max(0, Math.min(Screen.desktopAvailableWidth - numBlockWidth, screenX))
			screenY = Math.max(0, Math.min(Screen.desktopAvailableHeight - numBlockHeight, screenY))


			// -------------------- Show Window -------------------
			numBlockWindow.x = screenX
			numBlockWindow.y = screenY

			numBlockWindow.show()
			numBlockWindow.requestActivate();
			// workaround for sizing bug on OS X:
			numBlockWindow.width = numBlockWidth
			numBlockWindow.height = numBlockHeight
		}
	}

	// ------------------------ NumBlock Window ------------------------

    property real numBlockWidth: 50*3 + 2  // 1px border * 2
    property real numBlockHeight: 50*5 + 30 + 2  // 1px border * 2

	Window {
		id: numBlockWindow
        minimumWidth: numBlockWidth
        minimumHeight: numBlockHeight
		maximumWidth: numBlockWidth
		maximumHeight: numBlockHeight
		width: numBlockWidth
		height: numBlockHeight
		flags: Qt.platform.os == "osx" ? Qt.FramelessWindowHint : Qt.Popup
        modality: Qt.platform.os == "osx" ? Qt.ApplicationModal : Qt.NonModal

		Timer {
			// Resets the justClosed flag after a short time
			id: justClosedResetTimer
			interval: 100
			repeat: false
			running: false
			onTriggered: priv.justClosed = false
		}

		onActiveChanged: {
			// close NumBlock window when it loses focus:
			if (!activeFocusItem) {
				// set justClosed flag to prevent open the popup again by the same widget
				priv.justClosed = true
				justClosedResetTimer.start()
				close()
			}
		}

		// Background and border:
		Rectangle {
			anchors.fill: parent
			color: "#444"
			border.width: 1
			border.color: "#aaa"
		}

		// NumBlock:
		Loader {
			id: contentLoader
			active: numBlockWindow.visible
			anchors.fill: parent
			anchors.margins: 1  // 1px border
			sourceComponent: numBlockComponent
			focus: true
		}
	}

	// ------------------------ NumBlock Component ------------------------

	Component {
		id: numBlockComponent

		Column {
			id: numBlock
			width: 50*3
			height: 50*5 + 30

			// Button input is stored as a string in rawNumber
			// this string will in the end be parsed by parseFloat() in setValueFromString() (see above)
			property string rawNumber: ""

			// ---------------- Top Number Label ------------
			Text {
				width: parent.width - 10
				height: 30
				font.pointSize: 12
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignRight
				text: rawNumber ? (root.prefix + rawNumber + root.suffix) : displayedText.text
				color: rawNumber ? "#b5b7ba" : "#666"
				font.italic: !rawNumber
			}
			// ---------------- Grid with Digits ------------
			Grid {
				columns: 3

				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "7"; id: digit7
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "8"; id: digit8
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "9"; id: digit9
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "4"; id: digit4
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "5"; id: digit5
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "6"; id: digit6
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "1"; id: digit1
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "2"; id: digit2
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "3"; id: digit3
				}
				NumBlockButton {
					id: backspaceButton
					width: 50; height: 50
					onClicked: {
						rawNumber = rawNumber.slice(0, rawNumber.length - 1)
					}
					bold: false
					text: "<-"
				}
				NumBlockButton {
					width: 50; height: 50; onClicked: rawNumber += text
					text: "0"; id: digit0
				}
				NumBlockButton {  // only visible if decimals are allowed
					id: dotButton
					visible: root.decimals > 0
					width: 50; height: 50
					onClicked: {
						if (rawNumber.indexOf(".") <= 0 && root.decimals > 0) rawNumber += "."
					}
					text: "."
				}
				Item {  // Placeholder if no decimals allowed
					visible: root.decimals <= 0
					width: 50; height: 50
				}
			}  // Grid

			// --------------- Esc + Enter Button ------------
			Row {
				width: 50*3
				height: 50
				NumBlockButton {
					id: escButton
					width: 50
					height: 50
					bold: false
					text: "Esc"

					onClicked: {
						numBlockWindow.close()
					}
				}
				NumBlockButton {
					id: enterButton
					width: 100
					height: 50
					bold: false
					text: "Enter"

					onClicked: {
						if (rawNumber !== "") {
							root.setValueFromString(rawNumber)
							rawNumber = ""
						}
						numBlockWindow.close()
					}
				}
			}  // Row Esc + Enter

			// -------------------- Keyboard Handling ---------------
			focus: true
			Keys.onDigit0Pressed: digit0.simulatePress()
			Keys.onDigit1Pressed: digit1.simulatePress()
			Keys.onDigit2Pressed: digit2.simulatePress()
			Keys.onDigit3Pressed: digit3.simulatePress()
			Keys.onDigit4Pressed: digit4.simulatePress()
			Keys.onDigit5Pressed: digit5.simulatePress()
			Keys.onDigit6Pressed: digit6.simulatePress()
			Keys.onDigit7Pressed: digit7.simulatePress()
			Keys.onDigit8Pressed: digit8.simulatePress()
			Keys.onDigit9Pressed: digit9.simulatePress()
			Keys.onEscapePressed: escButton.simulatePress()
			Keys.onEnterPressed: enterButton.simulatePress()
			Keys.onReturnPressed: enterButton.simulatePress()
			Keys.onPressed: {
				if (event.key === Qt.Key_Backspace) {
					backspaceButton.simulatePress()
				} else if (event.key === Qt.Key_Period || event.key === Qt.Key_Comma) {
					dotButton.simulatePress()
				}
			}

		}  // Column numBlock
	}  // Component numBlockComponent

}
