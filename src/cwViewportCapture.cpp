/**************************************************************************
**
**    Copyright (C) 2014 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Qt includes
#include <QPen>

//Our includes
#include "cwViewportCapture.h"
#include "cwScale.h"
#include "cwScene.h"
#include "cwCamera.h"
#include "cw3dRegionViewer.h"
#include "cwScreenCaptureCommand.h"
#include "cwGraphicsImageItem.h"
#include "cwDebug.h"
#include "cwLength.h"

//undef these because micrsoft is fucking retarded...
#ifdef Q_OS_WIN
#undef far
#undef near
#endif

cwViewportCapture::cwViewportCapture(QObject *parent) :
    cwCaptureItem(parent),
    Resolution(300),
    ScaleOrtho(new cwScale(this)),
    ItemScale(1.0),
    PreviewCapture(true),
    PaperUnit(cwUnits::Inches),
    CapturingImages(false),
    NumberOfImagesProcessed(0),
    Columns(0),
    Rows(0),
    TileSize(1024, 1024),
    CaptureCamera(new cwCamera(this)),
    PreviewItem(nullptr),
    Item(nullptr)
{
    connect(ScaleOrtho, &cwScale::scaleChanged, this, &cwViewportCapture::updateTransformForItems);
    connect(this, &cwViewportCapture::positionOnPaperChanged, this, &cwViewportCapture::updateItemsPosition);
}

cwViewportCapture::~cwViewportCapture()
{
    if(PreviewItem != nullptr) {
        delete PreviewItem;
    }

    if(Item != nullptr) {
        delete Item;
    }
}

/**
* @brief cwScreenCaptureManager::setView
* @param view
*/
void cwViewportCapture::setView(cw3dRegionViewer* view) {
    if(View != view) {
        View = view;
        emit viewChanged();
    }
}

/**
* @brief cwViewportCapture::setResolution
* @param resolution
*/
void cwViewportCapture::setResolution(int resolution) {
    if(Resolution != resolution) {
        Resolution = resolution;
        emit resolutionChanged();
    }
}

/**
 * @brief cwScreenCaptureManager::setViewport
 * @param viewport
 *
 * This is the capturing viewport in opengl. This is the area that will be captured
 * and saved by the manager. The rectangle should be in pixels.
 */
void cwViewportCapture::setViewport(QRect viewport) {
    if(Viewport != viewport) {
        Viewport = viewport;
        emit viewportChanged();

        if(!View.isNull()) {
            cwCamera* camera = View->camera();
            CaptureCamera->setProjection(camera->projection());
            CaptureCamera->setViewport(camera->viewport());
            CaptureCamera->setViewMatrix(camera->viewMatrix());
        }
    }
}

/**
 * @brief cwViewportCapture::capture
 */
void cwViewportCapture::capture()
{
    if(CapturingImages) { return; }
    CapturingImages = true;

    if(!viewport().size().isValid()) {
        qWarning() << "Viewport isn't valid for export:" << viewport();
        return;
    }

    cwScene* scene = view()->scene();
    cwCamera* camera = CaptureCamera;
    cwProjection originalProj = camera->projection();

    QRect viewport = this->viewport();
    QRect cameraViewport = camera->viewport();

    //Flip the viewport so it's in opengl coordinate system instead of qt
    viewport.moveTop(cameraViewport.height() - viewport.bottom());

    double imageScale;
    if(previewCapture()) {
        if(PreviewItem != NULL) {
            delete PreviewItem;
        }
        PreviewItem = new QGraphicsItemGroup();
        previewItemChanged();

        imageScale = 1.0;
    } else {
        if(Item != NULL) {
            delete Item;
        }
        Item = new QGraphicsItemGroup();
        fullResolutionItemChanged();

        imageScale = PreviewItem->scale() * resolution();
    }

    //Updates the scale for the items
    updateTransformForItems();

    QPointF previewItemPosition = PreviewItem->pos();

    QRectF onPaperViewport = QRectF(QPoint(previewItemPosition.x() * imageScale, previewItemPosition.y() * imageScale),
                                  QSizeF(viewport.width() * imageScale, viewport.height() * imageScale));

    QSize tileSize = TileSize;
    QSize imageSize = onPaperViewport.size().toSize(); //QSize(viewport.width() * imageScale, viewport.height() * imageScale);

    int columns = imageSize.width() / tileSize.width();
    int rows = imageSize.height() / tileSize.height();

    if(imageSize.width() % tileSize.width() > 0) { columns++; }
    if(imageSize.height() % tileSize.height() > 0) { rows++; }

    NumberOfImagesProcessed = 0;
    Columns = columns;
    Rows = rows;

    IdToOrigin.clear(); //This is used to keep track the positions of the images

    cwProjection croppedProjection = tileProjection(viewport,
                                                    camera->viewport().size(),
                                                    originalProj);

    for(int column = 0; column < columns; column++) {
        for(int row = 0; row < rows; row++) {

            int x = tileSize.width() * column;
            int y = tileSize.height() * row;

            QSize croppedTileSize = calcCroppedTileSize(tileSize, imageSize, row, column);

            QRect tileViewport(QPoint(x, y), croppedTileSize);
            cwProjection tileProj = tileProjection(tileViewport, imageSize, croppedProjection);

            cwScreenCaptureCommand* command = new cwScreenCaptureCommand();

            cwCamera* croppedCamera = new cwCamera(command);
            croppedCamera->setViewport(QRect(QPoint(), croppedTileSize));
            croppedCamera->setProjection(tileProj);
            croppedCamera->setViewMatrix(camera->viewMatrix());

            command->setCamera(croppedCamera);
            command->setScene(scene);

            int id = row * columns + column;
            command->setId(id);

            double originX = column * tileSize.width();
            double originY = onPaperViewport.height() - (row * tileSize.height() + croppedTileSize.height());
            QPointF origin(originX, originY);

            IdToOrigin[id] = origin;

            connect(command, SIGNAL(createdImage(QImage,int)),
                    this, SLOT(capturedImage(QImage,int)),
                    Qt::QueuedConnection);
            scene->addSceneCommand(command);
        }
    }

    view()->update();
}

