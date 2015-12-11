/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWTREEIMPORTDATANODE_H
#define CWTREEIMPORTDATANODE_H

//Our includes
class cwSurveyChunk;
class cwShot;
class cwTeam;
class cwTripCalibration;
#include "cwStation.h"
#include "cwSurvexLRUDChunk.h"

//Qt includes
#include <QList>
#include <QStringList>
#include <QString>
#include <QObject>
#include <QDate>


class cwTreeImportDataNode : public QObject
{
    Q_OBJECT

    friend class cwSurvexImporter;
    friend class cwSurvexGlobalData;
    friend class cwWallsImporter;
    friend class cwTreeImportData;

public:
    enum ImportType {
        NoImport, //!< Don't import this block
        Cave, //!< This block is a cave
        Trip, //!< This block is a trip
        Structure //!< This is neither a Cave nor a Trip, but a imported survex block
    };

    cwTreeImportDataNode(QObject* parent = 0);

    void clear();

    int childBlockCount();
    cwTreeImportDataNode* childBlock(int index);

    int chunkCount();
    cwSurveyChunk* chunk(int index);

    cwTreeImportDataNode* parentBlock() const;

    void setName(QString name);
    QString name() const;

    void addToEquated(QStringList stationNames);
    QStringList equatedStations(QString fullStationName) const;

    void addExportStations(QStringList exportStations);
    QStringList exportStations() const;

    void setDate(QDate date);
    QDate date() const;

    cwTeam* team() const;
    cwTripCalibration* calibration() const;

    void setImportType(ImportType type);
    ImportType importType() const;
    static QString importTypeToString(ImportType type);

    QList<cwSurveyChunk*> chunks();
    QList<cwTreeImportDataNode*> childBlocks();

    int stationCount() const;
    cwStation station(int index) const;

    int shotCount() const;
//    cwShot shot(int index) const;
    cwSurveyChunk* parentChunkOfShot(int shotIndex) const;
    int chunkShotIndex(int shotIndex) const;

    void setIncludeDistance(bool includeLength);
    bool isDistanceInclude() const;

signals:
    void nameChanged();
    void importTypeChanged();

private:
    QList<cwSurveyChunk*> Chunks;
    QList<cwSurvexLRUDChunk> LRUDChunks;
    QList<cwTreeImportDataNode*> ChildBlocks;
    QList<QStringList> EqualStations;  //Each entry hold a list of station names's that are the same.
    QSet<QString> ExportStations; //Holds a station name that is exported for equates
    cwTreeImportDataNode* ParentBlock;

    //Mutible elements
    QString Name;
    ImportType Type;

    QDate Date;
    cwTeam* Team;
    cwTripCalibration* Calibration;

    bool IncludeDistance;

    //For caves, used station names, and equating stations
    QMap<QString, QString> EquateMap;  //All stations get added to the map

    void addChildBlock(cwTreeImportDataNode* blockData);
    void addChunk(cwSurveyChunk* chunk);
    void addLRUDChunk();

    void setParentBlock(cwTreeImportDataNode* parentBlock);

    bool isTrip() const;
};

/**
  \brief Gets the number of child blocks
  */
inline int cwTreeImportDataNode::childBlockCount() {
    return ChildBlocks.size();
}

/**
  \brief Get's the number of chunks
  */
inline int cwTreeImportDataNode::chunkCount() {
    return Chunks.size();
}

/**
  \brief Get's the name of the block
  */
inline QString cwTreeImportDataNode::name() const {
    return Name;
}

/**
 * @brief cwSurvexBlockData::addToEquated
 * @param adds a list of stationNames that are equal to each other.
 */
inline void cwTreeImportDataNode::addToEquated(QStringList stationNames) {
    EqualStations.append(stationNames);
}


/**
  \brief Get's all the chunks held by the block
  */
inline QList<cwSurveyChunk*> cwTreeImportDataNode::chunks() {
    return Chunks;
}

/**
  \brief Get's all the child blocks held by the this block
  */
inline QList<cwTreeImportDataNode*> cwTreeImportDataNode::childBlocks() {
    return ChildBlocks;
}

/**
  \brief Get's the parent block
  */
inline cwTreeImportDataNode* cwTreeImportDataNode::parentBlock() const {
    return ParentBlock;
}

/**
  \brief Set's the parent block
  */
inline void cwTreeImportDataNode::setParentBlock(cwTreeImportDataNode* parentBlock) {
    ParentBlock = parentBlock;
}

inline cwTreeImportDataNode::ImportType cwTreeImportDataNode::importType() const {
    return Type;
}

/**
  Sets the date for the block
  */
inline void cwTreeImportDataNode::setDate(QDate date) {
    Date = date;
}

/**
  Gets the date for the block
  */
inline QDate cwTreeImportDataNode::date() const {
    return Date;
}

/**
  Gets the survey team for the block
  */
inline cwTeam* cwTreeImportDataNode::team() const {
    return Team;
}

/**
  Gets the survey's calibration
  */
inline cwTripCalibration* cwTreeImportDataNode::calibration() const {
    return Calibration;
}

/**
 * @brief cwSurvexBlockData::exportStations
 * @return All the export stations in this block
 */
inline QStringList cwTreeImportDataNode::exportStations() const
{
    return QStringList(ExportStations.toList());
}

/**
 * @brief cwSurvexBlockData::setIncludeDistance
 * @param includeLength
 *
 * If false the distance shot length will be excluded
 */
inline void cwTreeImportDataNode::setIncludeDistance(bool includeLength)
{
    IncludeDistance = includeLength;
}

/**
 * @brief cwSurvexBlockData::isDistanceInclude
 * @return True if the distance is include, and false, if it's not
 */
inline bool cwTreeImportDataNode::isDistanceInclude() const
{
    return IncludeDistance;
}


#endif // CWTREEIMPORTDATANODE_H