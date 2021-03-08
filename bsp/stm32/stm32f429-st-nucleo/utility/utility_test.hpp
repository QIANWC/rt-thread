
#include "basic_utility.hpp"


/*测试主要划分为三类：
 *纯软件测试（包括可以独立运行或者使用虚拟设备的算法）
 *硬件功能测试，主要测试硬件完好性和驱动库基本功能。测试完成后应该尽力恢复测试前状态
 *  比如系统定时器时基，时间戳获取，adc/pwm/crc/rng/硬件加密单元等
 *  [问题，那么编码器应该属于哪一类？
 *      如果手动转动就是硬件基本功能，但是需要人
 *      如果代码控制转动甚至还需要电子换向驱动电机就比较复杂了，
 *      我觉得编码器测试应该设计为开环测试，只需要简单的PWM就可以完成]
 *混合测试，比如实物的闭环控制，更加复杂的和上位机交互
 *      
 *本文件只负责代码库测试，不涉及硬件控制
 *需要修改硬件配置的测试可能对系统运行有影响，必须小心谨慎。
 **/
//software
#include "PlatformInfo.hpp"
#include "QNum.hpp"
#include "cstring.hpp"
#include "NamedVariant.hpp"
#include "basic_filter.hpp"

//hardware
#include "Microsecond.hpp"
#include "Encoder.hpp"

namespace utility
{
    static int platforminfo_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("Platform test:");
        
        int retval = 0;
        if (PlatformInfo::IsCTypeStandard())
        {
            LOG_I("platform has standard type");
            retval = 0;
        }
        else
        {
            LOG_E("platform type is not standard");
            retval = -1;
        }
        return retval;
    }


    using qmath::QNum;
    //helper function
    static bool near(float a, float b)
    {
        return fabs(a - b) <= 0.001f;
    }
    static int qnum_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("QNum test:");
        int pass_cnt = 0, test_cnt = 0;
        
        //q15_int32 or say q15_precision32
        using q15 = QNum<int32_t, 15>;
        q15 v1 = 1.0f, v2 = 2.0f, v3 = 3.0f, v0_1 = 0.1f, v0_5 = 0.5f;
        LOG_D("test values:%f,%f,%f,%f,%f", (float)v1, (float)v2, (float)v3, (float)v0_1, (float)v0_5);
            
        //目前的测试方法不一定对，精度方面还欠考虑，覆盖面还不够
        test_assert(near(q15(1) + q15(2), 3));
        test_assert(near(q15(1.0f - 2.0f), -1.0f));
        test_assert(near(v1 + v2, 3.0f));
        test_assert(near(v1 - 2, -1));
        test_assert(near(v1 - v2, -1.0f));
        test_assert(near(v1 * v0_5, v0_5));
        test_assert(near(v1 / v2, 0.5f));
        //...
            
failexit :
        if(pass_cnt == test_cnt)
        {
            LOG_I("QNum test passed %d/%d", pass_cnt, test_cnt);
            return 0;
        }
        else
        {
            LOG_E("QNum test failed %d/%d", pass_cnt, test_cnt);
            return -1;
        }
    }


    static vector<string> vstr(4);
    static void print_vstr(vector<string> vs)
    {
        printf("vector<string>:\n");
        for (auto s : vs)
        {
            printf("%s\n", s.c_str());
        }
    }
    static int cstring_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("String test:");
        int test_cnt = 0, pass_cnt = 0;

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
failexit :
        if(pass_cnt == test_cnt)
        {
            LOG_I("string test passed %d/%d", pass_cnt, test_cnt);
            return 0;
        }
        else
        {
            LOG_E("string test failed %d/%d", pass_cnt, test_cnt);
            return -1;
        }
    }


    using variant::NamedVariant;
    static int namedvariant_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("NamedVariant test:");
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
            LOG_I("namedvariant test passed %d/%d", pass_cnt, test_cnt);
            return 0;
        }
        else
        {
            LOG_E("namedvariant test failed %d/%d", pass_cnt, test_cnt);
            return -1;
        }
    }


    static int filter_test(bool failstop)
    {
        LOG_I("Filter test:developing...");
        //TODO:filter test developing...
        filter::FIR<float, 3> fir2 = { 1.0f, 1.0f, 1.0f };
        filter::FIR<float, 3> fir3({ 1.0f, 1.0f, 1.0f });
        filter::FIR<float, 3> fir4(std::initializer_list<const float>{ 0.3f, 0.3f, 0.3f });

        return 0;
    }


    inline int software_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("[Software test]");
        
        platforminfo_test(failbehavior);
        qnum_test(failbehavior);
        cstring_test(failbehavior);
        namedvariant_test(failbehavior);
        return 0;
    }
    
    
    //32位时间戳获取测试，未测试溢出表现
    inline int microsecond_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("Microsecond API test:");
        int pass_cnt = 0, test_cnt = 0;
        
        static timestamp_t stamp = 0, last_stamp = 0;
        last_stamp = us_tick();
        for (int k = 0; k < 5; ++k)
        {
            delayms(1000);
            stamp = us_tick();
            timestamp_t dt = stamp - last_stamp;
            last_stamp = stamp;
            LOG_I("us_tick:%d,0x%08X,dt=%d", stamp, stamp, dt);
            int error = dt - 1000'000U;
            if (std::abs(error) >= 3000)
            {
                LOG_E("us tick offset too large,dt=%d,error=%f%%", dt, error / 1000'000.0f * 100.0f);
                return -1;//时间戳误差过大
            }
        }
        return 0;
    }

    
    inline int hardware_test(int failbehavior = TESTFAIL_BREAKPOINT)
    {
        LOG_I("[Hardware test]");
        
        microsecond_test();
        return 0;
    }
}
