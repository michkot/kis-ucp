import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Page {
    id: page
    title: qsTr("Settings")

    property string backText: qsTr("\u2716 close")
    property bool backEnabled: true
    property bool settingsEnabled: false

    GridLayout {
        anchors.fill: parent
        anchors.rightMargin: 100
        anchors.leftMargin: 100
        anchors.bottomMargin: 100
        anchors.topMargin: 100

        columns: 3
        columnSpacing: 15
        rowSpacing: 10

        Label {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            text: qsTr("API settings")
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("API base URL")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            id: apiBaseUrlField
            text: settings.apiBaseUrl
            validator: RegExpValidator { regExp: /^(?:https?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/ }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true
        }
        Button {
            text: qsTr("Save")
            onClicked: settings.apiBaseUrl = apiBaseUrlField.text;

            enabled: apiBaseUrlField.acceptableInput && apiBaseUrlField.text !== Qt.resolvedUrl(settings.apiBaseUrl)
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("eduId session timeout")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            id: eduidTimeoutField
            text: String(settings.eduidSessionTimeout)
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator{ bottom: 1; top: 1000 }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true
        }
        Button {
            text: qsTr("Save")
            onClicked: settings.eduidSessionTimeout = Number(eduidTimeoutField.text)

            enabled: eduidTimeoutField.acceptableInput && Number(eduidTimeoutField.text) !== settings.eduidSessionTimeout
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("eduID session timeout in minutes. This must be set to the same value as the backend for correct operation.")
            wrapMode: Text.WordWrap
            font.italic: true
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            implicitHeight: 20
        }

        Label {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            text: qsTr("RFID reader settings")
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("RFID reader URL")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            id: rfidReaderUrlField
            text: settings.rfidReaderUrl
            validator: RegExpValidator { regExp: /^(?:wss?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/ }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true

            enabled: !reader.connected
        }
        Button {
            text: qsTr("Save")
            onClicked: settings.rfidReaderUrl = rfidReaderUrlField.text;

            enabled: rfidReaderUrlField.acceptableInput && rfidReaderUrlField.text !== Qt.resolvedUrl(settings.rfidReaderUrl)
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("You have to disconnect the reader prior to changing its URL. You also have to save any changes before reconnecting. Only web socket schemas allowed.")
            font.italic: true
            color: "grey"
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Connection")
            horizontalAlignment: Text.AlignRight
        }
        RowLayout {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            spacing: 15

            Button {
                text: qsTr("Connect")
                enabled: !reader.connected && (rfidReaderUrlField.text === Qt.resolvedUrl(settings.rfidReaderUrl))
                onClicked: reader.connect(Qt.resolvedUrl(rfidReaderUrlField.text))
            }
            Button {
                text: qsTr("Disconnect")
                enabled: reader.connected
                onClicked: reader.disconnect()
            }
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            implicitHeight: 20
        }

        Label {
            Layout.columnSpan: 3
            Layout.fillWidth: true

            text: qsTr("Other application settings")
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignRight

            text: qsTr("Login timeout")
            horizontalAlignment: Text.AlignRight
        }
        TextField {
            Layout.fillWidth: true

            id: loginTimeoutField
            text: String(settings.loginTimeout)
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator{ bottom: 1; top: 1000 }
            color: acceptableInput ? "black" : "red"
            selectByMouse: true
        }
        Button {
            text: qsTr("Save")
            onClicked: settings.loginTimeout = Number(loginTimeoutField.text)

            enabled: loginTimeoutField.acceptableInput && Number(loginTimeoutField.text) !== settings.loginTimeout
        }
        Text {}
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: qsTr("After given number of inactive time (in seconds) the currently logged in user is logged out.")
            wrapMode: Text.WordWrap
            font.italic: true
        }


        Item {
            Layout.columnSpan: 3
            Layout.fillHeight: true
        }
    }
}
