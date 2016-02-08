/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#include "cwTreeImportData.h"
#include "cwCavingRegion.h"
#include "cwSurveyChunk.h"
#include "cwCave.h"
#include "cwTripCalibration.h"
#include "cwTrip.h"
#include "cwTeam.h"
#include <QDebug>

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

cwCave* cwTreeImportData::toCave(cwTreeImportDataNode *node) {
    cwCave* cave = new cwCave();

    cave->setName(node->name());
    foreach (cwTreeImportDataNode* child, node->childNodes()) {
        cave->addTrip(toTrip(child));
    }

    return cave;
}

cwTrip* cwTreeImportData::toTrip(cwTreeImportDataNode *node) {
    cwTrip* result = new cwTrip();

    result->setName(node->name());
    result->setDate(node->date());
    *result->calibrations() = *node->calibration();
    toTripHelper(result, node);

    return result;
}

void cwTreeImportData::toTripHelper(cwTrip *trip, cwTreeImportDataNode *node) {
//    if (node->team() != nullptr) {
//        trip->team()->merge(*node->team());
//    }

    foreach (cwSurveyChunk* srcChunk, node->chunks()) {
        cwSurveyChunk* chunk = new cwSurveyChunk(trip);
        for (int i = 0; i < srcChunk->shotCount(); i++) {
            chunk->appendShot(srcChunk->station(i), srcChunk->station(i + 1), srcChunk->shot(i));
        }
        trip->addChunk(chunk);
    }

    foreach (cwTreeImportDataNode* child, node->childNodes()) {
        toTripHelper(trip, child);
    }
}

bool cwTreeImportData::canImport() {
    foreach (cwTreeImportDataNode* node, RootNodes) {
        if (canImportHelper(node)) {
            return true;
        }
    }
    return false;
}

bool cwTreeImportData::canImportHelper(cwTreeImportDataNode *node) {
    switch (node->importType()) {
    case cwTreeImportDataNode::NewCave:
        return true;
    case cwTreeImportDataNode::AddToCave:
        if (node->targetCave()) {
            return true;
        }
        break;
    case cwTreeImportDataNode::ReplaceTrip:
        if (node->targetCave() && node->targetTrip()) {
            return true;
        }
        break;
    default:
        break;
    }

    foreach (cwTreeImportDataNode* child, node->childNodes()) {
        if (canImportHelper(child)) {
            return true;
        }
    }
    return false;
}

void cwTreeImportData::importInto(cwCavingRegion *region) {
    foreach (cwTreeImportDataNode* node, RootNodes) {
        importIntoHelper(region, node);
    }
}

void cwTreeImportData::importIntoHelper(cwCavingRegion *region, cwTreeImportDataNode *node) {
    switch (node->importType()) {
    case cwTreeImportDataNode::NewCave: {
        cwCave* cave = toCave(node);
        cave->moveToThread(region->thread());
        region->addCave(cave);
        break;
    }
    case cwTreeImportDataNode::AddToCave: {
        if (node->targetCave() != nullptr) {
            cwTrip* trip = toTrip(node);
            trip->moveToThread(node->targetCave()->thread());
            node->targetCave()->addTrip(trip);
        }
        break;
    }
    case cwTreeImportDataNode::ReplaceTrip: {
        if (node->targetTrip() != nullptr) {
            cwTrip* trip = toTrip(node);
            trip->moveToThread(node->targetTrip()->thread());
            *node->targetTrip() = *trip;
        }
        break;
    }
    case cwTreeImportDataNode::ExistingTrip: {
        break;
    }
    default: {
        foreach (cwTreeImportDataNode* child, node->childNodes()) {
            importIntoHelper(region, child);
        }
        break;
    }
    }
}
