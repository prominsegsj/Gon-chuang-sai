#ifndef __TASK_WORK_H__
#define __TASK_WORK_H__

#define Front 1
#define Back  2
#define Left  3
#define Right 4
#define Turn_L  5
#define Turn_L1  6
#define Turn_L2  7
#define Turn_L3  8
#define Turn_R  9
#define Turn_R1  10
#define Turn_R2  11
#define Turn_R3  12

void Set_Go(uint8_t Order,float Pos,uint8_t Angle,uint16_t V,uint8_t Acc);
void AD_Power(void);
void Contl_Servo(uint8_t command);

void Usart2WriteBuf(uint8_t *buf, uint8_t len);
void runActionGroup(uint8_t numOfAction, uint16_t Times);
uint8_t OpenMv_Part(uint8_t Command);
void Ring_Adjust(void);

void Try(uint8_t num);
void TRY(uint8_t i);
void Run();

#endif
