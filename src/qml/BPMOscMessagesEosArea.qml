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
import QtQuick.Dialogs 1.2

import "oscUtils.js" as Utils

import "style"  // import all files in style dir



// ----------- Content of the BPM OSC dialog for EOS console type -----------
Item {
    id: eosArea
    anchors.fill: parent

    property var messages

    Component.onCompleted: {
        restoreFromMessages(controller.getBPMOscCommands())
    }

    function firstFreeEffectMessage() {
        for (var freeEffect = 1; freeEffect < 9999; freeEffect++) {
            var occupied = false
            for (var i = 0; i < messages.length; i++) {
                var message = messages[i]
                if (message.indexOf("/eos/user/0/newcmd/Effect/") === -1) {
                    continue
                }

                var effectNo = Utils.partAsInt(message, 6, 0)
                if (effectNo === freeEffect) {
                    occupied = true
                    break
                }
            }
            if (!occupied) {
                break
            }
        }
        return "/eos/user/0/newcmd/Effect/" + freeEffect + "/BPM/<BPM1>/#"
    }

    function addMessage() {
        var message = firstFreeEffectMessage()
        messages.push(message)
        restoreFromMessages(messages)
        listView.positionViewAtEnd()
    }

    function setMessageAtIndex(index, message) {
        var effectNo = Utils.partAsFloat(message, 6, 0)
        for (var i = 0; i < messages.length; i++) {
            if (i === index) {
                continue
            }

            var otherMessage = messages[i]
            var otherEffectNo = Utils.partAsInt(otherMessage, 6, 0)
            if (effectNo === otherEffectNo) {

                invalidFXNumberDialog.text = "Effect " + effectNo + " is already in use"
                invalidFXNumberDialog.open()
                return
            }
        }
        messages[index] = message
    }

    MessageDialog {
        id: invalidFXNumberDialog
        title: "Invalid Effect Number"
        modality: Qt.ApplicationModal
        onAccepted: {
            invalidFXNumberDialog.close()
        }
    }

    function getMessages() {
        return messages
    }

    function restoreFromMessages(newMessages) {
        messages = newMessages
        if (newMessages.length === 0) {
            listView.visible = false
            noEffectsText.visible = true
            listView.model = messages
        } else {
            listView.visible = true
            noEffectsText.visible = false
            listView.model = messages
        }
    }

    Component {
        id: effectLine
        Item {
            id: effectLineItem
            height: 108
            width: 340

            Component.onCompleted: {
                restoreFromMessage(modelData)
                setMessageAtIndex(index, getMessage())
            }

            Column {
                anchors.fill: parent
                spacing: 3
                Row {
                    height: 30
                    width: parent.width
                    CenterLabel {
                        height: parent.height
                        width: parent.width * 0.5
                        text: "Effect Number:"
                    }
                    NumericInput {
                        id: effectNumber
                        height: parent.height
                        width: parent.width * 0.5
                        decimals: 2
                        minimumValue: 1
                        maximumValue: 9999
                        onValueChanged: {
                            setMessageAtIndex(index, effectLineItem.getMessage())
                            restoreFromMessage(messages[index])
                        }
                    }
                }
                Row {
                    height: 30
                    width: parent.width
                    CenterLabel {
                        height: parent.height
                        width: parent.width * 0.5
                        text: "Beat Multiplier:"
                    }
                    DarkComboBox {
                        id: multiplier
                        height: parent.height
                        width: parent.width * 0.5
                        model: ["1/32", "1/16", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16", "32"]
                        onCurrentIndexChanged:  {
                            messages[index] = effectLineItem.getMessage()
                        }
                    }
                }
                Row {
                    height: 30
                    width: parent.width
                    // Spacer
                    Item {
                        height: parent.height
                        width: parent.width * 0.5
                    }
                    DarkButton {
                        id: remove
                        height: parent.height
                        width: parent.width * 0.5
                        text: "Remove"
                        onPressedChanged: {
                            if (!pressed) {
                                messages.splice(index,1)
                                restoreFromMessages(messages)
                            }
                        }
                    }
                }
                Row {
                    height: 1
                    width: parent.width
                    Rectangle {
                        height: parent.height
                        width: parent.width
                        color: "#789"
                    }
                }
            }

            function restoreFromMessage(message) {
                if (message.indexOf("/eos/user/0/newcmd/Effect/") === -1) {
                    message = firstFreeEffectMessage()
                }

                effectNumber.value = Utils.partAsFloat(message,6,1)
                var bpmQualifier = Utils.partAsString(message,8,"<BPM>")
                if (bpmQualifier === "<BPM>" || bpmQualifier === "<BPM1>") {
                    multiplier.currentIndex = 5
                } else if (bpmQualifier === "<BPM1-2>") {
                    multiplier.currentIndex = 4
                } else if (bpmQualifier === "<BPM1-4>") {
                    multiplier.currentIndex = 3
                } else if (bpmQualifier === "<BPM1-8>") {
                    multiplier.currentIndex = 2
                } else if (bpmQualifier === "<BPM1-16>") {
                    multiplier.currentIndex = 1
                } else if (bpmQualifier === "<BPM1-32>") {
                    multiplier.currentIndex = 0
                } else if (bpmQualifier === "<BPM2>") {
                    multiplier.currentIndex = 6
                } else if (bpmQualifier === "<BPM4>") {
                    multiplier.currentIndex = 7
                } else if (bpmQualifier === "<BPM8>") {
                    multiplier.currentIndex = 8
                } else if (bpmQualifier === "<BPM16>") {
                    multiplier.currentIndex = 9
                } else if (bpmQualifier === "<BPM32>") {
                    multiplier.currentIndex = 10
                }
            }

            function getMessage() {
                var effect = effectNumber.value.toString()
                var bpmQualifier = "<BPM>"
                switch (multiplier.currentIndex) {
                case 0:
                    bpmQualifier = "<BPM1-32>"
                    break;
                case 1:
                    bpmQualifier = "<BPM1-16>"
                    break;
                case 2:
                    bpmQualifier = "<BPM1-8>"
                    break;
                case 3:
                    bpmQualifier = "<BPM1-4>"
                    break;
                case 4:
                    bpmQualifier = "<BPM1-2>"
                    break;
                case 5:
                    bpmQualifier = "<BPM1>"
                    break;
                case 6:
                    bpmQualifier = "<BPM2>"
                    break;
                case 7:
                    bpmQualifier = "<BPM4>"
                    break;
                case 8:
                    bpmQualifier = "<BPM8>"
                    break;
                case 9:
                    bpmQualifier = "<BPM16>"
                    break;
                case 10:
                    bpmQualifier = "<BPM32>"
                    break;
                default:
                    break;
                }

                return "/eos/user/0/newcmd/Effect/" + effect + "/BPM/" + bpmQualifier + "/#"
            }
        }
    }



    GreyText {
        // Console Label in the top right corner
        id: eosLabel
        text: "Eos"
        height: 30
        anchors.right: parent.right
        font.pointSize: 12
        font.bold: true
    }

    CenterLabel {
        id: noEffectsText

        y: eosLabel.height
        width: parent.width
        height: parent.height - eosLabel.height

        text: "Press \"Add\" to add a message that will send the BPM"
    }

    ListView {
        id: listView

        y: eosLabel.height
        width: parent.width
        height: parent.height - eosLabel.height
        clip: true

        delegate: effectLine
    }

}
