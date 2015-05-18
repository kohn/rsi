#include "tools.h"
#include <fstream>
#include <iostream>

bool Tools::IsSpace(char c){
    if (' ' == c || '\t' == c)
        return true;
    return false;
}

bool Tools::IsCommentChar(char c){
    switch(c) {
    case COMMENT_CHAR:
        return true;
    default:
        return false;
    }
}

void Tools::Trim(std::string & str)
{
    if (str.empty()) {
        return;
    }
    
    int i, start_pos, end_pos;
    for (i = 0; i < (int)str.size(); ++i) {
        if (!IsSpace(str[i])) {
            break;
        }
    }
    if (i == (int)str.size()) { // 全部是空白字符串
        str = "";
        return;
    }
    
    start_pos = i;
    for (i = (int)str.size() - 1; i >= 0; --i) {
        if (!IsSpace(str[i])) {
            break;
        }
    }
    end_pos = i;
    
    str = str.substr(start_pos, end_pos - start_pos + 1);
}

bool Tools::AnalyseLine(const std::string & line, std::string & key, std::string & value)
{
    if (line.empty())
        return false;
    int start_pos = 0, end_pos = line.size() - 1, pos;
    if ((pos = line.find(COMMENT_CHAR)) != -1) {
        if (0 == pos) {  // 行的第一个字符就是注释字符
            return false;
        }
        end_pos = pos - 1;
    }
    std::string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分
    
    if ((pos = new_line.find('=')) == -1)
        return false;  // 没有=号
        
    key = new_line.substr(0, pos);
    value = new_line.substr(pos + 1, end_pos + 1- (pos + 1));
    
    Trim(key);
    if (key.empty()) {
        return false;
    }
    Trim(value);
    return true;
}

bool Tools::ReadConfig(const std::string & filename, std::map<std::string, std::string> & m)
{
    m.clear();
    std::ifstream infile(filename.c_str());
    if (!infile) {
        return false;
    }
    std::string line, key, value;
    while (std::getline(infile, line)) {
        if (AnalyseLine(line, key, value)) {
            m[key] = value;
        }
    }
    infile.close();
    return true;
}

void Tools::PrintConfig(const std::map<std::string, std::string> & m)
{
    std::map<std::string, std::string>::const_iterator mite = m.begin();
    for (; mite != m.end(); ++mite) {
        std::cout << mite->first << "=" << mite->second << std::endl;
    }
}