/**
 * @brief cwViewportCapture::setPaperWidthOfItem
 * @param width
 *
 * This sets the width in the paper units of how big the catpure is. This will mantain the
 * aspect of the catpure
 */
void cwViewportCapture::setPaperWidthOfItem(double width)
{
    //Using a fuzzy compare, to prevent recursive stack overflow
    //Mathmatically using "!=" should work, but there's a little bit of a fuzzy
    //to the scale values
    if(!qFuzzyCompare(paperSizeOfItem().width(), width)) {

        double scale =  width / (double)viewport().width();
        double height = viewport().height() * scale;

        setPaperSizeOfItem(QSizeF(width, height));
        setImageScale(scale);
    }
}

/**
 * @brief cwViewportCapture::setPaperHeightOfItem
 * @param height
 *
 * This sets the height in the paper units of how big the capture is. This will mantain the
 * aspect of the catpure
 */
void cwViewportCapture::setPaperHeightOfItem(double height)
{
    //Using a fuzzy compare, to prevent recursive stack overflow
    //See comment in setPaperWidthOfItem for more details
    if(!qFuzzyCompare(paperSizeOfItem().height(), height)) {
        double scale =  height / (double)viewport().height();
        double width = viewport().width() * scale;

        setPaperSizeOfItem(QSizeF(width, height));
        setImageScale(scale);
    }
}

/**
 * @brief cwScreenCaptureManager::tileProjection
 * @param viewport
 * @param originalProjection
 * @return
 *
 * This will take original viewport and original projection and find the
 * tile projection matrix using the tileViewport. The tileViewport
 * should be a sub rectangle of the original viewport. This function
 * will work with orthognal and perspective projections
 */
cwProjection cwViewportCapture::tileProjection(QRectF tileViewport,
                                               QSizeF imageSize,
                                               const cwProjection &originalProjection) const
{
    double originalProjectionWidth = originalProjection.right() - originalProjection.left();
    double originalProjectionHeight = originalProjection.top() - originalProjection.bottom();

    double left = originalProjection.left() + originalProjectionWidth
            * (tileViewport.left() / imageSize.width());
    double right = left + originalProjectionWidth * tileViewport.width() / imageSize.width();
    double bottom = originalProjection.bottom() + originalProjectionHeight
            * (tileViewport.top() / imageSize.height());
    double top = bottom + originalProjectionHeight * tileViewport.height() / imageSize.height();

    cwProjection projection;
    switch(originalProjection.type()) {
    case cwProjection::Perspective:
    case cwProjection::PerspectiveFrustum:
        projection.setFrustum(left, right,
                              bottom, top,
                              originalProjection.near(), originalProjection.far());
        return projection;
    case cwProjection::Ortho:
        projection.setOrtho(left, right,
                            bottom, top,
                            originalProjection.near(), originalProjection.far());
        return projection;
    default:
        qWarning() << "Can't return tile matrix because original matrix isn't a orth or perspectiveMatrix" << LOCATION;
        break;
    }
    return projection;
}

/**
 * @brief cwScreenCaptureManager::croppedTile
 * @param tileSize
 * @param imageSize
 * @param row
 * @param column
 * @return Return the tile size of row and column
 *
 * This may crop the tile, if the it goes beyond the imageSize
 */
