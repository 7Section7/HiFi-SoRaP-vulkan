#include "visualizationwindow.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

VisualizationWindow::VisualizationWindow(QWidget *parent):QWidget(parent)
{
    for(int i=0; i< MAX_SUFACES; i++){
        surfaces[i].container = QWidget::createWindowContainer(&surfaces[i].surface);

        surfaces[i].container->setMinimumSize(QSize(1280/ 2, 500 / 1.6));
        surfaces[i].container->setMaximumSize(QSize(1280 / 2, 500 / 1.6));
        surfaces[i].container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        surfaces[i].container->setFocusPolicy(Qt::StrongFocus);
        surfaces[i].vLayout = new QVBoxLayout();

        QLabel *label = new QLabel();
        if(i == MeanF)
            label->setText(QStringLiteral("|F|"));
        else if(i == Fx)
            label->setText(QStringLiteral("Fx"));
        else if(i == Fy)
            label->setText(QStringLiteral("Fy"));
        else if(i == Fz)
            label->setText(QStringLiteral("Fz"));

        label->setAlignment(Qt::AlignCenter);

        surfaces[i].vLayout->addWidget(label,Qt::AlignCenter);
        surfaces[i].vLayout->addWidget(surfaces[i].container);
    }

    gLayout = new QGridLayout();

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setAlignment(Qt::AlignRight);

    QSpacerItem *spacerItem = new QSpacerItem(40,40,QSizePolicy::Fixed,QSizePolicy::Fixed);
    spacerItem->setGeometry(QRect(10,10,10,10));
    hLayout->addItem(spacerItem);

    QLabel *info = new QLabel();
    info->setGeometry(QRect(100, 0, 0, 0));
    info->setText(QString("Click on the right button to download the data into a txt file:"));
    info->setMaximumWidth(420);
    hLayout->addWidget(info);

    QToolButton *downloadButton;
    downloadButton = new QToolButton();
    downloadButton->setIcon(QIcon("://resources/images/download.jpg"));
    downloadButton->setIconSize(QSize(80,80));
    downloadButton->setMinimumWidth(80);
    downloadButton->setMinimumHeight(100);
    downloadButton->setMaximumWidth(80);
    downloadButton->setMaximumHeight(100);
    hLayout->addWidget(downloadButton);

    QObject::connect(downloadButton, &QToolButton::pressed, [this]() {
                this->saveResults();
            });

    gLayout->addLayout(hLayout, 0,2);

    gLayout->addLayout(surfaces[0].vLayout, 1,1);
    gLayout->addLayout(surfaces[1].vLayout, 1,2);
    gLayout->addLayout(surfaces[2].vLayout, 2,1);
    gLayout->addLayout(surfaces[3].vLayout, 2,2);

    this->setLayout(gLayout);
}

VisualizationWindow::~VisualizationWindow()
{
    gLayout->removeWidget(surfaces[0].container);
    gLayout->removeWidget(surfaces[1].container);
    gLayout->removeWidget(surfaces[2].container);
    gLayout->removeWidget(surfaces[3].container);
    surfaces[0].container->close();
    surfaces[1].container->close();
    surfaces[2].container->close();
    surfaces[3].container->close();
}

void VisualizationWindow::plotResults(Grid *results,  QString name)
{
    this->results = results;

    this->setWindowTitle(name);
    plotSurface(results,MeanF);
    plotSurface(results,Fx);
    plotSurface(results,Fy);
    plotSurface(results,Fz);

    this->show();
}

void VisualizationWindow::saveResults()
{
    QString path = QCoreApplication::applicationDirPath() + QString("/resources/output/");
    QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save Results"), path,
                tr("All Files (*)"));

    if (fileName.isNull()) return;
    std::string output = fileName.toStdString();
    results->saveData(AZstep,ELstep,output);
}

void VisualizationWindow::setAZstep(int value)
{
    AZstep = value;
}

void VisualizationWindow::setELstep(int value)
{
    ELstep = value;
}

void VisualizationWindow::plotSurface(Grid *results, VisualizationMode mode)
{
    results->updateExtremeValues();

    surfaces[mode].surface.axisX()->setTitle(QStringLiteral("Elevation"));
    surfaces[mode].surface.axisY()->setTitle(QStringLiteral("Force"));
    surfaces[mode].surface.axisZ()->setTitle(QStringLiteral("Azimuth"));

    surfaces[mode].surface.axisX()->setLabelFormat("%.1f º");
    surfaces[mode].surface.axisY()->setLabelFormat("%.2e");
    surfaces[mode].surface.axisZ()->setLabelFormat("%.1f º");

    Q3DTheme *theme = new Q3DTheme(Q3DTheme::Theme(0));
    QFont font;
    font.setBold(true);
    font.setWeight(QFont::Black);
    font.setPointSize(25);

    theme->setFont(font);
    surfaces[mode].surface.setActiveTheme(theme);

    QVector2D sizes = results->getSizes();

    QSurfaceDataArray *data = new QSurfaceDataArray;

    for(int i=0; i<sizes.x(); i++){
        QSurfaceDataRow *dataRow = new QSurfaceDataRow();

        for(int j=0; j<sizes.y(); j++){
            if(mode == MeanF)
                *dataRow << (*results)(i,j).getOutputWithMeanForces();
            else if(mode == Fx)
                *dataRow << (*results)(i,j).getOutputWithForceX();
            else if(mode == Fy)
                *dataRow << (*results)(i,j).getOutputWithForceY();
            else if(mode == Fz)
                *dataRow << (*results)(i,j).getOutputWithForceZ();
        }
        *data << dataRow;
    }

    QSurface3DSeries *series = new QSurface3DSeries;
    series->dataProxy()->resetArray(data);

    surfaces[mode].surface.addSeries(series);

    surfaces[mode].surface.show();


    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    surfaces[mode].surface.seriesList().at(0)->setBaseGradient(gr);
    surfaces[mode].surface.seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    if(mode == MeanF){
        surfaces[mode].surface.setTitle(QStringLiteral("Mean F"));
    }
    else if(mode == Fx){
        surfaces[mode].surface.setTitle(QStringLiteral("Fx"));
    }
    else if(mode == Fy){
        surfaces[mode].surface.setTitle(QStringLiteral("Fy"));
    }
    else if(mode == Fz){
        surfaces[mode].surface.setTitle(QStringLiteral("Fz"));
    }
}
