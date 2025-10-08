#include "cxarguments.h"


using namespace std;


static string f_arg0;
static string f_arg1;
static string f_args;
static vector<string> f_argKeys;
static map<string, vector<string> > f_argMapVector;
static map<string, string> f_argMapString;

#define ESCAPE_HEX_CODE 0x5c
#define SGLQUOTE_HEX_CODE 0x27
#define DBLQUOTE_HEX_CODE 0x22
#define SPACE_HEX_CODE 0x20


/**
 * ArgvSplit
 */
class ArgvSplit {
private:
    bool prependName;
    std::string name;
    std::vector<std::string> arguments;
    std::vector<const char*> argv_arr;

    char quotes[3];
    char space[2];

    static bool isEscaped(const std::string& str, std::string::size_type idx) {
        // is first character, can not be escaped
        if ( idx == 0 ) return false;
        // is out of bounds (-1)
        if ( idx == std::string::npos ) return false;

        // preceeding character is backslash?
        if ( str.at(idx - 1) == ESCAPE_HEX_CODE )
            return true;
        else
            return false;
    }

    std::string::size_type findFirstQuote(const std::string& str) {
        std::string::size_type idx = 0;

        do {
            idx = str.find_first_of(quotes, idx == 0 ? 0 : idx + 1);
        } while ( isEscaped( str, idx ) && idx != std::string::npos);

        return idx;
    }

    std::string::size_type findMatching(const std::string& str, std::string::size_type match_idx) {
        if (match_idx == std::string::npos) return std::string::npos;

        char match[1];
        match[0] = str.at( match_idx );

        std::string::size_type idx = match_idx;

        do {
            idx = str.find_first_of( match, idx + 1 );
        } while ( isEscaped( str, idx ) && idx != std::string::npos);

        return idx;
    }

    void splitStrToVectorBy(const std::string& str, char delim, std::vector<std::string>& vec)
    {
        std::stringstream strStream( str );
        std::string element;

        while ( std::getline( strStream, element, delim ) )
        {
            if ( !element.empty() )
                vec.push_back( element );
        }
    }

    // function is called recursive
    // inefficient: (but ok for the intended use)
    //	- using substrings (copying) for recursion; C++17 string_view could fix this
    //	- no memory preallocation for arguments vector
    void _parse(const std::string& cmdLineStr) {
        if (cmdLineStr.empty())
            return;

        std::string quoted_str, pre_quoted_str, post_quoted_str;

        // w_front used as limit for string splitting on whitespace
        std::string::size_type w_front = cmdLineStr.size(); // if no quotes are found this is used as the substring SIZE for splitting by space
        std::string::size_type w_back = cmdLineStr.size() - 1; // if no quotes are found this is used as OFFSET for the next (and final) recursion

        std::string::size_type q_front = findFirstQuote(cmdLineStr);
        std::string::size_type q_back = findMatching(cmdLineStr, q_front);
        // found an unescaped qoute, are same when npos an so no quote found. are different when two quotes or only a first one are found
        if ( q_front != q_back )
        {
            // get quoted string
            // and get attached string after quoted string. open quotes will be treated as quotes til the end!

            // found matching quote to first
            if ( q_back != std::string::npos )
            {
                quoted_str = cmdLineStr.substr(q_front + 1, q_back - q_front - 1 );

                // find first whitespace after quote
                w_back = cmdLineStr.find_first_of(space, q_back + 1 );
                // none found? handle as quoted til the end
                if (w_back == std::string::npos)
                    w_back = cmdLineStr.size() - 1;

                post_quoted_str = cmdLineStr.substr(q_back + 1, w_back - q_back - 1);
            }
                // did not find matching quote further in string, everything til the end is quoted now
            else
            {
                quoted_str = cmdLineStr.substr(q_front + 1, std::string::npos);
            }


            // get string attached in front of quoted string

            // find last whitespace before quote
            w_front = cmdLineStr.find_last_of(space, q_front);
            // none found?
            if ( w_front == std::string::npos )
                w_front = - 1;

            pre_quoted_str = cmdLineStr.substr(w_front + 1, q_front - w_front - 1 );

        }

        // split by whitespace in [0, w_front[
        if(w_front != std::string::npos)
        {
            splitStrToVectorBy(cmdLineStr.substr(0, w_front), SPACE_HEX_CODE, arguments);
        }

        //add qouted string and surrounding
        if (!quoted_str.empty())
            arguments.push_back(pre_quoted_str + quoted_str + post_quoted_str);

        //recurse
        _parse(cmdLineStr.substr(w_back + 1, std::string::npos));
    }

public:
    ArgvSplit() : quotes{ SGLQUOTE_HEX_CODE, DBLQUOTE_HEX_CODE, 0}, space{ SPACE_HEX_CODE, 0}, prependName(false) {}

