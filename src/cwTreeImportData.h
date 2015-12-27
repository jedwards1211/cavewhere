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

//Qt includes
#include <QObject>
#include <QList>

class cwTreeImportData : public QObject
{
public:
    cwTreeImportData(QObject* object);

    QList<cwTreeImportDataNode*> nodes() const;
    void setNodes(QList<cwTreeImportDataNode*> nodes);

    virtual void importInto(cwCavingRegion* region) = 0;

private:
    QList<cwTreeImportDataNode*> RootNodes;
};

inline QList<cwTreeImportDataNode*> cwTreeImportData::nodes() const {
    return RootNodes;
}


#endif // CWTREEIMPORTDATA_H
