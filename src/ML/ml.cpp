/*
 * @author = Bc. David Pivovar
 */

#undef Classify
#include "ml.h"

ml::ml(char type, int n_signals, std::string path)
{
    this->type = type;

    switch (type) {
    case 'l':
    {
        std::pair<std::vector<std::vector<double>>, std::vector<unsigned long>> data = ml::read_csv(path);
        lg = std::make_unique <logistic_regression>(data.first, data.second, NODEBUG);
        lg->fit();
        lg->save_model("model.json");
        break;
    }
    case 'b':
    {
        std::pair<std::vector<std::vector<double>>, std::vector<unsigned long>> data = ml::read_csv(path);
        nb = std::make_unique <gaussian_naive_bayes>(data.first, data.second, NODEBUG);
        nb->fit();
        nb->save_model("model.json");
        break;
    }
    case 'm':
    {
       /* std::pair<arma::mat, arma::u64_rowvec> data = ml::read_csv_to_arma(path, n_signals * 4);
        m_nb = std::make_unique<mlpack::naive_bayes::NaiveBayesClassifier<>>(data.first, data.second, 2);
        break;*/
    }
    }
    
}

std::pair<std::vector<std::vector<double>>, std::vector<unsigned long>> ml::read_csv(std::string path) {
    std::string line;
    std::vector<std::vector<double>> mat;
    std::vector<unsigned long> label;

    std::ifstream f(path);

    try {
        if (!f.is_open()) {
            throw (path);
        };
    }
    catch (char* path) {
        throw std::invalid_argument(("error while opening file " + std::string(path)).c_str());
    }

    while (getline(f, line)) {
        std::string val;
        std::vector<double> row;
        std::stringstream s(line);

        getline(s, val, ','); //first column is label
        label.push_back(stoul(val));

        while (getline(s, val, ','))
            row.push_back(stod(val));
        mat.push_back(row);
    }
    f.close();

    return std::pair<std::vector<std::vector<double>>, std::vector<unsigned long>>(mat, label);
}

/*
std::pair<arma::mat, arma::u64_rowvec> ml::read_csv_to_arma(std::string path, int n_cols)
{
    std::ifstream file(path);
    std::string line;

    auto n_rows = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
    file.clear();
    file.seekg(0);

    //arma::mat data(n_rows, n_cols);
    arma::mat data_csv;
    data_csv.load(path, arma::file_type::csv_ascii);
    arma::mat data(data_csv.tail_cols(n_cols));

    file.clear();
    file.seekg(0);

    std::vector<uint64_t> l;
    while (getline(file, line)) {
        std::string val;
        std::stringstream s(line);
        getline(s, val, ','); //first column is label
        l.push_back(stoull(val));
    }
    file.close();

    arma::u64_rowvec labels(l);

    return std::pair<arma::mat, arma::u64_rowvec>(data.t(), labels); //data must be transposed
}*/

int ml::classify(std::vector<double> vec)
{
    int res = 0;

    switch (type) {
    case 'l':
    {
        std::map<unsigned long, double> r = lg->predict(vec);
        if (r[1] > r[0]) res = 1;
        break;
    }
    case 'b':
    {
        std::map<unsigned long, double> r = nb->predict(vec);
        if (r[1] > r[0]) res = 1;
        break;
    }
    case 'm':
    {
        //arma::u64_rowvec r;
        ////m_nb->Classify(vec, r); //causes bug
        //res = r[0];
        //break;
    }
    }

    return 0;
}
