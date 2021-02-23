#pragma once

#include <cmath>
#include "controller.hpp"

#include "main.h"
#include <stm32f4xx_hal_adc.h>

#include "Encoder.hpp"
#include "MotorParam.hpp"

/*TODO:位置和速度控制细化
 *TODO:刹车和缓停控制逻辑
 *TODO:环路参数写入
 **/
namespace BrushedDCM
{

    float midoffset = { 1.641f };
    float i, gi = 6.0f / 0.625f;
    float kadc = 3.3f / 4096.0f;
    int16_t ccr = 0, adc_sample_offset = 10;  //采样偏移
    float lpf_a = 0.90f;

    using namespace controller;
    class BDCM
    {
    public:
        BDCM(const MotorConfStruct& _mconf):mconf(_mconf)
        {
        }
        ~BDCM()
        {
        }

        int init(TIM_HandleTypeDef* tim,ADC_HandleTypeDef* adc,QuadratureEncoder* enc)
        {
            if (tim == NULL || adc == NULL)
                return -1;
            htim = tim;
            hadc = adc;
            penc = enc;
            mcpwm_cnt = SYSCLK_HZ/MCPWM_HZ;
            mcpwm_hz = MCPWM_HZ;
            mcpwm_s = 1.0f / MCPWM_HZ;
            HAL_ADC_Start_DMA(adc, (uint32_t*)&raw_current, 1);
            
            __HAL_TIM_DISABLE(htim);
            TIM_CCxChannelCmd(htim->Instance, tim_cha, TIM_CCx_ENABLE);
            TIM_CCxChannelCmd(htim->Instance, tim_chb, TIM_CCx_ENABLE);
            TIM_CCxChannelCmd(htim->Instance, TIM_CHANNEL_3, TIM_CCx_ENABLE);
            TIM_CCxChannelCmd(htim->Instance, TIM_CHANNEL_4, TIM_CCx_ENABLE);
            __HAL_TIM_MOE_ENABLE(htim);
            __HAL_TIM_ENABLE(htim);
            
//            HAL_TIM_PWM_Start(htim, TIM_CHANNEL_ALL);//old HAL doesn't support this
            HAL_TIM_Base_Start(htim);
            return 0;
        }
            
        void setControlMode(MotorCtrlMode mode)
        {
            switch (mode)
            {
            default://default as ConstVoltage
            case ConstVoltage:
                {
                    setVoltage(0.0f);
                    loop_cr.reg = mode;
                }break;
            case ConstTorque:
                {
                    setTorque(0.0f);
                    loop_cr.reg = mode | ILOOP_EN;
                }break;
            case ConstSpeed:
                {
                    setVelocity(0.0f);
                    loop_cr.reg = mode | ILOOP_EN | VLOOP_EN;
                }break;
            case ConstPosition:
                {
                    setPosition(state.pcnt);
                    loop_cr.reg = mode | ILOOP_EN | VLOOP_EN | PLOOP_EN;
                }break;
            }
        }
        
        void setPosition(float x)
        {
            ploop.set_target(x);
        }
        void setPosition(int64_t poscnt)
        {
            //TODO:
//            ploop.set_target(poscnt);
        }
        void setVelocity(float x)
        {
            vloop.set_target(x);
        }
        void setTorque(float x)
        {
            iloop.set_target(x);
        }
        void setVoltage(float x)
        {
            voltage_ctrl(x);
        }
        
        void slowdown()
        {
            switch (loop_cr.ctrl_mode)
            {
            default:
                break;
            case ConstVoltage:setVoltage(0.0f);break;
            case ConstTorque:setTorque(0.0f);break;
            case ConstSpeed:brake_mode = 1;setVelocity(0.0f);break;
            case ConstPosition:brake_mode = 1;break;
            //速度和位置模式缓停需要单独处理，未完成
//            case ConstSpeed:setSpeed(0.0f);break;
//            case ConstPosition:setPosition(0.0f);break;
            }
        }
        //stop motor immediately
        void stop(int key = 0)
        {
            if (key == 0x00ACCE55)
            {
                //usually used for emergency stop
            }
        }
        
        void PosSensor()
        {
            //位置解算
            state.pcnt = penc->getposcnt();
            state.P = penc->getpos();
            state.S = penc->getvel_rps();
            state.stamp_us = us();
        }
        void VISensor()
        {
            //电流，电压采集
            float i_temp = gi *(raw_current*kadc - midoffset);
            i = i*lpf_a + (1.0f - lpf_a)*i_temp;
            state.V = 0;
            state.A = i;
            state.stamp_us = us();
        }
        void MiscSensor()
        {
        }
        
