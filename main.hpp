#ifndef MAIN_HPP
#define MAIN_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cmath>

std::vector<std::string> ReadFileContentsIntoVector(const std::string &);

std::vector<std::pair<std::string,std::string>> ExtractDataPairs(const std::string &);

bool ContainsBinaryDigitsOnly(const std::string &);

void DoubleCheck(const std::vector<std::pair<std::string,std::string>> &, const std::vector<std::string> &, const size_t);

size_t FindLastFrame(const size_t, std::pair<int, int>, const std::vector<std::pair<std::string,std::string>> &, const std::vector<std::string> &, const size_t);

#endif // MAIN_HPP