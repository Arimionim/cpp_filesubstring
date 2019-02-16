#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calcthread.h"

#include <QModelIndex>
#include <vector>
#include <QDir>
#include <QList>
#include <iostream>
#include <QCryptographicHash>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath("");

    ui->dir->setModel(model);
    ui->progressBar->setValue(0);
    job = nullptr;


    ui->stopButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::dataChanged(){
    rootPath = "";
    trigrams.clear();
}


void MainWindow::on_goButton_clicked()
{
    QString path = ui->path->text();
    QListView *listView = ui->dir;

    if ((model->index(path)).isValid()){
        listView->setRootIndex(model->index(path));
    }
    else{
        ui->statusBar->showMessage("Incorrect path", 2000);
    }
}


void MainWindow::on_dir_doubleClicked(const QModelIndex &index){
    QListView *listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == ".."){
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
    }
    else if (fileInfo.fileName() == "."){
        listView->setRootIndex(model->index(""));
    }
    else if (fileInfo.isDir()){
        listView->setRootIndex(index);
    }

    ui->path->setText(model->filePath(listView->rootIndex()));
}

void MainWindow::on_startButton_clicked()
{
    QString temp = ui->temp->text();
    if (temp.size() == 0){
        ui->statusBar->showMessage("Enter template", 3000);
        return;
    }
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    calcThread *calc = new calcThread(model->filePath(ui->dir->rootIndex()), temp, trigrams, watcher, files);
    ui->result->clear();
    job = new QThread;

    if (model->filePath(ui->dir->rootIndex()) == rootPath){
        calc->dirChanged = false;
    }
    else{
        rootPath = model->filePath(ui->dir->rootIndex());
        calc->dirChanged = true;
    }
    connect(job, &QThread::started, calc, &calcThread::run);

    connect(calc, &calcThread::finished, this, &MainWindow::scanFinished);
    connect(calc, &calcThread::setProgress, this, &MainWindow::setProgress);
    connect(calc, &calcThread::sendFile, this, &MainWindow::getFile);
    connect(&calc->watcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::dataChanged);

    connect(job, &QThread::finished, calc, &calcThread::deleteLater);
    connect(job, &QThread::finished, job, &QThread::deleteLater);

    calc->moveToThread(job);
    ui->progressBar->setValue(0);
    ui->statusBar->showMessage("Searching...");
    job->start();

}

void MainWindow::scanFinished(int cnt){
    emit setProgress(100);
    ui->statusBar->showMessage("Finished. Checked " + QString::number(cnt) + " files", 5000);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->progressBar->setValue(100);
}

void MainWindow::getFile(QString path, int cnt, bool error = false){
    if (error){
        QTreeWidgetItem *item = new QTreeWidgetItem(
            ui->result,
            QStringList({"Error: " + path.remove(0, rootPath.size() + 1)})
        );

        item->setTextColor(0, Qt::red);
        //ui->result->expandItem(item);
        return;
    }

    QTreeWidgetItem *node = new QTreeWidgetItem(ui->result, QStringList{QString::number(cnt)
                                                                        + " time" + (cnt == 1 ? "" : "s") + ": "
                                                                        + path.remove(0, rootPath.size() + 1)});
    ui->result->expandItem(node);
}

void MainWindow::setProgress(int v){
    ui->progressBar->setValue(v);
}


void MainWindow::on_stopButton_clicked()
{
    dataChanged();
    job->requestInterruption();
}
