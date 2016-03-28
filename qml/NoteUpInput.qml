/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import Cavewhere 1.0

Item {

    property int scrapType
    property NoteTransform noteTransform
    property HelpArea northUpHelp
    property alias enable: setNorthButton.visible

    signal northUpInteractionActivated()

    implicitHeight: row.height
    implicitWidth: row.width

    RowLayout {
        id: row

        Button {
            id: setNorthButton
            iconSource: {
                switch(scrapType) {
                case Scrap.Plan:
                    return "qrc:/icons/north.png"
                case Scrap.RunningProfile:
                    return "qrc:/icons/up.png"
                default:
                    return "qrc:/icons/error.png"
                }
            }

            onClicked: northUpInteractionActivated()
        }

        LabelWithHelp {
            id: labelId
            helpArea: northUpHelp
            text: {
                switch(scrapType) {
                case Scrap.Plan:
                    return "North"
                case Scrap.RunningProfile:
                    return "Up"
                default:
                    return "Error"
                }
            }
            anchors.verticalCenter: parent.verticalCenter
        }

        ClickTextInput {
            id: clickInput
            readOnly: !enable
            text: ""
            onFinishedEditting: ( { } )
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: unit
            textFormat: Text.RichText
            text: "&deg"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    states: [
        State {
            when: noteTransform !== null

            PropertyChanges {
                target: clickInput
                text: noteTransform.northUp.toFixed(2)
                onFinishedEditting: noteTransform.northUp = newText
            }
        }

    ]


}