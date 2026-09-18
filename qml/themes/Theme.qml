import QtQuick

QtObject {
    property bool dark: settingsManager.theme === "midnight"
    readonly property color window: dark ? "#111916" : "#F7FAF7"
    readonly property color sidebar: dark ? "#17221D" : "#EDF6EF"
    readonly property color card: dark ? "#1B2923" : "#FFFFFF"
    readonly property color cardHover: dark ? "#23352D" : "#F6FBF7"
    readonly property color input: dark ? "#14201B" : "#F4F7F4"
    readonly property color text: dark ? "#ECF5EF" : "#20372A"
    readonly property color muted: dark ? "#B7C8BD" : "#708078"
    readonly property color primary: dark ? "#73C995" : "#3D8B5D"
    readonly property color primaryHover: dark ? "#82D7A3" : "#347A51"
    readonly property color primarySoft: dark ? "#254633" : "#DDF3E5"
    readonly property color border: dark ? "#3B5145" : "#DDE8E0"
    readonly property color danger: dark ? "#E59A9A" : "#A95656"
    readonly property color warning: dark ? "#E2BD75" : "#98702D"
    readonly property color shadow: dark ? "#55000000" : "#160D2416"
    readonly property int radius: 14
    readonly property int animationDuration: settingsManager.reduceAnimations ? 0 : 150
}
