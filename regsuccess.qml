import QtQuick 2.12
import QtQuick.Controls 2.5

Page {
    id: page

    title: qsTr("Success")
    property string backText: qsTr("\u2039 close")
    property bool backEnabled: true
    property bool settingsEnabled: true

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -0.6 * height

        text: qsTr("Registration successful!")
        font.pointSize: 16
    }

    Label {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: 0.6 * height

        text: qsTr("You can now close this dialog.")
    }

}
