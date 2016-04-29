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

// ---------------------- A component to simplify mouse and touch input ----------------
MultiPointTouchArea {
	mouseEnabled: true
	maximumTouchPoints: 1
	property bool pressed: false
	property var firstTouch: false
	property var secondTouch: false
	signal touchDown(TouchPoint touch)
	signal touchMove(TouchPoint touch)
	signal touchUp(TouchPoint touch)
	signal secondTouchDown(TouchPoint touch)

	onGestureStarted: {
		// gesture.grab()
		// console.log("Gesture started.")
	}

	onPressed: {
		var touch = touchPoints[0]
		if (firstTouch === false) {
			firstTouch = touch
		} else if (secondTouch === false) {
			secondTouch = touch
			secondTouchDown(touch)
		} else {
			return
		}
		pressed = true
		touchDown(touch)
	}

	onUpdated: {
		touchMove(touchPoints[0])
	}

	onReleased: {
		var touch = touchPoints[0]
		if (touch !== firstTouch && touch !== secondTouch) {
			return
		}
		touchUp(touch)
		if (touch === firstTouch) {
			firstTouch = secondTouch
			secondTouch = false
			if (!firstTouch) pressed = false
		} else if (touch === secondTouch) {
			secondTouch = false
		}
	}

	onCanceled: {
		pressed = false
		var touch = touchPoints[0]
		if (touch !== firstTouch && touch !== secondTouch) {
			return
		}
		touchUp(touch)
		if (touch === firstTouch) {
			firstTouch = secondTouch
			secondTouch = false
			if (!firstTouch) pressed = false
		} else if (touch === secondTouch) {
			secondTouch = false
		}
	}
}
