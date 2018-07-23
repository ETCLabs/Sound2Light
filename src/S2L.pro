TARGET = s2l
LANGUAGE = C++

TEMPLATE = app

QT += qml quick multimedia quickwidgets network widgets

CONFIG += c++11 thread

QMAKE_CXXFLAGS += -Wall

SOURCES += main.cpp \
    FFTAnalyzer.cpp \
    MainController.cpp \
    MonoAudioBuffer.cpp \
    QAudioInputWrapper.cpp \
    ScaledSpectrum.cpp \
    TriggerFilter.cpp \
    OSCParser.cpp \
    TriggerGenerator.cpp \
    TriggerGuiController.cpp \
    TriggerOscParameters.cpp \
    OSCMessage.cpp \
    OSCMapping.cpp \
    OSCNetworkManager.cpp \
    BPMDetector.cpp \
    BPMOscControler.cpp \
    BPMTapDetector.cpp

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
# include(deployment.pri)

HEADERS += \
    ffft/Array.h \
    ffft/Array.hpp \
    ffft/def.h \
    ffft/DynArray.h \
    ffft/DynArray.hpp \
    ffft/FFTReal.h \
    ffft/FFTReal.hpp \
    ffft/FFTRealFixLen.h \
    ffft/FFTRealFixLen.hpp \
    ffft/FFTRealFixLenParam.h \
    ffft/FFTRealPassDirect.h \
    ffft/FFTRealPassDirect.hpp \
    ffft/FFTRealPassInverse.h \
    ffft/FFTRealPassInverse.hpp \
    ffft/FFTRealSelect.h \
    ffft/FFTRealSelect.hpp \
    ffft/FFTRealUseTrigo.h \
    ffft/FFTRealUseTrigo.hpp \
    ffft/OscSinCos.h \
    ffft/OscSinCos.hpp \
    AudioInputInterface.h \
    BasicFFTInterface.h \
    FFTAnalyzer.h \
    FFTRealWrapper.h \
    MainController.h \
    MonoAudioBuffer.h \
    QAudioInputWrapper.h \
    ScaledSpectrum.h \
    TriggerGeneratorInterface.h \
    TriggerFilter.h \
    OSCParser.h \
    utils.h \
    TriggerGenerator.h \
    TriggerGuiController.h \
    TriggerOscParameters.h \
    versionInfo.h \
    OSCMessage.h \
    OSCMapping.h \
    OSCNetworkManager.h \
    QCircularBuffer.h \
    BPMDetector.h \
    BPMOscControler.h \
    BPMTapDetector.h

DISTFILES += \
    LICENSE.txt \
    README.md
