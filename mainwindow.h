#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeWidgetItem>
#include <QFileSystemWatcher>

#include <unordered_map>
#include <unordered_set>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QString rootPath;
    QThread* job;
    QFileSystemModel *model;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> trigrams;

    QFileSystemWatcher watcher;


private slots:
    void on_dir_doubleClicked(const QModelIndex &);
    void on_goButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void getFile(QString path, int, bool error);
    void setProgress(int);
    void dataChanged();
    void scanFinished(int);

};

#endif // MAINWINDOW_H
