/**************************************************************************
**
**    Copyright (C) 2016 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/


#ifndef STREAMOPERATOR
#define STREAMOPERATOR

//Catch includes
#include "catch.hpp"

//Qt includes
#include <QVector3D>
#include <QString>
#include <QVariant>
#include <QObject>
#include <QMetaProperty>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>

//Our includes
#include "cwStationPositionLookup.h"
#include "cwProject.h"

inline std::ostream& operator << ( std::ostream& os, QVector3D const& value ) {
    os << "(" << value.x() << ", " << value.y() << ", " << value.z() << ")";
    return os;
}

inline std::ostream& operator << ( std::ostream& os, QString const& value ) {
    os << value.toStdString();
    return os;
}

inline std::ostream& operator << ( std::ostream& os, QStringList const& value ) {
    if(!value.isEmpty()) {
        os << "[";
        for(auto iter = value.begin(); iter != value.end() - 1; iter++) {
            os << "\"" + iter->toStdString() + "\",";
        }
        os << "\"" + value.last() + "\"]";
    } else {
        os << "[]";
    }
    return os;
}

inline std::ostream& operator << ( std::ostream& os, QSet<QString> const& value ) {
    return operator <<(os, value.toList());
}

inline std::ostream& operator << ( std::ostream& os, QVariant const& value ) {
    os << value.toString().toStdString();
    return os;
}

inline std::ostream& operator << ( std::ostream& os, QMetaProperty const& value ) {
    os << value.name() << " type:" << value.typeName();
    return os;
}

inline void propertyCompare(QObject* tc1, QObject* tc2) {

    for(int i = 0; i < tc1->metaObject()->propertyCount(); i++) {
        QMetaProperty property = tc1->metaObject()->property(i);
        INFO("Testing property " << property);
        CHECK(property.read(tc1) == property.read(tc2));
    }
}

inline double roundToDecimal(double value, int decimals) {
    double decimalPlaces = 10.0 * decimals;
    return qRound(value * decimalPlaces) / decimalPlaces;
}

inline QVector3D roundToDecimal(QVector3D v, int decimals) {
    return QVector3D(roundToDecimal(v.x(), decimals),
                     roundToDecimal(v.y(), decimals),
                     roundToDecimal(v.z(), decimals));
}


inline void checkQVector3D(QVector3D v1, QVector3D v2, int decimals = 2) {
    if(v1 != v2) {
        v1 = roundToDecimal(v1, decimals);
        v2 = roundToDecimal(v2, decimals);
        CHECK(v1 == v2);
    }
}

/**
 * @brief checkStationLookup
 * @param lookup1
 * @param lookup2
 *
 * This isn't an exact eqauls comparison. This just makes sure all the stations in lookup1 are
 * equal to the stations in lookup2. Lookup2 could have extra stations that aren't checked. We
 * need this exact functionality for compass import export test. The compass exporter creates
 * extra stations at the end of the survey that has lrud data.
 */
inline void checkStationLookup(cwStationPositionLookup lookup1, cwStationPositionLookup lookup2) {
    foreach(QString stationName, lookup1.positions().keys()) {
        INFO("Checking position for " << stationName);
        CHECK(lookup2.hasPosition(stationName) == true);
        checkQVector3D(lookup1.position(stationName), lookup2.position(stationName));
    }
}

/**
 * Copyies filename to the temp folder
 */
inline QString copyToTempFolder(QString filename) {

    QFileInfo info(filename);
    QString newFileLocation = QDir::tempPath() + "/" + info.fileName();

    if(QFileInfo::exists(newFileLocation)) {
        bool couldRemove = QFile::remove(newFileLocation);
        INFO("Trying to remove " << newFileLocation);
        REQUIRE(couldRemove == true);
    }

    bool couldCopy = QFile::copy(filename, newFileLocation);
    INFO("Trying to copy " << filename << " to " << newFileLocation);
    REQUIRE(couldCopy == true);

    bool couldPermissions = QFile::setPermissions(newFileLocation, QFile::WriteOwner | QFile::ReadOwner);
    INFO("Trying to set permissions for " << filename);
    REQUIRE(couldPermissions);

    return newFileLocation;
}

/**
 * @brief fileToProject
 * @param filename
 * @return A new project generate from filename
 */
inline cwProject* fileToProject(QString filename) {
    QString datasetFile = copyToTempFolder(filename);

    cwProject* project = new cwProject();
    project->loadFile(datasetFile);
    project->waitLoadToFinish();

    return project;
}

#endif // STREAMOPERATOR

