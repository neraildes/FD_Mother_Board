/*-----------------------------------------------------------------------------/
 * File:   isr.h
 * Author: Neraildes
 * Revis�o 1.0.0
 * Created on 17 de Outubro de 2017, 06:38
 * Descri��o:
 *                Interrup��es do DipCoating
 -----------------------------------------------------------------------------*/

#ifndef ISR_H
#define ISR_H

#include "proculus.h"

//void __interrupt(high_priority) isrH(void);
void __interrupt(low_priority)  isr(void);




#endif        /* ISR_H */