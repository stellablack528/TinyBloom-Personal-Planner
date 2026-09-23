pragma Translator: "Sidebar"

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: bar
    property QtObject theme
    property int currentIndex: 0
    signal pageRequested(int index)

    color: theme.card
    border.color: theme.border
    implicitHeight: 70

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        spacing: 0

        Repeater {
            model: [
                { label: qsTranslate("Sidebar", "Today"), icon: "●" },
                { label: qsTranslate("Sidebar", "Tasks"), icon: "☰" },
                { label: qsTranslate("Sidebar", "Big Tasks"), icon: "⌘" },
                { label: qsTranslate("Sidebar", "Study Room"), icon: "✿" },
                { label: qsTranslate("Sidebar", "Settings"), icon: "⚙" }
            ]

            delegate: Item {
                id: navigationItem
                required property var modelData
                required property int index
                Layout.fillWidth: true
                Layout.fillHeight: true
                Accessible.role: Accessible.Button
                Accessible.name: modelData.label
                Accessible.focusable: true

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 3
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: navigationItem.modelData.icon
                        color: bar.currentIndex === navigationItem.index ? theme.primary : theme.muted
                        font.pixelSize: 18
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: navigationItem.modelData.label
                        color: bar.currentIndex === navigationItem.index ? theme.primary : theme.muted
                        font.pixelSize: 10
                        font.weight: bar.currentIndex === navigationItem.index ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: 24
                    height: 3
                    radius: 2
                    color: theme.primary
                    visible: bar.currentIndex === navigationItem.index
                }

                TapHandler {
                    onTapped: bar.pageRequested(navigationItem.index)
                }
            }
        }
    }
}
