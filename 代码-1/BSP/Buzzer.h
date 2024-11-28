#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "sys.h"

#define Beep PBout(5)
#define BEEP Beep_ON

void Beep_Init(void);
void Beep_ON(void);

#endif
