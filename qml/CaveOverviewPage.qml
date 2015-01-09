/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

import QtQuick 2.0
import Cavewhere 1.0
import QtQml 2.2
import QtQuick.Controls 1.2 as Controls;
import QtQuick.Layouts 1.1

Rectangle {
    id: cavePageArea

    property Cave currentCave: null

    anchors.fill: parent;

    function tripPageName(tripName) {
        return "Trip=" + tripName;
    }

    DoubleClickTextInput {
        id: caveNameText
        text: currentCave != null ? currentCave.name : "" //From c++
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        font.bold: true
        font.pointSize: 20

        onFinishedEditting: {
            currentCave.name = newText
        }
    }

    //    UsedStationsWidget {
    //        id: usedStationWidgetId

    //        anchors.left: lengthDepthContainerId.right
    //        anchors.top: caveNameText.bottom
    //        anchors.bottom: parent.bottom
    //        anchors.leftMargin: 5
    //        anchors.bottomMargin: 5
    //        width: 250
    //    }

    RowLayout {
        Rectangle {
            id: lengthDepthContainerId

            color: "lightgray"
//            anchors.left: parent.left
//            anchors.top: usedStationWidgetId.top
//            anchors.margins: 5

            implicitWidth:  caveLengthAndDepthId.width + 10
            implicitHeight: caveLengthAndDepthId.height + 10

            CaveLengthAndDepth {
                id: caveLengthAndDepthId

                anchors.centerIn: parent

                currentCave: cavePageArea.currentCave
            }
        }

        Controls.TableView {
            model: currentCave

            Controls.TableViewColumn{ role: "tripObjectRole"; title: "Trip"; }
            Controls.TableViewColumn{ role: "tripObjectRole"; title: "Date"; }
            Controls.TableViewColumn{ role: "tripObjectRole"; title: "Survey"; }
            Controls.TableViewColumn{ role: "tripObjectRole"; title: "Length"; }

            itemDelegate:
                Item {
                LinkText {
                    visible: styleData.column === 0
                    text: styleData.value.name
                    onClicked: {
                        rootData.pageSelectionModel.gotoPageByName(cavePageArea,
                                                                   tripPageName(styleData.value.name));
                    }
                }

//                TripLengthTask {
//                    id: tripLengthTask
//                    trip: styleData.value
//                }

                Text {
                    visible: styleData.column === 1
                    text: styleData.value.date
                }

                Text {
                    visible: styleData.column === 2
                    text: "Unknown"
                }

//                UnitValueInput {
//                    visible: styleData.column === 3
//                    unitValue: tripLengthTask.length
//                    valueReadOnly: true
//                }
            }
        }
    }


    Instantiator {
        id: instantiatorId

        onModelChanged: {
            console.log("Model changed:" + cavePageArea.currentCave + " " + model)
        }

        delegate: QtObject {
            id: delegateObjectId
            property Trip trip: tripObjectRole
            property Page page
            property Connections connections: Connections {
                target: trip
                onNameChanged: {
                    page.name = tripPageName(delegateObjectId.trip.name);
                }
            }
        }

        onObjectAdded: {
            //In-ables the link
            console.log("Added! Page for trip" + object.trip + object.trip.name) //cavingTrip);
            rootData.pageSelectionModel.registerPage(cavePageArea, //From
                                                     tripPageName(object.trip.name), //Name
                                                     surveyEditorComponent, //Function
                                                     {"currentTrip":object.trip}
                                                     )

        }

        onObjectRemoved: {
            //TODO, implement me
        }
    }

    onCurrentCaveChanged: {
        instantiatorId.model = cavePageArea.currentCave
    }

    Component {
        id: surveyEditorComponent
        SurveyEditor {
        }
    }
}
