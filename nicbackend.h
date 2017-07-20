#ifndef NICBACKEND_H
#define NICBACKEND_H

#include <QObject>
#include <QString>

class NicModel;


class NicBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name)

public:
    NicBackend(QObject *parent = 0);
    NicBackend(NicModel* model, QObject *parent = 0);

    bool state() const;
    void setState(bool newState);

    QString name() const;

signals:
    void stateChanged(bool newState);

private:
    NicModel*          p_pModel;

    //cache
    QString      m_name;
};

#endif // NICBACKEND_H
