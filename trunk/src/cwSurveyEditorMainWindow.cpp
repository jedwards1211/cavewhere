//Our includes
#include "cwSurveyEditorMainWindow.h"
#include "cwSurveyChunk.h"
#include "cwStation.h"
#include "cwShot.h"
#include "cwSurveyImporter.h"
#include "cwSurveryChunkGroup.h"


//Qt includes
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QFileDialog>
#include <QDebug>

cwSurveyEditorMainWindow::cwSurveyEditorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    SurvexImporter(NULL),
    ChunkGroup(new cwSurveyChunkGroup(this))
{
    setupUi(this);

    qmlRegisterType<cwStation>(); //"Cavewhere", 1, 0, "cwStation");
    qmlRegisterType<cwShot>(); //"Cavewhere", 1, 0, "cwShot");
    qmlRegisterType<cwSurveyChunk>();//"Cavewhere", 1, 0, "cwSurveyChunk");
    qmlRegisterType<cwSurveyChunkGroup>();

    connect(actionSurvexImport, SIGNAL(triggered()), SLOT(ImportSurvex()));
    connect(actionReloadQML, SIGNAL(triggered()), SLOT(ReloadQML()));

    //Initial chunk
    cwSurveyChunk* chunk = new cwSurveyChunk(ChunkGroup);
    chunk->AddNewShot(); //Add the first shot

    QList<cwSurveyChunk*> chunks;
    chunks.append(chunk);

    ChunkGroup->setChucks(chunks);

}

void cwSurveyEditorMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
  \brief Opens the suvrex import dialog
  */
void cwSurveyEditorMainWindow::ImportSurvex() {
    if(SurvexImporter == NULL) {
        SurvexImporter = new cwSurveyImporter();
        connect(SurvexImporter, SIGNAL(finishedImporting()), SLOT(UpdateSurveyEditor()));
    }

    QFileDialog* dialog = new QFileDialog(NULL, "Import Survex", SurvexImporter->lastImport(), "Survex *.svx");
    dialog->open(SurvexImporter, SLOT(importSurvex(QString)));
}

/**
  \brief Updates the survey editor
  */
void cwSurveyEditorMainWindow::UpdateSurveyEditor() {
    QList<cwSurveyChunk*> chunks = SurvexImporter->chunks();
    if(ChunkGroup == NULL) {
        ChunkGroup = new cwSurveyChunkGroup(this);
    }
    ChunkGroup->setChucks(chunks);

    ReloadQML();
}

void cwSurveyEditorMainWindow::ReloadQML() {
    QDeclarativeContext* context = DeclarativeView->rootContext();
    context->setContextProperty("survey", ChunkGroup);

    DeclarativeView->setSource(QUrl::fromLocalFile("qml/SurveyEditor.qml"));
}

