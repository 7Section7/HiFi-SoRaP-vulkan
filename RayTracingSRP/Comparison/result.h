#ifndef RESULT_H
#define RESULT_H

#include "DataVisualization/grid.h"
#include <QString>

/*
 * This class stores the output of a method for each azimuth and elevation.
 */

class Result
{

    Grid *output;
    QString name;

public:
    Result();

    QString getName() const;
    void setName(const QString &value);
    Grid *getOutput() const;
    void setOutput(Grid *value);
};

#endif // RESULT_H
