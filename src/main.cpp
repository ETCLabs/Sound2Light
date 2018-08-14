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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QSettings>
#include <QScreen>
#include <QSplashScreen>


int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/icons/etcicon.ico"));

	// these parameters are used by QSettings:
	QCoreApplication::setOrganizationName("ETC");
	QCoreApplication::setApplicationName("Sound2Light");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	// ----------- Show Splash Screen --------
	QPixmap pixmap(":/images/icons/etclogo.png");
	QSplashScreen splash(pixmap);
	splash.show();

	// call processEvents() to load image in Splash Screen:
	app.processEvents();

	// ----------- Load QML ---------------

	// create QmlEngine and MainController:
	QQmlApplicationEngine engine;
    MainController* controller = new MainController(&engine);

	// set global QML variable "controller" to a pointer to the MainController:
    engine.rootContext()->setContextProperty("controller", controller);

	// quit QGuiApplication when quit signal is emitted:
	QObject::connect(&engine, SIGNAL(quit()), &app, SLOT(quit()));
	// call onExit() method of controller when app is about to quit:
    QObject::connect(&app, SIGNAL(aboutToQuit()), controller, SLOT(onExit()));

    controller->initBeforeQmlIsLoaded();
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    controller->initAfterQmlIsLoaded();

	// ---------- Hide Splash Screen ---------
	splash.hide();

	// start main application loop:
	return app.exec();
}
