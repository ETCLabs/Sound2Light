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
import Qt.labs.folderlistmodel 2.1

import "style"  // import all files in style dir

// --------------------- A dialog with a list of all presets -------------
Dialog {
	id: root
	title: "Preset List"
	modality: Qt.NonModal

	contentItem: Item {
		implicitWidth: 200
		implicitHeight: 255

		DarkBlueStripedBackground {}

		Column {
			anchors.margins: 10
			anchors.fill: parent
			spacing: 0
			CenterLabel {
				width: parent.width
				height: 30
				font.pointSize: 10
				text: "Presets:"
			}

			FolderListModel {
				id: presetsModel
				folder: "file:" + controller.getPresetDirectory()
				nameFilters: ["*.s2l"]
				sortField: FolderListModel.Name
			}

			// --------------- ListView with Presets -------------

			Item {
				// this item either displays the listview or a label "No Presets"
				width: parent.width
				height: parent.height - 30 - 40

				ScrollView {
					anchors.fill: parent
					ListView {
						id: presetList
						model: presetsModel
						clip: true
						delegate: Row {
							width: parent.width
                            // workaround for autosave mistakenly showing in list:
                            height: fileBaseName === "autosave" ? 0 : 30
                            visible: fileBaseName !== "autosave"
							DarkButton {
								id: button
								width: parent.width - 30
								height: 30
								text: fileBaseName
								onClicked: {
									if (controller.presetChangedButNotSaved) {
										unsavedChangesDialog.presetPath = filePath
										unsavedChangesDialog.open()
									} else {
										controller.loadPreset(filePath)
									}
								}
								Component.onCompleted: updateHighlighted()
								function updateHighlighted() {
									highlighted = (fileBaseName === controller.getPresetName())
									if (controller.presetChangedButNotSaved) {
										highlightColor = "lightblue"
									} else {
										highlightColor = "lightgreen"
									}
								}
								Connections {
									target: controller
									onPresetChanged: button.updateHighlighted()
									onPresetChangedButNotSavedChanged: button.updateHighlighted()
								}
							}

							MessageDialog {
								id: unsavedChangesDialog
								property string presetPath
								title: "Unsaved Changes"
								text: "Discard unsaved changes?"
								standardButtons: StandardButton.Cancel | StandardButton.Discard | StandardButton.Save
								modality: Qt.ApplicationModal
								onAccepted: {
									// Save Button:
									controller.saveCurrentPreset()
									controller.loadPreset(presetPath)
									unsavedChangesDialog.close()
								}
								onDiscard: {
									// Discard Button:
									controller.loadPreset(presetPath)
									unsavedChangesDialog.close()
								}
								onRejected: {
									// Cancel Button:
									unsavedChangesDialog.close()
								}
							}

							DarkButton {
								width: 30
								height: 30
								onClicked: {
									deleteDialog.presetName = fileBaseName
									deleteDialog.presetPath = filePath
									deleteDialog.open()
								}

								Image {
									anchors.fill: parent
									anchors.margins: 7
									source: "qrc:/images/trash_icon.png"
									opacity: 0.5
								}
							}


							MessageDialog {
								id: deleteDialog
								property string presetName
								property string presetPath
								title: "Delete Preset"
								text: "Delete Preset '" + presetName + "'?"
								standardButtons: StandardButton.Cancel | StandardButton.Yes
								modality: Qt.ApplicationModal
								onYes: {
									controller.deletePreset(presetPath)
									deleteDialog.close()
								}
								onRejected: {
									deleteDialog.close()
								}
							}

						}  // end delegate
					}  // end ListView
				}  // end ScrollView

				CenterLabel {
					text: "No Presets"
					visible: presetsModel.count === 0
				}
			}

			// -------------------------- Close Button -------------------------

			Item {  // spacer
				width: 1
				height: 10
			}

			DarkButton {
				width: parent.width
				height: 30
				text: "Close"
				onClicked: {
					root.close()
					controller.dialogIsClosed(root)
				}
			}
		}
	}
}
