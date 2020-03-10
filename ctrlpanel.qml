import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import KIS.Users 1.0

Page {
    id: page
    title: qsTr("User control panel")

    property string backText: qsTr("\u2716 logout")
    property bool backEnabled: true
    property bool settingsEnabled: true

    property ExistingUser user

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

            text: user ? user.textUser : "<invalid user>"
            font.pointSize: 16
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Contributions")
            horizontalAlignment: Text.AlignRight
        }
        Text {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            text: user ? user.textContributions : ""
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Prestige")
            horizontalAlignment: Text.AlignRight
        }
        Text {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            text: user ? user.textPrestige : ""
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

            text: qsTr("Nickname")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            id: nicknameField
            text: user ? user.nickname : ""
            validator: RegExpValidator { regExp: /^[^\s]{0,32}$/ }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true

            enabled: !controller.busy
        }
        Button {
            text: qsTr("Save")
            onClicked: user.nickname = nicknameField.text

            enabled: user && !controller.busy && user.nickname !== nicknameField.text && nicknameField.acceptableInput
        }

        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("You can only change your nickname once in half a year. Nickname is optional. It is readable by staff and it is used in public displays in accordance with KIS policies. Please ask a staff member in case of any inquiries.")
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

            id: consentCheckbox
            checked: user && user.consent
            text: qsTr("I agree with a collection of virtual points (\"prestige\") and a participation in contests regarding these points.")
            onCheckStateChanged: if (user && checked != user.consent) { user.consent = checked; }

            contentItem: Label {
                text: parent.text
                font: parent.font
                wrapMode: Text.WordWrap

                leftPadding: 1.2 * parent.indicator.width
            }

            enabled: user && !controller.busy
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("PIN")
            horizontalAlignment: Text.AlignRight

            visible: user && user.pinVisible
        }
        TextField {
            Layout.fillWidth: true

            id: pinField
            validator: RegExpValidator { regExp: /^([0-9]{6,8}|)$/ }
            color: acceptableInput ? "black" : "red"
            text: user ? user.pin : ""
            echoMode: TextInput.PasswordEchoOnEdit
            selectByMouse: true

            enabled: user && !controller.busy
            visible: user && user.pinVisible
        }
        Button {
            text: qsTr("Save")
            onClicked: user.pin = pinField.text

            enabled: user && !controller.busy && user.pin !== pinField.text && pinField.acceptableInput
            visible: user && user.pinVisible
        }
        Text {
            visible: user && user.pinVisible
        }
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("PIN is used to log in with your RFID. If you leave it blank, you won't be able to log in this way.")
            wrapMode: Text.WordWrap
            font.italic: true

            visible: user && user.pinVisible
        }


        Item {
            Layout.columnSpan: 3
            Layout.fillHeight: true
        }

        Button {
            text: qsTr("Refresh")
            onClicked: {
                user.refresh();
                // restore the bindings
                pinField.text = Qt.binding(function() { return user ? user.pin : ""; });
                nicknameField.text = Qt.binding(function() { return user ? user.nickname : ""; });
                consentCheckbox.checked = Qt.binding(function() { return user && user.consent; });
            }

            enabled: user && !controller.busy
        }
    }
}
