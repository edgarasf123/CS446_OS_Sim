#ifndef _SIM_HELPERS
#define _SIM_HELPERS

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <cctype>

namespace SimHelpers {
    /**
     * @brief Trims string from white space at beginning and end.
     * 
     * @param str String to be trimmed.
     * @return Trimmed string.
     */
    std::string strTrim( const std::string &str )
    {
        size_t leftPos = 0;
        size_t rightPos = str.size();
        while(leftPos < str.size())
        {
            char c = str[leftPos];
            if( c != ' ' && c != '\t' && c != '\r' && c != '\n')
                break;
            leftPos += 1; 
        }
        while(rightPos > 0)
        {
            char c = str[rightPos-1];
            if( c != ' ' && c != '\t' && c != '\r' && c != '\n')
                break;
            rightPos -= 1; 
        }
        return str.substr(leftPos, rightPos-leftPos);
    }

    /**
     * @brief Splits string by delimiter and populates vector strings.
     * 
     * @param str String to process.
     * @param delim Delimeter for string seperation.
     * @param strings Vector to populate with strings.
     */
    void strSplit( const std::string &str, char delim, std::vector<std::string> &strings )
    {
        strings.clear();

        size_t lastPos = -1;
        size_t findPos = -1;
        do
        {
            lastPos = findPos+1;
            findPos = str.find(delim, lastPos);
            
            strings.push_back( str.substr(lastPos, findPos-lastPos) );
        }while( findPos != std::string::npos);
    }
    
    /**
     * @brief Converts all characters in str from upper case to lowercase
     *        and returns it as new string.
     * 
     * 
     * @param str String to convert.
     * @return Lowercased string.
     */
    std::string strLower( const std::string &str )
    {
        std::string ret = str;
        for( size_t i = 0; i < str.size(); i++ )
        {
            ret[i] = tolower(ret[i]);
        }
        return ret;
    }


}


#endif // _SIM_HELPERS