    explicit ArgvSplit(const std::string& sName) : ArgvSplit()
    {
        prependName = true;
        this->name = sName;
    }

    ~ArgvSplit() = default;

    std::vector<std::string> getArguments()
    {
        return arguments;
    }

    const char** argv() {
        //return argv_arr.data;
        return argv_arr.data();
    }

    std::string::size_type argc() {
        //return argv_arr.size;
        return argv_arr.size();
    }

    const char** parse(const std::string& cmdline) {
        // setup/cleanup argv pointer
        argv_arr.clear();

        // setup/cleanup arguments
        arguments.clear();
        if (prependName)
            arguments.push_back(name);


        // parse the string and populate arguments vector
        _parse(cmdline);

        // zero arguments? exit now. argv_arr=nullptr at this point; allow handling by argv()
        if (arguments.size() == 0)
            return argv();

        // populate argv array
        for (auto& it : arguments)
            argv_arr.push_back(it.c_str());
        // terminate with null as standard argv array
        argv_arr.push_back(nullptr);

        return argv();
    }
};

static void _printArgs(const char** args)
{
    int i = 0;
    while (args[i] != nullptr)
    {
        std::cout << args[i] << std::endl;
        ++i;
    }
    std::cout << std::endl;
}

int helloArg(int argc, const char* argv[])
{
    ArgvSplit parser;
    parser.parse("-p \"xx yy d\" xxx 'xxxxxxxx yasdf sdfa'");
//    parser.parse(""); // raise error

    const char** argv_parsed = parser.argv();
    const char** argv_cmdl = const_cast<const char**>( argv );

    if (parser.argc() == 0)
    {
        return 0;
    }
    _printArgs(argv_parsed);
    std::cout << "argc: " << argc << std::endl;
    _printArgs(argv_cmdl);

    int i = 1; //starting at 1 as we do not compare executable name
    while (argv_parsed[i] != nullptr && argv_cmdl[i] != nullptr)
    {
        assert(strcmp(argv_parsed[i], argv_cmdl[i]) == 0);
        ++i;
    }
    // one is nullptr
    assert(argv_parsed[i] == argv_cmdl[i]);

    return EXIT_SUCCESS;
}

const map<string, string> &CxArguments::getArgMapString()
{
    return f_argMapString;
}

const std::map<std::string, std::vector<std::string> >& CxArguments::getArgMapVector()
{
    return f_argMapVector;
}

const std::string & CxArguments::getArgs()
{
    return f_args;
}

std::string CxArguments::getCommandLine()
{
    string r;
    if (! f_arg0.empty())
    {
        r += f_arg0 + string(" ");
    }
    if (! f_arg1.empty())
    {
        r += f_arg1 + string(" ");
    }
    if (! f_args.empty())
    {
        r += f_args;
    }
    return r;
}

string CxArguments::getArgValue(const string &sKey)
{
    map<string, string>::const_iterator it = f_argMapString.find(sKey);
    if (it != f_argMapString.end())
        return it->second;
    else
        return std::string{};
}

