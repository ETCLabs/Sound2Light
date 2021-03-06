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



// ----------- Content of the BPM OSC dialog for an unsupported console type -----------
Item {
    id: otherArea
    anchors.fill: parent

    property var messages

    Component.onCompleted: {
        messages = controller.getBPMOscCommands()
    }


    function getMessages() {
        return messages
    }

    GreyText {
        // Console Label in the top right corner
        id: consoleLabel
        text: "Unsupported Console"
        height: 30
        anchors.right: parent.right
        font.pointSize: 12
        font.bold: true
    }

    CenterLabel {
        id: noEffectsText

        y: consoleLabel.height
        anchors.fill: parent

        text: "Transmitting BPM Information to the selected Console is not \n available in this Version of Sound2Light"
    }
}
