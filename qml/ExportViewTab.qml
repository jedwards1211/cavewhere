import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.1
import Cavewhere 1.0
import QtQuick.Dialogs 1.0 as Dialogs

Item {
    id: exportViewTabId

    implicitWidth: rowLayoutId.width

    property GLTerrainRenderer view
    property alias hideExternalTools: selectionTool.visible

    function updatePaperRectangle(paperWidth, paperHeight) {
        var i = paperSizeInteraction

        i.paperRectangle.paperWidth = i.paperRectangle.landScape ? paperHeight : paperWidth
        i.paperRectangle.paperHeight = i.paperRectangle.landScape ? paperWidth : paperHeight
        //        paperMarginGroupBoxId.unit = paperunits


        //Stretch the paper to the max width or height of the screen
        var paperAspect = paperWidth / paperHeight;
        if(i.paperRectangle.landScape) {
            paperAspect = 1.0 / paperAspect;
        }

        var viewerAspect = view.width / view.height;
        var aspect = paperAspect / viewerAspect
        if(aspect < 1.0) {
            i.paperRectangle.width = view.width * aspect
            i.paperRectangle.height = view.height
        } else {
            var aspect = viewerAspect / paperAspect
            i.paperRectangle.width = view.width
            i.paperRectangle.height = view.height * aspect
        }
    }

    SelectExportAreaTool {
        id: selectionTool
        parent: exportViewTabId.view
        view: exportViewTabId.view
        manager: screenCaptureManagerId
        visible: exportViewTabId.visible
    }

    ChoosePaperSizeInteraction {
        id: paperSizeInteraction
        parent: view
        visible: false; //view !== null

        onWidthChanged: paperComboBoxId.updatePaperRectangleFromModel()
        onHeightChanged: paperComboBoxId.updatePaperRectangleFromModel()
    }

    CaptureManager {
        id: screenCaptureManagerId
        view: exportViewTabId.view
        viewport: Qt.rect(paperSizeInteraction.captureRectangle.x,
                          paperSizeInteraction.captureRectangle.y,
                          paperSizeInteraction.captureRectangle.width,
                          paperSizeInteraction.captureRectangle.height)
        paperSize: Qt.size(paperSizeInteraction.paperRectangle.paperWidth,
                           paperSizeInteraction.paperRectangle.paperHeight)
        //        screenPaperSize: Qt.size(paperSizeInteraction.paperRectangle.width,
        //                                 paperSizeInteraction.paperRectangle.height)
        onFinishedCapture: Qt.openUrlExternally(filename)
    }

    RowLayout {
        id: rowLayoutId

        QuickSceneView {
            width: 500
            Layout.fillHeight: true
            scene: screenCaptureManagerId.scene

            CaptureItemManiputalor {
                anchors.fill: parent;
                manager: screenCaptureManagerId
            }
        }

        ColumnLayout {
            id: columnLayoutId

            GroupBox {
                title: "Paper Size"

                ColumnLayout {

                    ComboBox {
                        id: paperComboBoxId
                        model: paperSizeModel
                        textRole: "name"

                        //                    property alias paperRectangle: paperSizeInteraction.paperRectangle

                        function updatePaperRectangleFromModel() {
                            var item = paperSizeModel.get(currentIndex);

                            if(item) {
                                exportViewTabId.updatePaperRectangle(item.width, item.height)
                            }
                        }

                        function updateDefaultMargins() {
                            var item = paperSizeModel.get(currentIndex);
                            paperMarginGroupBoxId.setDefaultLeft(item.defaultLeftMargin)
                            paperMarginGroupBoxId.setDefaultRight(item.defaultRightMargin)
                            paperMarginGroupBoxId.setDefaultTop(item.defaultTopMargin)
                            paperMarginGroupBoxId.setDefaultBottom(item.defaultBottomMargin)
                        }

                        onCurrentIndexChanged: {
                            updatePaperRectangleFromModel()
                            updateDefaultMargins()
                        }

                        Component.onCompleted: {
                            updatePaperRectangleFromModel()
                            updateDefaultMargins()
                        }
                    }

                    RowLayout {
                        Text {
                            text: "Width"
                        }

                        ClickTextInput {
                            id: paperSizeWidthInputId
                            text: screenCaptureManagerId.paperSize.width
                            readOnly: paperComboBoxId.currentIndex != 3
                            onFinishedEditting: {
                                paperSizeModel.setProperty(paperComboBoxId.currentIndex, "width", newText)
                                paperComboBoxId.updatePaperRectangleFromModel();
                            }
                        }

                        Text {
                            text: "in"
                            font.italic: true
                        }

                        Text {
                            text: "Height"
                        }

                        ClickTextInput {
                            text: screenCaptureManagerId.paperSize.height
                            readOnly: paperSizeWidthInputId.readOnly
                            onFinishedEditting: {
                                paperSizeModel.setProperty(paperComboBoxId.currentIndex, "height", newText)
                                paperComboBoxId.updatePaperRectangleFromModel();
                            }
                        }

                        Text {
                            text: "in"
                            font.italic: true
                        }
                    }

                    RowLayout {
                        Text {
                            text: "Resolution"
                        }

                        SpinBox {
                            id: resolutionSpinBoxId
                            value: screenCaptureManagerId.resolution
                            stepSize: 100
                            maximumValue: 600

                            onValueChanged: {
                                screenCaptureManagerId.resolution = value
                            }

                            Connections {
                                target: screenCaptureManagerId
                                onResolutionChanged: resolutionSpinBoxId.value = screenCaptureManagerId.resolution
                            }
                        }

                        Text {
                            text: "DPI"
                            font.italic: true
                        }
                    }
                }
            }

            PaperMarginGroupBox {
                id: paperMarginGroupBoxId

                onLeftMarginChanged: screenCaptureManagerId.leftMargin = leftMargin
                onRightMarginChanged: screenCaptureManagerId.rightMargin = rightMargin
                onTopMarginChanged: screenCaptureManagerId.topMargin = topMargin
                onBottomMarginChanged: screenCaptureManagerId.bottomMargin = bottomMargin
            }

            GroupBox {
                title: "Orientation"

                RowLayout {
                    id: portraitLandscrapSwitch
                    Text {
                        text: "Portrait"
                    }

                    Switch {
                        id: orientationSwitchId
                        onCheckedChanged: {
                            paperSizeInteraction.paperRectangle.landScape = checked
                            paperComboBoxId.updatePaperRectangleFromModel()
                        }
                    }

                    Text {
                        text: "Landscrape"
                    }
                }
            }

            GroupBox {
                id: layersGroupBoxId
                title: "Layers"

                ScrollView {
                    ListView {
                        id: layerListViewId

                        model: screenCaptureManagerId

                        anchors.left: parent.left
                        anchors.right: parent.right

                        delegate: Text {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: 5
                            text: layerNameRole

                            MouseArea{
                                anchors.fill: parent
                                onClicked: {
                                    layerListViewId.currentIndex = index
                                }
                            }
                        }

                        highlight: Rectangle {
                            color: "#8AC6FF"
                            radius: 3
                        }

                        onCurrentIndexChanged: {
                            if(currentIndex !== -1) {
                                var modelIndex = screenCaptureManagerId.index(currentIndex);
                                var layerObject = screenCaptureManagerId.data(modelIndex, CaptureManager.LayerObjectRole);
                                layerProperties.layerObject = layerObject
                            } else {
                                layerProperties.layerObject = null;
                            }
                        }
                    }
                }
            }

            GroupBox {
                id: layerProperties

                property var layerObject: null

                title: "" //layerObject !== null ? "Properies of " + layerObject.name : ""
                visible: false //layerObject !== null

                ColumnLayout {

                    PaperScaleInput {
                        id: paperScaleInputId
                        usingInteraction: false
                        scaleObject: null
                    }

                    RowLayout {
                        Text {
                            text: "Size"
                        }

                        ClickTextInput {
                            id: sizeWidthInputId
                            text: "" //layerProperties.layerObject !== null ? layerProperties.layerObject.paperSizeOfItem.width : ""
                        }

                        Text {
                            text: "x"
                        }

                        ClickTextInput {
                            id: sizeHeightInputId
                            text: "" //layerProperties.layerObject !== null ? layerProperties.layerObject.paperSizeOfItem.height : ""
                        }

                    }

                    RowLayout {
                        Text {
                            text: "Position"
                        }

                        Text {
                            text: "x:"
                        }

                        ClickTextInput {
                            id: posXInputId
                            text: ""
                        }

                        Text {
                            text: "y:"
                        }

                        ClickTextInput {
                            id: posYInputId
                            text: ""
                        }
                    }


                }

                states: [
                    State {
                        when: layerProperties.layerObject !== null

                        PropertyChanges {
                            target: layerProperties
                            title: "Properies of " + layerObject.name
                            visible: true
                        }

                        PropertyChanges {
                            target: paperScaleInputId
                            scaleObject: layerProperties.layerObject.scaleOrtho
                        }

                        PropertyChanges {
                            target: sizeWidthInputId
                            text: layerProperties.layerObject.paperSizeOfItem.width
                            onFinishedEditting: {
                                layerProperties.layerObject.setPaperWidthOfItem(newText)
                            }
                        }

                        PropertyChanges {
                            target: sizeHeightInputId
                            text: layerProperties.layerObject.paperSizeOfItem.height
                            onFinishedEditting: {
                                layerProperties.layerObject.setPaperHeightOfItem(newText)
                            }
                        }

                        PropertyChanges {
                            target: posXInputId
                            text: layerProperties.layerObject.positionOnPaper.x
                            onFinishedEditting: {
                                var y = layerProperties.layerObject.positionOnPaper.y
                                layerProperties.layerObject.positionOnPaper = Qt.point(newText, y);
                            }
                        }

                        PropertyChanges {
                            target: posYInputId
                            text: layerProperties.layerObject.positionOnPaper.y
                            onFinishedEditting: {
                                var x = layerProperties.layerObject.positionOnPaper.x
                                layerProperties.layerObject.positionOnPaper = Qt.point(x, newText);
                            }
                        }
                    }
                ]
            }

            GroupBox {
                title: "File type"

                ComboBox {
                    id: fileTypeExportComboBox
                    model: ["PDF", "SVG", "PNG"]
                }
            }

            Button {
                Layout.alignment: Qt.AlignRight
                text: "Export"

                onClicked: {
//                    exportDialogId.open();
                    screenCaptureManagerId.filename = "file://Users/vpicaver/Documents/Projects/cavewhere/testcase/test.png"
                    screenCaptureManagerId.capture()
                }
            }
        }
    }

    Dialogs.FileDialog {
        id: exportDialogId
        title: "Export to " + fileTypeExportComboBox.currentText
        selectExisting: false
        onAccepted: {
            screenCaptureManagerId.filename = fileUrl
            screenCaptureManagerId.capture();
        }
    }

    ListModel {
        id: paperSizeModel

        ListElement {
            name: "Letter"
            width: 8.5
            height: 11
            units: "in"
            defaultLeftMargin: 1.0
            defaultRightMargin: 1.0
            defaultTopMargin: 1.0
            defaultBottomMargin: 1.0
        }

        ListElement {
            name: "Legal"
            width: 8.5
            height: 14
            units: "in"
            defaultLeftMargin: 1.0
            defaultRightMargin: 1.0
            defaultTopMargin: 1.0
            defaultBottomMargin: 1.0
        }

        ListElement {
            name: "A4"
            width: 8.26772
            height: 11.6929
            units: "in"
            defaultLeftMargin: 1.0
            defaultRightMargin: 1.0
            defaultTopMargin: 1.0
            defaultBottomMargin: 1.0
        }

        ListElement {
            name: "Custom Size"
            width: 8.5
            height: 11
            units: "in"
            defaultLeftMargin: 1.0
            defaultRightMargin: 1.0
            defaultTopMargin: 1.0
            defaultBottomMargin: 1.0
        }
    }
}
