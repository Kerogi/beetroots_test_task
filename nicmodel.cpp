#include "nicmodel.h"


NicModel::NicModel(OperationSystemIf *os, DeviceId deviceId, bool state, const QString& name)
  : m_os(os)
  , m_type(DeviceType::NetworkAdapter)
  , m_deviceId(deviceId)
  , m_state(state)
  , m_name(name)
{


}

NicModel::NicModel(NicModel&& tmp)
  : m_os(tmp.m_os)
  , m_type(DeviceType::NetworkAdapter)
  , m_deviceId(tmp.m_deviceId)
  , m_state(tmp.m_state)
  , m_name(tmp.m_name)
{
  tmp.m_os = 0;
  tmp.m_type = DeviceType::Nil;
  tmp.m_deviceId = -1;
  tmp.m_state = false;
  tmp.m_name = "";
}

bool NicModel::setState(bool newState, bool* pOldState)
{
  if(m_state != newState) {
    if(!newState) {
      if( m_os->TunrnOffDevice(m_type, m_deviceId)) {
        if(pOldState) *pOldState = m_state;
        m_state = newState;
        return true;
      }
    } else {
      if( m_os->TunrnOnDevice(m_type, m_deviceId)) {
        if(pOldState) *pOldState = m_state;
        m_state = newState;
        return true;
      }
    }
  }
  return false;
}
