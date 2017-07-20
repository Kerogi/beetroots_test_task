#ifndef WMI_SUBSYSTEM_H
#define WMI_SUBSYSTEM_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "nicmodel.h"
#include "osiface.h"

class COMInit
{
  static void TryInitDeinit(bool init);
  static int  sInitCounter;
public:
  COMInit();
  ~COMInit();
};

class WMI_SubSystem : public QObject, public OperationSystemIf
{
  Q_OBJECT
  COMInit m_auto_init_com;
  class WMI_SubSystem_Impl;
  QSharedPointer<WMI_SubSystem_Impl> m_pImpl;
public:
  explicit WMI_SubSystem(QObject *parent = nullptr);
  virtual ~WMI_SubSystem();

  bool Connect();
  QList<NicModel> GetNetworAdapters();

public:
    virtual bool TunrnOffDevice(DeviceType deviceType, DeviceId deviceId) override;
    virtual bool TunrnOnDevice(DeviceType deviceType, DeviceId deviceId) override;

signals:

public slots:

};

#endif // WMI_SUBSYSTEM_H
