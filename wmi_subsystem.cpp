#include "wmi_subsystem.h"
#include <QMutex>
#include <QMutexLocker>

#define _WIN32_DCOM
#include <windows.h>
#include <wbemidl.h>
//#include <comdef.h>
#include <sstream>

bool InitCOM() {
  HRESULT hr;
  hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
  if (FAILED(hr))
  {
    qFatal ("Failed to initialize COM library. Error code = 0x%lx",  hr );
    return false;
  }
  qInfo ("COM Inited (ret code 0x%lx)",  hr );

  hr = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
      );
  if (FAILED(hr))
  {
    qFatal ("Failed to setup security. Error code = 0x%lx",  hr );
    return false;
  }
  return true;
}

bool DenitCOM() {
  CoUninitialize();
  qInfo ("COM Deinited");
  return true;
}

int COMInit::sInitCounter = 0;
void COMInit::TryInitDeinit(bool init)
{
    static QMutex mutex;
    if(init){
      if (sInitCounter == 0) {
          QMutexLocker lock(&mutex);
          if (sInitCounter == 0) {
              if(InitCOM())
              {
                sInitCounter++;
              }
          }
      }
   } else {
      if (sInitCounter > 0) {
          QMutexLocker lock(&mutex);
          if (sInitCounter > 0) {
              if(DenitCOM())
              {
                sInitCounter--;
              }
          }
      }

   }
}
COMInit::COMInit() {TryInitDeinit(true);}
COMInit::~COMInit() {TryInitDeinit(false);}

class WMI_SubSystem::WMI_SubSystem_Impl{
  WMI_SubSystem* m_pParent;
  IWbemLocator *m_pWbemLocator;
  IWbemServices *m_pWbemServer;
  bool m_connected;
public :
  WMI_SubSystem_Impl(WMI_SubSystem* pParent)
    : m_pParent(pParent)
    , m_pWbemLocator(nullptr)
    , m_pWbemServer(nullptr)
    , m_connected(false)
  {

  }
  ~WMI_SubSystem_Impl()
  {
    if(m_connected) {
      Disconnect();
    }
  }

  bool Connect()
  {
      HRESULT hr;

      hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &m_pWbemLocator);

      if (FAILED(hr))
      {
        qCritical("Failed to create IWbemLocator object. Err code = 0x%lx",  hr );
        return false;
      }

      hr = m_pWbemLocator->ConnectServer(
            BSTR(L"\\\\.\\root\\CIMV2"),  // Namespace
            NULL,                         // User name
            NULL,                         // User password
            NULL,                         // Locale
            0,                            // Security flags
            NULL,                         // Authority
            NULL,                         // Context object
            &m_pWbemServer);   // IWbemServices proxy


      if (FAILED(hr))
      {
        qCritical("Could not connect. Error code = 0x%lx" ,hr);
        m_pWbemLocator->Release();
        return false;
      }

      qInfo("Connected to WMI");

      // Set the proxy so that impersonation of the client occurs.
      hr = CoSetProxyBlanket(m_pWbemServer,
                               RPC_C_AUTHN_WINNT,
                               RPC_C_AUTHZ_NONE,
                               NULL,
                               RPC_C_AUTHN_LEVEL_CALL,
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               NULL,
                               EOAC_NONE
                               );

      if (FAILED(hr))
      {
        qCritical("Could not set proxy blanket. Error code = 0x%lx" ,hr);
        m_pWbemServer->Release();
        m_pWbemLocator->Release();
        return false;
      }

