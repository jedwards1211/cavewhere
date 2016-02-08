/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Our includes
#include <QStandardItemModel>
#include "cwImportTreeDataDialog.h"
#include "cwTreeDataImporterModel.h"
#include "cwTreeDataImporter.h"
#include "cwGlobalIcons.h"
#include "cwTreeImportDataNode.h"
#include "cwTreeImportData.h"
#include "cwCavingRegion.h"
#include "cwTaskProgressDialog.h"
#include "cwStringListErrorModel.h"
#include "cwGlobalUndoStack.h"
#include "cwCave.h"
#include "cwTrip.h"

//Qt includes
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QPixmapCache>
#include <QMessageBox>
#include <QDebug>
#include <QThread>


cwImportTreeDataDialog::cwImportTreeDataDialog(Names names, cwTreeDataImporter* importer, cwCavingRegion* region, QWidget *parent) :
    QDialog(parent),
    Region(region),
    Model(new cwTreeDataImporterModel(this)),
    Importer(importer),
    SurvexSelectionModel(new QItemSelectionModel(Model, this)),
    ImportThread(new QThread(this)),
    Updating(false)
{
    setupUi(this);
    tabWidget->setTabText(tabWidget->indexOf(SurvexErrorsWidget), QApplication::translate("cwImportTreeDataDialog",
                                                                                          names.errorsLabel.toLocal8Bit().constData(), 0));

    setupTypeComboBox();

    //Move the importer to another thread
    Importer->setThread(ImportThread);

    //Connect the importer up
    connect(Importer, SIGNAL(finished()), SLOT(importerFinishedRunning()));
    connect(Importer, SIGNAL(stopped()), SLOT(importerCanceled()));

    connect(SurvexSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateCurrentItem(QItemSelection,QItemSelection)));
    connect(ImportButton, SIGNAL(clicked()), SLOT(import()));

    connect(Model, &cwTreeDataImporterModel::dataChanged, this, &cwImportTreeDataDialog::updateImportButton);

    connect(TargetCaveComboBox, SIGNAL(currentIndexChanged(int)), SLOT(targetCaveChanged(int)));
    connect(TargetTripComboBox, SIGNAL(currentIndexChanged(int)), SLOT(targetTripChanged(int)));

    SurvexTreeView->setModel(Model);
    SurvexTreeView->setSelectionModel(SurvexSelectionModel);
    SurvexTreeView->setHeaderHidden(true);
    SurvexTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    TypeComboBox->setEnabled(false);

    if (Region != nullptr) {
        TargetCaveComboBox->setModel(Region);
        TargetCaveComboBox->setRootModelIndex(QModelIndex());
    }

    splitter->setStretchFactor(1, 4);

    SurvexErrorListView->setWordWrap(true);
    //SurvexErrorListView->setUniformItemSizes(true);

    setWindowTitle(names.windowTitle);

    updateImportButton(Model->index(0, 0), Model->index(Model->rowCount() - 1, 0));
    updateCurrentItem(QItemSelection(), QItemSelection());
    updateTargets();
}

cwImportTreeDataDialog::~cwImportTreeDataDialog() {
    Importer->stop();

    ImportThread->quit();
    ImportThread->wait();

    delete Importer;
}

/**
  \brief Start to the dialog to import
  */
void cwImportTreeDataDialog::open() {
    if(Region == nullptr) {
        QMessageBox box(QMessageBox::Critical, "Broke Sauce!", "Oops, the programer has made a mistake and Cavewhere can't open Survex Import Dialog.");
        box.exec();
        return;
    }
}

/**
  \brief The survex file that'll be opened
  */
void cwImportTreeDataDialog::setInputFiles(QStringList filenames) {
    //The root filename
    FullFilename = filenames.isEmpty() ? "" : filenames[0];

    //Show a progress dialog
    cwTaskProgressDialog* progressDialog = new cwTaskProgressDialog(this);
    progressDialog->setTask(Importer);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    progressDialog->show();

    //Run the importer on another thread
    QMetaObject::invokeMethod(Importer, "setInputFiles", Q_ARG(QStringList, filenames));
    Importer->start();
}


void cwImportTreeDataDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
  \brief When the dialog is resized
  */
void cwImportTreeDataDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);

    QString cuttOffText =  FileLabel->fontMetrics().elidedText(FullFilename, Qt::ElideMiddle, FileLabel->width());
    FileLabel->setText(cuttOffText);
}



/**
  \brief Sets up the combo box for this dialog
  */
void cwImportTreeDataDialog::setupTypeComboBox() {
    QPixmap dontImportIcon;
    if(!QPixmapCache::find(cwGlobalIcons::NoImport, &dontImportIcon)) {
        dontImportIcon = QPixmap(cwGlobalIcons::NoImportFilename);
        cwGlobalIcons::NoImport = QPixmapCache::insert(dontImportIcon);
    }

    QPixmap caveIcon;
    if(!QPixmapCache::find(cwGlobalIcons::Cave, &caveIcon)) {
        caveIcon = QPixmap(cwGlobalIcons::CaveFilename);
        cwGlobalIcons::Cave = QPixmapCache::insert(caveIcon);
    }

    QPixmap tripIcon;
    if(!QPixmapCache::find(cwGlobalIcons::Trip, &tripIcon)) {
        tripIcon = QPixmap(cwGlobalIcons::TripFilename);
        cwGlobalIcons::Trip = QPixmapCache::insert(tripIcon);
    }

    for(int i = 0; i < NumberOfItems; i++) {
        TypeComboBox->addItem(QString());
    }

    TypeComboBox->setItemIcon((int)NoImportItem, QIcon(dontImportIcon));
    TypeComboBox->setItemText((int)NoImportItem, "Don't Import");
    TypeComboBox->setItemIcon((int)ExistingTripItem, QIcon(dontImportIcon));
    TypeComboBox->setItemText((int)ExistingTripItem, "Data already exists");
    TypeComboBox->setItemIcon((int)NewCaveItem, QIcon(caveIcon));
    TypeComboBox->setItemText((int)NewCaveItem, "Import as New Cave");
    TypeComboBox->setItemIcon((int)NewTripItem, QIcon(tripIcon));
    TypeComboBox->setItemText((int)NewTripItem, "Import as New Trip in Existing Cave");
    TypeComboBox->setItemIcon((int)ReplaceTripItem, QIcon(tripIcon));
    TypeComboBox->setItemText((int)ReplaceTripItem, "Replace Existing Trip Data");
    TypeComboBox->setCurrentIndex(-1);

    connect(TypeComboBox, SIGNAL(activated(int)), SLOT(setType(int)));
}

/**
  \brief This will go through the cwSurvexGlobalData and see if it's possible to import it

  The main purpose of this dialog is to check for name collision for stations in a cave
  */
void cwImportTreeDataDialog::updateImportErrors() {

}

/**
  \brief Updates the import warning label next to the import button
  */
void cwImportTreeDataDialog::updateImportWarningLabel() {
    QStringList errors;
    errors << Importer->parseErrors();
    errors << Importer->importErrors();
    int numberWarnings = 0;
    int numberErrors = 0;
    foreach(QString error, errors) {
        if(error.contains("error", Qt::CaseInsensitive)) {
            numberErrors++;
        } else if(error.contains("warning", Qt::CaseInsensitive)) {
            numberWarnings++;
        }
    }

    QString message;
    if(numberErrors > 0) {
        message.append(QString("<b>Errors: %1</b> ").arg(numberErrors));
    }
    if(numberWarnings > 0) {
        message.append(QString("Warnings: %2").arg(numberWarnings));
    }
    WarningLabel->setText(message);
}


/**
  \brief Updates this view with the current items that are selected
  */
