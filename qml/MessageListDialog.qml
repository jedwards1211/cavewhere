import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Dialog {
    id: messageListDialog
    property alias messages: messagesModel
    property font font: {family: 'Helvetica'}
    property int lineHeight: 18

    ListModel {
        id: messagesModel
    }

    property var iconSources: {
        "info": "qrc:icons/Information.png",
        "warning": "qrc:icons/warning.png",
        "error": "qrc:icons/stopSignError.png"
    }

    function scrollToBottom() {
        var flickable = messagesTable.flickableItem
        flickable.contentY = flickable.contentHeight - flickable.height
    }

    contentItem: TableView {
        id: messagesTable
        TableViewColumn {
            role: "severity"
            title: ""
            width: 25
            delegate: Item {
                Image {
                    anchors.centerIn: parent
                    width: messageListDialog.lineHeight - 2
                    height: messageListDialog.lineHeight - 2
                    source: (styleData.value && iconSources[styleData.value.toLowerCase()]) || ''
                }
            }
        }
        TableViewColumn {
            role: "message"
            title: "Message"
            width: 400
            delegate: Text {
                text: styleData.value
                color: styleData.textColor
                elide: Text.ElideRight
                clip: true
                font: messageListDialog.font
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                lineHeightMode: Text.FixedHeight
                lineHeight: messageListDialog.lineHeight
            }
        }
        TableViewColumn {
            role: "source"
            title: "File"
            width: 185
        }
        TableViewColumn {
            role: "startLine"
            title: "Line"
            width: 40
            delegate: Text {
                text: styleData.value
                anchors.centerIn: parent
                elide: Text.ElideRight
                color: styleData.textColor
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }
        }
        TableViewColumn {
            role: "startColumn"
            title: "Col"
            width: 40
            delegate: Text {
                text: styleData.value
                color: styleData.textColor
                elide: Text.ElideRight
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }
        }
        rowDelegate: Rectangle {
            width: childrenRect.width
            height: {
                var row = messagesModel.get(styleData.row)
                return ((row && row.lineCount) || 1) * messageListDialog.lineHeight
            }
            color: styleData.selected ? '#00d' : styleData.alternate ? '#eee' : 'white'
        }
        model: messagesModel
    }
}
