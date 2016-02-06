#ifndef CWWALLSIMPORTDATA_H
#define CWWALLSIMPORTDATA_H

#include "cwTreeImportData.h"
class cwCave;
class cwTrip;

class cwWallsImportData : public cwTreeImportData
{
    Q_OBJECT
public:
    cwWallsImportData(QObject* parent = 0);
    void importInto(cwCavingRegion *region);
    bool canImport();

private:
    void cavesHelper(QList<cwCave*>* caves, cwTreeImportDataNode* currentBlock, cwCave* currentCave, cwTrip* trip);
};

#endif // CWWALLSIMPORTDATA_H