bool CxArguments::isExistArgKey(const string &sKey)
{
    map<string, string>::const_iterator it = f_argMapString.find(sKey);
    return it != f_argMapString.end();
}

string CxArguments::argsToString(const map<string, string> &sArguments)
{
    string r;
    for (map<string, string>::const_iterator it = sArguments.begin(); it != sArguments.end(); ++it)
    {
        r.push_back('-');
        r.append(it->first);
        r.push_back(' ');
        r.append(it->second);
        r.push_back(' ');
    }
    if (!r.empty()) r.resize(r.size() - 1);
    return r;
}

std::string CxArguments::argsToCommandLine(int argc, const char **argv)
{
    string arg0 = (argv[0] != nullptr) ? argv[0] : string{};
    string args;
    vector<string> argKeys;

    string key;
    vector<string> values;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i)
    {
        if (CxString::beginWith(argv[i], "-"))
        {
            if (! key.empty() && !values.empty())
            {
                string value = CxString::join(values, ' ');
                args += string(" ") + value;
            }
            values.clear();
            if (CxString::beginWith(argv[i], "--"))
            {
                key = string(argv[i] + 2);
            }
            else
            {
                key = string(argv[i] + 1);
            }
            // error
            if (key.empty())
            {
                continue;
            }
            argKeys.push_back(key);
            args += !argKeys.empty() ? string(" ") + argv[i] : argv[i];
            values.clear();
        }
        else
        {
            values.emplace_back(argv[i]);
        }
    }
    if (! key.empty())
    {
        if (!values.empty())
        {
            string value = CxString::join(values, ' ');
            args += string(" ") + value;
        }
    }

    return args.empty() ? arg0 : arg0 + string(" ") + args;
}

map<string, string> CxArguments::parseCommandLine(const string &sCommandLine, std::string &sArgv0)
{
    ArgvSplit parser;
    parser.parse(sCommandLine);
    if (parser.argc() > 0 && parser.argv()[0] != nullptr)
    {
        sArgv0 = string(parser.argv()[0]);
    }
    return argsToMapString(parser.argc(), parser.argv());
}

map<string, string> CxArguments::argsToMapString(int argc, const char *argv[])
{
    map<string, string> argMapString;

    string key;
    vector<string> values;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i)
    {
        if (CxString::beginWith(argv[i], "-"))
        {
            if (! key.empty())
            {
                if (!values.empty())
                {
                    string value = CxString::join(values, ' ');
                    argMapString[key] = value;
                }
                else
                {
                    argMapString[key] = string{};
                }
            }
            values.clear();
            if (CxString::beginWith(argv[i], "--"))
            {
                key = string(argv[i] + 2);
            }
            else
            {
                key = string(argv[i] + 1);
            }
            // error
            if (key.empty())
            {
                continue;
            }
            values.clear();
        }
        else
        {
            values.emplace_back(argv[i]);
        }
    }
    if (! key.empty())
    {
        if (!values.empty())
        {
            string value = CxString::join(values, ' ');
            argMapString[key] = value;
        }
        else
        {
            argMapString[key] = string{};
        }
    }

    return argMapString;
}

std::string CxArguments::findEnv(const std::string &sName)
{
    char *p = getenv(sName.c_str());
    if (p != nullptr)
    {
        return string(p);
    }
    return string{};
}

bool CxArguments::existEnv(const std::string &sName)
{
    return getenv(sName.c_str()) != nullptr;
}

const std::vector<std::string> &CxArguments::getArgKeys()
{
    return f_argKeys;
}

