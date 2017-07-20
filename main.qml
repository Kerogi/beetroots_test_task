import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ApplicationWindow {
  visible: true
  width: 300
  height: 100
  title: qsTr("Killswitch")

  ListView {
    id: listView
    interactive: false
    orientation: ListView.Horizontal
    boundsBehavior: Flickable.StopAtBounds
    highlightRangeMode: ListView.NoHighlightRange
    anchors.fill: parent
    model: nicsListModel
    delegate: NicFrontend {
      text: model.modelData.name
      checked: model.modelData.state
      onClicked: {
          model.modelData.state = checked
      }
    }
  }
}
