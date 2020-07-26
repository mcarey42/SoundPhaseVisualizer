#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//
// Author: Mark Carey (mcarey AT solocre dot net)
//


#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loadButton_clicked();

    void on_startStopButton_clicked();

private:
    Ui::MainWindow *ui;

    bool fileChosen = false;
    QString fileName;
    uint64_t frames;

    QVector<double> *x = nullptr;
    QVector<double> *y = nullptr;

    bool playingAnimation = false;

    // Worker member function.
    void worker_thread(void);
};
#endif // MAINWINDOW_H