        void position_ctrl(const VASP& state)
        {
            if (!loop_cr.ploop_en)
                return;
            if (brake_mode)
            {
                ploop.reset();
                vloop.set_target(0.0f);
                if (abs(state.V) <= quasi_static_thres)
                    brake_mode = 0;
            }
            else
            {
                //float记录位置可能精度不够
                 float velocity = ploop.compute(state.P);
                vloop.set_target(velocity);
            }
        }
        void velocity_ctrl(const VASP& state)
        {
            if (!loop_cr.vloop_en)
                return;
            if (brake_mode)
            {
                vloop.reset();
                iloop.set_target(0.0f);
            }
            else
            {
                float torque = vloop.compute(state.S);
                iloop.set_target(torque);
            }
        }
        void current_ctrl(const VASP& state)
        {
            if (!loop_cr.iloop_en)
            {
                voltage_ctrl(volt_set);
                return;
            }
            if (brake_mode)
            {
                iloop.reset();
                voltage_ctrl(0.0f);
            }
            else
            {
                float volt = iloop.compute(state.A);
                voltage_ctrl(volt);
            }
        }
        void voltage_ctrl(const float volt)
        {
            if (brake_mode)
            {
                volt_set = 0;
                pwm_update(0);
                if (abs(state.V) <= quasi_static_thres)
                {
                    quasi_static = true;
                    brake_mode = 0;
                }
            }
            else
            {
                volt_set = volt;
                int dutycnt = std::round(volt / vbus*(float)mcpwm_cnt);
                dutycnt = std::clamp(dutycnt, -mcpwm_cnt + 10, mcpwm_cnt - 10);
                pwm_update(dutycnt);
            }
        }
            
        MotorConfStruct mconf;
        pidclass iloop, vloop, ploop;
        VASP state;
    private:
        //current,velocity,position
        uint16_t raw_current;
        union
        {
            struct
            {
                uint8_t ctrl_mode : 4;
                uint8_t iloop_en : 1;
                uint8_t vloop_en : 1;
                uint8_t ploop_en : 1;
            };
            uint8_t reg;
        }loop_cr;
        
        float volt_set = 0.0f;
        uint8_t quasi_static;//almost stop,really slow
        uint8_t brake_mode;//0->no brake,1->passive brake,2->active brake,3->active and mechanical brake)
        float mcpwm_hz, mcpwm_s;
        int mcpwm_cnt;
        float vbus = 10.0f;
        float quasi_static_thres =1.0f;
        uint8_t pwm_mode;//0->pwm1&pwm2,1->Phase&Enable
        TIM_HandleTypeDef *htim;
        ADC_HandleTypeDef *hadc;
        QuadratureEncoder* penc;
        uint8_t tim_cha = TIM_CHANNEL_2, tim_chb = TIM_CHANNEL_1;//根据线序切换
        
