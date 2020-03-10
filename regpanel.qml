import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import KIS.Users 1.0

Page {
    id: page
    title: qsTr("New user registration")

    property string backText: qsTr("\u2716 abort")
    property bool backEnabled: true
    property bool settingsEnabled: true

    property NewUser user

    BusyIndicator {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        id: indicator
        running: true

        visible: user && !user.initialized
    }

    Label {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: indicator.height

        text: qsTr("Please wait...")

        visible: user && !user.initialized
    }

    GridLayout {
        anchors.fill: parent
        anchors.rightMargin: 50
        anchors.leftMargin: 50
        anchors.bottomMargin: 50
        anchors.topMargin: 50

        columns: 3
        columnSpacing: 15
        rowSpacing: 10

        visible: !user || user.initialized

        //--------------------------------
        Label {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            text: qsTr("Mandatory information")
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Name")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            text: user ? user.name : "<invalid user>"
            enabled: false
            color: "black"
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Email address")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            text: user ? user.email : ""
            enabled: false
            color: "black"
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("To change your name or your email address contact your identity provider. The details will update automatically upon your next login to KIS.")
            wrapMode: Text.WordWrap
            font.italic: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("RFID")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            text: reader.waiting ? qsTr("touch the reader with your card now") : (user ? user.textRfid : "")
            enabled: false
            color: "black"
            font.italic: reader.waiting || !(user && user.rfidAssigned)
            //font.family: (user && user.rfidAssigned) ? Qt.application.font : fixedfont

        }
        Button {
            id: readButton
            text: reader.waiting ? qsTr("Cancel") : qsTr("Read")
            onClicked: reader.waiting ? reader.setIdle() : reader.readRfid()
            Component.onCompleted: width = Math.max(width, registerButton.width)
            onWidthChanged: width = Math.max(width, registerButton.width)

            enabled: user && !controller.busy && reader.connected
        }
        Text {
            visible: !reader.connected
        }
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("The reader is disconnected. To assign or change your RFID, please ask a staff member for assistance. You cannot finish your registration without assigning an RFID to your account.")
            wrapMode: Text.WordWrap
            color: "red"
            font.bold: true
            font.italic: true

            visible: !reader.connected
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("RFID is used for your authentication and as a proof of your membership. Use of your existing academic (ISIC) card or obtaining a\"Kachnakarta\" is recommended. RFID identifiers are hashed before storing. Please ask a staff member for assistance in case of any inquiries.")
            wrapMode: Text.WordWrap
            font.italic: true
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            implicitHeight: 20
        }

        //--------------------------------
        Label {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            text: qsTr("User preferences")
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Nickname")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            id: nicknameField
            validator: RegExpValidator { regExp: /^[^\s]{0,32}$/ }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true

            enabled: !controller.busy
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("Nickname is optional. It is readable by staff and it is used in public displays in accordance with KIS policies. You can change your nickname in the future, but only half a year after the last change. Please ask a staff member in case of any inquiries.")
            wrapMode: Text.WordWrap
            font.italic: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Gamification")
            horizontalAlignment: Text.AlignRight
        }
        CheckBox {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            id: userConsentCheckbox
            checked: true
            text: qsTr("I agree with a collection of virtual points (\"prestige\") and a participation in contests regarding these points.")

            contentItem: Label {
                text: parent.text
                font: parent.font
                wrapMode: Text.WordWrap

                leftPadding: 1.2 * parent.indicator.width
            }

            enabled: !controller.busy
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("You can change your consent at any time by logging in to your user control panel or asking a staff member.")
            wrapMode: Text.WordWrap
            font.italic: true
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillHeight: true
        }

        //--------------------------------
        Rectangle {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.minimumHeight: 1
            Layout.columnSpan: 3
            Layout.fillWidth: true

            height: 1
            color: "#b3b3b3"
        }

        CheckBox {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            id: agreement
            checked: false
            text: qsTr("I hereby declare I want to become a symathizing member of the FIT BUT SU. I have read and agree to the KIS terms of service.")

            contentItem: Label {
                text: parent.text
                font: parent.font
                wrapMode: Text.WordWrap

                leftPadding: 1.2 * parent.indicator.width
            }

            enabled: !controller.busy
        }
        Button {
            id: registerButton
            text: qsTr("Register")
            Component.onCompleted: width = Math.max(width, readButton.width)
            onWidthChanged: readButton.width = Math.max(width, readButton.width)
            onClicked: { user.updateNickname(nicknameField.text); user.updateConsent(userConsentCheckbox.checked); user.submit() }

            enabled: user && nicknameField.acceptableInput && agreement.checked && user.rfidAssigned && !controller.busy && !reader.waiting
        }
    }
}
