#ifndef CXARGUMENTS_H
#define CXARGUMENTS_H

/**
 * Attention ...
 * Attention ...
 * Attention ...
 * CxArguments Is No Thread Safe
 * CxArguments Is No Thread Safe
 * CxArguments Is No Thread Safe
 * CxArguments.CPP HAS GLOBAL VAR
 */

#include "cxstring.h"

// 1 - (int argc, const char **argv) : default is main(int argc, const char **argv)
// 2 - ArgMapString and ArgMapVector : default do not contain argv[0]
class CCXX_EXPORT CxArguments
{
public:
    // e.g.: -p 80
    // e.g.: --port 80
    // e.g.: --i 80 81 82
    static void init(int argc, const char *argv[]);

    static const std::vector<std::string> &getArgKeys();

    static const std::map<std::string, std::string> &getArgMapString();

    static const std::map<std::string, std::vector<std::string> > &getArgMapVector();

    static const std::string &getArgs();

    static const std::string &getArg0();

    static const std::string &getArg0FileName();

    static const std::string &getArg0PrefixName();

    static const std::string &getArg1();

    static std::string getCommandLine();

    //sArgumentName: std::string("name1")
    static std::string getArgValue(const std::string &sKey);

    //sArgumentName: std::string("name1")
    static bool isExistArgKey(const std::string &sKey);

    //sArguments : std::map<std::string, std::string> sArguments; sArguments["-name1"] = "value1"; sArguments["-name2"] = "value2";
    static std::string argsToCommandLine(int argc, const char *argv[]);

    static std::string argsToString(const std::map<std::string, std::string> &sArguments);

    static std::map<std::string, std::string> argsToMapString(int argc, const char *argv[]);

    static std::map<std::string, std::vector<std::string> > argsToMapVector(int argc, const char *argv[]);

    //sArg : "/tmp/app -name1 value1 value2 -name2 value3"
    static std::map<std::string, std::string> parseCommandLine(const std::string &sCommandLine, std::string &sArgv0);

    static std::map<std::string, std::vector<std::string> > parseCommandLine2(const std::string &sCommonLine);

    static std::string findEnv(const std::string &sName);

    static bool existEnv(const std::string &sName);

};

#endif //CXARGUMENTS_H
