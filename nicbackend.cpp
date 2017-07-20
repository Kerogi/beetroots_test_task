#include "nicbackend.h"
#include "nicmodel.h"

NicBackend::NicBackend(QObject *parent)
  : QObject(parent)
  , p_pModel(nullptr)
  , m_name("Unknown Network Card")
{
}

NicBackend::NicBackend(NicModel *model, QObject *parent)
  : QObject(parent)
  , p_pModel(model)
  , m_name(model->getName())
{
}

QString NicBackend::name() const
{
    return m_name;
}

bool NicBackend::state() const
{
    return p_pModel->getState();
}

void NicBackend::setState(bool newState)
{
    p_pModel->setState(newState);
    emit stateChanged(p_pModel->getState());
}

