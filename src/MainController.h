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

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "FFTAnalyzer.h"
#include "BPMDetector.h"
#include "BPMTapDetector.h"
#include "MonoAudioBuffer.h"
#include "AudioInputInterface.h"
#include "OSCNetworkManager.h"
#include "OSCMapping.h"
#include "versionInfo.h"

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QFileInfo>
#include <QQmlComponent>
#include <QUrl>
#include <QGuiApplication>
#include <QQuickItem>

#include <iostream>

// Rate to calculate the FFT (and Trigger signals) in Hz / FPS
static const int FFT_UPDATE_RATE = 44; // Hz

// Rate to send OSC Level Feedback (if activated) in Hz / FPS
static const int OSC_LEVEL_FEEDBACK_RATE = 15; // Hz


// Forward declarations:
class TriggerGenerator;
class EnvelopeTriggerGenerator;
class TriggerGuiController;
class EnvelopeTriggerGuiController;


// Processing chains in this software:
// 1. Chain:  AudioInput (async) -> MonoAudioBuffer
// 2. Chain:  QTimer(44Hz) -> FFTAnalyzer -> TriggerGenerator -> TriggerFilter -> OSCNetworkManager


// This class coordinates the communication of Model and GUI,
// initializes the objects and manages presets and persistence.
class MainController : public QObject
{
	Q_OBJECT

	// this property is used to show and hide the dB label and to activate the dB checkbox:
	Q_PROPERTY(bool decibelConversion READ getDecibelConversion NOTIFY decibelConversionChanged)
	// this property is used by the AGC checkbox:
	Q_PROPERTY(bool agcEnabled READ getAgcEnabled NOTIFY agcEnabledChanged)
	// the base name of the preset file to be displayed in GUI:
	Q_PROPERTY(QString presetName READ getPresetName NOTIFY presetNameChanged)
	// this property indicates if the current preset has been changed but not stored yet:
	Q_PROPERTY(bool presetChangedButNotSaved READ getPresetChangedButNotSaved NOTIFY presetChangedButNotSavedChanged)
    // this property is used for the lowSoloMode checkbox:
    Q_PROPERTY(bool lowSoloMode READ getLowSoloMode WRITE setLowSoloMode NOTIFY lowSoloModeChanged)

    Q_PROPERTY(bool autoBpm READ getAutoBpm WRITE setAutoBpm NOTIFY autoBpmChanged)
    Q_PROPERTY(bool waveformVisible READ getWaveformVisible WRITE setWaveformVisible NOTIFY waveformVisibleChanged)

public:
    explicit MainController(QQmlApplicationEngine* m_qmlEngine, QObject* parent = nullptr);
	~MainController();

	// initializes everything that has to be done before QML is loaded
	void initBeforeQmlIsLoaded();

	// initializes everything that has to be done after QML is loaded
	void initAfterQmlIsLoaded();

signals:
	// emitted when the input device was changed
	void inputChanged();

	// emitted when settings were changed
	void settingsChanged();

	// emitted when the setting to convert to dB values is changed
	void decibelConversionChanged();

	// emitted when the AGC state is changed
	void agcEnabledChanged();

	// emitted when presetChangedButNotSaved state changed
	void presetChangedButNotSavedChanged();

	// emitted when the preset name changed
	void presetNameChanged();

	// emitted when the gain has been changed by loading a preset file
	// (won't be emitted atm when gain is changed by agc)
	void gainChanged();

	// emitted when the compression has been changed by loading a preset file
	void compressionChanged();

	// emitted when a value of the preset changed
    void presetChanged();

    // emitted when lowSoloMode changed
    void lowSoloModeChanged();

    // emitted if the bpm activation changed
    void bpmActiveChanged();

    void autoBpmChanged();

    // emitted if the bpm range changed
    void bpmRangeChanged();

    // emitted if the waveform visiblity changed
    void waveformVisibleChanged();

    // emitted if the bpm mute changed
    void bpmMuteChanged();

	// forwarded from OSCNetworkManager:
	void messageReceived(OSCMessage msg);
	void packetSent();
	void useTcpChanged();
	void isConnectedChanged();
	void addressChanged();
	void oscLogChanged();


public slots:

	// initializes the audio input
	void initAudioInput();

    // activates or deactivates the bpm detectin
    void activateBPM();
    void deactivateBPM();

