import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import Cavewhere 1.0

Rectangle {
    id: leadPage

    property Cave cave;

    LeadModel {
        id: leadModel
        regionModel: rootData.regionTreeModel
        cave: leadPage.cave
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            TextField {
                id: searchBox

                placeholderText: "Filter..."
                inputMethodHints: Qt.ImhNoPredictiveText

                Layout.alignment: Qt.AlignRight

                implicitWidth: 150
            }

            Item { implicitWidth: 50 }

            Text {
                text: "Lead Distance from:"
            }

            ClickTextInput {
                text: leadModel.referanceStation.length === 0 ? "No Station" : leadModel.referanceStation
                onFinishedEditting: {
                    leadModel.referanceStation = newText
                }
            }

            InformationButton {
                onClicked: distanceStationHelpArea.visible = !distanceStationHelpArea.visible
            }

            HelpArea {
                id: distanceStationHelpArea
                implicitWidth: 200
                text: "Lead distance from a station, calculates the <b>line of sight</b> distance from the station to all the leads."
            }
        }

        TableView {
            id: tableView

            Layout.fillHeight: true
            Layout.fillWidth: true

            sortIndicatorVisible: true
            sortIndicatorColumn: 1


            implicitWidth: 400
            model: LeadsSortFilterProxyModel {
                source: leadModel

                sortOrder: tableView.sortIndicatorOrder
                sortCaseSensitivity: Qt.CaseInsensitive
                sortRole: tableView.getColumn(tableView.sortIndicatorColumn).role

                filterString: "*" + searchBox.text + "*"
                filterSyntax: SortFilterProxyModel.Wildcard
                filterCaseSensitivity: Qt.CaseInsensitive
                filterRole: ""
            }

            TableViewColumn { role: "leadCompleted"; width: 20 }
            TableViewColumn { role: "leadNearestStation"; title: "Station"; width: 65}
            TableViewColumn { role: "leadSizeAsString"; title: "Size"; width: 50 }
            TableViewColumn {
                role: "leadDistanceToReferanceStation";
                title: "Distance to " + leadModel.referanceStation;
                width: 100
            }
            TableViewColumn { role: "leadPosition"; title: "Goto"; width: 40 }
            TableViewColumn { role: "leadDescription"; title: "Description"; width: 400 }

            section.property: "leadCompleted"
            section.delegate: Rectangle {
                width: tableView.width
                height: childrenRect.height

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 3
                    text: section == "true" ? "Completed Leads" : "Leads"
                    font.bold: true
                }
            }

            itemDelegate: Item {
                Loader {
                    sourceComponent: {
                        switch(tableView.getColumn(styleData.column).role) {
                        case "leadCompleted":
                            return checkboxComponent;
                        case "leadNearestStation":
                        case "leadSizeAsString":
                        case "leadDescription":
                            return textComponent
                        case "leadPosition":
                            return gotoViewComponent
                        case "leadDistanceToReferanceStation":
                            return lengthComponent
                        }
                    }
                }

                Component {
                    id: checkboxComponent
                    CheckBox {
                        property bool modelChecked: styleData.value

                        onModelCheckedChanged: {
                            checked = modelChecked
                        }

                        onCheckedChanged: {
                            var index = tableView.model.index(styleData.row);
                            tableView.model.setData(index, checked, LeadModel.LeadCompleted);

                            //Update the checked, because the sorting probably change the
                            //data an styleData.row
                            index = tableView.model.index(styleData.row);
                            var newData = tableView.model.data(index, LeadModel.LeadCompleted);
                            if(typeof newData !== "undefined") {
                                checked = newData;
                            }
                        }

                        Component.onCompleted: {
                            checked = styleData.value
                        }
                    }
                }

                Component {
                    id: lengthComponent
                    Text {
                        text: Math.round(styleData.value) + " m"
                        x: 3
                    }
                }

                Component {
                    id: textComponent
                    Text {
                        text: styleData.value
                        x: 3
                    }
                }

                Component {
                    id: gotoViewComponent
                    LinkText {
                        text: "Goto"
                        onClicked: {
                            var index = tableView.model.index(styleData.row);
                            var scrap = tableView.model.data(index, LeadModel.LeadScrap);
                            var leadIndex = tableView.model.data(index, LeadModel.LeadIndexInScrap);

                            //Change to the view page, animate to the lead position, and select it
                            rootData.pageSelectionModel.currentPageAddress = "View"
                            pageView.currentPageItem.turnTableInteraction.centerOn(styleData.value, true);
                            pageView.currentPageItem.leadView.select(scrap, leadIndex);
                        }
                    }
                }
            }
        }
    }
}