void cwImportTreeDataDialog::updateCurrentItem(QItemSelection /*selected*/, QItemSelection /*deselected*/) {
    updateTargets();

//    QModelIndexList selectedIndexes = selected.indexes();
    QModelIndexList selectedIndexes = SurvexSelectionModel->selection().indexes();
    if(selectedIndexes.size() == 1) {
        //Only one item selected
        QModelIndex index = selectedIndexes.first();
        cwTreeImportDataNode* block = Model->toNode(index);
        if(block != nullptr) {

            //Set the index of the combo box
            TypeItem item = importTypeToTypeItem(block->importType());
            TypeComboBox->setCurrentIndex(item);

//            //Set the text for the combo box
//            QString name = block->name();
//            QString noImportStrting = QString("%1 %2").arg(cwTreeImportDataNode::importTypeToString(cwTreeImportDataNode::NoImport)).arg(name);
//            QString caveString  = QString("%1 is a %2").arg(name).arg(cwTreeImportDataNode::importTypeToString(cwTreeImportDataNode::NewCave));
//            QString tripString = QString("%1 is a %2").arg(name).arg(cwTreeImportDataNode::importTypeToString(cwTreeImportDataNode::NewTrip));
//            QString existingTripString = QString("%1 should %2").arg(name).arg(cwTreeImportDataNode::importTypeToString(cwTreeImportDataNode::ReplaceTrip));

//            TypeComboBox->setItemText((int)NoImportItem, noImportStrting);
//            TypeComboBox->setItemText((int)NewCaveItem, caveString);
//            TypeComboBox->setItemText((int)NewTripItem, tripString);
//            TypeComboBox->setItemText((int)ReplaceTripItem, existingTripString);
            TypeComboBox->setEnabled(true);

            return;
        }
    }

    TypeComboBox->setEnabled(false);
    TypeComboBox->setCurrentIndex(-1);
}

void cwImportTreeDataDialog::updateTargets() {
    Updating = true;

    try {
        QModelIndexList selectedIndexes = SurvexSelectionModel->selection().indexes();
        cwTreeImportDataNode* block = nullptr;
        if(selectedIndexes.size() == 1) {
            QModelIndex index = selectedIndexes.first();
            block = Model->toNode(index);
        }

        TargetCaveLabel->setVisible(block != nullptr && (
                    block->importType() == cwTreeImportDataNode::AddToCave ||
                    block->importType() == cwTreeImportDataNode::ReplaceTrip));
        TargetCaveComboBox->setVisible(TargetCaveLabel->isVisible());
        int caveIndex = block == nullptr ? -1 : Region->indexOf(block->targetCave());
        TargetCaveComboBox->setCurrentIndex(caveIndex);

        TargetTripLabel->setVisible(block != nullptr &&
                block->importType() == cwTreeImportDataNode::ReplaceTrip);
        TargetTripComboBox->setVisible(TargetTripLabel->isVisible());
        if (block != nullptr && block->targetCave() != nullptr) {
            TargetTripComboBox->setModel(block->targetCave());
            int tripIndex = block->targetTrip() == nullptr ?
                        -1 : block->targetCave()->indexOf(block->targetTrip());
            TargetTripComboBox->setCurrentIndex(tripIndex);
        }
        else {
            TargetTripComboBox->setModel(new QStandardItemModel);
        }
    }
    catch (...) {
        Updating = false;
        throw;
    }

    Updating = false;
}

/**
  \brief Sets the type for the currently select item(s)

  If multiple items are selected then
  */
void cwImportTreeDataDialog::setType(int index) {

    QModelIndexList selectedIndexes = SurvexSelectionModel->selectedIndexes();

    foreach(QModelIndex currentIndex, selectedIndexes) {
        cwTreeImportDataNode* block = Model->toNode(currentIndex);

        if(block != nullptr) {
            cwTreeImportDataNode::ImportType importType = (cwTreeImportDataNode::ImportType)typeItemToImportType((TypeItem)index);
            block->setImportType(importType);
        }
    }

    updateTargets();
}


/**
  \brief Converts a typeItem to a cwSurvexBlockData::ImportType
  */
int cwImportTreeDataDialog::typeItemToImportType(TypeItem typeItem) const {
    switch(typeItem) {
    case NoImportItem:
        return cwTreeImportDataNode::NoImport;
    case ExistingTripItem:
        return cwTreeImportDataNode::ExistingTrip;
    case NewCaveItem:
        return cwTreeImportDataNode::NewCave;
    case NewTripItem:
        return cwTreeImportDataNode::AddToCave;
    case ReplaceTripItem:
        return cwTreeImportDataNode::ReplaceTrip;
    default:
        return -1;
    }
}

    /**
      \brief Converts a cwSurvexBlockData::ImportType to a typeItem
      */
