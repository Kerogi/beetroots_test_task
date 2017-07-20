import QtQuick 2.0

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Killswitch 1.0

NicSwitch {
  Switch {
      id: nicswitch
      x: 0
      y: 0
      width: 200
      height: 50
      text: name
      checked: false;
  }
}

