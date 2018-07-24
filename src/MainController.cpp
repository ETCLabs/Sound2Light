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

#include "MainController.h"

#include "QAudioInputWrapper.h"
#include "TriggerGenerator.h"
#include "TriggerGuiController.h"
#include "OSCNetworkManager.h"

#include <QtMath>
#include <QQmlContext>
#include <QSettings>
#include <QQuickItem>
#include <QQmlComponent>
#include <QStandardPaths>
#include <QDateTime>
#include <QScreen>
#include <QFileDialog>


MainController::MainController(QQmlApplicationEngine* qmlEngine, QObject *parent)
	: QObject(parent)
	, m_qmlEngine(qmlEngine)
    , m_buffer(NUM_SAMPLES*4) // more buffer so the bpm detector can get overlaping data
    , m_audioInput(nullptr)
	, m_fft(m_buffer, m_triggerContainer)
	, m_osc()
	, m_consoleType("Eos")
	, m_oscMapping(this)
    , m_lowSoloMode(false)
    , m_bpmOSC(m_osc)
    , m_bpm(m_buffer, &m_bpmOSC)
    , m_bpmTap(&m_bpmOSC)
    , m_bpmActive(false)
    , m_waveformVisible(true)
    , m_autoBpm(false)
{
	m_audioInput = new QAudioInputWrapper(&m_buffer);

	initializeGenerators();
	connectGeneratorsWithGui();
}

MainController::~MainController()
{
	// delete all objects created on Heap:
    delete m_audioInput; m_audioInput = nullptr;

    delete m_bass; m_bass = nullptr;
    delete m_loMid; m_loMid = nullptr;
    delete m_hiMid; m_hiMid = nullptr;
    delete m_high; m_high = nullptr;
    delete m_envelope; m_envelope = nullptr;
    delete m_silence; m_silence = nullptr;

    delete m_bassController; m_bassController = nullptr;
    delete m_loMidController; m_loMidController = nullptr;
    delete m_hiMidController; m_hiMidController = nullptr;
    delete m_highController; m_highController = nullptr;
    delete m_envelopeController; m_envelopeController = nullptr;
    delete m_silenceController; m_silenceController = nullptr;
}

void MainController::initBeforeQmlIsLoaded()
{
	// init everything that can or must be done before QML file is loaded:
	connect(&m_osc, SIGNAL(messageReceived(OSCMessage)), this, SIGNAL(messageReceived(OSCMessage)));
	connect(&m_osc, SIGNAL(packetSent()), this, SIGNAL(packetSent()));
	connect(&m_osc, SIGNAL(useTcpChanged()), this, SIGNAL(useTcpChanged()));
	connect(&m_osc, SIGNAL(isConnectedChanged()), this, SIGNAL(isConnectedChanged()));
	connect(&m_osc, SIGNAL(isConnectedChanged()), this, SLOT(onConnectedChanged()));
	connect(&m_osc, SIGNAL(addressChanged()), this, SIGNAL(addressChanged()));
	connect(&m_osc, SIGNAL(logChanged()), this, SIGNAL(oscLogChanged()));
	connect(&m_osc, SIGNAL(messageReceived(OSCMessage)), &m_oscMapping, SLOT(handleMessage(OSCMessage)));
	connect(&m_oscUpdateTimer, SIGNAL(timeout()), &m_oscMapping, SLOT(sendLevelFeedback()));

	// load settings that may be used while QML file is loaded:
	loadPresetIndependentSettings();
	restorePreset();
}

