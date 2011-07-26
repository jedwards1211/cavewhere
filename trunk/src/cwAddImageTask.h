#ifndef CWLOADIMAGETASK_H
#define CWLOADIMAGETASK_H

//Our includes
#include "cwTask.h"
#include "cwImage.h"

//Qt includes
#include <QStringList>
#include <QString>
#include <QImage>
#include <QDir>
#include <QSqlDatabase>

class cwAddImageTask : public cwTask
{
    Q_OBJECT

public:
    cwAddImageTask(QObject* parent = NULL);

    //////////////// Parameters //////////////////
    void setBaseDirectory(QDir baseDirectory);
    void setImagesPath(QStringList imagePaths);

//    void setDatabasePath(const QString& databasePath);

    ///////////// Results ///////////////////
    QList<cwImage> images();

    //static int addImageToDatabase(QSqlDatabase& database, const cwImageData& data);

protected:
    virtual void runTask();

private:
    QDir BaseDirectory;
    QStringList ImagePaths;

    QList<cwImage> Images;
    QStringList Errors;

    static const QString IconsDirectory;
    static const QString MipmapsDirecotry;

    /**
      This will scale an image to size

      This class is used with Qt::Concurrent to scale a single image multiple times
      */
    class Scaler {
    public:
        Scaler(const QImage& image) { OriginalImage = image; }

        QImage operator()(int maxSideInPixels);

    private:
        QImage OriginalImage;
    };

    QImage imageExists(QString image);
    QString copyOriginalImage(QString image);
    QString createIcon(QImage originalImage, QString baseFilename);
    QStringList createMipmaps(QImage originalImage, QString baseFilename);
    void saveToDXT1Format(QImage image, QString path);

private slots:


};

/**
  \brie Sets the base directory of where the image will be stored to

  \param the baseDirectory
  */
inline void cwAddImageTask::setBaseDirectory(QDir baseDirectory) {
    BaseDirectory = baseDirectory;
}

/**
  \brief Sets the databasePath

  This should be set before calling start!  And shouldn't be changed until the task
  has finished
  */
inline void cwAddImageTask::setImagesPath(QStringList imagePaths) {
    ImagePaths = imagePaths;
}

/**
  Get's all the images that have been put into the database

  \brief This should only be called when the task is not running
  */
inline QList<cwImage> cwAddImageTask::images() {
    return Images;
}

#endif // CWLOADIMAGETASK_H
