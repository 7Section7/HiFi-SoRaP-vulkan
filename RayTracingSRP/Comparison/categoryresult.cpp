#include "categoryresult.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

CategoryResult::CategoryResult()
{
}

int CategoryResult::addResult(QString name, Grid *output)
{
	Result r;
	r.setName(name);
	r.setOutput(output);

	results.push_back(r);

	int idx;
	std::hash<std::string> hash_fn;

	size_t hashValue =hash_fn(name.toStdString());
	if(repetitions.find(hashValue) != repetitions.end()){
		idx = repetitions[hashValue]+1;
		repetitions[hashValue]+=1;
	}else{
		idx=-1;
		repetitions[hashValue]=1;
	}

	return idx;

}

std::vector<Result>& CategoryResult::getResults()
{
	return results;
}

void CategoryResult::setResults(const std::vector<Result> &value)
{
	results = value;
}

int CategoryResult::getRepetitions(QString name)
{
	std::hash<std::string> hash_fn;
	size_t hashValue =hash_fn(name.toStdString());
	return repetitions[hashValue];
}
