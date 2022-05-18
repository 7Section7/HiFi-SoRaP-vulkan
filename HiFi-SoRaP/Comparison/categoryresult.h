#ifndef CATEGORYRESULT_H
#define CATEGORYRESULT_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "result.h"
#include "unordered_map"

/*
 * This class stores, for one of the implemented methods, all the results that this method has produced.
 */

class CategoryResult
{
	std::vector<Result> results;
	std::unordered_map<size_t,int> repetitions;
public:
	CategoryResult();

	int addResult(QString name, Grid* output);

	std::vector<Result>& getResults();
	void setResults(const std::vector<Result> &value);
	int getRepetitions(QString name);
};

#endif // CATEGORYRESULT_H
