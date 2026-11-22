/**
  ******************************************************************************
  * @file    Servo.h
  * @brief   SG90（180° 型）舵机驱动，TIM2_CH1 输出 50Hz PWM
  ******************************************************************************
  * 时基前提（见 tim.c 的 MX_TIM2_Init）：
  *   TIM2CLK = 72MHz，PSC = 71  -> 计数频率 1MHz，1 个计数 = 1us
  *   ARR     = 19999            -> 周期 20000us = 20ms，即 50Hz
  * 因此写进 CCR1 的数值，单位就是微秒，CCR 数值 = 脉宽(us)。
  ******************************************************************************
  */

#ifndef __SERVO_H
#define __SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 脉宽区间：SG90 名义行程 500~2500us 对应 0~180°，
 * 与江协科技教程的 (Angle / 180 * 2000 + 500) 完全一致。 */
#define SERVO_MIN_US       500.0f
#define SERVO_MAX_US      2500.0f

/* 机械限位保护：真实 SG90 行程普遍比标称窄一点，打满会顶限位、堵转发热。
 * 换算出来的脉宽最后再夹到 550~2450us（对应名义 4.5°~175.5°），
 * 这样 0° 和 180° 仍然可以下指令，但实际不会顶到底。 */
#define SERVO_SAFE_MIN_US   550.0f
#define SERVO_SAFE_MAX_US  2450.0f

/* 角度区间 */
#define SERVO_ANGLE_MIN      0.0f
#define SERVO_ANGLE_MAX    180.0f

void     Servo_Init(void);
void     Servo_SetAngle(float angle);
uint16_t Servo_AngleToCCR(float angle);
uint16_t Servo_GetCCR(void);

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H */
