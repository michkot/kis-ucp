import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Page {
    id: page

    title: qsTr("An error occured")
    property string backText: qsTr("\u2716 close")
    property bool backEnabled: true
    property bool settingsEnabled: true

    ColumnLayout {
        anchors.fill: parent
        anchors.rightMargin: 100
        anchors.leftMargin: 100
        anchors.bottomMargin: 100
        anchors.topMargin: 100

        spacing: 10

        //--------------------------------
        Label {
            Layout.fillWidth: true

            text: controller.errorTitle
            font.pointSize: 16
            font.bold: true
        }

        Text {
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: controller.errorDescription
            wrapMode: Text.WordWrap
        }
    }
}
