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

#ifndef FFTREALWRAPPER_H
#define FFTREALWRAPPER_H

#include "BasicFFTInterface.h"
#include "ffft/FFTRealFixLen.h"

// An Implementation of the BasicFFTInterface with FFTReal
// see BasicFFTInterface.h for overridden functions
// LENGTH_EXPONENT is the number of samples used expressed as an exponent of two

template<int LENGTH_EXPONENT>
class FFTRealWrapper : public BasicFFTInterface
{

public:
	explicit FFTRealWrapper() { }

	void doFft(float *output, const float *input) override { m_fftreal.do_fft(output, input); }

protected:
	ffft::FFTRealFixLen<LENGTH_EXPONENT> m_fftreal;
};

#endif // FFTREALWRAPPER_H
