#include "calcthread.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <QTextCodec>
#include <QFileSystemWatcher>

void calcThread::listfilesindir(QString path, QList<QString> *files)
{
    QDir dir(path);

    bool ok = dir.exists();
    if (ok)
    {
        watcher.addPath(path);
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);
        dir.setSorting(QDir::Time | QDir::Reversed);
        QFileInfoList list = dir.entryInfoList();
        int l, r;
        l = 0;
        r = list.size();

        for (int i = l; i < r; ++i){
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit finished(0);
                return;
            }
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.isDir()){
                listfilesindir(fileInfo.filePath(), files);
            }
            else{
                numFiles++;
                watcher.addPath(fileInfo.filePath());
                files->append(fileInfo.filePath());
                if (trigrams.find(fileInfo.filePath().toStdString()) == trigrams.end()){
                    QFile file(fileInfo.filePath());

                    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                        return;
                    }

                    setTrigrams(file);
                }
            }
        }
    }
}

template<typename T>
uint64_t readBlock(QFile &file, T *buf, uint64_t const count) {
     return file.read(buf, count);
}

void calcThread::setTrigrams(QFile & file){

    std::string path = file.fileName().toStdString();
    char data[BUFF_SIZE];
    int len;
    trigrams[path] = std::unordered_set<uint32_t>();
    std::unordered_set<uint32_t>& tmp = trigrams[path];

    do{
        len = readBlock(file, data, BUFF_SIZE);
        if (!isTextData(QByteArray(data, len))){
            return;
        }
        addTrigram(tmp, data, len);
    } while (len > 0);
}

void z_func(QString string, std::vector<int> & res){
    int left = 0, right = 0, n = string.size();
    for (int i = 1; i < n; i++){

        size_t idx = static_cast<size_t>(i);
        res[idx] = std::max(0, std::min(right - i, res[idx - static_cast<size_t>(left)]));
        while (i + res[idx] < n && string[res[idx]] == string[i + res[idx]]){
            res[idx]++;
        }
        if (i + res[idx] > right){
            left = i;
            right = i + res[idx];
        }
    }
}

void calcThread::addTrigram(std::unordered_set<uint32_t>& result, const char *data, int size){
    if (size < 2){
        return;
    }
    uint32_t value = 256u * uint8_t(data[0]) + uint8_t(data[1]);
    for (int i = 2; i < size; i++) {
        value <<= 8;
        value += uint8_t(data[i]);
        value &= (1u << 24) - 1;
        result.insert(value);
    }
}

bool calcThread::isTextData(const QByteArray& data) {
    if (data.lastIndexOf('\0') >= 0)
        return false;
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    codec->toUnicode(data.constData(), data.size(), &state);
    return state.invalidChars <= 2;
}



void calcThread::check(const QList<QString> &files){
    std::unordered_set<uint32_t> tempTrigrams;
    addTrigram(tempTrigrams, temp.toStdString().data(), temp.size());
    for (int i = 0; i < files.size(); ++i){
        int cnt = 0;
        emit setProgress((i * 100) / numFiles);
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finished(i);
            return;
        }
        std::string fileName = files[i].toStdString();
        QFile file(files[i]);


        try {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                throw std::runtime_error("cannot open " + fileName);
            }

            char fromFile[BUFF_SIZE - TEXT_MAX_SIZE];

            std::unordered_set<uint32_t>& fileTrigrams = trigrams[fileName];
            for (uint32_t trigram: tempTrigrams){
                if (fileTrigrams.find(trigram) == fileTrigrams.end()){
                    break;
                }
            }


            uint64_t len = readBlock(file, fromFile, BUFF_SIZE - TEXT_MAX_SIZE);
            QString last;
            while (len > 0){
                QString string = temp + char(0) + QString(fromFile) + last;
                std::vector<int> z(string.size());
                z_func(string, z);
                int tsize = temp.size(), ssize = string.size();
                for (int i = tsize + last.size(); i < ssize; i++){
                    if (z[i] == tsize){
                        cnt++;
                    }
                }
                len = readBlock(file, fromFile, BUFF_SIZE - TEXT_MAX_SIZE);
                if (!isTextData(QByteArray(fromFile, len))){
                    cnt = 0;
                    break;
                }
                last = QString(fromFile).remove(0, len - TEXT_MAX_SIZE);
            }

            if (cnt > 0){
                emit sendFile(QString().fromStdString(fileName), cnt, false);
            }
        }
        catch (std::exception const &e){
            emit sendFile(QString().fromStdString(fileName), 0, true);
        }
    }
}

void calcThread::run()
{
    QList<QString> files;
    numFiles = 0;
    if (dirChanged){
        watcher.removePaths(watcher.directories());
        watcher.removePaths(watcher.files());
        listfilesindir(path, &files);
    }
    check(files);
    emit finished(numFiles);
}