      m_connected = true;
      return true;
  }
  void Disconnect()
  {
    if(!m_connected) return;
    m_pWbemServer->Release();
    m_pWbemLocator->Release();
     qInfo("Disconnected from WMI");
  }


  QList<NicModel> GetNetworAdapters() {
    if(!m_connected) return QList<NicModel>();

    HRESULT hr;
    IEnumWbemClassObject* pEnumerator = NULL;

    hr = m_pWbemServer->ExecQuery(
          BSTR(L"WQL"),
          BSTR(L"SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus = 2"),
          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
          NULL,
          &pEnumerator);

    if (FAILED(hr))
    {
      qCritical("Query for lists of proper nics failed. Error code = 0x%lx" ,hr);
      return QList<NicModel>();
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    QList<NicModel> real_nics;

    while (pEnumerator)
    {
      hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

      if (0 == uReturn)
      {
        break;
      }

      VARIANT vtProp;

      hr = pclsObj->Get(BSTR(L"Name"), 0, &vtProp, 0, 0);
      QString name(QString::fromUtf16(reinterpret_cast<const ushort*>((const WCHAR*)vtProp.bstrVal)));
      VariantClear(&vtProp);
      hr = pclsObj->Get(BSTR(L"DeviceId"), 0, &vtProp, 0, 0);
      QString id(QString::fromUtf16(reinterpret_cast<const ushort*>((const WCHAR*)vtProp.bstrVal)));
      VariantClear(&vtProp);
      hr = pclsObj->Get(BSTR(L"NetEnabled"), 0, &vtProp, 0, 0);
      bool state(vtProp.boolVal  == VARIANT_TRUE);
      VariantClear(&vtProp);
      real_nics.append(NicModel(m_pParent, id.toInt(), state, name));

      pclsObj->Release();
    }

    return real_nics;
  }

  bool SetNetworkAdapterEnabled(int deviceId, bool value) {
    if(!m_connected) return false;

    HRESULT hr;

    BSTR MethodName = (value)?SysAllocString(L"Enable"):SysAllocString(L"Disable");
    BSTR ClassName = SysAllocString(L"Win32_NetworkAdapter");
    QString method_name(QString::fromUtf16(reinterpret_cast<const ushort*>((const WCHAR*)MethodName)));

    IWbemClassObject* pClass = NULL;
    hr = m_pWbemServer->GetObject(ClassName, 0, NULL, &pClass, NULL);

    IWbemClassObject* pInParamsDefinition = NULL;
    hr = pClass->GetMethod(MethodName, 0, &pInParamsDefinition, NULL);

    IWbemClassObject* pClassInstance = NULL;
    if(pInParamsDefinition) {
      hr = pInParamsDefinition->SpawnInstance(0, &pClassInstance);
    }

    IWbemClassObject* pOutParams = NULL;

    std::stringstream ss;
    ss << "Win32_NetworkAdapter.DeviceID="<<'"'<<deviceId<<'"';
    int wslen = ::MultiByteToWideChar(CP_ACP, 0, ss.str().data(), ss.str().length(), NULL, 0);
    BSTR Query = ::SysAllocStringLen(NULL, wslen);
    ::MultiByteToWideChar(CP_ACP, 0, ss.str().data(), ss.str().length(), Query, wslen);

    hr = m_pWbemServer->ExecMethod(Query, MethodName, 0, NULL, pClassInstance, &pOutParams, NULL);
    if (FAILED(hr))
    {
        SysFreeString(ClassName);
        SysFreeString(MethodName);
        SysFreeString(Query);
        if (pClass) pClass->Release();
        if (pInParamsDefinition) pInParamsDefinition->Release();
        if (pClassInstance) pClassInstance->Release();
        if (pOutParams) pOutParams->Release();

        qWarning("Failed to exec method '%s' , of Win32_NetworkAdapter. Error code = 0x%lx", method_name.toStdString().c_str(), hr);

        return false;
    }

    int return_val = -1;
    VARIANT varReturnValue;
    hr = pOutParams->Get(L"ReturnValue", 0, &varReturnValue, NULL, 0);
    if (!FAILED(hr)){
      return_val = varReturnValue.intVal;
      if (return_val != 0 ){
        qWarning("Method '%s' fails (return values: %d) ", method_name.toStdString().c_str(), return_val);
      }
    }
    VariantClear(&varReturnValue);

    SysFreeString(ClassName);
    SysFreeString(MethodName);
    SysFreeString(Query);
    if (pClass) pClass->Release();
    if (pInParamsDefinition) pInParamsDefinition->Release();
    if (pClassInstance) pClassInstance->Release();
    if (pOutParams) pOutParams->Release();

    return return_val == 0;
  }
};

WMI_SubSystem::WMI_SubSystem(QObject *parent)
  : QObject(parent)
  , m_pImpl(new WMI_SubSystem_Impl(this))
{

}

WMI_SubSystem::~WMI_SubSystem()
{

}

bool WMI_SubSystem::Connect()
{
  return m_pImpl->Connect();
}

QList<NicModel> WMI_SubSystem::GetNetworAdapters()
{
  return m_pImpl->GetNetworAdapters();
}

bool WMI_SubSystem::TunrnOffDevice(DeviceType deviceType, DeviceId deviceId)
{
  if(deviceType == DeviceType::NetworkAdapter) {
    return m_pImpl->SetNetworkAdapterEnabled((int)deviceId, false);
  }
  return false;
}

bool WMI_SubSystem::TunrnOnDevice(DeviceType deviceType, DeviceId deviceId)
{
  if(deviceType == DeviceType::NetworkAdapter) {
     return m_pImpl->SetNetworkAdapterEnabled((int)deviceId, true);
  }
  return false;
}
