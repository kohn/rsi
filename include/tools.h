#ifndef TOOLS_H
#define TOOLS_H
#include <string>
#include <map>
#include <vector>
#define COMMENT_CHAR '#'

class Tools
{
private:
    bool IsSpace(char c);
    bool IsCommentChar(char c);
    void Trim(std::string &str);
    bool AnalyseLine(const std::string & line, std::string & key, std::string & value);
    
public:
    Tools(){}
    virtual ~Tools(){}

    bool ReadConfig(const std::string & filename, std::map<std::string, std::string> &m);
    void PrintConfig(const std::map<std::string, std::string> &m);
    std::string JsonEncode(std::map<std::string, std::string> &m);

    static void split(std::string s, std::vector<std::string> &v, char delim);
};
#endif
