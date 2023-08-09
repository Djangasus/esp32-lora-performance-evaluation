#include "main.hpp"

int main()
{
    std::string currentPath = "..\\..\\txtfiles\\"; // due to cmake on ./out/build/

    std::vector<std::string> filenames  = {
        "exp_PW05_SF07_BW125K_CI1_350ms.txt", 
        "exp_PW05_SF07_BW125K_CI4_350ms.txt", 
        "exp_PW05_SF07_BW500K_CI1_350ms.txt", 
        "exp_PW05_SF07_BW500K_CI4_350ms.txt", 
        "exp_PW05_SF12_BW125K_CI1_450ms.txt", 
        "exp_PW05_SF12_BW125K_CI4_480ms.txt", 
        "exp_PW05_SF12_BW500K_CI1_480ms.txt", 
        "exp_PW05_SF12_BW500K_CI4_480ms.txt",
        "exp_PW20_SF07_BW125K_CI1_350ms.txt",
        "exp_PW20_SF07_BW125K_CI4_350ms.txt",
        "exp_PW20_SF07_BW500K_CI1_350ms.txt",
        "exp_PW20_SF07_BW500K_CI4_350ms.txt",
        "exp_PW20_SF12_BW125K_CI1_450ms.txt",
        "exp_PW20_SF12_BW125K_CI4_480ms.txt",
        "exp_PW20_SF12_BW500K_CI1_480ms.txt",
        "exp_PW20_SF12_BW500K_CI4_480ms.txt"
    };
    std::cout << currentPath << filenames[0]<< std::endl;

    std::vector<std::string> originalLines = ReadFileContentsIntoVector(currentPath + "data_rds.txt");

    for (const auto& filename : filenames)
    {
        std::cout << filename << std::endl;
        std::vector<std::pair<std::string,std::string>> extractedData = ExtractDataPairs(currentPath + filename);

        DoubleCheck(extractedData, originalLines, 1000 * std::stoul(filename.substr(25, 3).c_str()));
    }

    return 0;
}

// Reads the contents of a file and store them in a vector
std::vector<std::string> ReadFileContentsIntoVector(const std::string& filename)
{
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Error opening file: " + filename);
    }
    else
    {
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();
    }
    return lines;
}

// Extracts SNTP time (us) and message received
std::vector<std::pair<std::string,std::string>> ExtractDataPairs(const std::string& filePath)
{
    std::ifstream inputFile(filePath.c_str(), std::ios_base::binary);

    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath);
    }

    std::vector<std::pair<std::string,std::string>> aaa;
    std::string line;
    while (std::getline(inputFile, line))
    {
        if (!line.empty() && line.length() > 80 && ContainsBinaryDigitsOnly(line.substr(line.rfind(",16:")+4, line.length() - line.rfind(",16:")+1 - 6)))
        {
            aaa.push_back(std::make_pair(line.substr(line.find("RX:") + 4, 16), line.substr(line.rfind(",16:")+4, line.length() - line.rfind(",16:")+1 - 6)));
        }
    }

    inputFile.close();
    return aaa;
}

// Checks if string has only binary digits
bool ContainsBinaryDigitsOnly(const std::string& str) {
    for (char ch : str) {
        if (ch != '0' && ch != '1') {
            return false;
        }
    }
    return true;
}

// DoubleCheck 
void DoubleCheck(const std::vector<std::pair<std::string,std::string>> &extractedData, const std::vector<std::string> &originalLines, const size_t timeBTWframesMS)
{
    std::vector<std::pair<int, int>> ccc = {
        {0, 515},
        {516, 849},
        {850, 1188},
        {1189, 1218},
        {1219, 1245}
    };
    
    // Find first occurance of subBand1
    size_t auxiliari = 0;
    for (size_t i = 0; i < extractedData.size(); ++i) {
        if (extractedData[i].second == originalLines[ccc[0].first]) {
            auxiliari = i; // Found the matching substring, save the index
            break;
        }
    }
    // std::cout << extractedData[auxiliari] << std::endl;

    // 
    std::vector<std::vector<std::pair<std::string, std::string>>> subbands[5];

    size_t aux = 0;
    size_t lastFrame;
    while (auxiliari + ccc[0].second - ccc[0].first + 1 < extractedData.size())
    {
        lastFrame = FindLastFrame(auxiliari, ccc[0], extractedData, originalLines, timeBTWframesMS);

        std::vector<std::pair<std::string, std::string>> newBlock(extractedData.begin() + auxiliari, extractedData.begin() + lastFrame);
        subbands[aux].push_back(newBlock);

        aux = (aux + 1) % 5;
        auxiliari = lastFrame + 1;
        std::rotate(ccc.begin(), ccc.begin() + 1, ccc.end());
    }

   // Count pairs of strings for each C block in each B block
    for (size_t a = 0; a < 5; ++a) {
        for (const auto& block : subbands[a]) {
            // std::cout << block.size() << std::endl;
        }
    }
    // std::cout << subbands[0].size() << " >= " << subbands[4].size() << std::endl;
    
    size_t max_size = subbands[4].size();
    size_t accumulator = 0;
    for (size_t i = 0; i < max_size; i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            accumulator += subbands[j][i].size();
        }
    }
    std::cout << "Missing " << std::to_string(1246 * max_size - accumulator) << " of " << std::to_string(1246 * max_size) << ". In percentage: " << std::to_string((1246 * max_size - accumulator)*100.0/(1246 * max_size))<< std::endl; 
    std::cout << "" << std::endl;
}

// Finds the limit between two RDS subbands using estimated NSTP time
size_t FindLastFrame(const size_t firstFrame, std::pair<int, int> idealPositions, const std::vector<std::pair<std::string,std::string>> &extractedData, const std::vector<std::string> &originalLines, const size_t timeBTWframesMS)
{
    size_t h = 0;
    const size_t subbandLengthMinusOne = idealPositions.second - idealPositions.first;

    const unsigned long long subbandInitTime    = std::stoull(extractedData[firstFrame].first);
    const unsigned long long auxiliarFactor     = timeBTWframesMS * (subbandLengthMinusOne + 1);

    unsigned long long subbandEndTime       = std::stoull(extractedData[firstFrame + subbandLengthMinusOne + h].first);
    unsigned long long NEXTsubbandInitTime  = std::stoull(extractedData[firstFrame + subbandLengthMinusOne + h + 1].first);

    do
    {
        h += ((subbandEndTime - subbandInitTime) / auxiliarFactor == 0) ? 1 : ((NEXTsubbandInitTime - subbandInitTime) / auxiliarFactor == 1) ? -1 : 0;

        subbandEndTime       = std::stoull(extractedData[firstFrame + subbandLengthMinusOne + h].first);
        NEXTsubbandInitTime  = std::stoull(extractedData[firstFrame + subbandLengthMinusOne + h + 1].first);

    } while ( ((subbandEndTime - subbandInitTime) / auxiliarFactor != 0) && ((NEXTsubbandInitTime - subbandInitTime) / auxiliarFactor != 1));

    return firstFrame + subbandLengthMinusOne + h;
}
