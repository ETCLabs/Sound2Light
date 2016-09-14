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

// ------------------ Visual preview of a BandpassTrigger ------------------
Item {
	id: root
	property QtObject bandpassController
	property color color: "darkblue"

	// ----------------------------  Visuals ----------------------

	// ----------- Area of the Bandpass -------
	Rectangle {
		id: area
		color: root.color
		height: parent.height * bandpassController.getThreshold()
		y: parent.height - height
		width: parent.width * bandpassController.getWidth()
		// midFreqNormalized is the position of the mid frequency in the scaled spectrum in the range 0...1
		x: parent.width * bandpassController.getMidFreqNormalized() - (width / 2)
		opacity: bandpassController.active ? 0.4: 0.15

		function update() {
			height = parent.height * bandpassController.getThreshold()
			width = parent.width * bandpassController.getWidth()
			x = parent.width * bandpassController.getMidFreqNormalized() - (width / 2)
		}
	}
	Connections {
		// update bandpass area when a parameter changes:
		target: bandpassController
		onParameterChanged: area.update()
	}

	onWidthChanged: area.update()
	onHeightChanged: area.update()

	// ---------- Line at the top -------------
	Rectangle {
		id: marker
		property bool active: false
        color: bandpassController.active ? "#FFFFFF" : "#95979A"
		height: 2
		width: area.width
		y: area.y
		x: area.x
	}


	// ------------------------ Interaction ---------------------------

	// ------- Top area that allows dragging in each direction ----
	TouchArea {
		property real startPosY: 0.0
		property real initialThreshold: 0.0
		property real startPosX: 0.0
		property real initialMidFreq: 0.0
		id: thresholdMouseArea
		width: area.width
		height: Math.min(area.height, 25)
		x: area.x
		y: area.y
		Rectangle {
			anchors.fill: parent
			color: Qt.rgba(1, 1, 1, 0.15)
		}

		onTouchDown: {
			startPosY = touch.sceneY
			initialThreshold = bandpassController.getThreshold()
			startPosX = touch.sceneX
			initialMidFreq = bandpassController.getMidFreqNormalized()
		}

		onTouchMove: {
			var thresholdChange = (startPosY - touch.sceneY) / root.height
			var newThreshold = Math.max(0.05, Math.min(initialThreshold + thresholdChange, 1.0))
			bandpassController.setThreshold(newThreshold)
			var freqChange = (touch.sceneX - startPosX) / root.width
			var newFreq = Math.max(0.0, Math.min(initialMidFreq + freqChange, 1.0))
			bandpassController.setMidFreqNormalized(newFreq)
		}
	}

	// ------- Bottom area that allows dragging horizontally only but also changing width ------
	TouchArea {
		property real lastX: 0.0
		property real lastTouchDistanceX: 0.0
		property real lastTouchDistanceY: 0.0
		property bool leftSide: false
		property real acurateMidFreq: 0.0
		id: freqMouseArea
		width: area.width
		height: Math.max(area.height - 25, 0)
		x: area.x
		y: area.y + 25
		maximumTouchPoints: 2

		onTouchDown: {
			lastX = touch.sceneX
			// midFreq is a rounded value in bandpassController
			// acurateMidFreq contains the real calculated midFreq:
			acurateMidFreq = bandpassController.getMidFreqNormalized()
			// check if touch is on left or right side of midFreq:
			var touchPosInSpectrum = root.mapFromItem(null, touch.sceneX, touch.sceneY)
			leftSide = touchPosInSpectrum.x < (acurateMidFreq * root.width)
		}

		onSecondTouchDown: {
			lastTouchDistanceX = Math.abs(firstTouch.sceneX - secondTouch.sceneX)
			lastTouchDistanceY = Math.abs(firstTouch.sceneY - secondTouch.sceneY)
		}

		onTouchMove: {
			// deltaX is the amount of pixel the touch moved since last event:
			var deltaX = touch.sceneX - lastX
			lastX = touch.sceneX

			if (secondTouch) {
				// a second touch exists, this is a resize gesture
				// calculate change in distance of both touches in each direction:
				var touchDistanceX = Math.abs(firstTouch.sceneX - secondTouch.sceneX)
				var distanceChangeX = touchDistanceX - lastTouchDistanceX
				lastTouchDistanceX = touchDistanceX
				var touchDistanceY = Math.abs(firstTouch.sceneY - secondTouch.sceneY)
				var distanceChangeY = touchDistanceY - lastTouchDistanceY
				lastTouchDistanceY = touchDistanceY

				if (touchDistanceX > touchDistanceY) {
					// horizontal pinch -> change width:
					// calculate new width:
					var newWidth = bandpassController.getWidth() + (distanceChangeX / root.width)
					// limit the width value:
					newWidth = Math.max(0.05, Math.min(newWidth, 1.0))
					bandpassController.setWidth(newWidth)
				} else {
					// vertical pinch -> change threshold:
					// calculate new threshold:
					var newThreshold = bandpassController.getThreshold() + (distanceChangeY / root.height)
					// limit the value:
					newThreshold = Math.max(0.05, Math.min(newThreshold, 1.0))
					bandpassController.setThreshold(newThreshold)
				}

			} else if (controller.controlIsPressed()) {
				// CTRL is pressed, width will be changed
				// if the touch is on the left of the bandpass the width must increase in that direction:
				var touchPosInSpectrum = root.mapFromItem(null, touch.sceneX, touch.sceneY)
				if (leftSide) deltaX *= -1
				// calculate new width:
				var newWidth = bandpassController.getWidth() + (deltaX / root.width) * 2
				// limit the width value:
				newWidth = Math.max(0.05, Math.min(newWidth, 1.0))
				bandpassController.setWidth(newWidth)

			} else {
				// this is a normal drag, midFreq will be changed
				acurateMidFreq += (deltaX / root.width)
				bandpassController.setMidFreqNormalized(acurateMidFreq)
			}
		}
	}

}
