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



// ----------- Content of the BPM OSC dialog for Cobalt (7.3+) console type -----------
Item {
    id: cobaltArea
    anchors.fill: parent

    property var messages

    Component.onCompleted: {
        restoreFromMessages(controller.getBPMOscCommands())
    }

    function firstFreeEffectMessage() {
        for (var freeEffect = 1; freeEffect < 101; freeEffect++) {
            var occupied = false
            for (var i = 0; i < messages.length; i++) {
                var message = messages[i]
                if (message.indexOf("/effects") === -1) {
                    continue
                }

                var effectNo = Utils.partAsInt(message, 3, 0)
                if (effectNo === freeEffect) {
                    occupied = true
                    break
                }
            }
            if (!occupied) {
                break
            }
        }
        if (freeEffect < 101) {
            return "/effects/bpm/" + freeEffect + "=<BPM1>"
        } else {
            return ""
        }
    }

    function firstFreeBeatBossMessage() {
        for (var freeBeatBoss = 1; freeBeatBoss < 101; freeBeatBoss++) {
            var occupied = false
            for (var i = 0; i < messages.length; i++) {
                var message = messages[i]
                if (message.indexOf("/cobalt/beatboss") === -1) {
                    continue
                }

                var beatBossNo = Utils.partAsInt(message, 3, 0)
                if (beatBossNo === freeBeatBoss) {
                    occupied = true
                    break
                }
            }
            if (!occupied) {
                break
            }
        }
        if (freeBeatBoss < 10) {
            return "/cobalt/beatboss/" + freeBeatBoss + "/bpm/<BPM1>"
        } else {
            return ""
        }
    }

    function addMessage() {
        var message = firstFreeEffectMessage()

        // If no message could be created, don't add anything
        if (message.indexOf("/effect") === -1) {
            return
        }

        messages.push(message)
        restoreFromMessages(messages)
        listView.positionViewAtEnd()
    }

    function setMessageAtIndex(index, message) {
        if (typeof index == 'undefined') {
            console.log("index undefined. aborting")
            return
        }

        if (message.indexOf("/effects") !== -1) {
            var effectNo = Utils.partAsFloat(message, 3, 0)
            for (var i = 0; i < messages.length; i++) {
                if (i === index) {
                    continue
                }

                var otherMessage = messages[i]
                if (otherMessage.indexOf("/effects/bpm") === -1) {
                    continue
                }

                var otherEffectNo = Utils.partAsInt(otherMessage, 3, 0)
                if (effectNo === otherEffectNo) {

                    invalidInputDialog.text = "Effect " + effectNo + " is already in use"
                    invalidInputDialog.open()
                    return
                }
            }
        } else if (message.indexOf("/cobalt/beatboss") !== -1) {
            var beatbossNo = Utils.partAsFloat(message, 3, 0)
            if (beatbossNo < 1 || beatbossNo > 9) {
                invalidInputDialog.text = "Beatboss must be between 1 and 9!"
                invalidInputDialog.open();

                return
            }

            for (i = 0; i < messages.length; i++) {
                if (i === index) {
                    continue
                }

                otherMessage = messages[i]
                if (otherMessage.indexOf("/cobalt/beatboss") === -1) {
                    continue
                }

                var otherBeatbossNo = Utils.partAsInt(otherMessage, 3, 1)
                if (beatbossNo === otherBeatbossNo) {

                    invalidInputDialog.text = "Beatboss " + otherBeatbossNo + " is already in use"
                    invalidInputDialog.open()
                    return
                }
            }
        }

        messages[index] = message
    }

    MessageDialog {
        id: invalidInputDialog
        title: "Invalid Input"
        modality: Qt.ApplicationModal
        onAccepted: {
            invalidInputDialog.close()
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
        id: targetLine
        Item {
            height: 150
            width: 340


            // ------------ ComboBox and Loader for line components -------
            Column {
                anchors.fill: parent
                spacing: 5
                Item {
                    width: parent.width
                    height: 30
                    DarkComboBox {
                        width: 130
                        height: 30
                        model: ["Effect", "BeatBoss"]
                        onCurrentIndexChanged: {
                            if (currentText !== "") {
                                updateSettingsArea()
                            }
                        }
                        Component.onCompleted: {
                            // This method tries to choose the appropriate form
                            // for the OSC message.

                            // try to find a not empty OSC message:
                            var currentMessage = messages[index]

                            // check which category fits the message:
                            if (currentMessage.indexOf("/effects") !== -1) {
                                currentIndex = model.indexOf("Effect")
                            } else if (currentMessage.indexOf("/cobalt/beatboss") !== -1) {
                                currentIndex = model.indexOf("BeatBoss")
                            } else {
                                setMessageAtIndex(index, firstFreeEffectMessage())
                                currentIndex = model.indexOf("Effect")
                            }

                            // load correct Settings Component:
                            updateSettingsArea()
                        }
                        function updateSettingsArea() {
                            // This method loads the correct Settings Component
                            // for the chosen message type category in this ComboBox.
                            if (currentText === "Effect") {
                                settingsArea.sourceComponent = effectLineItem
                                settingsArea.item.index = index
                            }  else if (currentText === "BeatBoss") {
                                settingsArea.sourceComponent = beatBossLineItem
                                settingsArea.item.index = index
                            }

                            // restore information in Component:
                            settingsArea.item.restoreFromMessage(messages[index])
                            setMessageAtIndex(index, settingsArea.item.getMessage())
                        }
                    }  // ComboBox end
                }  // Row end
                Loader {
                    // This loader holds the Settings Component for the chosen message type category.
                    id: settingsArea
                    width: parent.width
                    height: parent.height - 30
                }
            }
        }
    }

    Component {
        id: effectLineItem

        Column {
            id: effectLineItemColumn
            property var index

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
                    minimumValue: 1
                    maximumValue: 9999
                    onValueChanged: {
                        if (typeof index !== 'undefined') {
                            setMessageAtIndex(index, effectLineItemColumn.getMessage())
                            restoreFromMessage(messages[index])
                        }
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
                        if (typeof index !== 'undefined') {
                            setMessageAtIndex(index, effectLineItemColumn.getMessage())
                        }
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

            function restoreFromMessage(message) {
                if (message.indexOf("/effects/bpm") === -1) {
                    message = firstFreeEffectMessage()
                }
                effectNumber.value = Utils.partAsFloat(message,3,1)
                var bpmQualifier = Utils.argumentAsString(message, "<BPM>")
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

                return "/effects/bpm/" + effect + "=" + bpmQualifier
            }
        }
    }

    Component {
        id: beatBossLineItem

        Column {
            id: beatBossLineItemColumn
            property var index

            anchors.fill: parent
            spacing: 3

            Row {
                height: 30
                width: parent.width
                CenterLabel {
                    height: parent.height
                    width: parent.width * 0.5
                    text: "BeatBoss Number:"
                }
                NumericInput {
                    id: beatBossNumber
                    height: parent.height
                    width: parent.width * 0.5
                    minimumValue: 1
                    maximumValue: 9999
                    onValueChanged: {
                        if (typeof index !== 'undefined') {
                            setMessageAtIndex(index, beatBossLineItemColumn.getMessage())
                            restoreFromMessage(messages[index])
                        }
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
                        if (typeof index !== 'undefined') {
                            setMessageAtIndex(index, beatBossLineItemColumn.getMessage())
                        }
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


            function restoreFromMessage(message) {
                if (message.indexOf("/cobalt/beatboss") === -1) {
                    message = firstFreeBeatBossMessage()
                }
                beatBossNumber.value = Utils.partAsInt(message,3,1)
                var bpmQualifier = Utils.partAsString(message,5,"<BPM>")
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
                var beatboss = beatBossNumber.value.toString()
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

                return "/cobalt/beatboss/" + beatboss + "/bpm/" + bpmQualifier
            }
        }
    }



    GreyText {
        // Console Label in the top right corner
        id: cobaltLabel
        text: "Cobalt 7.3+"
        height: 30
        anchors.right: parent.right
        font.pointSize: 12
        font.bold: true
    }

    CenterLabel {
        id: noEffectsText

        y: cobaltLabel.height
        width: parent.width
        height: parent.height - cobaltLabel.height

        text: "Press \"Add\" to add a message that will send the BPM"
    }

    ListView {
        id: listView

        y: cobaltLabel.height
        width: parent.width
        height: parent.height - cobaltLabel.height
        clip: true

        delegate: targetLine
    }

}