void CxArguments::init(int argc, const char **argv)
{
    string arg0 = (argc > 0 && argv[0] != nullptr) ? argv[0] : string{};
    string arg1 = (argc > 1 && argv[1] != nullptr) ? argv[1] : string{};
    string args;
    vector<string> argKeys;
    map<string, vector<string> > argMapVector;
    map<string, string> argMapString;

    string key;
    vector<string> values;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i)
    {
        if (CxString::beginWith(argv[i], "-"))
        {
            if (! key.empty())
            {
                if (!values.empty())
                {
                    argMapVector[key] = values;
                    string value = CxString::join(values, ' ');
                    argMapString[key] = value;
                    args += string(" ") + value;
                }
                else
                {
                    argMapVector[key] = vector<string>();
                    string value = string{};
                    argMapString[key] = value;
                    args += value;
                }
            }
            values.clear();
            if (CxString::beginWith(argv[i], "--"))
            {
                key = string(argv[i] + 2);
            }
            else
            {
                key = string(argv[i] + 1);
            }
            // error
            if (key.empty())
            {
                continue;
            }
            argKeys.push_back(key);
            args += !argKeys.empty() ? string(" ") + argv[i] : argv[i];
            values.clear();
        }
        else
        {
            values.emplace_back(argv[i]);
        }
    }
    if (! key.empty())
    {
        if (!values.empty())
        {
            argMapVector[key] = values;
            string value = CxString::join(values, ' ');
            argMapString[key] = value;
            args += string(" ") + value;
        }
        else
        {
            argMapVector[key] = vector<string>();
            string value = string{};
            argMapString[key] = value;
            args += value;
        }
    }

    f_arg0 = arg0;
    f_arg1 = arg1;
    f_args = args;
    f_argKeys = argKeys;
    f_argMapVector = argMapVector;
    f_argMapString = argMapString;
}

std::map<std::string, std::vector<std::string> > CxArguments::argsToMapVector(int argc, const char **argv)
{
    map<string, vector<string> > argMapVector;

    string key;
    vector<string> values;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i)
    {
        if (CxString::beginWith(argv[i], "-"))
        {
            if (! key.empty() && !values.empty())
            {
                argMapVector[key] = values;
            }
            values.clear();
            if (CxString::beginWith(argv[i], "--"))
            {
                key = string(argv[i] + 2);
            }
            else
            {
                key = string(argv[i] + 1);
            }
            // error
            if (key.empty())
            {
                continue;
            }
            values.clear();
        }
        else
        {
            values.emplace_back(argv[i]);
        }
    }
    if (! key.empty() && !values.empty())
    {
        argMapVector[key] = values;
    }

    return argMapVector;
}

std::map<std::string, std::vector<std::string> > CxArguments::parseCommandLine2(const string &sCommonLine)
{
    ArgvSplit parser;
    parser.parse(sCommonLine);
    return argsToMapVector(parser.argc(), parser.argv());
}

const std::string &CxArguments::getArg0()
{
    return f_arg0;
}

const std::string &CxArguments::getArg0FileName()
{
    static string s;
    if (s.empty())
    {
        string fp = f_arg0;
        std::string::size_type found = fp.find_last_of("/\\");
        if (found == string::npos)
        {
            s = fp;
        }
        else
        {
            s = fp.substr(found + 1);
        }
    }
    return s;
}

const std::string &CxArguments::getArg0PrefixName()
{
    static string s;
    if (s.empty())
    {
        string fp = f_arg0;
        std::string::size_type foundR = fp.rfind('.');
        std::string::size_type foundL = fp.find_last_of("/\\");
        if (foundR == string::npos)
        {
            //* filename
            if (foundL == string::npos)
            {
                s = fp;
            }
                //* /tmp/dir/filename
            else
            {
                s = fp.substr(foundL + 1);
            }
        }
        else
        {
            //* filename.suffix
            if (foundL == string::npos)
            {
                s = fp.substr(0, foundR);
            }
                //* /tmp/dir/filename.suffix
            else
            {
                s = fp.substr(foundL + 1, foundR - foundL - 1);
            }
        }

    }
    return s;
}

const std::string &CxArguments::getArg1()
{
    return f_arg1;
}
