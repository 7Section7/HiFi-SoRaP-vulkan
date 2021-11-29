#include "result.h"


Result::Result()
{
}

Grid *Result::getOutput() const
{
    return output;
}

void Result::setOutput(Grid *value)
{
    output = value;
}
QString Result::getName() const
{
    return name;
}

void Result::setName(const QString &value)
{
    name = value;
}