cwImportTreeDataDialog::TypeItem cwImportTreeDataDialog::importTypeToTypeItem(int type) const {
    switch(type) {
    case cwTreeImportDataNode::NoImport:
        return NoImportItem;
    case cwTreeImportDataNode::ExistingTrip:
        return ExistingTripItem;
    case cwTreeImportDataNode::NewCave:
        return NewCaveItem;
    case cwTreeImportDataNode::AddToCave:
        return NewTripItem;
    case cwTreeImportDataNode::ReplaceTrip:
        return ReplaceTripItem;
    }
    return (cwImportTreeDataDialog::TypeItem)-1;
}

/**
  \brief Tries to import
  */
void cwImportTreeDataDialog::import() {
    cwTreeImportData* globalData = Importer->data();

    beginUndoMacro("Import data");
    globalData->importInto(Region);
    endUndoMacro();

//    if(!globalData->caves().isEmpty()) {
//        beginUndoMacro("Import survex");
//        Region->addCaves(globalData->caves());
//        endUndoMacro();
//    }
    accept();
}

/**
  \brief Called when the importer has finished

  All the data has been parsed out of the importer
  */
void cwImportTreeDataDialog::importerFinishedRunning() {
    //Move the importer back to this thread
    Importer->setThread(thread(), Qt::BlockingQueuedConnection);

    Importer->data()->moveToThread(thread());

    //Get the importer's data
    Model->setTreeImportData(Importer->data());

    //Cutoff to long text
    QString cutOffText = FileLabel->fontMetrics().elidedText(FullFilename, Qt::ElideMiddle, FileLabel->width());
    FileLabel->setText(cutOffText);

    //Load the parse error list view
    cwStringListErrorModel* parsingErrorsModel = new cwStringListErrorModel(this);
    parsingErrorsModel->setStringList(Importer->parseErrors());
    SurvexErrorListView->setModel(parsingErrorsModel);

    //Load the import error list view
    cwStringListErrorModel* importErrorsModel = new cwStringListErrorModel(this);
    importErrorsModel->setStringList(Importer->importErrors());
    ImportErrorListView->setModel(importErrorsModel);

    //Update the error / warning label at the bottom
    updateImportWarningLabel();

    //shut down the thread
    ImportThread->quit();

    show();
}

/**
  \brief Called if the importer has been canceled by the user
  */
void cwImportTreeDataDialog::importerCanceled() {
    ImportThread->quit();
    close();
}

/**
 * @brief cwImportTreeDataDialog::updateImportButton
 *
 * This updates the import button enable / disable
 */
void cwImportTreeDataDialog::updateImportButton(QModelIndex /*begin*/, QModelIndex /*end*/)
{
    ImportButton->setEnabled(Importer->data()->canImport());

    if(!ImportButton->isEnabled()) {
        HintLabel->setText("You need to select which caves you want to import");
    } else {
        HintLabel->setText(QString());
    }
}

void cwImportTreeDataDialog::targetCaveChanged(int index) {
    if (Updating) return;

    QModelIndexList selectedIndexes = SurvexSelectionModel->selection().indexes();
    cwTreeImportDataNode* block = nullptr;
    if(selectedIndexes.size() == 1) {
        QModelIndex index = selectedIndexes.first();
        block = Model->toNode(index);
    }
    if (block != nullptr && index < Region->caveCount()) {
        block->setTargetCave(Region->cave(index));
        updateTargets();
    }
}

void cwImportTreeDataDialog::targetTripChanged(int index) {
    if (Updating) return;

    QModelIndexList selectedIndexes = SurvexSelectionModel->selection().indexes();
    cwTreeImportDataNode* block = nullptr;
    if(selectedIndexes.size() == 1) {
        QModelIndex index = selectedIndexes.first();
        block = Model->toNode(index);
    }
    if (block != nullptr && block->targetCave() != nullptr && index < block->targetCave()->tripCount()) {
        block->setTargetTrip(block->targetCave()->trip(index));
        updateTargets();
    }
}
