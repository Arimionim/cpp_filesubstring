#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include "mainwindow.h"

#include <QThread>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QObject>

class calcThread : public QObject
 {
    Q_OBJECT
public:
    QString path;
    QString temp;

public slots:
    void run();

signals:
    void finished(int );
    void setProgress(int v);
    void sendFile(QString path, int, bool error);

private:
    const int BUFF_SIZE = 8192;
    const int TEXT_MAX_SIZE = 192;
    int numFiles;
    void listfilesindir(QString path, QList<QString>*);
    void check(const QList<QString>&);

};
#endif // CALCTHREAD_H
