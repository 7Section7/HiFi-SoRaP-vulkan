#ifndef CATEGORYRESULT_H
#define CATEGORYRESULT_H

#include "result.h"
#include "unordered_map"

/*
 * This class stores for one of the implemented method, all the results that this method has produced.
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
