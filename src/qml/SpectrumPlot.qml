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

import "style"  // import all files in style dir

// ------------------------- A widget that displayes a spectrum ---------------
Grid {
	columns: 2
	spacing: 0

	// ----------------------- Legend at the left -------------
	Rectangle {
		id: leftLegend
		width: 20
		height: parent.height - bottomLegend.height
		color: "#333333"
		GreyText {
			text: "dB"
			x: 2
			visible: controller.decibelConversion
		}
		GreyText {
			text: "0"
			x: 2
			y: 20
			visible: controller.decibelConversion
		}
		GreyText {
			text: "-60"
			x: 2
			y: parent.height - 20
			visible: controller.decibelConversion
		}
	}

	// ----------------------- Spectrum Plot -----------------------
	Canvas {
		id: canvas
		width: parent.width - leftLegend.width
		height: parent.height - bottomLegend.height
		onPaint: {
			// --- Prepare ---
			var points = controller.getSpectrumPoints()
			var pointCount = points.length;
			var pointWidth = width / pointCount
			var ctx = canvas.getContext('2d');
			ctx.fillStyle = "#272727";
			ctx.fillRect (0, 0, width, height);

			// --- Spectrum Geometry ---
			ctx.beginPath();
			ctx.moveTo(0, height);
			var lastValue = 1.0
			for (var i=0; i<pointCount; i++) {
				//if (points[i] !== lastValue) {
				//	ctx.lineTo(pointWidth*i, height * (1 - points[i]))
				//}
				ctx.lineTo(pointWidth*i, height * (1 - points[i]))
				lastValue = points[i]
				//ctx.lineTo(pointWidth*i, height * (1 - Math.random()))
			}
			ctx.lineTo(width, height);

			// --- Fill and Stroke Spectrum ---
			var gradient = ctx.createLinearGradient(0, 0, 0, height);
			gradient.addColorStop(0., '#B5B7BA');  // top
			gradient.addColorStop(1., '#555');  // bottom
			ctx.fillStyle = gradient;
			ctx.fill();
			ctx.lineWidth = 1
			ctx.lineJoin = "bevel"
			ctx.strokeStyle = "#B5B7BA"
			ctx.stroke();
		}
		Timer {
			// Spectrum will be updated with 50Hz:
			interval: 20; running: true; repeat: true
			onTriggered: canvas.requestPaint()
		}
		Rectangle {
			// left border
			width: 1
			height: parent.height
			color: "#1C2C40"
		}
		Rectangle {
			// bottom border
			width: parent.width
			height: 1
			y: parent.height - 1
			color: "#1C2C40"
		}

		// clip to prevent BandpassPreviews be drawn outside Spectrum:
		clip: true


		// --------------------------- BandpassPreviews ----------------------

		BandpassPreviewInSpectrum {
			anchors.fill: parent
			bandpassController: bassController
			color: Qt.rgba(1, 0.1, 0.1, 1)
		}

		BandpassPreviewInSpectrum {
			anchors.fill: parent
			bandpassController: loMidController
			color: Qt.rgba(1, 0.8, 0.1, 1)
		}

		BandpassPreviewInSpectrum {
			anchors.fill: parent
			bandpassController: hiMidController
			color: Qt.rgba(0.1, 1.0, 0.1, 1)
		}

		BandpassPreviewInSpectrum {
			anchors.fill: parent
			bandpassController: highController
			color: Qt.rgba(0.1, 0.2, 1, 1)
		}
	}

	// ---------------- Corner at the bottom left ------------
	Rectangle {
		id: bottomLeftCorner
		width: leftLegend.width
		height: bottomLegend.height
		color: "#333333"
	}

	// ----------------------- Legend at the bottom ------------------
	Rectangle {
		id: bottomLegend
		width: parent.width - leftLegend.width
		height: 20
		color: "#333333"
		GreyText {
			text: "100"
			x: parent.width * 0.22
		}
		GreyText {
			text: "440"
			x: parent.width * 0.43
		}
		GreyText {
			text: "1k"
			x: parent.width * 0.55
		}
		GreyText {
			text: "5k"
			x: parent.width * 0.77
		}
		GreyText {
			text: "Hz"
			x: parent.width - 20
		}
	}
}
