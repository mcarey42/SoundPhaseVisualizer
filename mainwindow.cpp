#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include	<sndfile.hh>

#include <iostream>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create graph, set plot type, and set ranges.
    auto graph = ui->customPlot->addGraph();
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, Qt::white, 1));
    graph->setLineStyle(QCPGraph::LineStyle::lsNone);

    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x=L");
    ui->customPlot->yAxis->setLabel("y=R");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(-32768, 32768);
    ui->customPlot->yAxis->setRange(-32768, 32768);

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Load button function.
void MainWindow::on_loadButton_clicked()
{
    // Get the filename
    fileName = QFileDialog::getOpenFileName(this, tr("Open Sound File"), "~", tr("Sound Files (*.wav)"));

    // Return if no file chosen.
    if (fileName == "" || fileName == nullptr)
        return;

    // Set state to file chosen.
    fileChosen = true;


    SndfileHandle file;
    file = SndfileHandle( fileName.toStdString().c_str());

    // Tell the user we're loading and log what we know about this file.
    QString tmp;
    tmp = "Loading audio file...   Please wait...";
    ui->logListWidget->addItem(tmp);
    qApp->processEvents();

    tmp = "";
    QTextStream(&tmp) << "format: " << file.format() << "\n";
    QTextStream(&tmp) << "channels: " << file.channels() << "\n";
    QTextStream(&tmp) << "samplerate: " << file.samplerate() << "\n";
    QTextStream(&tmp) << "frames: " << file.frames() << "\n";
    ui->logListWidget->addItem(tmp);
    qApp->processEvents();

    this->frames = file.frames();

    // Raw frame allocation for the libsndfile.
    int16_t *rawframes = new int16_t[frames];

    // Free if we are already allocated
    if (this->x != nullptr)
    {
        delete this->x;
        this->x = nullptr;
    }
    if (this->y != nullptr)
    {
        delete this->y;
        this->y = nullptr;
    }

    // the vectors we will use to animate with.
    x = new QVector<double>(frames/2);
    y = new QVector<double>(frames/2);

    // read into the raw frames.
    file.read(rawframes, frames);

    // process the raw stream into the x, y pair sets.
    for (uint64_t i=0; i<frames/2; i++)
    {
      (*x)[i] = (double) rawframes[(i<<1)];
      (*y)[i] = (double) rawframes[(i<<1)+1];
    }

    // we won't need this again, so jetison it.
    delete[] rawframes;

    // Enable the play/stop button
    ui->startStopButton->setEnabled(true);
}

void MainWindow::on_startStopButton_clicked()
{
    if (!fileChosen)
        return;

    if (!this->playingAnimation)
    {
        playingAnimation = true;
        ui->startStopButton->setText(QString("Stop"));
        worker_thread();
    }
    else
    {
        playingAnimation = false;
        ui->startStopButton->setText(QString("Start"));
    }

}

void MainWindow::worker_thread(void)
{
    int usecDelay = atoi(ui->usecDelay->toPlainText().toStdString().c_str());
    int ticks = atoi(ui->totalFrames->toPlainText().toStdString().c_str());

    // Setup prograss bar.
    ui->progressBar->setRange(0, ticks);

    int one_step_frames = (frames/2)/ticks;
    for(int tick = 0; tick < ticks; tick++)
    {
        // Check if we need to stop playing.
        if (!playingAnimation)
        {
            ui->progressBar->setValue(0);
            break;
        }
        else
            ui->progressBar->setValue(tick);

        // Process pending events (so I don't have to write another thread class and a bunch of other crap).
        qApp->processEvents();


        //std::cout << "Rendering frame: " << tick << std::endl;
        QVector<double> x1 = (*x).mid(one_step_frames * tick, one_step_frames);
        QVector<double> y1 = (*y).mid(one_step_frames * tick, one_step_frames);
        ui->customPlot->clearItems();
        ui->customPlot->graph(0)->setData(x1, y1);
        ui->customPlot->replot();
        usleep(usecDelay);    //  usec delay.
    }

    // Done.
    ui->progressBar->setValue(ticks);
    playingAnimation = false;
    ui->startStopButton->setText(QString("Start"));

}
