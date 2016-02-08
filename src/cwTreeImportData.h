/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWTREEIMPORTDATA_H
#define CWTREEIMPORTDATA_H

//Our includes
#include "cwTreeImportDataNode.h"
class cwCavingRegion;
class cwCave;
class cwTrip;

//Qt includes
#include <QObject>
#include <QList>

class cwTreeImportData : public QObject
{
public:
    cwTreeImportData(QObject* object);

    QList<cwTreeImportDataNode*> nodes() const;
    void setNodes(QList<cwTreeImportDataNode*> nodes);

    virtual void importInto(cwCavingRegion* region);
    virtual bool canImport();

    virtual cwCave* toCave(cwTreeImportDataNode* node);
    virtual cwTrip* toTrip(cwTreeImportDataNode* node);

private:
    QList<cwTreeImportDataNode*> RootNodes;

    virtual void toTripHelper(cwTrip* trip, cwTreeImportDataNode* node);
    bool canImportHelper(cwTreeImportDataNode* node);

    virtual void importIntoHelper(cwCavingRegion* region, cwTreeImportDataNode* node);
};

inline QList<cwTreeImportDataNode*> cwTreeImportData::nodes() const {
    return RootNodes;
}


#endif // CWTREEIMPORTDATA_H
