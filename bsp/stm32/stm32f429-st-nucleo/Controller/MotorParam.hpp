#pragma once

#include "NamedVariant.hpp"
using variant::NamedVariant;

//motor state,voltage,torque(current),speed,position
typedef struct
{
    float V;   //voltage,or average voltage when use PWM
    float A;   //current,can convert to/from torque
    float S;   //speed,velocity
    float P;   //position
    int64_t pcnt;   //precise postion in raw binary(eg:encoder cnt)
    int64_t stamp_us;   //time stamp us
}VASP;

enum MotorCtrlMode
{
    None = 0,
    ConstVoltage = 1,
    ConstTorque = 2,
    ConstSpeed = 3,
    ConstPosition = 4,
    //...

    ILOOP_EN = 0x10,
    VLOOP_EN = 0x20,
    PLOOP_EN = 0x40,
};
#define PARAM_F32(name) #name",float=0;\n"
#define PARAM_I32(name) #name",int=0;\n"
//#define PARAM_U32(name) #name",uint=0;\n"
//#define PARAM_STR(name) #name",string=0;\n"

//参数尽量使用FP32表示，方便数据交互和处理
typedef struct MechanicalParam_t
{
    float GearRatio;
    float num, den;      //准确的分数表示
    float ShaftSideInertial;
    float RotorSideInertial;
    float RotorTMax;             //电机最大扭矩,Nm
    float RotorAradMax;          //转子最大角加速度，rad/s^2
    float RotorDrMax;            //加速度在一个周期内引起的最大位移，=0.5aΔt^2*容差扩展系数，单位r
    float RotorDr_Kexpan;
    static constexpr char fmt[] = PARAM_F32(GearRatio)PARAM_F32(num)PARAM_F32(den)\
        PARAM_F32(ShaftSideInertial)PARAM_F32(RotorSideInertial)PARAM_F32(RotorTMax)\
        PARAM_F32(RotorAradMax)PARAM_F32(RotorDrmax)PARAM_F32(RotorDr_Kexpan);
}MechanicalParam_t;

typedef struct VoltageParam_t
{
    float MaxVolt;
    float MaxVoltOutBits;
    float VoltPerBit;
    static constexpr char fmt[] = \
        PARAM_F32(MaxVolt)PARAM_F32(MaxVoltOutBits)PARAM_F32(VoltPerBit);
}VoltageParam_t;

typedef struct CurrentParam_t
{
    float MaxAmp;
    float MaxAmpOutBits;
    float AmpPerBit;
    float TorquePerAmp;
    static constexpr char fmt[] = \
        PARAM_F32(MaxAmp)PARAM_I32(MaxAmpOutBits)PARAM_F32(AmpPerBit)PARAM_F32(TorquePerAmp);
}CurrentParam_t;

typedef struct VelocityParam_t
{
    float MaxSpeed;
    float MaxSpeedBits;
    float RPSPerBit;    //rps for rotation,m/s for linear motion
    static constexpr char fmt[] = \
        PARAM_F32(MaxSpeed)PARAM_F32(MaxSpeedBits)PARAM_F32(RPSPerBit);
}VelocityParam_t;

typedef struct PositionParam_t
{
    float RevPerBit;
    float PositionLimitEnable;    //限位
    float PositionPosLimitEn;
    float PositionNegLimitEn;
    float PositiveLimit, NegativeLimit;
    static constexpr char fmt[] = \
        PARAM_F32(RevPerBit)PARAM_F32(PositionLimitReg)PARAM_F32(PositiveLimit)PARAM_F32(NegativeLimit);
}PositionParam_t;


static int show_errorline(const char* errormsg, int line, const char *linedata)
{
    printf("line %d:%s\n%s\n", line, errormsg, linedata);
    return -1;
}

//也许应该返回解析之后的vector<NamedVariant>
static int param_fromstring(const string& src,const string& fmt,void* pdst)
{
    bool varok = true,fmtok=true;
    uint8_t* pdata = (uint8_t*)pdst;
    vector<NamedVariant> vars = NamedVariant::string_split_to_variant(src, &varok);
    vector<NamedVariant> fmts = NamedVariant::string_split_to_variant(fmt, &fmtok);
    if (!varok||!fmtok)
        return -1;//error

    size_t wpos = 0;
    //根据fmt查找并填写参数，fmt应当是src的子集
    for (auto f : fmts)
    {
        size_t n = 0;
        while (n < vars.size()&&strncmp(f.name, vars[n].name, NAMEDVARIANT_MAXNAMELEN) != 0)
            ++n;
        if (n >= vars.size())
            return -1;//参数缺失
        switch (f.typeinfo)
        {
        case NamedVariant::type_int:*(int*)(pdata + wpos) = vars[n].v.i32; wpos += sizeof(int); break;
        default:printf("param type missing!\n"); //assume as float
        case NamedVariant::type_float:*(float*)(pdata + wpos) = vars[n].v.f32; wpos += sizeof(float); break;
#if NAMEDVARIANT_SUPPORT_DOUBLE
        case NamedVariant::type_double:*(double*)(pdata + wpos) = vars[n].v.d64; wpos += sizeof(double); break;
#else
        case NamedVariant::type_double:*(uint64_t*)(pdata + wpos) = NAN; wpos += sizeof(double); break;
#endif // NAMEDVARIANT_SUPPORT_DOUBLE

        }
    }
    return 0;
}

