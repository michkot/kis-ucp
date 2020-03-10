import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Page {
    id: page

    title: qsTr("Login or register")
    property string backText: qsTr("\u2716 exit")
    property bool backEnabled: true
    property bool settingsEnabled: true

    RowLayout {
        anchors.fill: parent
        anchors.rightMargin: 100
        anchors.leftMargin: 100
        anchors.bottomMargin: 100
        anchors.topMargin: 100

        spacing: 100

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 20

            Label {
                Layout.fillWidth: true

                text: qsTr("Existing user login")
                wrapMode: Text.WordWrap
                font.bold: true
                font.pointSize: 36
            }

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("Login to see your basic statistics, change your nickname or gamification consent. You need your eduID identity provider credentials to login.")
                wrapMode: Text.WordWrap
            }

            Button {
                Layout.fillWidth: true

                text: qsTr("Login with eduID")
                onClicked: controller.loginClicked()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 20

            Label {
                Layout.fillWidth: true

                text: qsTr("New user registration")
                wrapMode: Text.WordWrap
                font.bold: true
                font.pointSize: 36
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Register with your eduID identity provider credentials. Apart of your credentials you'll need a RFID card (ISIC/Kachnakarta) to successfully finish your registration.")
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: qsTr("If you don't have an identity provider connected in the eduID federation, please ask a staff member for an assistance.")
                wrapMode: Text.WordWrap
            }

            Button {
                Layout.fillWidth: true

                text: qsTr("Register with eduID")
                onClicked: controller.registerClicked()
            }
        }
    }
}
