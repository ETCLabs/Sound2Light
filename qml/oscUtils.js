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


.pragma library


function argumentAsFloat(msg, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("=") === -1) return defaultValue
	var argument = msg.split("=").pop()
	argument = parseFloat(argument)
	return (argument === NaN) ? defaultValue : argument
}

function argumentAsString(msg, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("=") === -1) return defaultValue
	var argument = msg.split("=").pop()
	return argument
}

function argumentAsInt(msg, index, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("=") === -1) return defaultValue
	var arguments = msg.split("=").pop().split(",")
	var argument = parseInt(arguments[index])
	return (argument === NaN) ? defaultValue : argument
}

function lastPartAsInt(msg, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var lastPart = msg.split("/").pop().split("=")[0]
	lastPart = parseInt(lastPart)
	return (isNaN(lastPart)) ? defaultValue : lastPart
}

function thirdPartAsInt(msg, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var thirdPart = msg.split("/")[3]
	thirdPart = parseInt(thirdPart)
	return (isNaN(thirdPart)) ? defaultValue : thirdPart
}

function fourthPartAsString(msg, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var fourthPart = msg.split("/")[4]
	return fourthPart
}

function partAsInt(msg, part, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var number = msg.split("/")[part]
	number = parseInt(number)
	return (isNaN(number)) ? defaultValue : number
}

function partAsFloat(msg, part, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var number = msg.split("/")[part]
	number = parseFloat(number)
	return (isNaN(number)) ? defaultValue : number
}

function partAsString(msg, part, defaultValue) {
	if (!msg) return defaultValue
	if (msg.search("/") === -1) return defaultValue
	var text = msg.split("/")[part]
	return text
}
