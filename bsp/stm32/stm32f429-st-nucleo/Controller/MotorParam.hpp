#pragma once

#include "basic_utility.h"

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
    
typedef struct
{
    float GearRatio;
    uint16_t num, den;      //准确的分数表示
    float ShaftSideInertial;
    float RotorSideInertial;
    float RotorTMax;             //电机最大扭矩,Nm
    float RotorAradMax;          //转子最大角加速度，rad/s^2
    float RotorDrMax;            //加速度在一个周期内引起的最大位移，=0.5aΔt^2*容差扩展系数，单位r
    float RotorDr_Kexpan;
}MechanicalParam_t;
    
typedef struct
{
    float MaxVolt;
    int MaxVoltOutBits;
    float VoltPerBit;
}VoltageParam_t;
    
typedef struct
{
    float MaxAmp;
    int MaxAmpOutBits;
    float AmpPerBit;
    float TorquePerAmp;
}CurrentParam_t;
            
typedef struct
{
    float MaxSpeed;
    int MaxSpeedBits;
    float RPSPerBit;    //rps for rotation,m/s for linear motion
}VelocityParam_t;
            
typedef struct
{
    float RevPerBit;
    uint8_t PositionLimitEnable : 1;   //限位
    uint8_t PositionPosLimitEn : 1;
    uint8_t PositionNegLimitEn : 1;
    float PositiveLimit, NegativeLimit;
}PositionParam_t;

//copyable
typedef struct
//class MotorConfStruct
{
public:
//    MotorConfStruct();   // = delete;
//    ~MotorConfStruct() {}
    
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
    std::string tostring()
    {
        return std::string();
    }
    int fromstring(const std::string& s)
    {
        return 0;
    }
            
    int crc_check();
public:
    //电机类型，只取决于转子（本质属性）,M3508
    std::string MotorType = "DefaultType";
    //电机配置，比如减速器，M3508_P19，云台yaw/pitch
    std::string MotorConf = "DefaultConf";
	
    MechanicalParam_t MechanicalParam;
    VoltageParam_t VoltageParam;
    CurrentParam_t CurrentParam;
    VelocityParam_t VelocityParam;
    PositionParam_t PositionParam;
    uint32_t crc;
}MotorConfStruct;

int MConfGenerate_RM35()
{
    MotorConfStruct mconf;
    mconf.VoltageParam =  VoltageParam_t { .MaxVolt = 25.0f, .MaxVoltOutBits = 3000, .VoltPerBit = 1 };
    mconf.CurrentParam = CurrentParam_t{ .MaxAmp = 3.0f, .MaxAmpOutBits = 500, .AmpPerBit = 1, .TorquePerAmp = 1 };
    
    
    return 0;
}
