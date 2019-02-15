#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include "mainwindow.h"

#include <QThread>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QObject>

#include <unordered_set>

class calcThread : public QObject
{
    Q_OBJECT
public:
    calcThread(QString const & path, QString const & temp,
               std::unordered_map<std::string, std::unordered_set<uint32_t>>& trigrams, QFileSystemWatcher &watcher)
        :path(path), temp(temp), trigrams(trigrams), watcher(watcher), dirChanged(true){}

    QFileSystemWatcher &watcher;
    QString path;
    QString temp;
    bool dirChanged;
    std::unordered_map<std::string, std::unordered_set<uint32_t>>& trigrams;

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
    bool isTextData(const QByteArray&);
    void listfilesindir(QString path, QList<QString>*);
    void check(const QList<QString>&);
    void addTrigram(std::unordered_set<uint32_t>&, const char*, int);
    void setTrigrams(QFile &);

};
#endif // CALCTHREAD_H
