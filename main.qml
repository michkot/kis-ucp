import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Imagine 2.12
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12

ApplicationWindow {
    id: window
    visible: true
    visibility: Window.FullScreen

    title: qsTr("KIS UCP")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: backButton
                text: stackView.currentItem.backText
                enabled: stackView.currentItem.backEnabled
                font.bold: true
                onClicked: {
                    if (stackView.depth > 1) {
                        stackView.pop()
                        controller.backClicked(stackView.depth)
                    } else {
                        window.close()
                    }
                }

                contentItem: Label {
                    text: backButton.text
                    font: backButton.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Label {
                text: stackView.currentItem.title
                font.bold: true
                elide: Label.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            ToolButton {
                id: settingsButton
                enabled: stackView.currentItem.settingsEnabled

                text: qsTr("\u2699")
                font.bold: true
                onClicked: stackView.push("settings.qml")

                contentItem: Label {
                    text: settingsButton.text
                    font: settingsButton.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: "qrc:/home.qml"
        anchors.fill: parent

        Connections {
            target: controller
            onPushStackView: {
                stackView.push(url)
            }
            onReplaceStackView: {
                stackView.replace(url)
            }
            onReplaceWithUserProperty: {
                stackView.replace(url, {"user": user})
            }
            onResetAndPushStackView: {
                stackView.pop(null, StackView.Immediate)
                stackView.push(url)
            }
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            BusyIndicator {
                Layout.leftMargin: 5
                implicitHeight: 0.9 * timeoutLabel.height
                implicitWidth: 0.9 * timeoutLabel.height

                running: controller.busy

                Imagine.path: "qrc:/theme"
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                id: timeoutLabel
                text: controller.timeoutLabel
                font.bold: true
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }

            Label {
                Layout.minimumWidth: 120

                text: controller.timeoutValue
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            ToolSeparator {
                contentItem: Rectangle {
                    implicitWidth: 2
                    implicitHeight: timeoutLabel.height
                    color: timeoutLabel.color
                }
                visible: controller.timeoutLabel || controller.timeoutValue
            }

            Label {
                text: qsTr("Reader status:")
                font.bold: true
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }

            Label {
                Layout.rightMargin: 5
                Layout.minimumWidth: 120

                text: reader.state
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
