/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

// import QtQuick 2.0 // to target S60 5th Edition or Maemo 5
import QtQuick 2.0
import Cavewhere 1.0

Item {

    property alias noteModel: view.model

    signal imagesAdded(var images)

    LoadNotesWidget {
        visible: view.count == 0
        onFilesSelected: imagesAdded(images)
    }

    ListView {
        id: view

        anchors.fill: parent

        snapMode: ListView.SnapOneItem
        orientation: ListView.Horizontal
        spacing: 10
        interactive: false
        highlightMoveDuration: 200

        delegate: ImageExplorer {
            width: view.width
            height: view.height
            image: model.image
            rotation: model.noteObject.rotate
        }
    }

    ImageArrowNavigation {
        highlightedImageSource: "qrc:icons/previousPressed.png"
        imageSource: "qrc:icons/previousUnPressArrow.png"
        visible: view.currentIndex - 1 >= 0
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.topMargin: 10

        onClicked: {
            view.currentIndex = Math.max(0, view.currentIndex - 1)
        }
    }

    ImageArrowNavigation {
        highlightedImageSource: "qrc:icons/nextPressed.png"
        imageSource: "qrc:icons/nextUnPressArrow.png"
        visible: view.currentIndex + 1 < view.count
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.topMargin: 10

        onClicked: {
            view.currentIndex = Math.min(view.count - 1, view.currentIndex + 1)
        }
    }
}
