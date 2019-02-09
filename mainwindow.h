#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeWidgetItem>


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

private slots:
    void on_dir_doubleClicked(const QModelIndex &);
    void on_goButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void getFile(QString path, int, bool error);

    void setProgress(int);
    void scanFinished(int);
};

#endif // MAINWINDOW_H
