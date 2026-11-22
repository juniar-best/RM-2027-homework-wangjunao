#include "Servo.h"
#include "tim.h"

#define SERVO_TIM       (&htim2)
#define SERVO_CHANNEL   TIM_CHANNEL_1

static uint16_t lastCcr = 0;

void Servo_Init(void)
{
  if (HAL_TIM_PWM_Start(SERVO_TIM, SERVO_CHANNEL) != HAL_OK)
  {
    Error_Handler();
  }
}

uint16_t Servo_AngleToCCR(float angle)
{
  float clampedAngle = angle;
  float pulseUs;

  if (clampedAngle < SERVO_ANGLE_MIN) clampedAngle = SERVO_ANGLE_MIN;
  if (clampedAngle > SERVO_ANGLE_MAX) clampedAngle = SERVO_ANGLE_MAX;

  pulseUs = SERVO_MIN_US
          + clampedAngle / SERVO_ANGLE_MAX * (SERVO_MAX_US - SERVO_MIN_US);

  if (pulseUs < SERVO_SAFE_MIN_US) pulseUs = SERVO_SAFE_MIN_US;
  if (pulseUs > SERVO_SAFE_MAX_US) pulseUs = SERVO_SAFE_MAX_US;

  return (uint16_t)(pulseUs + 0.5f);
}

void Servo_SetAngle(float angle)
{
  lastCcr = Servo_AngleToCCR(angle);
  __HAL_TIM_SET_COMPARE(SERVO_TIM, SERVO_CHANNEL, lastCcr);
}

uint16_t Servo_GetCCR(void)
{
  return lastCcr;
}
