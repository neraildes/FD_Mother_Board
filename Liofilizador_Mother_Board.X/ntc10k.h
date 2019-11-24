/*-----------------------------------------------------------------------------/
 * File:   ntc10k.h                                                            /
 * Author: Neraildes                                                           /
 * Revis�o 1.0.0                                                               /
 * Created on 19 de Abril de 2018, 07:38                                       /
 * Descri��o:                                                                  /
 *                Leitura de sensor PT100 em entradas anal�gicas do PIC.                 /
 -----------------------------------------------------------------------------*/



#ifndef NTC10K_H
#define        NTC10K_H

#define BALANCE_RESISTOR  10000.0
#define MAX_ADC  1023.0
#define BETA     3974.0
#define ROOM_TEMP  301.05 // room temperature in Kelvin
#define RESISTOR_ROOM_TEMP  10000.0

double NTC10K_Read_Temperature(unsigned char canal);

#endif        /* NTC10K_H */