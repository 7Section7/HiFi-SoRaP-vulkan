#include "result.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

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
