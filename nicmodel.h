#ifndef NIC_MODEL_H
#define NIC_MODEL_H

#include <QString>
#include "osiface.h"

class NicModel
{
public:
  NicModel(OperationSystemIf *os, DeviceId deviceId, bool state, const QString& name);
  NicModel(const NicModel& copy) = default;
  NicModel(NicModel&& tmp);

  NicModel& operator=(const NicModel& copy) = default;

 bool setState(bool newState, bool* pOldState=nullptr);
 bool getState() const {
   return m_state;
 }

 const QString& getName() const {
   return m_name;
 }
private:
  OperationSystemIf* m_os;
  DeviceType         m_type;
  DeviceId           m_deviceId;
  bool               m_state;
  QString            m_name;
};

#endif // NIC_MODEL_H
