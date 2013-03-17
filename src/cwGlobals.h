#ifndef CWGLOBALS_H
#define CWGLOBALS_H

//Qt includes
#include <QString>
#include <QFileDialog>

class cwGlobals
{
public:
    cwGlobals();

    static const double PI;
    static const double RadiansToDegrees;
    static const double DegreesToRadians;

    static QString addExtension(QString filename, QString extensionHint);
    static QString openFile(QString caption,  QString filter, QString qSettingsKey = "", QFileDialog::Options = 0);
    static QStringList openFiles(QString caption, QString filter, QString qSettingsKey = "", QFileDialog::Options = 0);
};

#endif // CWGLOBALS_H