	// saves settings before application exits
	void onExit();

	// minimizes or restores the dialogs depending on visibility
	void onVisibilityChanged();

	// save all settings to QSettings
	void savePresetIndependentSettings() const;
	// load all settings from QSettings
	void loadPresetIndependentSettings();
	// restores the window size and position
	void restoreWindowGeometry();

    // update function passed to the FFTAnalyzer
    void updateFFT() { m_fft.calculateFFT(m_lowSoloMode); }

    // update function passed to the BPMDetector
    void updateBPM() { m_bpm.detectBPM(); }

	// ------------------- Presets --------------------------------

	// load a preset file, creates a new file if it does not exist
	void loadPreset(const QString& fileName, bool createIfNotExistent = false);
	// saves state to a preset file
	void savePresetAs(const QString& fileName, bool isAutosave = false);
	// saves state to currently loaded preset file
	void saveCurrentPreset();
	// save unsaved changes in an autosave file
	void autosave();
	// restores the preset saved in independent settings
	void restorePreset();

	// resets all values to their defaults
	void resetPreset();

	// deletes the file of a preset
	void deletePreset(const QString& fileName);

	// returns the directory where the preset files are stored
	QString getPresetDirectory() const;

	// returns the base name of the preset file to be displayed in GUI
	QString getPresetName() const;

	// returns if the current preset has been changed but not stored yet
	bool getPresetChangedButNotSaved() const { return m_presetChangedButNotSaved; }

	// updates the presetChangedButNotSaved status
	void onPresetChanged();

	// opens a QFileDialog to choose the fileName for a preset and calls savePresetAs()
	void openSavePresetAsDialog();

	// opens a QFileDialog to choose the fileName of a preset and calls loadPreset()
	void openLoadPresetDialog();


	// ------------------- Functions used by GUI -----------------------------

	// returns the chosen consoleType as a string
	QString getConsoleType() const { return m_consoleType; }
	// sets the used console type (either "EOS" or "Cobalt")
	void setConsoleType(QString value);

    // returns if low solo mode is active
    bool getLowSoloMode() const { return m_lowSoloMode; }
    // enables or disables low solo mode
    void setLowSoloMode(bool value) { m_lowSoloMode = value; emit lowSoloModeChanged(); }

	// returns the current spectrum outline as a list of qreal values in the range 0...1
	// used in GUI to display SpectrumPlot
	QList<qreal> getSpectrumPoints();

	// returns the current version string
	QString getVersionString() { return VERSION_STRING; }

    // Triggers a BPM Tap
    void triggerBeat();

    // Sets a manual BPM
    void setBPM(float value);

    // returns if the bpm detection is currently active
    bool getBPMActive() { return m_bpmActive; }
    bool getBPMManual() { return m_bpmTap.hasBpm() && !m_autoBpm; }
    // enable or disables bpm detection
    void setBPMActive(bool value);

    bool getAutoBpm() const { return m_autoBpm; }
    void setAutoBpm(bool value);

    // forward calls to BPMDetector
    // returns the current bpm
    float getBPM() { return getBPMManual() || m_bpm.getBPM() == 0.0f ? m_bpmTap.getBpm() : m_bpm.getBPM(); }
    // returns if the detected bpm is old and should be marked as such in the gui
    bool bpmIsOld() { return m_bpm.bpmIsOld(); }
    // sets the minium bpm of the range
    void setMinBPM(int value);
    // gets the minium bpm of the range
    int getMinBPM() { return m_bpm.getMinBPM(); }

    // set/get bpm mute
    bool getBPMMute() { return m_bpmOSC.getBPMMute(); }
    void toggleBPMMute() { m_bpmOSC.toggleBPMMute(); emit bpmMuteChanged(); }

    // set/get the waveform visibility
    bool getWaveformVisible() { return m_waveformVisible; }
    void setWaveformVisible(bool value);

    // returns the last seconds of waveform as a list of qreal values in the range 0...1
    // as well as the marks where a peak was detected as a boolean value and colors that represent the spectrum
    // used in GUI to display the BeatPlot
    QList<qreal> getWavePoints();
    QList<bool> getWaveOnsets();
    QList<QString> getWaveColors();