QSize cwViewportCapture::calcCroppedTileSize(QSize tileSize, QSize imageSize, int row, int column) const
{
    QSize croppedTileSize = tileSize;

    bool exactEdgeX = imageSize.width() % tileSize.width() == 0;
    bool exactEdgeY = imageSize.height() % tileSize.height() == 0;
    int lastColumnIndex = imageSize.width() / tileSize.width();
    int lastRowIndex = imageSize.height() / tileSize.height();


    Q_ASSERT(column <= lastColumnIndex);
    Q_ASSERT(row <= lastRowIndex);

    if(column == lastColumnIndex && !exactEdgeX)
    {
        //Edge tile, crop it
        double tilesInImageX = imageSize.width() / (double)tileSize.width();
        double extraX = tilesInImageX - floor(tilesInImageX);
        double imageWidthCrop = ceil(tileSize.width() * extraX);
        croppedTileSize.setWidth(imageWidthCrop);
    }

    if(row == lastRowIndex && !exactEdgeY)
    {
        //Edge tile, crop it
        double tilesInImageY = imageSize.height() / (double)tileSize.height();
        double extraY = tilesInImageY - floor(tilesInImageY);
        double imageHeightCrop = ceil(tileSize.height() * extraY);
        croppedTileSize.setHeight(imageHeightCrop);
    }

    return croppedTileSize;
}

/**
 * @brief cwViewportCapture::setImageScale
 * @param scale
 *
 * This sets the scaling for the preview item and the full resolution image.
 *
 * If an orthognal projection is being used the scaleOrtho is also update.
 */
void cwViewportCapture::setImageScale(double scale)
{
    if(ItemScale != scale) {
        if(CaptureCamera->projection().type() == cwProjection::Ortho) {
            double meterToPaperUnit = cwUnits::convert(1.0, cwUnits::Meters, PaperUnit);
            ScaleOrtho->setScale(scale * CaptureCamera->pixelsPerMeter() * 1.0 / meterToPaperUnit);
        } else {
            ItemScale = scale;
            updateTransformForItems();
        }
    }
}

/**
 * @brief cwScreenCaptureManager::capturedImage
 * @param image
 */
void cwViewportCapture::capturedImage(QImage image, int id)
{
    Q_UNUSED(id)

    Q_ASSERT(CapturingImages);

    QPointF origin = IdToOrigin.value(id);

    QGraphicsItem* parent = previewCapture() ? PreviewItem : Item;

    cwGraphicsImageItem* graphicsImage = new cwGraphicsImageItem(parent);
    graphicsImage->setImage(image);
    graphicsImage->setPos(origin);

    //For debugging tiles
//    QRectF tileRect = QRectF(origin, image.size());
//    QGraphicsRectItem* rectItem = new QGraphicsRectItem(parent);
//    rectItem->setPen(QPen(Qt::red));
//    rectItem->setRect(tileRect);
//    QGraphicsSimpleTextItem* textItem = new QGraphicsSimpleTextItem(parent);
//    textItem->setText(QString("Id:%1").arg(id));
//    textItem->setPen(QPen(Qt::red));
//    textItem->setPos(tileRect.center());

    NumberOfImagesProcessed++;

    if(NumberOfImagesProcessed == Rows * Columns) {
        //Finished capturing images
        NumberOfImagesProcessed = 0;
        Rows = 0;
        Columns = 0;
        CapturingImages = false;
        emit finishedCapture();
    }
}

/**
 * @brief cwViewportCapture::updateScaleForItems
 *
 * This updates the scale for QGraphicsItems (Preview Item and the Full resolution item)
 */
void cwViewportCapture::updateTransformForItems()
{
    double meterToPaperUnit = cwUnits::convert(1.0, cwUnits::Meters, PaperUnit);
    if(CaptureCamera->projection().type() == cwProjection::Ortho) {
        ItemScale = ScaleOrtho->scale() * (1.0 / CaptureCamera->pixelsPerMeter()) * (meterToPaperUnit);
    }

    double paperWidth = viewport().size().width() * ItemScale;
    setPaperWidthOfItem(paperWidth);

    if(previewItem() != nullptr) {
        previewItem()->setScale(ItemScale);
    }

    if(fullResolutionItem() != nullptr) {
        double hiResScale = paperSizeOfItem().width() / (previewItem()->scale() * resolution() * viewport().width());
        fullResolutionItem()->setScale(hiResScale);
        fullResolutionItem()->setPos(previewItem()->pos());
    }
}

/**
 * @brief cwViewportCapture::updateItemsPosition
 * @param positionOnPaper
 * This sets the position of the viewport capture on the paper. This is
 * in paper units.
 */
void cwViewportCapture::updateItemsPosition()
{
    if(previewItem() != nullptr) {
        previewItem()->setPos(positionOnPaper());
    }

    if(fullResolutionItem() != nullptr) {
        fullResolutionItem()->setPos(positionOnPaper());
    }

}

/**
* @brief cwScreenCaptureManager::view
* @return
*/
cw3dRegionViewer* cwViewportCapture::view() const {
    return View;
}
