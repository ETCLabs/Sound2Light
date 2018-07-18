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
import QtQuick.Dialogs 1.2

import "style"  // import all files in style dir

// --------------------- A dialog containing the application settings -------------
Dialog {
	id: root
	title: "Settings"
	modality: Qt.platform.os == "osx" ? Qt.NonModal : Qt.ApplicationModal

	// update the input device list when the dialog becomes visible
	// because it could have changed:
	onVisibleChanged: {
		if (visible) {
			inputList.model = []
			inputList.model = controller.getAvailableInputs()
		}
	}

	contentItem: Item {
		implicitWidth: 270
		implicitHeight: 400

		DarkBlueStripedBackground {}

		Column {
			anchors.margins: 10
			anchors.fill: parent

			// ----------------------- OSC IP Address ----------------------
			Row {
				width: parent.width
				height: 30
				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "OSC IP Address:"
				}
				DarkTextField {
					width: parent.width  * 0.5
					height: parent.height
					text: controller.getOscIpAddress()
					onTextChanged: if (controller.getOscIpAddress() !== text) controller.setOscIpAddress(text)
				}
			}

			// ----------------------- OSC Protocol ----------------------
			Row {
				width: parent.width
				height: 30

				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "OSC Protocol:"
				}

				DarkComboBox {
					property var protocols: ["UDP", "TCP 1.0", "TCP 1.1"]
					property bool initialized: false
					width: parent.width  * 0.5
					height: parent.height
					model: protocols
					Component.onCompleted: {
						var tcp = controller.getUseTcp()
						var osc_1_1 = controller.getUseOsc_1_1()
						if (tcp) {
							if (osc_1_1) {
								currentIndex = protocols.indexOf("TCP 1.1")
							} else {
								currentIndex = protocols.indexOf("TCP 1.0")
							}
						} else {
							currentIndex = protocols.indexOf("UDP")
						}
						initialized = true
					}
					onCurrentTextChanged: {
						if (!initialized) return
						if (currentText === "UDP") {
							controller.setUseTcp(false)
						} else if (currentText === "TCP 1.0") {
							controller.setUseTcp(true)
							controller.setUseOsc_1_1(false)
						} else if (currentText === "TCP 1.1") {
							controller.setUseTcp(true)
							controller.setUseOsc_1_1(true)
						}
					}
				}
			}

			// ----------------------- OSC Ports ----------------------
			Connections {
				target: controller
				onUseTcpChanged: {
					udpTxPort.visible = !controller.getUseTcp()
					udpRxPort.visible = !controller.getUseTcp()
					tcpPort.visible = controller.getUseTcp()
				}
			}

			// ------------ OSC UDP Tx Port
			Row {
				id: udpTxPort
				width: parent.width
				height: visible ? 30 : 0
				visible: !controller.getUseTcp()

				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "OSC UDP Tx Port:"
				}
				NumericInput {
					width: parent.width  * 0.5
					height: parent.height
					minimumValue: 0
					maximumValue: 65535
					value: controller.getOscUdpTxPort()
					onValueChanged: if (controller.getOscUdpTxPort() !== value) controller.setOscUdpTxPort(value)
				}
			}

			// ------------ OSC UDP Rx Port
			Row {
				id: udpRxPort
				width: parent.width
				height: visible ? 30 : 0
				visible: !controller.getUseTcp()

				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "OSC UDP Rx Port:"
				}
				NumericInput {
					width: parent.width  * 0.5
					height: parent.height
					minimumValue: 0
					maximumValue: 65535
					value: controller.getOscUdpRxPort()
					onValueChanged: if (controller.getOscUdpRxPort() !== value) controller.setOscUdpRxPort(value)
				}
			}

			// ------------ OSC TCP Port
			Row {
				id: tcpPort
				width: parent.width
				height: visible ? 30 : 0
				visible: controller.getUseTcp()

				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "OSC TCP Port:"
				}
				NumericInput {
					width: parent.width  * 0.5
					height: parent.height
					minimumValue: 0
					maximumValue: 65535
					value: controller.getOscTcpPort()
					onValueChanged: if (controller.getOscTcpPort() !== value) controller.setOscTcpPort(value)
				}
			}

			// ----------------------- Console Type ----------------------
			Row {
				width: parent.width
				height: 30

				CenterLabel {
					width: parent.width  * 0.5
					height: parent.height
					text: "Console Type:"
				}

				DarkComboBox {
					property var consoleTypes: ["Eos", "Cobalt 7.2", "Cobalt 7.3+", "ColorSource", "Hog 4"]
					width: parent.width  * 0.5
					height: parent.height
					model: consoleTypes
					currentIndex: consoleTypes.indexOf(controller.getConsoleType())
					onCurrentTextChanged: {
						if (currentText && currentText !== controller.getConsoleType()) {
							controller.setConsoleType(currentText)
						}
					}
				}
			}

			// ----------------------- Input Device List ----------------------
			GreyText {
				width: parent.width
				height: 40
				text: "Input:"
				font.pointSize: 12
				horizontalAlignment: Text.AlignLeft
				verticalAlignment: Text.AlignVCenter
			}
			ExclusiveGroup { id: inputListGroup }
			ScrollView {
				width: parent.width
				height: parent.height - 40*2 - 30*5 - 20 - (udpTxPort.visible ? 30 : 0)
				ListView {
					id: inputList
					model: controller.getAvailableInputs()
					delegate: RadioButton {
						height: 30
						width: inputList.width
						text: modelData
						Component.onCompleted: {
							checked = (modelData === controller.getActiveInputName())
						}

						exclusiveGroup: inputListGroup
						onClicked: {
							checked = true
							if (modelData && modelData !== controller.getActiveInputName()) {
								controller.setInputByName(modelData)
							}
						}
						style: RadioButtonStyle {
							label: GreyText {
								text: control.text
							}
						}
					}
				}
			}

			// ----------------------- OK and About Button ----------------------

            DarkButton {
                width: parent.width
                height: 30
                text: "About"
                highlightColor: "#555"
                highlighted: true
                onClicked: controller.openDialog("qrc:/qml/AboutDialog.qml")
            }


			Item {  // spacer
				height: 20
				width: 1
			}

			DarkButton {
				width: parent.width
				height: 40
				text: "OK"
				onClicked: {
					root.close()
					controller.dialogIsClosed(root)
				}
			}
		}
	}
}