    // Gets or sets the osc commands sent by the bpm detector
    QStringList getBPMOscCommands() { return m_bpmOSC.getCommands(); }
    void setBPMOscCommands(const QStringList commands) { m_bpmOSC.setCommands(commands); }

	// forward calls to AudioInputInterface
	// see AudioInputInterface.h for documentation
	QStringList getAvailableInputs() const { return m_audioInput->getAvailableInputs(); }
	QString getActiveInputName() const { return m_audioInput->getActiveInputName(); }
	void setInputByName(const QString& name) { m_audioInput->setInputByName(name); emit inputChanged(); emit presetChanged(); }
	qreal getVolume() const { return m_audioInput->getVolume(); }
	void setVolume(const qreal& value) { m_audioInput->setVolume(value); emit presetChanged(); }

	// forward calls to ScaledSpectrum of FFTAnalyzer
	// see ScaledSpectrum.h for documentation
	qreal getFftGain() const { return m_fft.getScaledSpectrum().getGain(); }
	void setFftGain(const qreal& value) { m_fft.getScaledSpectrum().setGain(value); emit gainChanged(); emit presetChanged(); }
	qreal getFftCompression() const { return m_fft.getScaledSpectrum().getCompression(); }
	void setFftCompression(const qreal& value) { m_fft.getScaledSpectrum().setCompression(value); emit compressionChanged(); emit presetChanged(); }
	bool getDecibelConversion() const { return m_fft.getScaledSpectrum().getDecibelConversion(); }
	void setDecibelConversion(bool value) { m_fft.getScaledSpectrum().setDecibelConversion(value); emit decibelConversionChanged(); emit presetChanged(); }
	bool getAgcEnabled() const { return m_fft.getScaledSpectrum().getAgcEnabled(); }
	void setAgcEnabled(bool value) { m_fft.getScaledSpectrum().setAgcEnabled(value); emit agcEnabledChanged(); emit presetChanged(); }

	// forward calls to OSCNetworkManager
	// see OSCNetworkManager.h for documentation
	QString getOscIpAddress() const { return m_osc.getIpAddress().toString(); }
	void setOscIpAddress(const QString& value) { m_osc.setIpAddress(QHostAddress(value)); emit settingsChanged(); }
	quint16 getOscUdpTxPort() const { return m_osc.getUdpTxPort(); }
	void setOscUdpTxPort(const quint16& value) { m_osc.setUdpTxPort(value); emit settingsChanged(); }
	quint16 getOscUdpRxPort() const { return m_osc.getUdpRxPort(); }
	void setOscUdpRxPort(const quint16& value) { m_osc.setUdpRxPort(value); emit settingsChanged(); }
	quint16 getOscTcpPort() const { return m_osc.getTcpPort(); }
	void setOscTcpPort(const quint16& value) { m_osc.setTcpPort(value); emit settingsChanged(); }
	bool getOscEnabled() const { return m_osc.getEnabled(); }
	void setOscEnabled(bool value);
	bool getUseTcp() const { return m_osc.getUseTcp(); }
	void setUseTcp(bool value) { m_osc.setUseTcp(value); }
	bool getUseOsc_1_1() const { return m_osc.getUseOsc_1_1(); }
	void setUseOsc_1_1(bool value) { m_osc.setUseOsc_1_1(value); }
	bool isConnected() const { return m_osc.isConnected(); }
	QStringList getOscLog() const { return m_osc.getLog(); }
	bool getOscLogIncomingIsEnabled() const { return m_osc.getLogIncomingIsEnabled(); }
	bool getOscLogOutgoingIsEnabled() const { return m_osc.getLogOutgoingIsEnabled(); }
	void enableOscLogging(bool incoming, bool outgoing) { m_osc.enableLogging(incoming, outgoing); }
	void sendOscMessage(QString message, bool forced) { m_osc.sendMessage(message, forced); }
	void sendOscMessage(QString path, QString argument, bool forced) { m_osc.sendMessage(path, argument, forced); }
	void clearOscLog() const { m_osc.clearLog(); }

	// forward calls to OSCMapping
	// see OSCMapping.h for documentation
	bool getOscInputEnabled() const { return m_oscMapping.getInputEnabled(); }
	void setOscInputEnabled(bool value) { m_oscMapping.setInputEnabled(value); }

	// sends an OSC test message
	void sendOscTestMessage(QString message);

