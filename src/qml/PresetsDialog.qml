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

// --------------------- A dialog with options for presets -------------
Dialog {
	id: root
	title: "Presets"
	modality: Qt.ApplicationModal

	contentItem: Item {
		implicitWidth: 200
		implicitHeight: 285

		DarkBlueStripedBackground {}

		// --------------------------- Column with Buttons -------------------

		Column {
			anchors.margins: 10
			anchors.fill: parent
			spacing: 0

			// ------------ Info Label
			CenterLabel {
				width: parent.width
				height: 15
				font.pointSize: 12
				text: "Preset:"
			}
			CenterLabel {
				id: statusText
				width: parent.width
				height: 30
				font.pointSize: 10
				Component.onCompleted: updateText()
				function updateText() {
					text = (controller.presetName ? controller.presetName : "No Preset")
							+ (controller.presetChangedButNotSaved ? " [modified]" : "")
				}
				Connections {
					target: controller
					onPresetNameChanged: statusText.updateText()
					onPresetChangedButNotSavedChanged: statusText.updateText()
				}
			}

			DarkButton {
				width: parent.width
				height: 35
				text: "Preset List"
                onClicked: {
					root.close()
                    controller.dialogIsClosed(root)
                    controller.openDialog("qrc:/qml/PresetListDialog.qml")
				}
			}
			DarkButton {
				width: parent.width
				height: 35
				text: "Save"
				onClicked: {
					controller.saveCurrentPreset()
					root.close()
					controller.dialogIsClosed(root)
				}
			}
			DarkButton {
				width: parent.width
				height: 35
				text: "Save As"
				onClicked: {
					//controller.openDialog("qrc:/qml/SavePresetAsDialog.qml")
					controller.openSavePresetAsDialog()
					root.close()
					controller.dialogIsClosed(root)
				}
			}
			DarkButton {
				width: parent.width
				height: 35
				text: "Load From File"
				onClicked: {
					if (controller.presetChangedButNotSaved) {
						unsavedChangesDialog.open()
					} else {
						//controller.openDialog("qrc:/qml/LoadPresetDialog.qml")
						controller.openLoadPresetDialog()
                    }
				}

				MessageDialog {
					id: unsavedChangesDialog
					title: "Unsaved Changes"
					text: "Discard unsaved changes?"
					standardButtons: StandardButton.Cancel | StandardButton.Discard | StandardButton.Save
					modality: Qt.ApplicationModal
					onAccepted: {
						// Save Button:
						controller.saveCurrentPreset()
						//controller.openDialog("qrc:/qml/LoadPresetDialog.qml")
						controller.openLoadPresetDialog()
						unsavedChangesDialog.close()
                        root.close()
                        controller.dialogIsClosed(root)
					}
					onDiscard: {
						// Discard Button:
						//controller.openDialog("qrc:/qml/LoadPresetDialog.qml")
						controller.openLoadPresetDialog()
						unsavedChangesDialog.close()
                        root.close()
                        controller.dialogIsClosed(root)
					}
					onRejected: {
						// Cancel Button:
						unsavedChangesDialog.close()
                        root.close()
                        controller.dialogIsClosed(root)
					}
				}

			}

			DarkButton {
				width: parent.width
				height: 35
				text: "Reset"
				onClicked: {
                    resetDialog.open()
				}


				MessageDialog {
					id: resetDialog
					title: "Reset to Factory Settings"
					text: "Reset to Factory Settings?"
					standardButtons: StandardButton.Cancel | StandardButton.Yes
					modality: Qt.ApplicationModal
					onYes: {
						controller.resetPreset()
						resetDialog.close()
                        root.close()
                        controller.dialogIsClosed(root)
					}
					onRejected: {
						resetDialog.close()
                        root.close()
                        controller.dialogIsClosed(root)
					}
				}

			}

			Item {
				width: parent.width
				height: 10
			}

			// ----------------------- OK Button ----------------------
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
