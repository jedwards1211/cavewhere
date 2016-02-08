/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Our includes
#include "cwTreeImportDataNode.h"
#include "cwCave.h"
#include "cwTrip.h"
#include "cwSurveyChunk.h"
#include "cwStation.h"
#include "cwTeam.h"
#include "cwTripCalibration.h"

cwTreeImportDataNode::cwTreeImportDataNode(QObject* parent) :
    QObject(parent),
    ParentNode(nullptr),
    Type(NoImport),
    TargetCave(nullptr),
    TargetTrip(nullptr),
    Team(new cwTeam(this)),
    Calibration(new cwTripCalibration(this)),
    IncludeDistance(true)
{
}

/**
  \brief Gets the survex block at index
  */
 cwTreeImportDataNode* cwTreeImportDataNode::childNode(int index) {
     return ChildNodes[index];
 }

/**
  \brief Get's the chunk at index
  */
 cwSurveyChunk* cwTreeImportDataNode::chunk(int index) {
     return Chunks[index];
 }

 /**
   \brief Sets the name of the block data
   */
 void cwTreeImportDataNode::setName(QString name) {
     if(Name != name) {
         Name = name;
         emit nameChanged();
     }
 }

 cwCave* cwTreeImportDataNode::targetCave() const {
     return TargetCave;
 }

 void cwTreeImportDataNode::setTargetCave(cwCave *targetCave) {
     if (TargetCave != targetCave) {
         TargetTrip = nullptr;
         TargetCave = targetCave;
         emit importTypeChanged();
     }
 }

 cwTrip* cwTreeImportDataNode::targetTrip() const {
     return TargetTrip;
 }

 void cwTreeImportDataNode::setTargetTrip(cwTrip *targetTrip) {
     if (TargetTrip != targetTrip) {
         TargetTrip = targetTrip;
         emit importTypeChanged();
     }
 }

 /**
   \brief Sets how this block will be exported
   */
 void cwTreeImportDataNode::setImportType(ImportType type) {
     if(Type != type) {
         //Go through children and update thier type
         Type = type;
         emit importTypeChanged();

         foreach(cwTreeImportDataNode* child, ChildNodes) {
             if(Type != NoImport) {
                 if(child->isTrip()) {
                     child->setImportType(AddToCave);
                 } else {
                     child->setImportType(Structure);
                 }
             } else {
                 child->setImportType(NoImport);
             }
         }
     }
 }

 /**
   \brief Checks to see if this is a trip
   */
 bool cwTreeImportDataNode::isTrip() const {
     cwTreeImportDataNode* parent = parentNode();
     while(parent != nullptr) {
         if(parent->importType() == AddToCave) {
             return false;
         }
         parent = parent->parentNode();
     }

     return !Chunks.isEmpty();
 }

 /**
   \brief Converts the export type to a string
   */
 QString cwTreeImportDataNode::importTypeToString(ImportType type) {
     switch(type) {
     case NoImport:
         return QString("Don't Import");
     case ExistingTrip:
         return QString("Trip already exists");
     case NewCave:
         return QString("New Cave");
     case AddToCave:
         return QString("New Trip");
     case ReplaceTrip:
         return QString("Replace Existing Trip");
     default:
         break;
     }
     return QString();
 }

 /**
   \brief Adds a child block to the data
   */
 void cwTreeImportDataNode::addChildNode(cwTreeImportDataNode* blockData) {
     blockData->ParentNode = this;
     ChildNodes.append(blockData);
 }

 /**
   \brief Adds a chunk to the data
   */
 void cwTreeImportDataNode::addChunk(cwSurveyChunk* chunk) {
     chunk->setParent(this);
     Chunks.append(chunk);
 }

 /**
   \brief Clears the data from this object

   This will delete the block, if they're still owned by this object
   */
 void cwTreeImportDataNode::clear() {
     Chunks.clear();
     ChildNodes.clear();
 }

 /**
   \brief Get's all the station for this survex block
   */
 int cwTreeImportDataNode::stationCount() const {
     int numStations = 0;
     foreach(cwSurveyChunk* chunk, Chunks) {
         numStations += chunk->stationCount();
     }
     return numStations;
 }

 /**
   \brief Gets the station at index

   Returns null if the index is out of bounds. The index is out of bound when index >=
   stationCount()
   */
 cwStation cwTreeImportDataNode::station(int index) const {
     foreach(cwSurveyChunk* chunk, Chunks) {
         if(index < chunk->stationCount()) {
             return chunk->station(index);
         }
         index -= chunk->stationCount();
     }
     return cwStation();
 }

/**
  \brief Gets the number of shots in the block
  */
 int cwTreeImportDataNode::shotCount() const {
     int numShots = 0;
     foreach(cwSurveyChunk* chunk, Chunks) {
         numShots += chunk->shotCount();
     }
     return numShots;
 }

 /**
   Get's the parent cwSurveyChunk at shot index
   */
 cwSurveyChunk *cwTreeImportDataNode::parentChunkOfShot(int shotIndex) const {
     foreach(cwSurveyChunk* chunk, Chunks) {
         if(shotIndex < chunk->shotCount()) {
             return chunk;
         }
         shotIndex -= chunk->shotCount();
     }
     return nullptr;
 }

 int cwTreeImportDataNode::chunkShotIndex(int shotIndex) const
 {
     foreach(cwSurveyChunk* chunk, Chunks) {
         if(shotIndex < chunk->shotCount()) {
             return shotIndex;
         }
         shotIndex -= chunk->shotCount();
     }
     return -1;
 }
