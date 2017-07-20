#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>

#include "nicmodel.h"
#include "wmi_subsystem.h"
#include "nicbackend.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);
  WMI_SubSystem wmi(&app);
  if(!wmi.Connect())
  {
    qFatal ("Failed to initialize WMI subsystem");
    return -1;
  }

  //qmlRegisterType<Killswitch>("Killswitch", 1,0, "Killswitch");
  qmlRegisterType<NicBackend>("Killswitch", 1,0, "NicSwitch");
  auto nics = wmi.GetNetworAdapters();
  QList<QObject*> nicsList;
  for(auto &nic: nics)
    nicsList.append(new NicBackend(&nic, &app));


  QQmlApplicationEngine engine;

  QQmlContext *childContext = new QQmlContext(&engine, &engine);
  childContext->setContextProperty("nicsListModel", QVariant::fromValue(nicsList));

  QQmlComponent *component = new QQmlComponent(&engine, &engine);
  component->loadUrl(QUrl("qrc:///main.qml"));

  // Create component in child context
  QObject *o = component->create(childContext);
  QQuickWindow* window = qobject_cast<QQuickWindow*>(o);
  window->show();

//  engine.load(QUrl(QLatin1String("qrc:/main.qml")));
//  if (engine.rootObjects().isEmpty())
//    return -1;

//  QQmlContext *ctxt = engine.rootContext();;
//  ctxt->setContextProperty("nicsListModel", QVariant::fromValue(nicsList));

  return app.exec();
}
