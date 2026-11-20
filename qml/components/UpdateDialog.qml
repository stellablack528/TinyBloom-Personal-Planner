import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(560, parent ? parent.width - 24 : 560)
    height: Math.min(620, parent ? parent.height - 32 : 620)
    modal: true
    title: qsTr("A new TinyBloom version is available")
    closePolicy: Popup.CloseOnEscape
    padding: 0
    background: Rectangle { radius: 18; color: theme.card; border.color: theme.border }
    header: Rectangle {
        implicitHeight: 72; color: "transparent"
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 22; anchors.rightMargin: 14
            Text {
                Layout.fillWidth: true; text: dialog.title
                color: theme.text; font.pixelSize: 18; font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }
            ToolButton {
                id: closeButton
                text: "×"; Accessible.name: qsTr("Close")
                contentItem: Text { text: closeButton.text; color: theme.muted; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter }
                onClicked: dialog.close()
            }
        }
    }
    contentItem: ColumnLayout {
        spacing: 12
        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 22; Layout.rightMargin: 22
            implicitHeight: versionRow.implicitHeight + 24; radius: 12; color: theme.primarySoft
            RowLayout {
                id: versionRow; anchors.fill: parent; anchors.margins: 12
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 2
                    Text {
                        text: updateManager.releaseName.length > 0
                            ? updateManager.releaseName : qsTr("TinyBloom %1").arg(updateManager.latestVersion)
                        color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold
                    }
                    Text {
                        text: qsTr("Current %1  →  New %2")
                            .arg(updateManager.currentVersion).arg(updateManager.latestVersion)
                        color: theme.muted; font.pixelSize: 12
                    }
                }
                Text { text: "🌱"; font.pixelSize: 24 }
            }
        }
        Text {
            Layout.leftMargin: 22; Layout.rightMargin: 22
            text: qsTr("Update announcement")
            color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold
        }
        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.leftMargin: 22; Layout.rightMargin: 22
            clip: true
            TextArea {
                text: updateManager.releaseNotes.length > 0
                    ? updateManager.releaseNotes : qsTr("This update contains new features and reliability improvements.")
                color: theme.text; font.pixelSize: 13; wrapMode: Text.WordWrap
                textFormat: Text.MarkdownText; readOnly: true; selectByMouse: true
                background: Rectangle { color: theme.input; radius: 10; border.color: theme.border }
                padding: 14
                onLinkActivated: link => Qt.openUrlExternally(link)
            }
        }
    }
    footer: Rectangle {
        implicitHeight: 76; color: "transparent"
        RowLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 8
            Button {
                id: ignoreButton
                text: qsTr("Ignore this version"); flat: true
                contentItem: Text { text: ignoreButton.text; color: theme.muted; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                onClicked: { updateManager.skipLatestVersion(); dialog.close() }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; primary: false; text: qsTr("Later"); onClicked: dialog.close() }
            AppButton {
                theme: dialog.theme; text: mobilePlatform ? qsTr("Download APK") : qsTr("Download update")
                onClicked: { updateManager.downloadUpdate(); dialog.close() }
            }
        }
    }
}
