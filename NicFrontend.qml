import QtQuick 2.0
import QtQuick.Controls 2.0

Switch {
  x: 0
  y: 0
  font.wordSpacing: 2
  font.pointSize: 10
  spacing: 5

    BusyIndicator {
        id: busyIndicator
        x: 114
        y: 0
        width: 60
        height: 40
        visible: false
        running: false
    }
    onToggled: {
      busyIndicator.visible = true
      busyIndicator.running = true
    }
    onChildrenChanged: {
      busyIndicator.visible = false
    }
}
