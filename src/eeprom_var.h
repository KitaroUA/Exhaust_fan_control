#ifndef eeprom_var
#define eerpom_var
#include <Arduino.h>

typedef struct {
    byte mode1;                 // автоматичний - 0, ручний - 1
    byte mode2;                 // для ручного режиму, один слайдер керує обома кулерами - 0, незалежне керування - 1  
    int  slyder1value;          // значення першого слайдера (в режимі спільного керування - керує обома кулерами)  
//    int  slyder2value;          // --//-- другого
    int mintemp1;               // для авт. режиму CPU1 - мін значення темп. керування*10

} eeprom_var;












#endif