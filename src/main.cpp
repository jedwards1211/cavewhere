//Qt includes
#include <QGuiApplication>
#include <QApplication>
#include <QThread>
#include <QtQuick/QQuickView>
#include <QQmlContext>
#include <QModelIndex>
#include <QtWidgets/QWidget>
#include <QDir>
#include <QQmlEngine>
#include <QImageReader>

//Our includes
//#include "cwMainWindow.h"
#include "cwStation.h"
#include "cwSurveyChunk.h"
#include "cwCavingRegion.h"
#include "cwImage.h"
#include "cwGlobalDirectory.h"
#include "cwStation.h"
#include "cwQMLRegister.h"
#include "cwRootData.h"
#include "cwProject.h"
#include "cwProjectImageProvider.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    foreach(QByteArray imageFormats, QImageReader::supportedImageFormats()) {
//        qDebug() << "Image formats:" << imageFormats;
//    }

    qRegisterMetaType<QThread*>("QThread*");
    qRegisterMetaType<cwCavingRegion>("cwCavingRegion");
    qRegisterMetaType<QList <QString> >("QList<QString>");
    qRegisterMetaType<QList <cwImage> >("QList<cwImage>");
    qRegisterMetaType<QList <cwStation > >("QList<cwStation>");
    qRegisterMetaType<QModelIndex>("QModelIndex");
    qRegisterMetaType<cwImage>("cwImage");

    QApplication::setOrganizationName("Vadose Solutions");
    QApplication::setOrganizationDomain("cavewhere.com");
    QApplication::setApplicationName("Cavewhere");
    QApplication::setApplicationVersion("0.1");

    cwGlobalDirectory::setupBaseDirectory();

    //Register all of the cavewhere types
    cwQMLRegister::registerQML();

    QQuickView view;

    QSurfaceFormat format = view.format();
    format.setSamples(4);

    cwRootData* rootData = new cwRootData(&view);
    rootData->project()->load(QDir::homePath() + "/Dropbox/quanko.cw");
    QQmlContext* context = view.rootContext();
    context->setContextObject(rootData);
    context->setContextProperty("mainWindow", &view);

    //This allow to extra image data from the project's image database
    cwProjectImageProvider* imageProvider = new cwProjectImageProvider();
    imageProvider->setProjectPath(rootData->project()->filename());
    QObject::connect(rootData->project(), SIGNAL(filenameChanged(QString)), imageProvider, SLOT(setProjectPath(QString)));
    context->engine()->addImageProvider(cwProjectImageProvider::Name, imageProvider);

    view.setFormat(format);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl::fromLocalFile("qml/CavewhereMainWindow.qml"));
    view.setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    view.show();

    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
