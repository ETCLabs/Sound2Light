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

import "style"  // import all files in style dir

// ---------- Displays the detected BPM and includes controls to influence the detection ------------

Item {
    id: root

    property bool minimalMode: false

    Column {
        anchors.fill: parent

        // --------------------------- Current BPM Number + Tap Button (in one) -----------------------
        Item {
            id: bpmLabelItem
            width: parent.width
            height: 60

            DarkButton {
                id: bpmLabel
                anchors.fill: parent
                text: controller.getBPM() > 0 ? controller.getBPM().toFixed(1) : "";
                fontColor: '#666666'
                fontPointSize: 36
                onClicked: controller.triggerBeat()


                // -------------------------------- Tempo LED (flashes with the beat) ----------------
                Rectangle {
                    id: tempoLed

                    width: 7
                    height: width
                    x: parent.width - 2*width
                    y: 2*width
                    radius: width / 2
                    color: "lightgreen"
                    visible: false

                    Timer {
                        id: tempoLedTimer
                        interval: 250
                        repeat: true
                        running: true
                        onTriggered: {
                            if (controller.getBPM() !== 0) {
                                tempoLed.visible = !tempoLed.visible
                                tempoLedTimer.interval = 60000 / controller.getBPM() / 2
                            } else {
                                tempoLed.visible = false
                            }
                        }
                    }
                }
            }


            Timer {
                //BPM will be updated with 1 Hz (1000 stands for 1000ms interval)
                interval: 1000; running: true; repeat: true;
                onTriggered: bpmLabelItem.refreshBPMValue()
            }

            function refreshBPMValue() {
                if (controller.getBPMManual()) {
                    bpmLabel.fontColor = "#FF6633"
                } else if (controller.bpmIsOld() || !controller.getBPMActive()) {
                    bpmLabel.fontColor = '#666666'
                } else {
                    bpmLabel.fontColor = '#b5b7ba'
                }
                bpmLabel.text = controller.getBPM() > 0 ? controller.getBPM().toFixed(1) : "";
            }

        }

        // ------------------------- Activation and Edit Button ---------------------------
        Row {
            id: activationRow
            height: 30
            width: parent.width - 2*x - editButton.width
            x: 10

            // ----------------------- Activation -----------------------------------------
            DarkCheckBox {
                id: bpmActiveCheckbox
                height: parent.height
                onCheckedChanged: {
                    if (controller.autoBpm !== checked) {
                        controller.autoBpm = checked
                    }
                }
                text: "Auto BPM"

                Component.onCompleted: checked = controller.autoBpm

                Connections {
                    target: controller
                    onAutoBpmChanged: bpmActiveCheckbox.checked = controller.autoBpm
                }
            }

            // ------------------------------ Spacer --------------------------------------
            Item {
                height: parent.height
                width: parent.width - bpmActiveCheckbox.width
            }

            // ----------------------- Edit Button (manual BPM value) ---------------------
            DarkButton {
                id: editButton
                y: 2
                width: 25
                height: width
                text: ""
                visible: !minimalMode

                onClicked: {
                    bpmNumberInput.initialized = false
                    bpmNumberInput.value = controller.getBPM()
                    bpmNumberInput.initialized = true
                    bpmNumberInput.click()
                }
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/edit.png"
                }

                // Invisible Numeric Input to popup a num pad
                NumericInput {
                    property bool initialized
                    id: bpmNumberInput
                    width: 0
                    height : 0
                    visible: false
                    decimals: 1
                    minimumValue: 0
                    maximumValue: 300
                    onValueChanged: {
                        if (initialized) {
                            controller.setBPM(bpmNumberInput.value)
                            bpmLabelItem.refreshBPMValue()
                        }
                    }
                }
            }
        }

        // ------------------------- Waveform Activation ------------------------
        DarkCheckBox {
            id: waveformVisibleCheckbox
            visible: !minimalMode
            width: parent.width - 2*x
            height: 30
            x: 10
            checked: controller.waveformVisible
            onClicked: controller.waveformVisible = checked
            text: "Waveform"
        }

        // ------------------------------ Range Picker ---------------------------
        Column {
            id: rangePicker
            width: parent.width
            height: bpmRangeLabel.height + rangeComboBox.height + 20

            // -------------------------- Text "BPM Range" -----------------------
            CenterLabel {
                id: bpmRangeLabel
                text: "BPM Range"
                color: bpmActiveCheckbox.checked ? "#BBB" : "#666"
            }

            // ------------------------ Range Selection --------------------------
            DarkComboBox {
                id: rangeComboBox
                property var ranges: ["Auto", "Low (50 - 99)", "Medium (75 - 149)", "High (100 - 199)", "Ultra (150 - 299)"]
                property bool initialized: false // boolean that is set once the value is received from the UI. to prevent a mystical automatic reset to index 0 on startup
                width: parent.width - 2*x
                x: 10
                height: 30
                model: ranges

                Component.onCompleted: rangeComboBox.updateFromController()

                onModelChanged: rangeComboBox.updateFromController()

                Connections {
                    target: controller
                    onBpmRangeChanged: rangeComboBox.updateFromController()
                }

                function updateFromController() {
                    var minBpm = controller.getMinBPM();
                    if (minBpm === 0) {
                        rangeComboBox.currentIndex = 0
                    } else if (minBpm < 75) {
                        rangeComboBox.currentIndex = 1
                    } else if (minBpm < 100) {
                        rangeComboBox.currentIndex = 2
                    } else if (minBpm < 150){
                        rangeComboBox.currentIndex = 3
                    } else {
                        rangeComboBox.currentIndex = 4
                    }

                    initialized = true
                }

                onCurrentIndexChanged: {
                    if (!initialized) return
                    if (currentIndex == 0) {
                        controller.setMinBPM(0);
                    } else if (currentIndex == 1) {
                        controller.setMinBPM(50);
                    } else if (currentIndex == 2) {
                        controller.setMinBPM(75);
                    } else if (currentIndex == 3) {
                        controller.setMinBPM(100);
                    } else if (currentIndex == 4) {
                        controller.setMinBPM(150);
                    }
                }
            }
        }

        // ---------------------------- OSC Settings ----------------------
        DarkButton {
            id: oscSettingsButton
            visible: !minimalMode
            x: 10
            width: parent.width - 2*x
            height: 30
            text: "BPM Target"
            onClicked: controller.openDialog("qrc:/qml/BPMOscMessageDialog.qml")
        }
        // ------------------------------- Mute ---------------------------
        DarkButton {
            id: muteButton
            visible: !minimalMode
            x: 10
            width: parent.width - 2*x
            height: 30
            text: ""
            highlighted: true
            highlightColor: controller.getBPMMute() ? "#FF6633" : "lightgreen"
            Image {
                id: playimage
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: height
                source: "qrc:/images/play.png"
                visible: controller.getBPMMute()
            }
            Image {
                id: pauseimage
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: height
                source: "qrc:/images/pause.png"
                visible: !controller.getBPMMute()
            }
            Connections {
                target: controller
                onBpmMuteChanged: {
                    muteButton.highlightColor = controller.getBPMMute() ? "#FF6633" : "lightgreen"
                    playimage.visible = controller.getBPMMute()
                    pauseimage.visible = !controller.getBPMMute()
                }
            }
            onClicked: controller.toggleBPMMute()
        }

        // --------------------------- Spacer -----------------------------
        Item {
            width: parent.width
            height: parent.height - bpmLabelItem.height - activationRow.height - rangePicker.height - (minimalMode ? 0 : waveformVisibleCheckbox.height + oscSettingsButton.height + muteButton.height) - minimalModeButton.height - 10
        }

        // ----------------------- Minimal Mode Button --------------------
        DarkButton {
            id: minimalModeButton
            width: 25
            height: width
            x: parent.width - width - 10
            text: ""
            onClicked: minimalMode = !minimalMode
            Image {
                anchors.fill: parent
                source: "qrc:/images/minimize.png"
                visible: !minimalMode
            }
            Image {
                anchors.fill: parent
                source: "qrc:/images/maximize.png"
                visible: minimalMode
            }
        }
    }
}