        void pwm_update(int cnt)
        {
            int32_t ccr = abs(cnt);
            switch (pwm_mode)
            {
            case 0:
                {
                    if (cnt >= 0)
                    {
                        __HAL_TIM_SET_COMPARE(htim, tim_cha, ccr);
                        __HAL_TIM_SET_COMPARE(htim, tim_chb, 0);
                    }
                    else
                    {
                        __HAL_TIM_SET_COMPARE(htim, tim_cha, 0);
                        __HAL_TIM_SET_COMPARE(htim, tim_chb, ccr);
                    }
                    
                }break;
            case 1:
                {
                    //使用普通GPIO可能比较难保证方向切换时的时序，尽量使用PWM驱动GPIO
                    if (cnt >= 0)
                    {
                        //gpio set
                        __HAL_TIM_SET_COMPARE(htim, tim_cha, ccr);
                    }
                    else
                    {
                        //gpio reset
                        __HAL_TIM_SET_COMPARE(htim, tim_cha, ccr);
                    }
                }break;
            default:
                break;
            }
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (uint16_t)(ccr*0.7f+mcpwm_cnt*0.3f));
        }
        void lock()
        {
            //disable isr
        }
        void unlock()
        {
            //enable isr
        }
    };
    
    
    //仿真用虚拟电机
    class VirtualBDCM
    {
    public:
        VirtualBDCM(const MotorConfStruct& conf)
            : mconf(conf)
        {
        }
        
        ~VirtualBDCM()
        {
        }

        float getShuntCurrent();
        float getShuntVoltage();
        
        void applyPWMVoltage();
        
        // run@pwm frequency
        void stateUpdate()
        {
            
        }
        
        MotorConfStruct mconf;
        VASP state = { 0, 0, 0, 0, 0, 0 };
        
        //default value:RM35 Brushed DC motor
        float L = 120E-6f, R = 1.5f;
        float J = 0.5f * 0.1f * 0.01f * 0.01f;// 无直接依据，假设等效0.1kg@1cm
        float b = 2.23E-6f;// 无直接依据，从空载电流推算
        float kt = 3.53E-3f;// Nm/A
        float ke = 0.219f;// V/rps
        float pwm_hz = 20'000.0f, Ts = 1 / pwm_hz;
    private:
    };


    namespace Test
    {
        int32_t ticks[6];
        MotorConfStruct mconf =  { 
            .MechanicalParam={.GearRatio=1.0f},
            .VoltageParam={.MaxVolt=12.0f},
            .CurrentParam={.MaxAmp=1.0f},
            .VelocityParam={.MaxSpeed=100.0f},
//            .FeedbackFreq = 10'000,
//            .FeedbackPeriod = 1 / mconf.FeedbackFreq,
//            .RevolutionQuantums = 1000,
//            .MaxTorque = 0.1f,
            //TODO:BDCM测试参数
            //...
            };
        QuadratureEncoder encoder(2000);
        BDCM motor(mconf);

        void currentsample_callback()
        {
            ticks[0] = htim1.Instance->CNT;
            //假设位置测量和电流测量同时更新
            motor.VISensor();
            ticks[1] = htim1.Instance->CNT;
            motor.PosSensor();
            ticks[2] = htim1.Instance->CNT;
            
            encoder.update(QuadEncInfo(htim4.Instance->CNT, us()));
            ticks[3] = htim1.Instance->CNT;
            motor.current_ctrl(motor.state);
            ticks[4] = htim1.Instance->CNT;
        }
        
        void supervisor_callback()
        {
            motor.MiscSensor();
        }
            
        void motionctrl_callback(void*)
        {
            static uint32_t cnt = 0;
            encoder.update_speed();
            
            //ensure motor.state update normally
            if (cnt == 0)
            {
                motor.position_ctrl(motor.state);
            }
            motor.velocity_ctrl(motor.state);
            cnt = (cnt + 1) % 10;
        }
        
        extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
        {
            if (hadc == &hadc1)
            {
                currentsample_callback();
            }
        }
        
        int test()
        {
            HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
            
            motor.init(&htim1, &hadc1,&encoder);
            
            //当前版本的旧HAL库还不支持注册中断回调函数，故未使用PWM中断
            Ticker ticker1,ticker2;
//            ticker1.attach(currentsample_callback, 1 / 20'000.0f);
            ticker2.attach(motionctrl_callback, 1 / 1000.0f);  
            int ticks = 3000;
            
            //电压模式
            motor.setControlMode(ConstVoltage);
            motor.setVoltage(motor.mconf.VoltageParam.MaxVolt * 0.10f);
            delayms(ticks);
            motor.slowdown();//slow down
            delayms(ticks / 2);

            //力矩模式注意安全
            motor.iloop.set_gain(1.0f, 500.0f, 0.0f);
            motor.iloop.set_max(5.0f, 9.0f, 0.0f);
            motor.iloop.set_umax(9.0f);
            //motor.iloop.set_lpf_coef();//not used
            motor.iloop.set_ts(1.0f/MCPWM_HZ);
            motor.setControlMode(ConstTorque);
            motor.setTorque(motor.mconf.CurrentParam.MaxAmp * 0.08f);//默认输出最大力矩的8%
            delayms(ticks);
            motor.slowdown();
            delayms(ticks / 2);

            //速度模式
            motor.vloop.set_gain(0.1f, 1.0f, 0.0f);
            motor.vloop.set_max(0.3f, 0.3f, 0.0f);
            motor.vloop.set_umax(0.3f);
            motor.vloop.set_ts(0.001f);
            motor.setControlMode(ConstSpeed);
            motor.setVelocity(motor.mconf.VelocityParam.MaxSpeed * 0.10f);
            delayms(ticks);
            motor.slowdown();
            delayms(ticks / 2);
            
            //保持当前位置不动
            motor.ploop.set_gain(10.0f, 5.0f, 0.0f);
            motor.ploop.set_max(10.0f, 2.0f, 0.0f);
            motor.ploop.set_umax(10.0f);
            motor.ploop.set_ts(0.01f);
            motor.setControlMode(ConstPosition);
            motor.setPosition(motor.state.P);
            delayms(ticks);
            for (;;)
            {
                delayms(ticks / 5);
                HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
            }
            motor.slowdown();
            delayms(ticks / 2);

            //安全停止，切换到电压模式并置零输出，绝对不会跑飞。
            //如果需要保持位置请改成位置控制，前提是控制环路稳定
            motor.setControlMode(ConstVoltage);
            motor.setVoltage(0.0f);
            motor.slowdown();

            return 0;
        }
    }
    
}