static string param_tostring(const string fmt, void* pv)
{
    bool fmtok = false;
    string result;
    vector<NamedVariant> vars = NamedVariant::string_split_to_variant(fmt, &fmtok);
    uint8_t* pdata = (uint8_t*)pv;
    size_t rpos = 0;
    if (!fmtok)
        return result;
    for (auto v : vars)
    {
        string s = v.name;
        switch (v.typeinfo)
        {
        case NamedVariant::type_int:s += ",int=" + string::to_string(*(int*)(pdata + rpos)); rpos += sizeof(int); break;
        case NamedVariant::type_float:s += ",float=" + string::to_string(*(float*)(pdata + rpos)); rpos += sizeof(float); break;
#if NAMEDVARIANT_SUPPORT_DOUBLE
        case NamedVariant::type_double:s += ",double=" + to_string(*(double*)(pdata + rpos)); rpos += sizeof(double); break;
#else
        case NamedVariant::type_double:s += ",double=NaN"; rpos += sizeof(double); break;
#endif
        default:s += ",unknown=0"; break;
        }
        s += ';';
        result += s;
    }
    return result;
}

//copyable
typedef struct MotorConfStruct
//class MotorConfStruct
{
public:
//    MotorConfStruct();   // = delete;
//    ~MotorConfStruct() {}
//
//    MotorConfStruct(const MotorConfStruct &src)
//    {
//        (*this) = src;
//    }
//
//    void operator=(const MotorConfStruct &src)
//    {
//        memcpy(this, &src, sizeof(MotorConfStruct));
//    }

    //与存档交互参数，后续可能改用json
    string tostring()
    {
        string s;
        s += param_tostring(MechanicalParam_t::fmt, &MechanicalParam);
        //...

        return string();
    }
    //使用stream接口应该更好一些
    int fromstring(const string& s)
    {
        //        param_fromstring(psrc, MechanicalParam_t::fmt, &MechanicalParam);
        string paragram;
        size_t startpos = 0, endpos = 0;
        string title = "MechanicalParam";
        startpos = s.find(s.c_str()) + s.size();
        //jump CRLF
        endpos = s.find(s.c_str());
        paragram = s.substr(startpos, endpos - startpos);
        param_fromstring(s, string(MechanicalParam_t::fmt), &MechanicalParam);
        startpos = endpos + title.size();
        param_fromstring(s, string(VoltageParam_t::fmt), &VoltageParam);
        param_fromstring(s, string(CurrentParam_t::fmt), &CurrentParam);
        //        param_fromstring(psrc, VoltageParam_t::fmt, &VoltageParam);
        //        param_fromstring(psrc, CurrentParam_t::fmt, &CurrentParam);
        //        MechanicalParam.fromstring(nullptr);
                return 0;
    }

    int crc_check();
public:
    //电机类型，只取决于转子（本质属性）,M3508
    string MotorType = "DefaultType";
    //电机配置，比如减速器，M3508_P19，云台yaw/pitch
    string MotorConf = "DefaultConf";

    MechanicalParam_t MechanicalParam;
    VoltageParam_t VoltageParam;
    CurrentParam_t CurrentParam;
    VelocityParam_t VelocityParam;
    PositionParam_t PositionParam;
    uint32_t crc;
}MotorConfStruct;

static MotorConfStruct mconf;
static string src = "MaxVolt,float=30.0;MaxVoltOutBits,int=3000;VoltPerBit,float=0.01,"\
    "MaxAmp,float=1.0\nMaxAmpOutBits,int=1000,AmpPerBit,float=0.001,TorquePerAmp,int=0.2";
inline int MConfGenerate_RM35()
{
    //    mconf.VoltageParam =  VoltageParam_t { .MaxVolt = 25.0f, .MaxVoltOutBits = 3000, .VoltPerBit = 1 };
    //    mconf.CurrentParam = CurrentParam_t{ .MaxAmp = 3.0f, .MaxAmpOutBits = 500, .AmpPerBit = 1, .TorquePerAmp = 1 };

    //    mconf.MechanicalParam = MechanicalParam_t { .GearRatio = 0.1f, .num = 11, .den = 37, .ShaftSideInertial = 1, .RotorSideInertial = 1, .RotorTMax = 1, .RotorAradMax = 1, .RotorDrMax = 1, .RotorDr_Kexpan = 1 };
    //    mconf.CurrentParam.tostring(buf);


    memset(&mconf.CurrentParam, 0, sizeof(mconf.CurrentParam));
    //    mconf.CurrentParam = CurrentParam_t{ .MaxAmp = 3.0f, .MaxAmpOutBits = 1000, .AmpPerBit = 100, .TorquePerAmp = 0.1f };
    //    param_fromstring(src, CurrentParam_t::fmt, &mconf.CurrentParam);
    mconf.fromstring(src);
    return 0;
}
