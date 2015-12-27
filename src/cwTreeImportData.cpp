/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#include "cwTreeImportData.h"

cwTreeImportData::cwTreeImportData(QObject* parent) :
    QObject(parent)
{

}

void cwTreeImportData::setNodes(QList<cwTreeImportDataNode*> nodes) {
    foreach(cwTreeImportDataNode* node, nodes) {
        node->setParent(this);
        node->setParentNode(nullptr);
    }

    RootNodes = nodes;
}
