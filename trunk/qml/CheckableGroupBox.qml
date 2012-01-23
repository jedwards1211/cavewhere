// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import QtDesktop 0.1 as Desktop

Item {

    property color backgroundColor: "white"
    property alias checked: checkbox.checked
    property alias text: checkbox.text
    property bool contentsVisible: true
    property int contentHeight
    default property alias contentChildren: contentArea.children

    height: contentsVisible ? checkbox.height + contentHeight + 3 : checkbox.height

    Style {
        id: style
    }

    Rectangle {
        id: checkBoxGroup
        border.width: 1
        radius: style.floatingWidgetRadius
        color: "#00000000"
        visible: contentsVisible

        anchors.top: checkbox.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Item {
            id: contentArea
            anchors.top: checkBoxGroup.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            anchors.topMargin: checkbox.height / 2
            anchors.leftMargin: 3
            anchors.rightMargin: 3
            anchors.bottomMargin: 3
        }
    }

    Rectangle {
        color: backgroundColor
        anchors.fill: checkbox
        visible: contentsVisible
    }

    Desktop.CheckBox {
        id: checkbox
        anchors.left: checkBoxGroup.left
        anchors.leftMargin: 6
    }

}