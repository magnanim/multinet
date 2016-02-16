/*
 * utils.h
 *
 * Contains:
 * - logging functions
 * - basic IO (csv file reading)
 * - random functions
 */

#ifndef MULTIPLENETWORK_UTILS_H_
#define MULTIPLENETWORK_UTILS_H_

#include "datastructures.h"
#include "exceptions.h"
#include <unordered_set>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>

namespace mlnet {

typedef int verbosity;

const int VERBOSITY=2;
const int V_OFF=0;
const int V_BASIC=1;
const int V_VERBOSE=2;
const int V_DEBUG=3;

void log(std::string s);
void log(std::string s, bool new_line);
void log(std::string s, verbosity v);
void log(std::string s, verbosity v, bool new_line);
void warn(std::string s);
void err(std::string s);

template <class T> void print_set(std::set<T>& input) {
	typename std::set<T>::iterator it;
	for (it=input.begin(); it!=input.end(); ++it) {
		if (it!=input.begin()) std::cout << ",";
		std::cout << (*it);
	}
	std::cout << std::endl;
}

/***********************************/
/** IO                            **/
/***********************************/
class CSVReader {
private:
	std::ifstream infile;
	std::string next;
	bool has_next;
	bool remove_trailing_spaces;
	int row_number;
	char field_separator;

public:
	CSVReader();
	virtual ~CSVReader();

	void open(std::string path);
	bool hasNext();
	std::vector<std::string> getNext();
	int rowNum();
	void trimFields(bool value);
	void setFieldSeparator(char separator);
};

/***********************************/
/** Pretty printing               **/
/***********************************/
void print(const mlnet::MLNetworkSharedPtr mnet);

template <typename T> std::string to_string ( T Number ) {
	std::ostringstream ss;
    ss << Number;
    return ss.str();
}

double to_double(const std::string &double_as_string);


/***********************************/
/** Math                          **/
/***********************************/

double mean(const std::vector<double>& vec);
double stdev(const std::vector<double>& vec);


template <class T>
std::unordered_set<T> intersect(std::unordered_set<T> set1, std::unordered_set<T> set2) {
	std::unordered_set<T> result;
	for (T l: set1) {
		if (set2.count(l)==1) {
			result.insert(l);
		}
	}
	return result;
}

template <class T>
std::unordered_set<T> set_union(std::unordered_set<T> set1, std::unordered_set<T> set2) {
	std::unordered_set<T> result;
	result.insert(set1.begin(), set1.end());
	result.insert(set2.begin(), set2.end());
	return result;
}

}

#endif /* MULTIPLENETWORK_UTILS_H_ */
