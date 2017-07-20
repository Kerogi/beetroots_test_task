#ifndef OSIFACE_H
#define OSIFACE_H

enum class DeviceType {
  Nil,
  NetworkAdapter,
  Otehr
};

using DeviceId = int;

class OperationSystemIf {
public:
    virtual ~OperationSystemIf() {}

    virtual bool TunrnOffDevice(DeviceType deviceType, DeviceId deviceId) = 0;
    virtual bool TunrnOnDevice(DeviceType deviceType, DeviceId deviceId) = 0;
};

#endif // OSIFACE_H
