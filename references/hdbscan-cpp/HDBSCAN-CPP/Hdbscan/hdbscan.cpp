#include "hdbscan.hpp"
#include<iostream>
#include<fstream>
#include<sstream>
#include<set>
#include<map>
#include<cstdint>
using namespace std;

string Hdbscan::getFileName() {
	return this->fileName;
}
/// <summary>
///	Loads the csv file as specified by the constructor.CSV
/// </summary>
/// <param name="numberOfalues">A List of attributes to be choosen</param>
/// <param name="skipHeader">Bool value to skip header or not</param>
/// <returns>1 if successful, 0 otherwise</returns>

int Hdbscan::loadCsv(int (*shape)[2],  bool skipHeader) {
	string attribute;
	string line = "";
	string size;

	int currentAttributes;
	vector<vector<double> > dataset;

	string fileName = this->getFileName();
	ifstream file(fileName, ios::in);
	if (!file)
		return 0;
	if (skipHeader) {
		getline(file, line);
	}
	getline(file, line);
	stringstream header_stream(line);
    // extract the header information (m, n, std) : (n_samples, n_features, std
    // of blobs)
    getline(header_stream, size, ',');
    (*shape)[0] = std::atoi(size.c_str());
    getline(header_stream, size, ',');
    (*shape)[1] = std::atoi(size.c_str());

	while (getline(file, line)) {      //Read through each line
		stringstream s(line);
		vector<double> row;
		currentAttributes = (*shape)[1];
		while (getline(s, attribute, ',') && currentAttributes != 0) {
			row.push_back(stod(attribute));
			currentAttributes--;
		}
		dataset.push_back(row);

	}
	this->dataset = dataset;
	return 1;
}

void Hdbscan::execute(int minPoints, int minClusterSize, string distanceMetric) {
	//Call The Runner Class here
	hdbscanRunner runner;
	hdbscanParameters parameters;
	uint32_t noisyPoints = 0;
	set<int> numClustersSet;
	map<int, int> clustersMap;
	vector<int> normalizedLabels;

	parameters.dataset = this->dataset;
	parameters.minPoints = minPoints;
	parameters.minClusterSize = minClusterSize;
	parameters.distanceFunction = distanceMetric;
    	this->result = runner.run(parameters);
	this->labels_ = result.labels;
	this->outlierScores_ = result.outliersScores;
	for (uint32_t i = 0; i < result.labels.size(); i++) {
		if (result.labels[i] == 0) {
			noisyPoints++;
		}
		else {
			numClustersSet.insert(result.labels[i]);
		}
	}
	this->numClusters_ = numClustersSet.size();
	this->noisyPoints_ = noisyPoints;
	int iNdex = 1;
	for (auto it = numClustersSet.begin(); it != numClustersSet.end(); it++) {
		clustersMap[*it] = iNdex++;
	}
	for (int i = 0; i < labels_.size(); i++) {
		if (labels_[i] != 0)
			normalizedLabels.push_back(clustersMap[labels_[i]]);
		else if (labels_[i] == 0) {
			normalizedLabels.push_back(-1);
		}

	}
	this->normalizedLabels_ = normalizedLabels;
	this->membershipProbabilities_ = result.membershipProbabilities;
}

void Hdbscan::displayResult() {
	hdbscanResult result = this->result;
	uint32_t numClusters = 0;

	cout << "HDBSCAN clustering for " << this->dataset.size() << " objects." << endl;

	for (uint32_t i = 0; i < result.labels.size(); i++) {
		cout << result.labels[i] << " ";
	}

	cout << endl << endl;

	cout << "The Clustering contains " << this->numClusters_ << " clusters with " << this->noisyPoints_ << " noise Points." << endl;

}
