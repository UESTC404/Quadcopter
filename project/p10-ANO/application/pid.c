#include "pid.h"
#include "PWM-RCV.h"
#include "motor-PWM.h"

#define max_angle_pr 20
#define max_angle_yaw 180
#define max_reaction 60 /// range_pwm 映射限定参数 //// 等待讨论

PID_Typedef pitch_angle_PID;    // pitch角度PID
PID_Typedef roll_angle_PID;
PID_Typedef yaw_angle_PID;

FLOAT_ANGLE EXP_ANGLE;         // 期望角度
extern float yaw, pitch, roll; // 观测角度，解算出来的

/**
    * @brief  PID初始化
    * @param  None
    * @retval None
    */

void PID_init(void)
{
    pitch_angle_PID.P = 1;
    pitch_angle_PID.I = 0;
    pitch_angle_PID.D = 0;

    roll_angle_PID.P = 1;
    roll_angle_PID.I = 0;
    roll_angle_PID.D = 0;

    yaw_angle_PID.P = 1;
    yaw_angle_PID.I = 0;
    yaw_angle_PID.D = 0;
}

/**
    * @brief  位置式PID：输入期望值、观测值，得到输出值
    * @param  PID_Typedef * PID： PID结构体
              float target：      期望值
              float measure：     观测值
              int32_t dertT：     时间间隔，用于计算积分
    * @retval None
    */
static void PID_postion_cal(PID_Typedef * PID, float target, float measure, int32_t dertT)
{
    // 计算误差 = 期望值 - 观测值
    PID->Error = target - measure;
    // 计算积分
    PID->Integ += (double)PID->Error * dertT / 1000000.0;
    // 计算微分
    PID->Deriv = PID->Error - PID->PreError;
    // 计算输出
    PID->Output = PID->P * PID->Error + PID->I * PID->Integ + PID->D * PID->Deriv;
    // 保留上一次误差
    PID->PreError = PID->Error;
}

static float range_trans(uint16_t Rcvr_ch, uint16_t max_angle)
{
    // 此函数要求通道捕获值为1000～2000
    if (Rcvr_ch<=1000) Rcvr_ch = 1001;
    if (Rcvr_ch>=2000) Rcvr_ch = 1999;
    return (Rcvr_ch-1500)/500.f*max_angle;
}

static inline uint16_t range_pwm(float motor_pwm, uint16_t thr)
{
    if (thr<=1000) thr = 1001;
    if (thr>=2000) thr = 1999;
    if (motor_pwm>max_reaction) motor_pwm = max_reaction;
    if (motor_pwm<-max_reaction) motor_pwm = -max_reaction;
    // 1000..thr
    return 1000 + (motor_pwm/max_reaction+1)/2 * (thr-1000);
}

void PID_calculate(void)
{

    // 从遥控接收机获取期望值
    EXP_ANGLE.Roll  = range_trans(u16Rcvr_ch1, max_angle_pr);       // f(u16Rcvr_ch1) +
    EXP_ANGLE.Pitch = range_trans(u16Rcvr_ch2, max_angle_pr);       // f(u16Rcvr_ch2) +
    EXP_ANGLE.Yaw   = 0; // range_trans(3000-u16Rcvr_ch4, max_angle_yaw); // f(u16Rcvr_ch4) -
                                     // !!!! // 假通道，没有需求

    PID_postion_cal(&roll_angle_PID,  EXP_ANGLE.Roll,  roll,  0); //要求平稳飞行 实际角度很小，0代表没有积分
    PID_postion_cal(&pitch_angle_PID, EXP_ANGLE.Pitch, pitch, 0);
    PID_postion_cal(&yaw_angle_PID,   EXP_ANGLE.Yaw,   yaw,   0);

    // 获取输出值
    uint16_t Thr = u16Rcvr_ch3; /// 不用转换量程，因为通道捕获和电调输出都是1000～2000
    float Pitch  = pitch_angle_PID.Output;
    float Roll   = roll_angle_PID.Output;
    float Yaw    = yaw_angle_PID.Output;

    // 输出值融合到四个电机
    motor_pwm_1 = range_pwm( - Pitch - Roll + Yaw, Thr);
    motor_pwm_2 = range_pwm( + Pitch - Roll - Yaw, Thr);
    motor_pwm_3 = range_pwm( + Pitch + Roll + Yaw, Thr);
    motor_pwm_4 = range_pwm( - Pitch + Roll - Yaw, Thr);
}
