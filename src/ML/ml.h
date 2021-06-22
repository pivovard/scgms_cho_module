/*
 * @author = Bc. David Pivovar
 */

#pragma once

//#include <mlpack/core.hpp>
//#include <mlpack/methods/naive_bayes/naive_bayes_classifier.hpp>

#include "sklearn/naive_bayes.h"
#include "sklearn/logistic_regression.h"

//#include <Eigen/Core>
//#include <ldaplusplus/LDA.hpp>
//#include <ldaplusplus/LDABuilder.hpp>

/**
* Wrapper for machine learning - test purposes
*/
class ml
{
public:
	/*Creates classifier and train it.*/
	ml(char type, int n_signals, std::string path);

	/*Load csv*/
	static std::pair<std::vector<std::vector<double>>, std::vector<unsigned long>> read_csv(std::string path);
	/* Load csv file to arma matrix.
	 * Returns transposed matrix of data and row of labels.
	 */
	//static std::pair<arma::mat, arma::u64_rowvec> read_csv_to_arma(std::string path, int n_cols);
	

	/*Classify data*/
	int  classify(std::vector<double> vec);

private:
	char type;

	std::unique_ptr <logistic_regression> lg;
	std::unique_ptr <gaussian_naive_bayes> nb;
	//ldaplusplus::LDA<double>* lda;
	//std::unique_ptr<mlpack::naive_bayes::NaiveBayesClassifier<>> m_nb;
};