	// opens a dialog from a QML file
	void openDialog(const QString& qmlDialogFile, QString propertyName = "", QVariant propertyValue = QVariant());

	// removes dialog from openend dialogs list
	void dialogIsClosed(QObject* object);

	// returns if the CTRL key is pressed
	bool controlIsPressed() const { return QGuiApplication::keyboardModifiers() & Qt::ControlModifier; }

	// returns the geometry of the main window
	QRect getWindowGeometry() const { return getMainWindow()->geometry(); }

	// returns the geometry of the window that contains the item
	QRect getWindowGeometryOfItem(const QVariant& item) const { return item.value<QQuickItem*>()->window()->geometry(); }

	// sets a property of a QQuickItem to a value without changing its bindings (in opposite to setting it from QML directly)
	void setPropertyWithoutChangingBindings(const QVariant& item, QString name, QVariant value);

	// returns if level feedback via OSC is enabled
	bool oscLevelFeedbackIsEnabled();
	// enables or disables the transmission of level feedback via OSC
	void enableOscLevelFeedback(bool value);

private:
	// instantiates the trigger generators
	void initializeGenerators();

	// connects the trigger generators with the GUI by creating GuiControllers and setting context properties
	void connectGeneratorsWithGui();

	// returns the main window
	QQuickWindow* getMainWindow() const;

	// checks if settings format is valid
	bool settingsFormatIsValid(QSettings& settings) const;

private slots:
	// sends current state via OSC if connection changed
	void onConnectedChanged();

public:  // to allow access from OSCMapping class without getters
	TriggerGuiController* m_bassController;  // GUI Controller for Bass TriggerGenerator
	TriggerGuiController* m_loMidController;  // GUI Controller for LoMid TriggerGenerator
	TriggerGuiController* m_hiMidController;  // GUI Controller for HiMid TriggerGenerator
	TriggerGuiController* m_highController;  // GUI Controller for High TriggerGenerator
    TriggerGuiController* m_envelopeController;  // GUI Controller for Level TriggerGenerator
	TriggerGuiController* m_silenceController;  // GUI Controller for Silence TriggerGenerator

protected:
	QQmlApplicationEngine*		m_qmlEngine;  // pointer to QmlEngine (created in main.cpp)
	QVector<TriggerGeneratorInterface*> m_triggerContainer;  // list of all TriggerGenerators
	MonoAudioBuffer				m_buffer;  // MonoAudioBuffer instance
	AudioInputInterface*		m_audioInput;  // pointer to AudioInputInterface implementation
	FFTAnalyzer					m_fft;  // FFTAnalyzer instance
	OSCNetworkManager			m_osc;  // OSCNetworkManager instance
	QString						m_consoleType;  // console type as string
	QTimer						m_fftUpdateTimer;  // Timer used to trigger FFT update
	QString						m_currentPresetFilename;  // file path and name of active preset
	bool						m_presetChangedButNotSaved;  // true, if the preset has been changed but not saved yet
	QMap<QString, QObject*>		m_dialogs;  // list of all open dialogs (QML-filename -> GUI element instance)
	OSCMapping					m_oscMapping;  // OSCMapping instance
	QTimer						m_oscUpdateTimer;  // Timer used to trigger OSC level feedback
    bool                        m_lowSoloMode;  // true if low solo mode is active
    BPMOscControler             m_bpmOSC; // Manages transmiting the bpm via osc
    BPMDetector                 m_bpm; // BPMDetector instance
    BPMTapDetector              m_bpmTap; // BPMTapDetector instance
    bool                        m_bpmActive; // true if the bpm detection is active
    QTimer                      m_bpmUpdatetimer; // Timer to trigger bpm update
    bool                        m_waveformVisible; // true if the waveform is visible
    bool                        m_autoBpm; // true if BPM should be set automatically

	TriggerGenerator* m_bass;  // pointer to Bass TriggerGenerator instance
	TriggerGenerator* m_loMid;  // pointer to LoMid TriggerGenerator instance
	TriggerGenerator* m_hiMid;  // pointer to HiMid TriggerGenerator instance
	TriggerGenerator* m_high;  // pointer to High TriggerGenerator instance
	TriggerGenerator* m_envelope;  // pointer to Level TriggerGenerator instance
	TriggerGenerator* m_silence;  // pointer to Silence TriggerGenerator instance

};

#endif // MAINCONTROLLER_H
