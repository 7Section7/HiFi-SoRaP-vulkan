#ifndef COMPARISONWINDOW_H
#define COMPARISONWINDOW_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "categoryresult.h"
#include "DataVisualization/visualizationwindow.h"

#include <QWidget>
#include <QtWidgets>

#include <QComboBox>
#include <QToolButton>
#include <unordered_map>

/*
 * This class creates the window that allows the user to compare two graphics.
 */

class ComparisonWindow: public QWidget
{
    CategoryResult *results;

    QComboBox *leftBox, *rightBox;

    QToolButton *compareButton;
    QLabel *errorComparison, *errorComparisonExtraInfo;

    std::unordered_map<int,QVector3D> map;
    std::vector<VisualizationWindow*> visualizationWindows;

public:
    ComparisonWindow(QWidget *parent = 0);
    ~ComparisonWindow();

    void setResults(CategoryResult *value,int maxSRPModels);

private slots:
    void computeDifference();
};

#endif // COMPARISONWINDOW_H
