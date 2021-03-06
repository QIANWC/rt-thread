
#include "basic_utility.h"


/******cstring test*****/
#include "cstring.hpp"
#include "NamedVariant.hpp"
using namespace variant;

namespace utility
{
    static void print_vstr(vector<string> vs)
    {
        printf("vector<string>:\n");
        for (auto s : vs)
        {
            printf("%s\n", s.c_str());
        }
    }

    vector<string> vstr(4);
    int cstring_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        int test_cnt = 0, pass_cnt = 0;

        printf("String Test:\n");

        string svoid;
        string snull = (char*)nullptr;
        string sa("stringA"), sb = "stringB", sc("stringC", 6, 1), sd(4, 'D');
        string s = "lite string";
        string &s1 = vstr[0], &s2 = vstr[1], &s3 = vstr[2], &s4 = vstr[3];

        test_assert(svoid.size() == 0&&svoid.capacity() == ASIO::base_size&&svoid.data() != nullptr);
        test_assert(snull.size() == 0&&snull.capacity() == ASIO::base_size&&snull.data() != nullptr);
        test_assert(sa.size() == 7&&sb.size() == 7);
        test_assert(sc.size() == 1&&sc == "C");
        test_assert(sd.size() == 4&&sd == "DDDD");

        test_assert(s.data()&&s.c_str()&&s.capacity()&&s.size());
        test_assert(strcmp(s.c_str(), "lite string") == 0);
        test_assert(s[5] == 's'&&s.at(s.size() - 1) == s[s.size() - 1]);
        test_assert(s != "lite string.");
        test_assert(s != "lite.string");
        test_assert(s.compare("Lite String", false) == 0);
        test_assert(s != "abcdef");
        test_assert(s.startswith("lite"));
        test_assert(!s.startswith("string"));
        test_assert(s.endswith("string"));
        test_assert(!s.endswith("lite"));
        test_assert(s.substr(5, 6) == "string");


        s = string("abc") + "def";
        s += "ghi";
        test_assert(s == "abcdefghi");
        s.clear();

        s = "abc,def,ghi";
        vstr = s.split(',');
        print_vstr(vstr);
        test_assert(vstr.size() == 3&&vstr[0] == "abc"&&vstr[1] == "def"&&vstr[2] == "ghi");

        s = "name,type=value;";
        vstr.clear();
        vstr = s.split_cascade(",=");
        print_vstr(vstr);
        test_assert(vstr.size() == 3&&vstr[0] == "name"&&vstr[1] == "type"&&vstr[2] == "value;");

        vstr.clear();
        vstr = s.split_cascade(",=;");
        print_vstr(vstr);
        test_assert(vstr.size() == 4&&vstr[0] == "name"&&vstr[1] == "type"&&vstr[2] == "value"&&vstr[3] == "");

        vstr.clear();
        vstr = s.split_cascade(",=;;");
        print_vstr(vstr);
        test_assert(vstr.size() == 4&&vstr[0] == "name"&&vstr[1] == "type"&&vstr[2] == "value"&&vstr[3] == "");

        //insert
        s = "aaabbb";
        test_assert(s.substr(0).insert(0, "ccc") == "cccaaabbb");
        test_assert(s.substr(0).insert(3, "ccc") == "aaacccbbb");
        test_assert(s.substr(0).insert(6, "ccc") == "aaabbbccc");
        test_assert(s.substr(0).insert(10, "ccc") == "aaabbbccc");//目前未确定该情况的行为
        //erase
        test_assert(s.substr(0).erase(3) == "aaa");
        test_assert(s.substr(0).erase(3, 2) == "aaab");
        test_assert(s.substr(0).erase(3, 4) == "aaa");
        test_assert(s.substr(0).erase(10) == "aaabbb");

        //TODO:replace
failexit:
        if (pass_cnt == test_cnt)
        {
            printf("string test passed %d/%d\n", pass_cnt, test_cnt);
            return 0;
        }
        else
        {
            printf("string test failed %d/%d\n", pass_cnt, test_cnt);
            return -1;
        }
    }


    int namedvariant_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        int test_cnt = 0, pass_cnt = 0;
        NamedVariant var;
        string s = "A,int=32;BB,float=0.1;CCC,double=0.2;\
            D:int=1;E,char=1.2;F,float=1.0";
        vector<string> lines = s.split(';');

        for (auto l : lines)
        {
            var.from_line(l);
            string linestr = var.to_line();
            printf("%s\n", linestr.c_str());
        }

        //浮点型末尾0可能略有变动
        test_assert(var.from_line(lines[0]).to_line() == "A,int=32;");
        test_assert(var.from_line(lines[1]).to_line() == "BB,float=0.100000;");
#if NAMEDVARIANT_SUPPORT_DOUBLE
        test_assert(var.from_line(lines[2]).to_line() == "CCC,double=0.200000;");//double的末尾0可能更多一些
#else
        test_assert(var.from_line(lines[2]).to_line() == "CCC,double=NaN;");
#endif
        test_assert(var.from_line(lines[3]).to_line() == ",unknown=0;");
        test_assert(var.from_line(lines[4]).to_line() == "E,unknown=0;");
        test_assert(var.from_line(lines[5]).to_line() == "F,float=1.000000;");


failexit :
        if (pass_cnt == test_cnt)
        {
            printf("namedvariant test passed %d/%d\n", pass_cnt, test_cnt);
            return 0;
        }
        else
        {
            printf("namedvariant test failed %d/%d\n", pass_cnt, test_cnt);
            return -1;
        }
    }

    int utility_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        cstring_test(failbehavior);
        namedvariant_test(failbehavior);
        return 0;
    }
}


