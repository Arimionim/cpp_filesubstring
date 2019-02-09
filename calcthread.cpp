#include "calcthread.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>

void calcThread::listfilesindir(QString path, QList<QString> *files)
{
    QDir dir(path);

    bool ok = dir.exists();
    if (ok)
    {
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
                files->append(fileInfo.filePath());
            }
        }
    }
}

template<typename T>
uint64_t readBlock(std::istream &in, T *buf, uint64_t const count) {
    int64_t len = 0;
    auto const size = static_cast<const size_t>(count * sizeof(T));

    in.read(reinterpret_cast<char *>(buf), static_cast<std::streamsize>(size));
    len = static_cast<int64_t >(in.gcount());

    if (len < 0) {
        throw std::runtime_error("cannot read block");
    }
    return static_cast<uint64_t>(len);
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

void calcThread::check(const QList<QString> &files){
    for (int i = 0; i < files.size(); ++i){
        int cnt = 0;
        emit setProgress((i * 100) / numFiles);
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finished(i);
            return;
        }
        std::string fileName = files[i].toStdString();
        try {
            std::ifstream input_file(fileName, std::ios::binary);
            if (input_file.fail()){
                throw std::runtime_error("cannot open " + fileName);
            }

            char fromFile[BUFF_SIZE - TEXT_MAX_SIZE];


            uint64_t len = readBlock(input_file, fromFile, BUFF_SIZE - TEXT_MAX_SIZE);
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
                len = readBlock(input_file, fromFile, BUFF_SIZE - TEXT_MAX_SIZE);
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
    listfilesindir(path, &files);
    check(files);
    for (QString i: files){
        std::cout << i.toStdString() << std::endl;
    }
    emit finished(numFiles);
}

