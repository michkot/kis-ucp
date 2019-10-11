import QtQuick 2.12
import QtQuick.Controls 2.5
import QtWebView 1.1

Page {
    id: page

    title: qsTr("Login with your eduID identity provider")
    property string backText: qsTr("\u2039 back")
    property bool backEnabled: true
    property bool settingsEnabled: true

    WebView {
        id: web
        anchors.fill: parent
        url: controller.loginWayfUrl

        onLoadingChanged: function(request) {
            if(request.status === WebView.LoadFailedStatus) {
                controller.loginFailed()
            }
            else if(request.status === WebView.LoadSucceededStatus) {
                console.log(request.url)
                if(Qt.resolvedUrl(request.url) === Qt.resolvedUrl(controller.loginCaptureUrl)) {
                    controller.loginComplete()
                }
            }
        }
    }
}