void MainController::initAfterQmlIsLoaded()
{
	// init things that depend on the loaded QML file:
	initAudioInput();
	restoreWindowGeometry();
	autosave();

	// create global QML property that points to the main Window:
	m_qmlEngine->rootContext()->setContextProperty("mainWindow", QVariant::fromValue<QQuickWindow*>(getMainWindow()));

	connect(this, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(getMainWindow(), SIGNAL(visibilityChanged(QWindow::Visibility)), this, SLOT(onVisibilityChanged()));
}

void MainController::initializeGenerators()
{
	// create TriggerGenerator objects:
	m_bass = new TriggerGenerator("bass", &m_osc, true, false, 80);
	m_loMid = new TriggerGenerator("loMid", &m_osc, true, false, 400);
	m_hiMid = new TriggerGenerator("hiMid", &m_osc, true, false, 1000);
	m_high = new TriggerGenerator("high", &m_osc, true, false, 5000);
	m_envelope = new TriggerGenerator("envelope", &m_osc, false);
	m_silence = new TriggerGenerator("silence", &m_osc, false, true);

	// append triggerGenerators to triggerContainer:
	// (container is used to access all generators at once)
    // order is important because of lowSoloMode
	m_triggerContainer.append(m_bass);
	m_triggerContainer.append(m_loMid);
	m_triggerContainer.append(m_hiMid);
	m_triggerContainer.append(m_high);
	m_triggerContainer.append(m_envelope);
	m_triggerContainer.append(m_silence);
}

void MainController::connectGeneratorsWithGui()
{
	// create TriggerGuiController objects
	// and initialize them with the correct triggerGenerators:
	m_bassController = new TriggerGuiController(m_bass);
	m_loMidController = new TriggerGuiController(m_loMid);
	m_hiMidController = new TriggerGuiController(m_hiMid);
	m_highController = new TriggerGuiController(m_high);
	m_envelopeController = new TriggerGuiController(m_envelope);
	m_silenceController = new TriggerGuiController(m_silence);

	// set a QML context property for each
	// so that for example the m_bassController is accessible as "bassController" in QML:
	m_qmlEngine->rootContext()->setContextProperty("bassController", m_bassController);
	m_qmlEngine->rootContext()->setContextProperty("loMidController", m_loMidController);
	m_qmlEngine->rootContext()->setContextProperty("hiMidController", m_hiMidController);
	m_qmlEngine->rootContext()->setContextProperty("highController", m_highController);
	m_qmlEngine->rootContext()->setContextProperty("envelopeController", m_envelopeController);
	m_qmlEngine->rootContext()->setContextProperty("silenceController", m_silenceController);

	// connect the presetChanged signal to the onPresetChanged slot of this controller:
	connect(m_bassController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(m_loMidController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(m_hiMidController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(m_highController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(m_envelopeController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));
	connect(m_silenceController, SIGNAL(presetChanged()), this, SLOT(onPresetChanged()));

}

QQuickWindow *MainController::getMainWindow() const
{
	QQuickWindow* window = qobject_cast<QQuickWindow*>(m_qmlEngine->rootObjects()[0]);
	return window;
}

bool MainController::settingsFormatIsValid(QSettings &settings) const
{
	// check format version:
	int formatVersion = settings.value("formatVersion").toInt();
	if (formatVersion == 0) {
		// this is the first start of the software, nothing to restore
		qDebug() << "this is the first start of the software, nothing to restore";
		return false;
	} else if (formatVersion < SETTINGS_FORMAT_VERSION) {
		// the format of the settings is too old, cannot restore
		qDebug() << "the format of the settings is too old, cannot restore";
		return false;
	}
	return true;
}

void MainController::onConnectedChanged()
{
	// send current state via OSC if a new connection was made:
	if (m_osc.isConnected()) {
		m_oscMapping.sendCurrentState();
	}
}

void MainController::initAudioInput()
{
	QString inputDeviceName;
	// get input device name from settings:
	QSettings settings;
	if (settingsFormatIsValid(settings)) {
		inputDeviceName = settings.value("inputDeviceName").toString();
	}
	// if it is empty, set it to default input:
	if (inputDeviceName.isEmpty()) {
		inputDeviceName = m_audioInput->getDefaultInputName();
	}
	if (inputDeviceName.isEmpty()) {
		// if there is no default input, open NoInputDeviceDialog:
		openDialog("qrc:/qml/NoInputDeviceDialog.qml");
	} else {
		// otherwise set the new input by its name:
		m_audioInput->setInputByName(inputDeviceName);
		emit inputChanged();
	}

	// start FFT update timer:
	connect(&m_fftUpdateTimer, SIGNAL(timeout()), this, SLOT(updateFFT()));
	m_fftUpdateTimer.start(1000.0 / FFT_UPDATE_RATE);

    // set up the BPM timer and start it
    connect(&m_bpmUpdatetimer, SIGNAL(timeout()), this, SLOT(updateBPM()));
    setBPMActive(m_bpmActive);
}

void MainController::triggerBeat()
{
    setAutoBpm(false);
    m_bpmTap.triggerBeat();
}

void MainController::setBPM(float value)
{
    setAutoBpm(false);
    m_bpmTap.setBpm(value);
}

void MainController::activateBPM()
{
    m_bpm.resetCache();
    m_bpmTap.reset();
    m_bpmUpdatetimer.start(1000.0 / BPM_UPDATE_RATE);
}

void MainController::deactivateBPM()
{
    m_bpmUpdatetimer.stop();
}

void MainController::setConsoleType(QString value)
{
	if (value.isEmpty()) return;
	if (value == "EOS") value = "Eos";
	if (value == "Cobalt") value = "Cobalt 7.2";
	m_consoleType = value;
	emit settingsChanged();
	emit presetChanged();
}

QList<qreal> MainController::getSpectrumPoints()
{
	// convert const QVector<float>& to QList<qreal> to be used in GUI:
	QList<qreal> points;
	const QVector<float>& spectrum = m_fft.getNormalizedSpectrum();
	for (int i=0; i<spectrum.size(); ++i) {
		points.append(spectrum[i]);
	}
	return points;
}

QList<qreal> MainController::getWavePoints()
{
    // convert const QVector<float>& to QList<qreal> to be used in GUI:
    QList<qreal> points;
    const Qt3DCore::QCircularBuffer<float>& wave = m_bpm.getWaveDisplay();
    for (int i = 0; i < wave.size(); ++i) {
        points.append(wave.at(i) / 350 * m_fft.getScaledSpectrum().getGain());
    }
    return points;
}

QList<bool> MainController::getWaveOnsets()
{
    // conert const QVector<bool>& to QList<bool> to be used in GUI:
    QList<bool> points;
    const QVector<bool>& peaks = m_bpm.getOnsets();
    for (int i = 0; i < peaks.size(); ++i) {
        points.append(peaks[i]);
    }
    return points;
}

QList<QString> MainController::getWaveColors()
{
    // convert const QVector<QColoer>& to QList<QString> to be used in GUI:
    QList<QString> points;
    const Qt3DCore::QCircularBuffer<QColor>& colors = m_bpm.getWaveColors();
    for (int i = 0; i < colors.size(); ++i) {
        points.append(colors.at(i).name());
    }
    return points;
}

void MainController::setOscEnabled(bool value) {
	m_osc.setEnabled(value); emit settingsChanged();
	m_osc.sendMessage(QString("/s2l/out/enabled=").append(value ? "1" : "0"), true);
}

// enable or disables bpm detection
void MainController::setBPMActive(bool value) {
    if (value == m_bpmActive) return;
    m_bpmActive = value;
    if (m_bpmActive) {
        activateBPM();
    } else {
        deactivateBPM();
    }
    emit bpmActiveChanged();
    emit waveformVisibleChanged(); // because wavformvisible is only true if bpm is active
    m_osc.sendMessage("/s2l/out/bpm/enabled", (value ? "1" : "0"), true);
}

void MainController::setAutoBpm(bool value) {
    if (value == m_autoBpm) return;
    m_autoBpm = value;
    qDebug() << m_autoBpm;
    m_bpm.setTransmitBpm(m_autoBpm);
    if (m_autoBpm && !m_bpmActive) {
        setBPMActive(true);
    } else if (!m_autoBpm && !m_waveformVisible && m_bpmActive) {
        setBPMActive(false);
    }
    emit autoBpmChanged();
}

// sets the minium bpm of the range
void MainController::setMinBPM(int value) {
    m_bpm.setMinBPM(value);
    m_bpmTap.setMinBPM(value);
    emit bpmRangeChanged();
    m_osc.sendMessage("/s2l/out/bpm/range", QString::number(value), true);
}

void MainController::setWaveformVisible(bool value) {
    m_waveformVisible = value;
    if (m_waveformVisible && !m_bpmActive) {
        setBPMActive(true);
    } else if (!m_autoBpm && !m_waveformVisible && m_bpmActive) {
        setBPMActive(false);
    }
    emit waveformVisibleChanged();
}

void MainController::onExit()
{
    savePresetIndependentSettings();
    autosave();
}

void MainController::onVisibilityChanged()
{
	// if main Window is minimized:
	if (getMainWindow()->visibility() == QWindow::Minimized) {
		// iterate over all open dialogs:
		QMap<QString, QObject*>::iterator i = m_dialogs.begin();
		for (; i != m_dialogs.end(); ++i) {
			// check if dialog exists:
			if (!i.value()) continue;
			// check if it should be minimized or closed:
			if (i.value()->property("modality").toInt() == Qt::NonModal) {
				// a non modal dialog will be minimized together with the main window:
				// FIXME: is a QML Dialog minimizable?
				// it seems it is not, it will be closed instead:
				QMetaObject::invokeMethod(i.value(), "close");
			} else {
				// a modal dialog will be closed when the main window is minimized:
				QMetaObject::invokeMethod(i.value(), "close");
			}
		}
	}
}

void MainController::savePresetIndependentSettings() const
{
	QSettings independentSettings;
	// save preset independent settings:
	independentSettings.setValue("version", VERSION_STRING);
	independentSettings.setValue("formatVersion", SETTINGS_FORMAT_VERSION);
	independentSettings.setValue("changedAt", QDateTime::currentDateTime().toString());
	independentSettings.setValue("oscIpAddress", getOscIpAddress());
	independentSettings.setValue("oscTxPort", getOscUdpTxPort());
	independentSettings.setValue("oscRxPort", getOscUdpRxPort());
	independentSettings.setValue("oscTcpPort", getOscTcpPort());
	independentSettings.setValue("oscIsEnabled", getOscEnabled());
	independentSettings.setValue("oscUseTcp", getUseTcp());
	independentSettings.setValue("oscUse_1_1", getUseOsc_1_1());
    QRect windowGeometry = getMainWindow()->geometry();
    if (windowGeometry.width() < 300) {
        // -> minimal mode, save default geometry
        windowGeometry.setWidth(1200);
        windowGeometry.setHeight(800);
    }
    independentSettings.setValue("windowGeometry", windowGeometry);
	bool maximized = (getMainWindow()->width() == QGuiApplication::primaryScreen()->availableSize().width());
	independentSettings.setValue("maximized", maximized);
	independentSettings.setValue("inputDeviceName", getActiveInputName());
	independentSettings.setValue("presetFileName", m_currentPresetFilename);
	independentSettings.setValue("presetChangedButNotSaved", m_presetChangedButNotSaved);
	independentSettings.setValue("oscLogSettingsValid", true);
	independentSettings.setValue("oscLogIncomingIsEnabled", getOscLogIncomingIsEnabled());
	independentSettings.setValue("oscLogOutgoingIsEnabled", getOscLogOutgoingIsEnabled());
	independentSettings.setValue("oscInputEnabledValid", true);
	independentSettings.setValue("oscInputEnabled", getOscInputEnabled());
}

void MainController::loadPresetIndependentSettings()
{
	QSettings independentSettings;
	// do not restore anything if the format is not valid (or the file is new):
	if (!settingsFormatIsValid(independentSettings)) return;

	// restore preset independent settings:
	setOscIpAddress(independentSettings.value("oscIpAddress").toString());
	setOscUdpTxPort(independentSettings.value("oscTxPort").toInt());
	setOscUdpRxPort(independentSettings.value("oscRxPort", 8000).toInt());
	setOscTcpPort(independentSettings.value("oscTcpPort", 3032).toInt());
	setOscEnabled(independentSettings.value("oscIsEnabled").toBool());
	setUseTcp(independentSettings.value("oscUseTcp").toBool());
	setUseOsc_1_1(independentSettings.value("oscUse_1_1").toBool());
	if (independentSettings.value("oscLogSettingsValid").toBool()) {
		enableOscLogging(independentSettings.value("oscLogIncomingIsEnabled").toBool(), independentSettings.value("oscLogOutgoingIsEnabled").toBool());
	} else {
		enableOscLogging(true, true);
	}
	if (independentSettings.value("oscInputEnabledValid").toBool()) {
		setOscInputEnabled(independentSettings.value("oscInputEnabled").toBool());
	} else {
		setOscInputEnabled(true);
	}
}

void MainController::restoreWindowGeometry()
{
	QSettings independentSettings;
	// do not restore anything if the format is not valid (or the file is new):
	if (!settingsFormatIsValid(independentSettings)) return;

	QRect windowGeometry = independentSettings.value("windowGeometry").toRect();
	bool maximized = independentSettings.value("maximized").toBool();
	QQuickWindow* window = getMainWindow();
	if (!window) return;
	if (!windowGeometry.isNull()) window->setGeometry(windowGeometry);
	if (maximized) window->showMaximized();
	connect(window, SIGNAL(closing(QQuickCloseEvent*)), m_qmlEngine, SIGNAL(quit()));

	// TODO: restore visibility of details in trigger settings
	// TODO: restore position of splitView handle
}

void MainController::loadPreset(const QString &constFileName, bool createIfNotExistent)
{
	// prepare the filename:
	QString fileName(constFileName);
	// fileName returned from FileDialog starts with "file:///"
	// this has to be removed:
	if (fileName.startsWith("file:///")) {
		fileName = fileName.remove(0, 8);
	}

	// check if file exists:
	if (!createIfNotExistent && !QFile(fileName).exists()) {
		m_osc.sendMessage("/s2l/out/error", QString("Preset does not exist: ").append(fileName), true);
		return;
	}

	// loads the file or creates it if not existent:
	QSettings settings(fileName, QSettings::IniFormat);
	// do not restore anything if the format is not valid (or the file is new):
	if (!settingsFormatIsValid(settings)) return;

	// restore all general, not independent settings from the preset file:
	setDecibelConversion(settings.value("dbConversion").toBool());
	setFftGain(settings.value("fftGain").toReal());
	setFftCompression(settings.value("fftCompression").toReal());
	setAgcEnabled(settings.value("agcEnabled").toBool());
	setConsoleType(settings.value("consoleType").toString());
    setLowSoloMode(settings.value("lowSoloMode").toBool());
    setBPMActive(settings.value("bpm/Active", false).toBool());
    setAutoBpm(settings.value("autoBpm", false).toBool());
    setWaveformVisible(settings.value("waveformVisible", true).toBool());

	// restore the settings in all TriggerGenerators:
	for (int i=0; i<m_triggerContainer.size(); ++i) {
		m_triggerContainer[i]->restore(settings);
	}

    // Restore the settings in the BPMDetector (from here to keep BPM Detector modular)
    setMinBPM(settings.value("bpm/Min", 75).toInt());

    // Restore the settings in the BPMOscController
    m_bpmOSC.restore(settings);

    // Restore the manual BPM
    m_bpmTap.setBpm(settings.value("bpm/tapvalue", 60).toInt());

    m_bpmOSC.setBPMMute(settings.value("bpm/mute", false).toBool());


	// this is now the loaded preset, update the preset name:
	m_currentPresetFilename = fileName; emit presetNameChanged();
	m_presetChangedButNotSaved = false; emit presetChangedButNotSavedChanged();
	QString baseName = QFileInfo(m_currentPresetFilename).baseName();
	// give feedback over OSC:
	m_osc.sendMessage("/s2l/out/active_preset", baseName, true);
	m_osc.sendMessage("/s2l/out/error", "-", true);

	// notify the GUI of the changes:
	emit decibelConversionChanged();
	emit agcEnabledChanged();
	emit gainChanged();
	emit compressionChanged();
    emit bpmActiveChanged();
    emit bpmRangeChanged();
    emit waveformVisibleChanged();
    emit bpmMuteChanged();

	emit m_bassController->parameterChanged();
	emit m_loMidController->parameterChanged();
	emit m_hiMidController->parameterChanged();
	emit m_highController->parameterChanged();
	emit m_envelopeController->parameterChanged();
	emit m_silenceController->parameterChanged();

	emit m_bassController->oscLabelTextChanged();
	emit m_loMidController->oscLabelTextChanged();
	emit m_hiMidController->oscLabelTextChanged();
	emit m_highController->oscLabelTextChanged();
	emit m_envelopeController->oscLabelTextChanged();
	emit m_silenceController->oscLabelTextChanged();

    emit m_bassController->muteChanged();
    emit m_loMidController->muteChanged();
    emit m_hiMidController->muteChanged();
    emit m_highController->muteChanged();
    emit m_envelopeController->muteChanged();
    emit m_silenceController->muteChanged();
}

void MainController::savePresetAs(const QString &constFileName, bool isAutosave)
{
	if (constFileName.isEmpty()) return;
	// prepare the filename:
	QString fileName(constFileName);
	// fileName returned from FileDialog starts with "file:///"
	// this has to be removed:
	if (fileName.startsWith("file:///")) {
		fileName = fileName.remove(0, 8);
	}
	// all preset files have to end on ".s2l":
	if (!fileName.toLower().endsWith(".s2l") && !isAutosave) {
		fileName = fileName + ".s2l";
	}

	// save all general, not independent settings to the preset file:
	QSettings settings(fileName, QSettings::IniFormat);
	settings.setValue("version", VERSION_STRING);
	settings.setValue("formatVersion", SETTINGS_FORMAT_VERSION);
	settings.setValue("changedAt", QDateTime::currentDateTime().toString());
	settings.setValue("dbConversion", getDecibelConversion());
	settings.setValue("fftGain", getFftGain());
	settings.setValue("fftCompression", getFftCompression());
	settings.setValue("agcEnabled", getAgcEnabled());
	settings.setValue("consoleType", getConsoleType());
    settings.setValue("lowSoloMode", getLowSoloMode());
    settings.setValue("bpm/Active", getBPMActive());
    settings.setValue("autoBpm", getAutoBpm());
    settings.setValue("waveformVisible", m_waveformVisible); // store property because getter is for GUI, and only returns true if bpm is active

	// save the settings in all TriggerGenerators:
	for (int i=0; i<m_triggerContainer.size(); ++i) {
		m_triggerContainer[i]->save(settings);
	}

    // save the settings in the BPMDetector (from here to keep BPM Detector modular)
    settings.setValue("bpm/Min", m_bpm.getMinBPM());

    // save the settings in the BPMOscController
    m_bpmOSC.save(settings);

    // save the manual BPM
    settings.setValue("bpm/tapvalue", m_bpmTap.getBpm());

    // save bpm mute
    settings.setValue("bpm/mute", m_bpmOSC.getBPMMute());

	if (!isAutosave) {
		// this is now the loaded preset, update the preset name:
		m_currentPresetFilename = fileName; emit presetNameChanged();
		m_presetChangedButNotSaved = false; emit presetChangedButNotSavedChanged();
	}
}

void MainController::saveCurrentPreset()
{
	if (m_currentPresetFilename.isEmpty()) {
		openSavePresetAsDialog();
	} else {
		savePresetAs(m_currentPresetFilename);
	}
}

void MainController::autosave()
{
	savePresetAs(getPresetDirectory() + "/autosave.ats", true);
}

void MainController::restorePreset()
{
	loadPreset(getPresetDirectory() + "/autosave.ats");

	QSettings independentSettings;
	QString presetFileName = independentSettings.value("presetFileName").toString();

	if (presetFileName.isEmpty()) {
		// there wasn't a preset in last session
		m_currentPresetFilename = "";
		m_presetChangedButNotSaved = independentSettings.value("presetChangedButNotSaved").toBool();
	} else if (QFile(presetFileName).exists()) {
		// there was a preset and it does exist
		// this is now the loaded preset, update the preset name:
		m_currentPresetFilename = presetFileName;
		m_presetChangedButNotSaved = independentSettings.value("presetChangedButNotSaved").toBool();
	} else {
		// there was a preset but it doesn't anymore exist
		m_currentPresetFilename = "";
		m_presetChangedButNotSaved = true;
	}

	emit presetNameChanged();
	emit presetChangedButNotSavedChanged();
}

void MainController::resetPreset()
{
	// reset all values to default:
	setFftGain(1.0);
	setFftCompression(1.0);
	setAgcEnabled(true);
	setDecibelConversion(false);
    setLowSoloMode(false);
    setBPMActive(false);
    setMinBPM(75);
    setBPMOscCommands(QStringList());
    setWaveformVisible(true);

    emit bpmActiveChanged();
    emit bpmRangeChanged();
    emit waveformVisibleChanged();

	// reset values in all TriggerGuiControllers:
	m_bassController->resetParameters();
	m_loMidController->resetParameters();
	m_hiMidController->resetParameters();
	m_highController->resetParameters();
	m_envelopeController->resetParameters();
	m_silenceController->resetParameters();

	// clear currentPresetFilename:
	m_currentPresetFilename = ""; emit presetNameChanged();
	m_presetChangedButNotSaved = false; emit presetChangedButNotSavedChanged();
}

void MainController::deletePreset(const QString &fileName)
{
	// if the preset to delete is currently loaded, reset to default settings:
	if (m_currentPresetFilename == fileName) {
		resetPreset();
	}

	QFile file(fileName);
	// delete the file:
	if (file.exists()) {
		file.remove();
	}
}

QString MainController::getPresetDirectory() const
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString MainController::getPresetName() const
{
	if (m_currentPresetFilename.isEmpty()) return "";
	return QFileInfo(m_currentPresetFilename).baseName();
}

void MainController::onPresetChanged()
{
	if (!m_presetChangedButNotSaved) {
		m_presetChangedButNotSaved = true;
		emit presetChangedButNotSavedChanged();
	}
}

void MainController::sendOscTestMessage(QString message)
{
	m_osc.sendMessage(message, true);
}

void MainController::openDialog(const QString &qmlDialogFile, QString propertyName, QVariant propertyValue)
{
	// check if dialog is already opened:
	if (m_dialogs.find(qmlDialogFile) != m_dialogs.end()) {
		// dialog is open
		QObject* dialog = m_dialogs[qmlDialogFile];
		if (dialog && dialog->property("visible").toBool()) {
			dialog->setProperty("visible", false);
			dialog->setProperty("visible", true);
			return;
		}
	}
	// create new dialog from QML file:
	QQmlComponent* component = new QQmlComponent(m_qmlEngine, QUrl(qmlDialogFile));
	QObject* dialog = component->beginCreate(m_qmlEngine->rootContext());
	if (!propertyName.isEmpty()) {
		dialog->setProperty(propertyName.toLatin1(), propertyValue);
	}
	component->completeCreate();
	QMetaObject::invokeMethod(dialog, "open");
	m_dialogs[qmlDialogFile] = dialog;
}

void MainController::dialogIsClosed(QObject *dialog)
{
	if (!dialog) return;
	// remove dialog from open dialogs map:
	QString qmlDialogFile = m_dialogs.key(dialog);
	m_dialogs.remove(qmlDialogFile);
	// delete dialog object:
	// FIXME: assertion error when deleting dialog
	dialog->deleteLater();
}

void MainController::setPropertyWithoutChangingBindings(const QVariant &item, QString name, QVariant value)
{
	QQuickItem* qitem = item.value<QQuickItem*>();
	qitem->setProperty(name.toLatin1().data(), value);
}

bool MainController::oscLevelFeedbackIsEnabled()
{
	return m_oscUpdateTimer.isActive();
}

void MainController::enableOscLevelFeedback(bool value)
{
	if (value) {
		// start OSC level feedback timer:
		m_oscUpdateTimer.start(1000.0 / OSC_LEVEL_FEEDBACK_RATE);
	} else {
		// stop OSC level feedback timer:
		m_oscUpdateTimer.stop();
	}
	// give feedback about state via OSC:
	m_osc.sendMessage(QString("/s2l/out/level_feedback=").append(value ? "1" : "0"), true);
}

void MainController::openSavePresetAsDialog()
{
	// open QWidget based dialog and wait for result (blocking):
	QString fileName = QFileDialog::getSaveFileName(0, tr("Save Preset As"),
							   getPresetDirectory(),
							   tr("Sound2Light Presets (*.s2l)"));
	savePresetAs(fileName);
}

void MainController::openLoadPresetDialog()
{
	// open QWidget based dialog and wait for result (blocking):
	QString fileName = QFileDialog::getOpenFileName(0, tr("Open Preset"),
							   getPresetDirectory(),
							   tr("Sound2Light Presets (*.s2l)"));
	loadPreset(fileName);
}
