/*------------------------------------------------------------------------------
 * LIOFILIZADOR JJ CIENTIFICA
 * DATA 05/01/2019
 * COMPILADOR XC8 Free
 * NIVEL DE OTIMIZA��O  2
 * 
------------------------------------------------------------------------------*/

#include <xc.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "global.h"
#include "Liofilizador Placa Mae.h"
#include "isr.h"
#include "proculus.h"
#include "usart.h"
#include "I2C.h"
#include "adc.h"
#include "timedate.h"
#include "util.h"
#include "protocolo.h"
#include "eeprom.h"
#include "EEPROM_24C1025.h"
#include "voltagem.h"
#include "versao.h"

#define OFFSET_EEPROM       52     //Deslocamento para deixar os primeiros bytes livres (10)
#define TUPLA_EEPROM_SIZE   18     //Tamanho maximo de uma tupla na eeprom (18)
#define TUPLA_VP_SIZE       12     //Tamanho maximo de uma tupla no display(12)

#define CLK at PORTCbits.RD3       //
#define DTA at PORTCbits.RD4       //

#define FORMAT 0  //
#define LOAD   1  // Constantes do TrendCurveFuncao
#define SAVE   2  //

#define PPCANAL 1789
#define PPCOR   1787

#define FATOR_PADRAO 1.0
#define FATOR_TENSAO 1.0 //0.4546
#define FATOR_VACUO  1.0 //0.05

//------------------------------------------------------------------------------
const char *boardtype[5]={"Mother Board",
                          "Vaccum Board",
                          "PT100 Board ",
                          "NTC Board   ",
                          "Relay_Board "}; 

#define Mother_Board 0
#define Vaccum_Board 1
#define PT100_Board  2
#define NTC_Board    3 
#define Relay_Board  4
//------------------------------------------------------------------------------

/*----------------------------------------------------------------------------*/
volatile unsigned char usart_buffer[USART_BUFFER_SIZE]; 
volatile unsigned char usart_buffer_fila[USART_LINE_BUFFER_SIZE][USART_BUFFER_SIZE];


//-----------------timer1-----------------------
//volatile unsigned int  milisegundo ;
//volatile unsigned char segundo     ;
//volatile unsigned char minuto      ;
//volatile unsigned char hora        ;

volatile unsigned int  tempodecorrido        ;
volatile unsigned int  tempocaptura          ; //variavel de captura de dados para memoria datalog
volatile unsigned int  tempocapturaconstante ; //variavel de memoria

volatile unsigned char processo_segundo ;
volatile unsigned char processo_minuto ;
volatile unsigned char processo_hora   ;

//==============================================================================

volatile unsigned int tmr_led_memory;
volatile unsigned int Delay_Led_Memory;

volatile unsigned int tmr_led_usart;
volatile unsigned int Delay_Led_Usart;

volatile unsigned int tmr_led_tmr0;
volatile unsigned int Delay_Led_Tmr0;


//------------------------------------------------------------------------------

volatile unsigned int delaycheckscreen;
volatile signed int senhacount;
volatile unsigned int delay_update_display;

//------------------------------------------------------------------------------

t_usart_protocol usart_protocol;
t_proculus       proculus;
t_liofilizador   liofilizador[10];
t_receita        receita;
t_fat            processo;

//------------------------------------------------------------------------------

char  texto[74];
char  buffer[74];

//------------------------------------------------------------------------------

char maincnt;
unsigned int  pagina=15;
unsigned int  paginamemo=15;


//------------------------------------------------------------------------------



//------------------------------------------------------------------------------





volatile unsigned char flag_senha_global_liberada;
volatile unsigned char flag_senha_liberada;
volatile unsigned char totalboard;
volatile signed int senhacount;
unsigned long senha_atual;
char senhavetor[4];


int Condensador     ;       
int Vacuometro      ;
int Voltimetro      ;

int Seg_Condensador ;
int Seg_Vacuo       ;
int Seg_Aq_cond     ;
int Seg_Aq_vacuo    ;
int leitura[0x0F]   ;


char Index_Receita;
char Index_Sel_Rec;


unsigned long add_datalog;

char returnToScreen;

volatile char MonitorBuffer;

//char trendCurve[15];
//char trendChannel[15];
char trendExist;
int  trendvp=0x0310;
char icone ; 


unsigned char nabucodonozor=0;

unsigned int vpPrint=2000;

volatile unsigned char flag_array_slave_WDT[15];

unsigned char memo_statuspower;

volatile unsigned char delay_condensador;

int index;

T_mapa mapa;


void main(void) 
{
     my_delay_ms_CLRWDT(100);
     
     OSCCONbits.IRCF0=1; //Frequencia do oscilador interno = 8Mhz
     OSCCONbits.IRCF1=1;
     OSCCONbits.IRCF2=1;
     OSCTUNEbits.PLLEN=1; //pll 8*4 = 32Mhz

     TRISA=0b11101111;    //Inicializa Portas
     TRISB=0b00000001;
     TRISC=0b11110000;
     TRISD=0b11100011;
     TRISE=0xFF;

     
     
     //-------------------------------------------------------------------------
     T0CONbits.T0CS  = 0;
     T0CONbits.PSA   = 0;
     T0CONbits.T0PS1 = 1;
     T0CONbits.T0PS1 = 1;
     T0CONbits.T0PS2 = 1;      
     INTCON2bits.TMR0IP =0;  //Timer zero, interrup��o em baixa prioridade
     INTCONbits.T0IE    =1;  //Timer0 habilitado
     
     
     //-------------------------------------------------------------------------
     RBIF=0;
     INTCONbits.INT0IE   =0;  //Interrup��o em RB0
     
     //-------------------------------------------------------------------------
     PIR1bits.TMR1IF = 0;   // Reset the TMR1IF flag bit
     IPR1bits.TMR1IP=0;     //Timer 1 com interrup��o de baixa prioridade     
     TMR1H = 0x0B;          // Set initial value for the timer TMR1
     TMR1L = 0xBA;
     TMR1CS = 0;            // Timer1 counts pulses from internal oscillator
     T1CKPS0 = 1;           // Assigned prescaler rate is 1:8 
     T1CKPS1 = 1; 
     T1RUN=1;
     T1OSCEN=1;
     PIE1bits.TMR1IE=1;      // Enable interrupt flag_time_process          
     TMR1ON = 1;             // Turn the timer TMR1 on 
     
     //-------------------------------------------------------------------------
     //IPEN=1;  //Habilita prioridade na interrup��o.    
     
     //-------------------------------------------------------------------------
     INTCONbits.PEIE    =1;  //Habilita Interrup��o em Perifericos  
     
     //-------------------------------------------------------------------------    
     //INTCONbits.GIEL     =1;  //Global Interrupt                           
     //INTCONbits.GIEH     =1;  //Global Interrupt  
     INTCONbits.GIE        =1;  //Global Interrupt 
     //=========================================================================


     
     //=========================================================================

     Delay_Led_Tmr0=0;
     flag_led_tmr0=0;
     Delay_Led_Usart=0;
     flag_led_usart=0;
     Delay_Led_Memory=0;
     flag_led_memory=0; 
     

     //--------------------- INICIALIZA PERIFERICOS-----------------------------
     USART_init(115200);
     My_ADC_init();
     I2C_Master_Init(100000);
     my_delay_ms_CLRWDT(500);
     
    //--------------------------------------------------------------------------
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     /*
     while(1)
          {
             asm("CLRWDT");
             //------------INTERPRETA COMANDO DO MICROCOMPUTADOR--------------------                 
             if(flag_usart_rx)
                { 
                 flag_usart_rx=0;
                 USART_put_int(0x1234);
                //Comando_Protocolo_Serial(); 
                }                    

             //----------------INTERPRETA COMANDO DO DISPLAY----------------
             if(flag_usart_rx)
                {
                flag_usart_rx=0; 
                USART_put_int(0x5678);
                //Comando_Display();
                }//flag_usart_rx           
          }
     
     */
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     //------------------------------------------------------------------------- 
     statuspower.bits=EEPROM_Read_Byte(16);
     if(statuspower.bits==0) 
       {
       clear_screen();
       PROCULUS_Show_Screen(0);   
       }  
         
     
     
     {//-----------TOTALIZADOR DE RESET-------------         
     unsigned int reset;    
     reset=EEPROM_Read_Integer(34);
     if(reset==0xFFFF)
       {  
       EEPROM_Write_Integer(34,0);   //Totalizador de Reset
       EEPROM_Write_Integer(0xE8,0); //Totalizador de repique do vacuo;
       }
     reset++;
     EEPROM_Write_Integer(34,reset);
     flag_Vacuo_estava_ligado=0;
     //PROCULUS_VP_Write_UInt16(0x6BBB,reset);
     }
     
     
     
     
     //---------------------------Tempo para Update-----------------------------
#if ((defined __18F4620) || (defined __18F4525))     
     INTCONbits.GIE=0;
     POWER_ON();
     UPDATE_ENABLE();
     LATB=255;
     //my_delay_ms_CLRWDT(3000); 
     UPDATE_DISABLE();
     LATB=0;
     INTCONbits.GIE=1;
#endif     
     
     //-----------------------------------------------------------------------
   
     
     //======================== INFORMA��ES INICIAIS ===========================
     my_delay_ms_CLRWDT(300); 
     print("JJ Cientifica Ind. e Com. de Eq. Cientificos.");     
     my_delay_ms_CLRWDT(300);
     print("Inicializando o Sistema...");
     my_delay_ms_CLRWDT(300);
     print("Analisando Hardware. Aguarde...");
     my_delay_ms_CLRWDT(300);
     PROCULUS_VP_Read_String(1990, buffer);
     strcpy(texto,"* : Display      ");
     strcat(texto,buffer);
     print(texto);
     my_delay_ms_CLRWDT(300);

     
     {
     char i;    
     ShowHardwareInfo();
     
     my_delay_ms_CLRWDT(1000);     
     PROCULUS_VP_Write_UInt16(0x02,0);  //Valor inicial do botao Datalog
     PROCULUS_VP_Write_UInt16(0x03,0);  //Valor inicial do botao Condensador
     PROCULUS_VP_Write_UInt16(0x04,0);  //Valor inicial do botao Vacuo
     PROCULUS_VP_Write_UInt16(0x05,0);  //Valor inicial do botao Aquecimento Global

     flag_global_datalog=0;
     flag_global_condensador=0;
     flag_global_vacuo=0;     
     flag_global_aquecimento=0; 
     Contagem_Tempo_de_Processo(FALSE);
     
     //-------------------------------------------------------------------------
     if(EEPROM_Read_Byte(OFFSET_EEPROM)==0xFF)
       {         
       print("Formatando memoria principal..."); 
       Formatar_Banco_de_Dados(0,10);
       Formatar_Lista_de_Receitas();
       Formatar_Dados_de_Seguranca();
       Formatar_Datalog();
       flag_senha_global_liberada=FALSE;
       flag_senha_liberada=FALSE;
       Gravar_Status_da_Senha_Global();
       EEPROM_Write_Byte(16,0);// statuspower (byte de status)
       EEPROM_Write_Integer(0x09,10); // Valor inicial do tempo de captura de log
       EEPROM_Write_Long32(11,123456);//Valor inicial da senha do administrador  
       TrendCurveFuncao(FORMAT); 
       EEPROM_Write_Byte(17,0);//processo_Hora
       EEPROM_Write_Byte(18,0);//processo_Minuto       
       }   
     RecallBlackoutStatus();
     TrendCurveFuncao(LOAD);
     

     //-------------------------------------------------------------------------
     if(statuspower.bits!=0)
        { 
        print("Cond. de blackout encontrada!");
        print("Aguardande...");
        my_delay_ms_CLRWDT(15000);
        print("Concluido.");
        }
     //------------Valores Iniciais da tela Principal---------------------------
     print("Analisando dados...");  
     for(i=0;i<15;i++)
        {
        asm("CLRWDT"); 
        ShowStaticValueGrid(i);
        }    
     //-----------------------Carrega valores de sensores-----------------------
     
     
     
     
     //------------------------Carrega Lista de Receita-------------------------
     for(i=0;i<8;i++)
         {
         asm("CLRWDT");
         Exibe_Receita(i);
         }
     //-------------------------------------------------------------------------
     senha_atual=EEPROM_Read_Long32(11);
     //-------------------------Carrega Status de Senha-------------------------
     Carregar_Status_da_Senha_Global();     
     }
     
    //==========================================================================
     ShowSensorRealTimeHS();
     /*-------------------------------------------------------------------------
                          INICIALIZA�AO DOS PARAMETROS
     -------------------------------------------------------------------------a*/
     Carregar_Parametros_de_Seguranca();
     Carregar_tempo_de_datalog();
     //-------------------------------------------------------------------------

     
     //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     //                       M  A  I  N      L  O  O  P
     //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 
     Exibe_Tempo_de_Processo();
     Icones_de_alarmes();        
     if(statuspower.bits==0)
       {
       PROCULUS_Show_Screen(15);       
       }  
     else
       {
       print("Iniciando Liofilizador.");
       asm("CLRWDT");
       global_condensador();
       my_delay_ms(10000);
       global_vacuo();
       my_delay_ms(10000);
       global_aquecimento();
       PROCULUS_Show_Screen(15); 
       }  
     
     pagina=0;
     paginamemo=0;
     maincnt=0;     
     delaycheckscreen=0;     
     MonitorBuffer=0;
     //-------------------------
     Vacuometro=0;
     Voltimetro=0;
     Condensador=0;

     //--------timer 1----------
     Exibe_Hora_Data(FALSE);
     rtc.milisegundo=0;
     rtc.segundo=0;
     processo_hora=EEPROM_Read_Byte(17);
     processo_minuto=EEPROM_Read_Byte(18);
     processo_segundo=0;
     memo_statuspower=statuspower.bits;
     delay_condensador=0;
     
     //=========================================================================
     //                              M A I N
     //=========================================================================

     
        
        while(1)
             {
             flag_main_loop_WDT=TRUE;
             
             Buffer_Manager(); // Buffer de recep��o de dados seriais [10]
                  
             //=========================SELECAO DE PAGINA============================
             if(delaycheckscreen>1000)
               {
               delaycheckscreen=0;      
               pagina = Captura_Pagina();
               if(pagina!=paginamemo)
                  {
                  paginamemo=pagina;                  
                  }//pagina!= 
               }              
             
             //-----------------------------------------------------------------

                if(rtc.milisegundo<2) if(pagina!=25) Exibe_Hora_Data(FALSE); //Exibe data e hora sem segundos
                if(flag_time_process==TRUE) SaveBlackoutStatusRuning(); //Salva status e tempo de processo a cada 10 minutos
                Exibe_Tempo_de_Processo();
                Icones_de_alarmes();    

                Gerenciador_de_Senha();  //Habilita acesso global por 30 segundos
                Gerenciador_de_Senha_Global(); //Libera Senha Global eternamente                       

                global_datalog(); // LEITURA DOS SENSORES
                __delay_ms(32);                 
                global_vacuo();
                __delay_ms(32);
                global_condensador();
                __delay_ms(32);
                global_aquecimento();
                __delay_ms(32);

                //BILD Ligar no SAC para Taisa.
                if(memo_statuspower!=statuspower.bits) 
                  { 
                  PROCULUS_OK();  
                  SaveBlackoutStatus();
                  memo_statuspower=statuspower.bits;
                  }                          


                if((processo_segundo==0) || (processo_segundo==30))
                    { 
                    if(flag_wakeup==1)
                       {
                       flag_wakeup=0; 
                       AcordaFilha(); 
                       }        
                    }
                else
                    {
                    flag_wakeup=1;
                    }                  
                Check_And_Send_Capture_Datalog();
                //------------------CODIGO RAPIDO---------------------
                ShowSensorRealTimeHS();                          
                //----------------------------------------------------  
                showTotalReset();

                          


            //=================AJUSTA O CONTADOR DE ACORDO COM A P�GINA=============
            switch(pagina)
                  {                                          
                  //case 15: //---------------- PAGINA PRINCIPAL----------------------                            
                           //ShowStaticValueGrid(maincnt);   //Atualiza tudo                    
                           //maincnt++;
                           //if(maincnt>=15) maincnt=0; 
                                                      
                           //break;
                  case 19: //------------PAGINA PARA SETAR AQUECIMENTO--------------
                  case 21: //Exibe somente os 5 primeiros dados da tupla
                           //Permite rolagem                           
                           //---------------------------------------------------
                           for(char i=0; i<10; i++)
                               {
                               //flag_main_loop_WDT=1;
                               if(PROCULUS_VP_Read_UInt16(7+i)==1)
                                  { 
                                  PROCULUS_VP_Write_UInt16(7+i,0);
                                  Index_Sel_Rec=i;
                                  if(Index_Sel_Rec<=4)
                                     { 
                                     returnToScreen=19;
                                     PROCULUS_Show_Screen(27);
                                     }
                                  else
                                     { 
                                     returnToScreen=21; 
                                     PROCULUS_Show_Screen(28);                                     
                                     }
                                  }
                               }
                          pagina_19();
                          break;
                  case 23:// P�gina de par�metros de seguran�a                                       
                          if(PROCULUS_VP_Read_UInt16(1)==1)
                             { 
                             PROCULUS_VP_Write_UInt16(1,0);
                             pagina_23(); //Salva Parametros
                             }                                                            
                          break;                        

                  case 25: // Ajuste de data e hora
                         if(maincnt==0)
                            {
                            Exibe_Hora_Data(TRUE);
                            maincnt++;
                            }
                          if(PROCULUS_VP_Read_UInt16(152)==1)
                             { 
                             PROCULUS_VP_Write_UInt16(152,0);
                             Exibe_Hora_Data(TRUE);
                             }                                                     
                          if(PROCULUS_VP_Read_UInt16(174)==1)
                             { 
                             PROCULUS_VP_Write_UInt16(174,0);
                             pagina_25(); //Salva Data e Hora
                             } 
                          break; 
                  case 27:
                  case 28://Selecao de Receita- Pagina 1
                          for(Index_Receita=0;Index_Receita<8;Index_Receita++)        
                              {
                              if(PROCULUS_VP_Read_UInt16(0x0020+Index_Receita)==1) //Selecionou algo
                                 {
                                 PROCULUS_VP_Write_UInt16(0x0020+Index_Receita,0); //Desabilita pra n�o entrar de  
                                 PROCULUS_Show_Screen(returnToScreen);
                                 Carrega_Tupla_Receita(Index_Receita, &receita);
                                 Set_Receita(Index_Sel_Rec,TRUE); // Index_Sel_Rec                                 
                                 }                               
                              }                        

                              if(PROCULUS_VP_Read_UInt16(17)==1) //Menu ESC
                                 {
                                 PROCULUS_VP_Write_UInt16(17,0);
                                 PROCULUS_Show_Screen(returnToScreen);
                                 }   
                          
                              if(PROCULUS_VP_Read_UInt16(18)==1) //Menu Excluir
                                 {
                                 PROCULUS_VP_Write_UInt16(18,0);
                                 receita.histerese=0;
                                 strcpy(receita.nome,"");
                                 receita.potenciaOFF=0;
                                 receita.potenciaON=0;
                                 receita.setpoint=0;
                                 Set_Receita(Index_Sel_Rec,FALSE); // In
                                 PROCULUS_Show_Screen(returnToScreen);
                                 }                           
                          
                          break;

                  case 29: // Ajuste de tempo de captura de datalog
                          if(PROCULUS_VP_Read_UInt16(175)==1)
                             { 
                             PROCULUS_VP_Write_UInt16(175,0);
                             pagina_29(); 
                             }  
                          break;
                  case 31: //Troca de Senha
                          {
                          #define KEY 0xE4BA2F10 
                             
                          unsigned long dica;
                          
                          dica=~(senha_atual^KEY);
                          
                          ultoa(dica,texto,16);                          
                         
                          PROCULUS_VP_Write_String(1660,texto);
                          my_delay_ms_TMR1(500);
                          }
                          if(PROCULUS_VP_Read_UInt16(386)==1)
                             { 
                             PROCULUS_VP_Write_UInt16(386,0);
                             pagina_31(); 
                             }             
                          break;                          
                  case 35:// Seleciona cor da linha do grafico. Imprimindo cores correspondentes.
                            {   
                            int tv;                            
                            char total;
                            static int canal=0;
                            flag_proculus_hs=TRUE;
                            
                            /*
                            for(canal=0;canal<14;canal++)
                                PROCULUS_VP_Write_UInt16(1000+canal,*mapa.entrada[canal]*mapa.fator[canal]);      
                            */

                            for(trendvp=0x0310;trendvp<0x031D;trendvp++)
                                  {	
                                  icone=trendvp-0x0310;					  								  
                                  if(PROCULUS_VP_Read_UInt16(trendvp)==14)
                                          {
                                          if(flag_senha_liberada)
                                               {
                                               #ifdef Display_8_Polegadas
                                               canal=MenorCanalLivre();
                                               #else
                                               canal=icone;
                                               #endif

                                               if(canal<8)
                                                  { 											                                                                 
                                                  PROCULUS_VP_Write_UInt16(0x310+icone,icone+1); //Colore o quadrado com uma cor fixa                                           
                                                  PROCULUS_VP_Write_UInt16((canal*10+PPCANAL),(canal<<8)|(0x0001)); //Seta um canal para um dos 13 icones
                                                  PROCULUS_VP_Write_UInt16((canal*10+PPCOR),TrendColor[icone]);     //Seta uma cor de linha do grafico											                                             


                                                  mapa.canal[canal]=canal;   //Seleciona um Canal
                                                  mapa.icone[canal]=icone+1;              //Registra qual icone est� sendo tratado
                                                  mapa.vpIcone[icone]=icone+1;
                                                  mapa.cor[canal]=TrendColor[icone];
                                                  mapa.fator[canal]=1.0;
                                                  mapa.entrada[canal]=&leitura[saltaIndice4(icone)];    //Aponta para uma leitura

                                                  mapa.fator[canal]=FATOR_PADRAO;                  //Fator Padr�o

                                                  if(icone==0)mapa.fator[canal]=FATOR_TENSAO;      //Fator para Tens�o
                                                  if(icone==1)mapa.fator[canal]=FATOR_VACUO;      //Fator para Vacuo


                                                  //PROCULUS_Clear_Line_Graphic(icone);
                                                                                              //PROCULUS_VP_Write_UInt16(1000+canal,*mapa.entrada[canal]*mapa.fator[canal]);      
                                                  TrendCurveFuncao(SAVE); //Salva os icones acionados
                                                  } 
                                               else
                                                  {	
                                                  PROCULUS_VP_Write_UInt16((canal*10+PPCANAL),(canal<<8)|(0x0A00)); //Canal
                                                  PROCULUS_VP_Write_UInt16((canal*10+PPCOR),0xFFFF);//Cor                                               
                                                  PROCULUS_VP_Write_UInt16(trendvp,-1);  
                                                  }
                                               }
                                          else
                                               {
                                               PROCULUS_NOK();
                                               PROCULUS_VP_Write_UInt16(trendvp,-1);
                                               PROCULUS_Popup(ACESSO_NEGADO);
                                               }
                                          }
                                     else 
                                      if((PROCULUS_VP_Read_UInt16(trendvp)>=15)&&(PROCULUS_VP_Read_UInt16(trendvp)<=30))
                                          {
                                          if(flag_senha_liberada)
                                               {
                                               char canal_aleatorio, canal_sequencial;

                                               canal_sequencial=buscaIndex(mapa.icone,icone+1);
                                               canal_aleatorio=mapa.icone[icone]-1; 	                           //canal para lista aleatoria                                         

                                               PROCULUS_VP_Write_UInt16(trendvp,-1);                            //Apaga o quadrado colorido do display 
                                               PROCULUS_VP_Write_UInt16((canal_sequencial*10+PPCANAL),0x0A00);  //Libera o canal utilizado
                                               PROCULUS_VP_Write_UInt16((canal_sequencial*10+PPCOR),0xFFFF);               //Torna a cor padrao do canal em branco 

                                                                                       mapa.entrada[canal_sequencial]=NULL;                             //Torna a entrada do canal NULL  
                                               mapa.canal[canal_sequencial]=0X0A;                               //Seleciona um Canal
                                               mapa.icone[canal_sequencial]=-1;                                 //Registra qual icone est� sendo tratado
                                               mapa.vpIcone[icone]=-1;                                          //Desliga �cone aleatorio
                                               mapa.cor[canal_sequencial]=0xFFFF;                               //Desliga a Cor           
                                                                                   mapa.fator[canal_sequencial]=0.0;                                //Fator padr�o para Temperatura

                                                                                       //PROCULUS_Clear_Line_Graphic(canal+1);               //Apaga a Linha desenhada  										                      
                                               TrendCurveFuncao(SAVE);								//Salva as altera��es		                                  
                                               }
                                          else
                                               {
                                               PROCULUS_NOK();
                                               PROCULUS_Popup(ACESSO_NEGADO);
                                               #ifdef Display_8_Polegadas 
                                               PROCULUS_VP_Write_UInt16(trendvp,mapa.vpIcone[icone]);
                                               #else
                                               PROCULUS_VP_Write_UInt16(trendvp,mapa.icone[icone]);
                                               #endif                                            
                                               }
                                          }                                  
                                  }	
                                  
                                  
	


                            
                            flag_proculus_hs=FALSE;                            
                            break;
                            }
                  case 47: //Lista de Receitas
                          Atualizar_Lista_de_Receitas();

                          for(Index_Receita=0;Index_Receita<8;Index_Receita++)        
                              {
                              //flag_main_loop_WDT=1;
                              if(PROCULUS_VP_Read_UInt16(0x0020+Index_Receita)==1)
                                 {
                                 PROCULUS_VP_Write_UInt16(0x0020+Index_Receita,0);                                 
                                 pagina_47();                                 
                                 break;
                                 }                               
                              }                             
                                                     
                          break;  
                  case 49: //Edicao de receita
                          {
                          pagina_49();                    
                          }
                          break;
                  }//switch pagina

                 
            
                 __delay_ms(50);
                 USART_putc(0xCD);USART_putc(0xCD);USART_putc(0xCD);
                 USART_putc(0xCD);USART_putc(0xCD);
                 for(unsigned int tempo=0; tempo<500; tempo++)
                     {
                     if(flag_usart_rx==TRUE) break;
                     __delay_ms(1);
                     }            
                 

                 //------------INTERPRETA COMANDO DO MICROCOMPUTADOR--------------------                 
                 if(flag_usart_rx)
                    { 
                    Comando_Protocolo_Serial(); 
                    }                    
                  
                 //----------------INTERPRETA COMANDO DO DISPLAY----------------
                 if(flag_usart_rx)
                    { 
                    Comando_Display();
                    }//flag_usart_rx         

                          
                  
            }
      
}
//------------------------------------------------------------------------------
unsigned char countboard()
         {
         unsigned char retorno;
         retorno=0;
         for(char destino=0;destino<0x0F;destino++)
             {
             if(Send_To_Slave(destino, COMANDO_QUEM_EH_VOCE, 0, buffer)!=-1) retorno++;                 
             }
         return retorno;
         } 


//------------------------------------------------------------------------------
 int  Send_To_Slave(char destino, char comando, char size, char * buffer)
{
     unsigned int contador;
     int retorno=-1;
     char i;
     
     USART_put_int(HEADER_LIOFILIZADOR);
     USART_putc(BOARD_ADD);
     USART_putc(destino);
     USART_putc(comando);
     USART_putc(size);
     for(i=0;i<size;i++)
          USART_putc(buffer[i]);     
     __delay_us(80); //Para efeito de debug, vizualiza ap�s ultimo byte ser enviado
     
     
     flag_usart_rx=0;
     for(int contador=0;contador<RX_MAX_WAIT_TIME;contador++)
         {
          __delay_us(200);
          if(flag_usart_rx==1)
             {
             __delay_ms(2); 
             flag_usart_rx=0;
             size=usart_buffer[5];
             retorno = (usart_buffer[6]<<8)|(usart_buffer[7]);
             for(i=0;i<size;i++)
                 buffer[i]=usart_buffer[i+6];          
             contador=0;
             break;
             }
          } 
     return retorno;
}




int  Send_To_Slave_EMULA(char destino, char comando, char size, char * buffer)
{
	switch(destino)
	      {
	      case 0://printf("Erro no c�digo. Endereco zero � da placa mae!");
	      	     break;
	      case 1:if(buffer[0]==0)                    
                        return 100;  //VOLTIMETRO                    
	             else
			return 200; //VACUO  
		     break;
	      case 2:if(buffer[0]==0)
	                return 300;   //CONDENSADOR
	             else
			return -1;   
		     break;
	      case 3:if(buffer[0]==0)                      
                        return 400;                    
	               else                    
                        return 500;//sin(tmp01)*500;                    				       
		       break;
	      case 4:if(buffer[0]==0)                      
                        return 600;                   
	               else                    
                        return 700;//sin(tmp01)*500;                    				       
		       break;
	      case 5:if(buffer[0]==0)                      
                        return 800;                    
	               else                    
                        return 900;//sin(tmp01)*500;                    				       
		       break;
	      case 6:if(buffer[0]==0)                      
                        return 1100;                    
	               else                    
                        return 500;//sin(tmp01)*500;                    				       
		       break;
	      case 7:if(buffer[0]==0)                      
                        return 1200;                    
	               else                    
                        return 1300;//sin(tmp01)*500;                    				       
		       break;	  		 				 				 				 				 			 				 				 				 	
		  }
    return 0;
}
 




 
 
 
void ShowSensorRealTimeHS(void)
     {     
     char bb[3];
     char SlaveBoard;
     char canal;
     char tupla;
     int  vp, vpicone;
     
     //-------------------------LEITURA DAS PLACAS------------------------------
     //Faz leitura de todas as tuplas, inclusive da tupla vazia da placa 2.
     //Inicia no vetor numero zero.
     //-------------------------------------------------------------------------
     for(tupla=0;tupla<(totalboard*2);tupla++)
        { 
        SlaveBoard  = (tupla / 2)+1; 
        canal = tupla % 2;
        bb[0]=canal; 
        leitura[tupla]=Send_To_Slave_EMULA(SlaveBoard, COMMAND_READ_ANALOG, 1, bb);
        flag_array_slave_WDT[SlaveBoard]=TRUE;
        }
     
     
     //------------------------GRAVA NO DISPLAY---------------------------------
     my_delay_ms(50);
     flag_proculus_hs=TRUE;
     for(tupla=0;tupla<(totalboard*2);tupla++)
        { 
        switch(tupla)
              {
              case 0:
                     PROCULUS_VP_Write_UInt16(153,leitura[tupla]); //Voltimetro 
                     Voltimetro=leitura[tupla];
                     break;               
              case 1:
                     PROCULUS_VP_Write_UInt16(151,leitura[tupla]); //Vacuometro 
                     Vacuometro=leitura[tupla];
                     break;  
              case 2:
                     PROCULUS_VP_Write_UInt16(150,leitura[tupla]); //Condensador  
                     Condensador=leitura[tupla];
                     break; 
              case 3:
                     //sem sensor nesta tupla
                     break;
              default:
                  canal=tupla-4;
                  vp    = 230+(canal*TUPLA_VP_SIZE);
                  vpicone     = 400+canal;
                  PROCULUS_VP_Write_UInt16(vp+1,leitura[canal+4]);     

                  if(leitura[canal+4]<-400)PROCULUS_VP_Write_UInt16(vpicone,1); //sem sensor
                  else if(leitura[canal+4]==-1)PROCULUS_VP_Write_UInt16(vpicone,0); //sem placa                 
                  else PROCULUS_VP_Write_UInt16(vpicone,3); //Temperatura normal                  
                  
                  break;
              }  
        }
      flag_proculus_hs=FALSE;        
     
     }
 
 


 
 
 
 

void Carrega_Tupla_Receita(char index, t_receita *receita){
     unsigned int addeeprom;
     
     addeeprom=OFFSET_EEPROM_RECEITA+RECEITA_EEPROM_SIZE*index;
    
     receita->setpoint=EEPROM_Read_Integer(addeeprom+0); //Setpoint
     receita->potenciaON=EEPROM_Read_Byte (addeeprom+2); //Pot>
     receita->potenciaOFF=EEPROM_Read_Byte(addeeprom+3); //Pot<
     receita->histerese=EEPROM_Read_Byte  (addeeprom+4); //Histerese
     EEPROM_Read_String(addeeprom+5,texto); //Receita
     strcpy(receita->nome,texto);
}

void Grava_Receita(char index, t_receita *receita){
     unsigned int addeeprom;
     unsigned int vp;
     
     addeeprom=OFFSET_EEPROM_RECEITA+RECEITA_EEPROM_SIZE*index;
     
     EEPROM_Write_Integer(addeeprom+0 ,receita->setpoint); //Setpoint
     EEPROM_Write_Byte   (addeeprom+2 ,receita->potenciaON); //Pot>
     EEPROM_Write_Byte   (addeeprom+3 ,receita->potenciaOFF); //Pot<
     EEPROM_Write_Byte   (addeeprom+4 ,receita->histerese); //Histerese
     EEPROM_Write_String (addeeprom+5 ,receita->nome); //Receita
}




void Exibe_Receita(int index){
     unsigned int addeeprom;
     unsigned int vp;
     
     addeeprom=OFFSET_EEPROM_RECEITA+RECEITA_EEPROM_SIZE*index;
     vp       =OFFSET_VP_RECEITA+VP_RECEITA_SIZE*index;
     
     
     PROCULUS_VP_Write_UInt16(vp+0,EEPROM_Read_Integer(addeeprom));   //SetPoint
     PROCULUS_VP_Write_UInt16(vp+1,EEPROM_Read_Byte(addeeprom+2));    // POT>
     PROCULUS_VP_Write_UInt16(vp+2,EEPROM_Read_Byte(addeeprom+3));    // POT<
     PROCULUS_VP_Write_UInt16(vp+3,EEPROM_Read_Byte(addeeprom+4));    // Histerese
     EEPROM_Read_String(addeeprom+5,texto); //Receita
     texto[8]=0;
     PROCULUS_VP_Write_String(vp+4,texto);  // Nome da Receita 8 Letras   
}















 //------------------------------------------------------------------------------
//Alterar parametros de liofilizacao

void DataBaseBackupMain(unsigned char tupla)
      {
      unsigned int  vp;
      unsigned char addEEPROM;

      vp         = 230+(tupla*TUPLA_VP_SIZE);
      addEEPROM  = ((tupla)*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
      
      
      EEPROM_Write_Byte   (addEEPROM+0 ,tupla);//PROCULUS_VP_Read_UInt16(vp+0)); //Plataforma
      //----------------Reservado para leitura em tempo real -------------------
      EEPROM_Write_Integer(addEEPROM+1 ,PROCULUS_VP_Read_UInt16(vp+2)); //Setpoint
      EEPROM_Write_Byte   (addEEPROM+3 ,PROCULUS_VP_Read_UInt16(vp+3)); //Pot>
      EEPROM_Write_Byte   (addEEPROM+4 ,PROCULUS_VP_Read_UInt16(vp+4)); //Pot<
      EEPROM_Write_Byte   (addEEPROM+5 ,PROCULUS_VP_Read_UInt16(vp+5)); //Histerese
      PROCULUS_VP_Read_String(vp+6,texto);
      EEPROM_Write_String (addEEPROM+6 ,texto); //Receita
      EEPROM_Write_Integer(addEEPROM+16,PROCULUS_VP_Read_UInt16(vp+11)); //Status
      
      }














//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//Salva uma tupla do liofilizador na memoria EEPROM interna com o endere�o
//inicial igual OFFSET_EEPROM.
 void SaveLiofilizadorOnMemory(char index,t_liofilizador *liofilizador)
      {
      char CanalAD;
      unsigned char addEEPROM;
      addEEPROM  = (index*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
      CanalAD    = (unsigned char) (index % 2);
      
      EEPROM_Write_Byte(addEEPROM+0,liofilizador->plataforma);
      //------ RESERVADO PARA LEITURA EM TEMPO REAL ----------
      EEPROM_Write_Integer(addEEPROM    +1 , liofilizador->setpoint);
      EEPROM_Write_Byte(addEEPROM       +3 , liofilizador->tempoON);
      EEPROM_Write_Byte(addEEPROM       +4 , liofilizador->tempoOFF);
      EEPROM_Write_Byte(addEEPROM       +5 , liofilizador->histerese);
      EEPROM_Write_String(addEEPROM     +6 , liofilizador->receita);
      EEPROM_Write_Integer(addEEPROM   +16 , liofilizador->status);
      }
      

 
 
 void Set_Receita(unsigned char index, char status)
      {
      int  vp;
      unsigned int addEEPROM;

      addEEPROM   = OFFSET_EEPROM_RECEITA+RECEITA_EEPROM_SIZE*index;
      vp          = 230+(index*TUPLA_VP_SIZE);              
     
      PROCULUS_VP_Write_UInt16(vp+2,receita.setpoint);     
      PROCULUS_VP_Write_UInt16(vp+3,receita.potenciaON);   
      PROCULUS_VP_Write_UInt16(vp+4,receita.potenciaOFF);  
      PROCULUS_VP_Write_UInt16(vp+5,receita.histerese);    
      PROCULUS_VP_Write_String(vp+6,receita.nome);
      if(status==TRUE)
         PROCULUS_VP_Write_UInt16(vp+11,1); //Status                            
      else
         PROCULUS_VP_Write_UInt16(vp+11,0); //Status 
      }
  
 

 
//-----------------------------------------------------------------------------
 //Preenchimento de dados estaticos da tela principal
 void ShowStaticValueGrid(unsigned char tupla)
      {
      char CanalAD;
      char SlaveBoard;
      int  vp, vpicone;
      char  bb[2];
      unsigned char addEEPROM;
      int temperatura;


      addEEPROM   = (tupla*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
      CanalAD     = (unsigned char) (tupla % 2);
      bb[0]       = CanalAD;
      SlaveBoard  = (unsigned char) (tupla / 2)+1;
      vp          = 230+(tupla*TUPLA_VP_SIZE);              


      if(tupla>=4){
         tupla-=4;
         addEEPROM   = (tupla*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
         CanalAD     = (unsigned char) (tupla % 2);
         bb[0]       = CanalAD;
         SlaveBoard  = (unsigned char) (tupla / 2)+1+2;
         vp          = 230+(tupla*TUPLA_VP_SIZE);
         vpicone     = 400+tupla;               
         PROCULUS_VP_Write_UInt16(vp+0,EEPROM_Read_Byte    (addEEPROM+0)+1); //Plataforma
         PROCULUS_VP_Write_UInt16(vp+2,EEPROM_Read_Integer (addEEPROM+1)); //SetPoint 1e2
         PROCULUS_VP_Write_UInt16(vp+3,EEPROM_Read_Byte    (addEEPROM+3)); //Pot>
         PROCULUS_VP_Write_UInt16(vp+4,EEPROM_Read_Byte    (addEEPROM+4)); //Pot<
         PROCULUS_VP_Write_UInt16(vp+5,EEPROM_Read_Byte    (addEEPROM+5)); //Histerese
         EEPROM_Read_String(addEEPROM+6,texto); //Nome da Receita
         PROCULUS_VP_Write_String(vp+6,texto);
         PROCULUS_VP_Write_UInt16(vp+11,EEPROM_Read_Integer(addEEPROM+16)); //Status                             
         }
} 


 

 
 

//Salva e atualiza grafico
void save_datalog(unsigned int add){
     char index;
     char bb[3];
     char boardadd;
     
     //===================================================================================
     Send_To_Slave(TODOS, COMMAND_SAVE_LOG , 0, bb); 
     //===================================================================================
     for(index=0;index<(totalboard*2);index++)
	     {
         if(mapa.entrada[index]!=NULL) 
            {	
            PROCULUS_graphic_plot(mapa.canal[index]+1,*mapa.entrada[index]*mapa.fator[index]);
            }
         }      
}


 

//------------------------------------------------------------------------------
 //Alterar parametros de liofilizacao
 //Rolar tela para visualizar os outros dados
 //Salvar dados
 //C�digo Funcionando Perfeitamente
 void ShowAndSetSlaveParameters(unsigned char tupla)
      {
      unsigned char CanalAD;
      unsigned char SlaveBoard;
      unsigned char addEEPROM;
      unsigned int  vp;
      unsigned char bb[2];
      //char texto[15];
 
      vp         = 230+(tupla*TUPLA_VP_SIZE);
      //tupla     += offset;
      addEEPROM  = (tupla*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
      CanalAD    = (unsigned char) (tupla % 2);
      SlaveBoard = (unsigned char) (tupla / 2);

      bb[0]      = CanalAD;
      PROCULUS_VP_Write_UInt16(vp+0,EEPROM_Read_Byte(addEEPROM+0)); //---------------------   //Plataforma
      
      //PROCULUS_VP_Write_UInt16(170,((tupla/9.0)*100.0)); //Progresso em porcentagem
      
      //-- NESTA POSICAO (vp+1) � feito a leitura em tempo real        //Temperatura em tempo real
      
      PROCULUS_VP_Write_UInt16(vp+2,EEPROM_Read_Integer(addEEPROM+1));  //SetPoint
      PROCULUS_VP_Write_UInt16(vp+3,EEPROM_Read_Byte   (addEEPROM+3));  ///Pot>
      PROCULUS_VP_Write_UInt16(vp+4,EEPROM_Read_Byte   (addEEPROM+4));  //Pot<
      PROCULUS_VP_Write_UInt16(vp+5,EEPROM_Read_Byte   (addEEPROM+5));  //Histerese
      EEPROM_Read_String (addEEPROM+6,texto);
      PROCULUS_VP_Write_String(vp+6,texto);                             //Receita
      PROCULUS_VP_Write_UInt16(vp+11,EEPROM_Read_Integer(addEEPROM+16));//Status
       
      }








//------------------------------------------------------------------------------
void Send_to_PC(unsigned char size){
//     unsigned char i;
     //
     USART_put_int(HEADER_LIOFILIZADOR);
     USART_putc(usart_protocol.destino);
     USART_putc(usart_protocol.origem);
     USART_putc(usart_protocol.command);
     USART_putc(size);
     
     //USART_putc(0x0D);
     //USART_putc(0x17);
     /*        
     USART_put_int(usart_protocol.header);
     USART_putc(BOARD_ADD);
     USART_putc(0xC0);
     USART_putc(usart_protocol.command);
     USART_putc(size);
     */ 
}




void Comando_Microcomputador(void){
     USART_to_Protocol(&usart_protocol);
     if(usart_protocol.header==HEADER_LIOFILIZADOR)
        if(usart_protocol.destino==BOARD_ADD) 
        { 
        Decodify_Command();
        flag_usart_rx=0;
        }    
}




//------------------------------------------------------------------------------
void Decodify_Command(void){
    int  dados;
    char tempchar;
    int  tempint;
    unsigned long add_24LCxxxx;
    
    High(add_24LCxxxx)=(usart_protocol.value[1]);
    Lower(add_24LCxxxx)=(usart_protocol.value[2]);
    Hi(add_24LCxxxx)=(usart_protocol.value[3]);
    Lo(add_24LCxxxx)=(usart_protocol.value[4]);    

    switch(usart_protocol.command){
        /*
        case COMMAND_PING:
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_PONG:
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_POWER_ON:
             Send_to_PC(3);
             SEND_REPLY_OK();
             POWER_ON();
             break;
        case COMMAND_POWER_OFF:
             Send_to_PC(3);
             SEND_REPLY_OK();
             POWER_OFF();
             break;
        case COMMAND_UPDATE_ON:
             Send_to_PC(3);
             SEND_REPLY_OK();
             UPDATE_ENABLE();
             break;
        case COMMAND_UPDATE_OFF:
             Send_to_PC(3);
             SEND_REPLY_OK();
             UPDATE_DISABLE();
             break;
         */ 
        //---------------------- EEPROM INTERNA --------------------------------
        case COMMAND_IEE_W_BYTE:
             EEPROM_Write_Byte(usart_protocol.value[0],
                               usart_protocol.value[1]);
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_IEE_R_BYTE:
             tempchar=EEPROM_Read_Byte(usart_protocol.value[0]);            
             Send_to_PC(1);
             USART_putc(tempchar);              
             break;
        case COMMAND_IEE_W_INT:
             {
             unsigned char add;
             add=usart_protocol.value[0];
             dados=(usart_protocol.value[1]<<8)|usart_protocol.value[2];
             EEPROM_Write_Integer(add,dados);
             Send_to_PC(3);
             SEND_REPLY_OK();
             }
             break;
        case COMMAND_IEE_R_INT:
             tempint=EEPROM_Read_Integer(usart_protocol.value[0]);
             Send_to_PC(2);
             USART_put_int(tempint);
             break;
             
        case COMMAND_IEE_W_STR :
             EEPROM_Write_String(usart_protocol.value[0],
                                &usart_protocol.value[1]);
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_IEE_R_STR:
             {
             //unsigned char texto[20];
             EEPROM_Read_String(usart_protocol.value[0],texto);
             Send_to_PC(sizeof(texto));
             USART_put_string(texto);
             break;
             }

        //---------------------- EEPROM EXTERNA --------------------------------
        case COMMAND_EEE_W_BYTE:
             EEPROM_24C1025_Write_Byte(usart_protocol.value[0],      //CHIP NUMBER
                                                  add_24LCxxxx,      //ADD of Memory                                                  
                                      &usart_protocol.value[5]);     //VALUE
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_EEE_R_BYTE:
             tempchar=EEPROM_24C1025_Read_Byte(usart_protocol.value[0],  //CHIP NUMBER
                                                         add_24LCxxxx);  //ADD of Memory
             
             
             Send_to_PC(1);
             USART_putc(tempchar);
             break;
        case COMMAND_EEE_W_INT:
             {
             char valor[2];
             valor[0]=(usart_protocol.value[5]);
             valor[1]=(usart_protocol.value[6]);
             EEPROM_24C1025_Write_Int(usart_protocol.value[0],  //CHIP NUMBER
                                                 add_24LCxxxx,  //Add of memory
                                                       valor);  //
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
             }
        case COMMAND_EEE_R_INT:
             tempint=EEPROM_24C1025_Read_Int(usart_protocol.value[0],    //CHIP NUMBER
                                                       add_24LCxxxx);
             Send_to_PC(2);
             USART_put_int(tempint);
             break;
             
             
        case COMMAND_EEE_R_BUF:
             {                 
             char sizedata;
             sizedata=usart_protocol.value[5];             
             EEPROM_24C1025_Read_Buffer(usart_protocol.value[0],  //CHIP NUMBER
                                        add_24LCxxxx,  //Add of memory
                                        sizedata,  //SIZEDATA
                                        buffer);  //Buffer of data 
             
            
             
             Send_to_PC(sizedata);
             USART_put_buffer(buffer,sizedata); 
             }
             break;
             
        case COMMAND_EEE_W_BUF: 
             EEPROM_24C1025_Write_Buffer(usart_protocol.value[0],     //CHIP NUMBER
                                         add_24LCxxxx,     //ADD Long
                                         usart_protocol.value[5],     //SIZEDATA
                                         &usart_protocol.value[6]);    //Inicio do Buff
             
             Send_to_PC(3);
             SEND_REPLY_OK();            
             break;                    

        case COMMAND_EEE_W_STR :
             EEPROM_24C1025_Write_Str(usart_protocol.value[0],   //CHIP NUMBER
                                                 add_24LCxxxx,   //ENDERECO 24LCXXXX
                                    &usart_protocol.value[5]);   //Ponteiro String
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        case COMMAND_EEE_R_STR:
             {
             EEPROM_24C1025_Read_Str(usart_protocol.value[0],       //CHIP NUMBER
                                                add_24LCxxxx,   //ENDERECO 24LCXXXX
                                                      texto);
                      
             Send_to_PC(strlen(texto));
             USART_put_string(texto);
             break;
             }
        
        case COMMAND_EEE_FILL_ALL:
             EEPROM_24C1025_Fill_All(usart_protocol.value[0], //chip_add, unsigned char value){
                                     usart_protocol.value[1]);
             Send_to_PC(3);
             SEND_REPLY_OK();            
             break;
             
         /* 
        case COMMAND_READ_ANALOG:
             tempint=((1023-ADC_Read(usart_protocol.value[0]))/10)-26;
             Send_to_PC(2);
             USART_put_int(tempint);
             break;

        case COMMAND_CLK_PIC_R:
             {
             char hh[10];
             char mm[10];
             char ss[10];             
        
             texto_pnt=itoa(hora,10);
             texto_pnt=itoa(minuto,10);
             texto_pnt=itoa(segundo,10);
            
            
             strcpy(texto,"");             
             if (hh[1]==0) 
                {
                strcat(texto,"0");
                hh[2]=0;
                }
             strcat(texto,hh);             
             strcat(texto,":");
             if(mm[1]==0) 
                {
                strcat(texto,"0");
                mm[2]=0;
                }
             strcat(texto,mm);
             strcat(texto,":");
             if(ss[1]==0) 
                { 
                strcat(texto,"0");
                ss[2]=0;
                }
             strcat(texto,ss);
             Send_to_PC(strlen(texto)); 
             USART_put_string(texto);
             USART_putc(0);
             }
             break;                     
             */    
             /*    
             //unsigned char texto[20];
             char hh[10];
             char mm[10];
             char ss[10];

             IntToStr(hora, hh);
             IntToStr(minuto, mm);
             IntToStr(segundo, ss);
             strcat(texto,"");
             strcat(texto,hh);
             strcat(texto,":");
             strcat(texto,mm);
             strcat(texto,":");
             strcat(texto,ss);

             Send_to_PC(strlen(texto));
             USART_put_string(texto);
             USART_putc(0);
             }
             break;
             */
             /*
        case COMMAND_CLK_PIC_W:
             hora=usart_protocol.value[0];
             minuto=usart_protocol.value[1];
             segundo=usart_protocol.value[2];
             Send_to_PC(3);
             SEND_REPLY_OK();
             break ;

        case COMMAND_LDC_PAGE:
             PROCULUS_Show_Screen(usart_protocol.value[0]);
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;
        */
        case COMMAND_PROCULUS_Buzzer:
             PROCULUS_Buzzer((usart_protocol.value[0]<<8)+
                             (usart_protocol.value[1]));
             Send_to_PC(3);
             SEND_REPLY_OK();
        /*
        case COMMAND_LCD_W_VP_INT:
             PROCULUS_VP_Write_Int16((usart_protocol.value[0]<<8)+
                                      (usart_protocol.value[1]),
                                      (usart_protocol.value[2])+
                                      (usart_protocol.value[3]<<8));
             Send_to_PC(3);
             SEND_REPLY_OK();
             break;


        case COMMAND_LCD_R_VP_INT:
             {
             //unsigned char texto[20];
             tempint=PROCULUS_VP_Read_Int16((usart_protocol.value[0]<<8)+
                                             (usart_protocol.value[1]));

             Send_to_PC((strlen(texto)));
             USART_put_int(tempint);
             break;
             }
        case COMMAND_LCD_W_VP_STR:
             PROCULUS_VP_Write_String((usart_protocol.value[0]<<8)+

                                      (usart_protocol.value[1]),
                                     &(usart_protocol.value[2]));

             Send_to_PC(3);

             SEND_REPLY_OK();

        case COMMAND_LCD_R_VP_STR:
             {
             //unsigned char texto[20];
             PROCULUS_VP_Read_String((usart_protocol.value[0]<<8)+
                                     (usart_protocol.value[1]),
                                      texto);
             Send_to_PC((strlen(texto)));
             USART_put_string(texto);
             break;
             }
        */
    }
}

void SEND_REPLY_OK(void){
     USART_put_string("OK");
}


void Comando_Protocolo_Serial(void){
 //======================= COMUNICACAO COM O PC =========================
        unsigned char size, i, OrigemMemo;
               
        USART_to_Protocol(&usart_protocol);
        if(usart_protocol.header==HEADER_LIOFILIZADOR)   
            {            
            if(usart_protocol.origem==0xC0)
                {
                Delay_Led_Usart=DEFAULT_LEDS;
                if(usart_protocol.destino==0x00)
                   {
                   Decodify_Command();
                   flag_usart_rx=0;
                   }
                else 
                   { 
                   /*
                   Origem = 0XC0
                   Destino= 0X01 a 0X0F
                   */
                   OrigemMemo=usart_protocol.origem;
                   Send_To_Slave(usart_protocol.destino,
                                 usart_protocol.command,
                                 usart_protocol.size,
                                 usart_protocol.value
                                 );
                   flag_usart_rx=0;
                   
                   /*
                   Origem = 0x01 a 0x0F
                   Destino = 0x00                  
                   */
                   
                   
                   USART_to_Protocol(&usart_protocol);
                   USART_put_int(HEADER_LIOFILIZADOR);
                   USART_putc(usart_protocol.origem);// 0X01 a 0x0F
                   USART_putc(OrigemMemo);
                   USART_putc(usart_protocol.command);
                   USART_putc(usart_protocol.size);
                   for(i=0;i<usart_protocol.size;i++)
                         USART_putc(usart_protocol.value[i]);

                   flag_usart_rx=0;               
                   }
                }
        flag_usart_rx=0;     
        }//HEADER_LIOFILIZADOR
}


void Formatar_Banco_de_Dados(char inicio, char total){
     for(char j=inicio;j<(inicio+total);j++)
          {
          //flag_main_loop_WDT=1;
          liofilizador[j].plataforma=j;          
          liofilizador[j].setpoint=0;
          liofilizador[j].tempoON=0;
          liofilizador[j].tempoOFF=0;
          liofilizador[j].histerese=0;
          strcpy(liofilizador[j].receita,"");          
          liofilizador[j].status=0;//IconStatusTrue[j]-1; //Fix - Colocar NOK         
          SaveLiofilizadorOnMemory(j,&liofilizador[j]);          
          }
    
}


//Provavel ponto que est� dando erro
void Upload_Data_to_Slave(void){
     char index;     
     char board;
     char canal;
     char addEEPROM;
     char buffer[3];
     int  temp;
     

     for(index=4;index<15;index++){
         board=(index/2)+1;
         canal=(index%2);         
         addEEPROM  = ((index-4)*TUPLA_EEPROM_SIZE)+OFFSET_EEPROM;
         
         //-----------------------PLATAFORMA--------------------------
         //temp=EEPROM_Read_Integer(addEEPROM+0);
         
         //-----------------------SETPOINT--------------------------
         temp=EEPROM_Read_Integer(addEEPROM+1);
         buffer[0]=(canal*7)+0;
         buffer[1]=Hi(temp);
         buffer[2]=Lo(temp);
         Send_To_Slave(board,COMMAND_IEE_W_INT,3,buffer);
         __delay_ms(150);
         //-----------------------TEMPO ON---------------------------
         buffer[0]=(canal*7)+2;
         buffer[1]=EEPROM_Read_Byte(addEEPROM+3);
         Send_To_Slave(board,COMMAND_IEE_W_BYTE,2,buffer);
         __delay_ms(150);
         //-----------------------TEMPO OFF---------------------------
         buffer[0]=(canal*7)+3;
         buffer[1]=EEPROM_Read_Byte(addEEPROM+4);
         Send_To_Slave(board,COMMAND_IEE_W_BYTE,2,buffer);   
         __delay_ms(150);
         //----------------------- HISTERESE---------------------------
         buffer[0]=(canal*7)+4;
         buffer[1]=EEPROM_Read_Byte(addEEPROM+5);
         Send_To_Slave(board,COMMAND_IEE_W_BYTE,2,buffer);  
         __delay_ms(150);
         //-----------------------STATUS------------------------------
         temp=EEPROM_Read_Integer(addEEPROM+16);
         buffer[0]=(canal*7)+5;
         buffer[1]=Hi(temp);
         buffer[2]=Lo(temp);
         Send_To_Slave(board,COMMAND_IEE_W_INT,3,buffer); 
         __delay_ms(150);
         if(canal==1) Send_To_Slave(board,COMMAND_LOAD_WORK,0,buffer); // Status
        
         }

}



void ShowMessage(char mensagem[20],unsigned int delay, char SoundType, char retem){
     char texto[30];
     //unsigned char center=0;
     //if (SoundType==SOUND_OK) PROCULUS_OK();
     strcpy(texto,"                             ");
     texto[(30-strlen(mensagem))/2]=0;
     strcat(texto,mensagem);
     PROCULUS_VP_Write_String(180,texto);
     my_delay_ms(delay);
     if(!retem) PROCULUS_VP_Write_String(180,"");
}


void global_datalog(void){
        if((PROCULUS_VP_Read_UInt16(2)==1)&&(flag_global_datalog==0))
           {
            char bb[2];
            Send_To_Slave(TODOS, COMMAND_SYNCRONIZE , 0, bb);
            Carregar_tempo_de_datalog();
            flag_global_datalog=1;             
           }
        else if((PROCULUS_VP_Read_UInt16(2)==0)&&(flag_global_datalog==1))
               {
               //Instru�oes aqui ao desligar 
               flag_global_datalog=0;               
               }       
}




void Condensador_Switch(unsigned char estado){
     char buffer[2];
     buffer[0]=0;
     buffer[1]=estado;
     Send_To_Slave(0x02,COMMAND_RELAY,2,buffer);
}

void Vaccum_Switch(unsigned char estado){
     char buffer[2];
     buffer[0]=0;
     buffer[1]=estado;
     Send_To_Slave(0x01,COMMAND_RELAY,2,buffer);          
}


void global_condensador(void){     
        if((PROCULUS_VP_Read_UInt16(0x03)==1)&&(flag_global_condensador==0))
            {
            if(delay_condensador==0)
               {
               flag_global_condensador=1; 
               Condensador_Switch(ON);           
               }
            else
               {
               PROCULUS_Buzzer(1000); 
               PROCULUS_VP_Write_UInt16(0x03,0); 
               } 
            }
        if((PROCULUS_VP_Read_UInt16(0x03)==0)&&(flag_global_condensador==1))
           {
           flag_global_condensador=0; 
           Condensador_Switch(OFF);
           delay_condensador=30; //Intervalo para conseguir religar o condensador
           }       
}




void global_vacuo(void){     
        if((PROCULUS_VP_Read_UInt16(0x04)==1)&&(flag_global_vacuo==0)) 
           {
           flag_global_vacuo=1; 
           if(Condensador<Seg_Condensador)
              { 
              Vaccum_Switch(TRUE); 
              Contagem_Tempo_de_Processo(TRUE); 
              }
           else
              { 
              Vaccum_Switch(FALSE); 
              Incrementa_Contador_de_Repique_do_Vacuo();              
              }
           }
        else if((PROCULUS_VP_Read_UInt16(0x04)==0)&&(flag_global_vacuo==1))
                {                
                flag_global_vacuo=0;
                Vaccum_Switch(FALSE); 
                PROCULUS_VP_Write_UInt16(6,0);

                PROCULUS_Popup(DESEJA_ENCERRAR_PROCESSO);
                PROCULUS_VP_Write_UInt16(0x0016,0);
                for(int i=0;i<10000;i++)
                     {
                     my_delay_ms(1);
                     if (PROCULUS_VP_Read_UInt16(6)!=0) break;
                     asm("CLRWDT");
                     }

                if(PROCULUS_VP_Read_UInt16(6)==241)//sim
                     { 
                     Contagem_Tempo_de_Processo(FALSE);
                     processo_hora=0;
                     processo_minuto=0;
                     PROCULUS_OK();               
                     }                              

                                   
                }
        else if((PROCULUS_VP_Read_UInt16(0x04)==1)&&(flag_global_vacuo==1))
                {
                if(Condensador<Seg_Condensador)
                   { 
                   Vaccum_Switch(TRUE); 
                   flag_Vacuo_estava_ligado=1;
                   }
                else
                   { 
                   Vaccum_Switch(FALSE);
                   Incrementa_Contador_de_Repique_do_Vacuo();
                   }            
                }
}        


void Global_Aquecimento_Switch(unsigned char estado){
     char buffer[2]; 
     unsigned char board;
     for(board=3;board<(totalboard*2-1);board++)
        {
        //flag_main_loop_WDT=1; 
        buffer[0]=estado;
        Send_To_Slave(board,COMMAND_GLOBAL_HOT,1,buffer);               
        }     
}


void global_aquecimento(void){  
        if((PROCULUS_VP_Read_UInt16(5)==1)&&(flag_global_aquecimento==0))
           {            
           if((Condensador<Seg_Aq_cond)&&(Vacuometro<Seg_Aq_vacuo))
                     {
                     flag_global_aquecimento=1; 
                     Global_Aquecimento_Switch(ON);                                          
                     }
           }
        if((PROCULUS_VP_Read_UInt16(5)==0)&&(flag_global_aquecimento==1))
           {//Desliga Aquecimento no Bot�o
           Global_Aquecimento_Switch(OFF);
           flag_global_aquecimento=0;           
           PROCULUS_VP_Write_String(1970,"");           
           }        
        else if ((PROCULUS_VP_Read_UInt16(5)==1)&&(flag_global_aquecimento==1))
           {
           if((Condensador>Seg_Aq_cond)||(Vacuometro>Seg_Aq_vacuo)) 
              {
              flag_global_aquecimento=0;              
              //PROCULUS_VP_Write_String(1970,"");
              //PROCULUS_VP_Write_UInt16(5,0); //APAGA O BOTAO AQUECIMENTO
              Global_Aquecimento_Switch(OFF);             
              //--------------------------------------------------------          
              }
           }
                
}       






void Comando_Display(void){                        
unsigned char i;    
//=================== COMUNICACAO SERIAL PARA PROCULUS =================
  PROCULUS_Buffer_to_Proculus(&proculus);
  if(proculus.header==PROCULUS_HEADER)
     {
     flag_usart_rx=0;     
     if(proculus.page==0x15) pagina_15();  // Visualiza��o de todos os parmetros
     if(proculus.page==0x19) pagina_19();  // Tela de edi��o de aquecimento    
     if(proculus.page==0x23) pagina_23();  // Salvar dados de Seguran�a.
     if(proculus.page==0x25) pagina_25();  // Editar e Salvar Data e Hora
     if(proculus.page==0x29) pagina_29();  // Captura de Datalog, altera��o de valor   
     if(proculus.page==0x31) pagina_31();  // ALTERAR SENHA 
     if(proculus.page==0x47) pagina_47();  // Lista de Receita
     if(proculus.page==0x49) pagina_49();  // Edicao de Receita  
     proculus.header=0;
     }//Header                  
} //funcao   


void pagina_49(void){     
     t_liofilizador liofilizador;
     
     if(PROCULUS_VP_Read_UInt16(390)==1)
                   {                        
                   //Excluir
                   receita.setpoint=0;
                   receita.potenciaON=0;
                   receita.potenciaOFF=0;
                   receita.histerese=0;
                   strcpy(receita.nome,"");           
                   Grava_Receita(Index_Receita, &receita);                   
                   Exibe_Receita(Index_Receita); 

                   PROCULUS_VP_Write_UInt16(410,0);
                   PROCULUS_VP_Write_UInt16(411,0);
                   PROCULUS_VP_Write_UInt16(412,0);
                   PROCULUS_VP_Write_UInt16(413,0);
                   PROCULUS_VP_Write_String(414,"");

                   Atualizar_Lista_de_Receitas();
                   PROCULUS_VP_Write_UInt16(390,0);

                   PROCULUS_Popup(OP_REALIZADA_COM_SUCESSO);
                   PROCULUS_OK();
                   PROCULUS_VP_Write_String(1910,"Item Excluido com sucesso!"); 
                   my_delay_ms(1500);
                   PROCULUS_VP_Write_String(1910,""); 
                   }
                   
                   
                   
                   
     if(PROCULUS_VP_Read_UInt16(391)==1)
                   {                                           
                   //Gravar
                   receita.setpoint=PROCULUS_VP_Read_UInt16(410);
                   receita.potenciaON=PROCULUS_VP_Read_UInt16(411);
                   receita.potenciaOFF=PROCULUS_VP_Read_UInt16(412);
                   receita.histerese=PROCULUS_VP_Read_UInt16(413);
                   PROCULUS_VP_Read_String(414,receita.nome);
                   Grava_Receita(Index_Receita, &receita);               
                   //PROCULUS_Control_Activation(POPUP_SALVO_COM_SUCESSO);
                   
                   Atualizar_Lista_de_Receitas();
                   PROCULUS_VP_Write_UInt16(391,0);
                   
                   PROCULUS_Popup(SALVO_COM_SUCESSO);
                   PROCULUS_OK();
                   PROCULUS_VP_Write_String(1910,"Dados Gravados com sucesso!"); 
                   my_delay_ms(1500);
                   PROCULUS_VP_Write_String(1910,"");                   
                   }
                   
                   
     if(PROCULUS_VP_Read_UInt16(392)==1)
                   {                                           
                   //Aplicar
                   PROCULUS_VP_Write_String(1910,"Aguarde... Carregando Dados!"); 
                   Carrega_Tupla_Receita(Index_Receita, &receita); 
                   PROCULUS_VP_Write_String(1910,"Aguarde... Armazenando Dados!");
                   for(char i=0;i<(totalboard-2)*2;i++)
                       { 
                       //flag_main_loop_WDT=1;
                       liofilizador.plataforma=i;
                       liofilizador.setpoint=receita.setpoint;
                       liofilizador.tempoON=receita.potenciaON;
                       liofilizador.tempoOFF=receita.potenciaOFF;
                       liofilizador.histerese=receita.histerese;
                       strcpy(liofilizador.receita,receita.nome);
                       liofilizador.status=1;
                       SaveLiofilizadorOnMemory(i,&liofilizador);
                       } 
                   PROCULUS_VP_Write_String(1910,"Aguarde... Transferindo dados para placas externas!");
                   Upload_Data_to_Slave(); 
                   PROCULUS_VP_Write_String(1910,"Aguarde... Atualizando Tabela!");
                   for(char i=0;i<15;i++)ShowStaticValueGrid(i);
                   for(char i=0;i<10;i++)ShowAndSetSlaveParameters(i);
                   
                   
                   PROCULUS_VP_Write_UInt16(392,0);
                   PROCULUS_Popup(OP_REALIZADA_COM_SUCESSO);
                   PROCULUS_OK();
                   PROCULUS_VP_Write_String(1910,"");
                   } 
                   
     if(PROCULUS_VP_Read_UInt16(393)==1) //Voltar
                   {
                   PROCULUS_VP_Write_UInt16(393,0);                     
                   PROCULUS_Show_Screen(47);
                   }           
     
}


void pagina_47(void){  //Carregar receita para Edicao;
     t_receita receita;
     if(flag_senha_liberada)
         { 
         PROCULUS_Show_Screen(49); //Chama a tela de Edi��o
         //Index_Receita=proculus.button;
         Carrega_Tupla_Receita(Index_Receita, &receita);

         PROCULUS_VP_Write_UInt16(410,receita.setpoint);   //SetPoint
         PROCULUS_VP_Write_UInt16(411,receita.potenciaON);    // POT>
         PROCULUS_VP_Write_UInt16(412,receita.potenciaOFF);    // POT<
         PROCULUS_VP_Write_UInt16(413,receita.histerese);    // Histerese
         PROCULUS_VP_Write_String(414,receita.nome);  // Nome da Receita 8 Letras          
         }
     else
         {
         PROCULUS_Popup(ACESSO_NEGADO);
         PROCULUS_NOK();
         }          
 
    

}




//-----------------------------------Pagina 15----------------------------------
//Exibe todos os par�metros.
void pagina_15(void){                     
   if(proculus.button==3)
      { 
      PROCULUS_Buzzer(2000); 
      }

   if(proculus.button==2)
      { //Desligar o sistema
      if(proculus.status==0){
         //Send_To_PC(3);
         //SEND_REPLY_OK();
         POWER_OFF();
          }//status 0
      } //botao 2


   if (proculus.button==5) //Entra em tela de aquecimento
      {                    //para visualiza��o e edi��o
      if(proculus.status==1)
         {
         //pagina=19;
         //PROCULUS_Show_Screen(19);                                     
         //PROCULUS_OK();
         maincnt=0; 
         }
     }//Botao 5

}// PAGINA 15


//---------------------------- Pagina 19 ---------------------------------------
void pagina_19(void)  
{
            
       if(PROCULUS_VP_Read_UInt16(168)==1) //UNDO
         {  
         PROCULUS_VP_Write_UInt16(168,0);   
         if(flag_senha_liberada)  
            { 
            for(char i=0;i<15;i++) 
                {
                //flag_main_loop_WDT=1;
                ShowStaticValueGrid(i);
                }
            PROCULUS_OK();
            }
         else
            { 
            PROCULUS_Popup(ACESSO_NEGADO);
            PROCULUS_NOK();                                 
            }                                     
         }

       
       
       
       
       //TRISDbits.RD5=0;
       //PORTDbits.RD5=1;
       if(PROCULUS_VP_Read_UInt16(167)==1) //GRAVAR
         {
         PROCULUS_VP_Write_UInt16(167,0);  
         if(flag_senha_liberada)  
            {                                
             ShowMessage("Gravando Dados, Aguarde!",2000,SOUND_SINGLE,TRUE);
             for(char i=0;i<10;i++)
                 {
                 //flag_main_loop_WDT=1;
                 DataBaseBackupMain(i);
                 }                                 
             ShowMessage("SUCESSO!!!",2000,SOUND_OK,FALSE);                               
             PROCULUS_OK();                                 
            }
         else
            { 
            PROCULUS_Popup(ACESSO_NEGADO);
            PROCULUS_NOK();                                 
            }             
         }
       //PORTDbits.RD5=0;
       
       
       
       
       

       if(PROCULUS_VP_Read_UInt16(166)==1) //UPLOAD
         {  
         PROCULUS_VP_Write_UInt16(166,0);  
         if(flag_senha_liberada)  
            {                                                  
             ShowMessage("Descarregando Dados",2000,SOUND_SINGLE,TRUE); 
             Upload_Data_to_Slave();
             ShowMessage("SUCESSO!!!",2000,SOUND_OK,FALSE);                                  
             PROCULUS_OK();
            }
         else
            { 
            PROCULUS_Popup(ACESSO_NEGADO);
            PROCULUS_NOK();                                 
            }             
         }

       
       
       
       
       if(PROCULUS_VP_Read_UInt16(169)==1) //FORMAT
         {
         PROCULUS_VP_Write_UInt16(169,0);  
         if(flag_senha_liberada)  
            {                                     
             char inicio;
             ShowMessage("Formatando",2000,SOUND_SINGLE,TRUE);
             
             if(pagina==19)inicio=0; 
             if(pagina==21)inicio=5;
             //PROCULUS_VP_Write_UInt16(395,inicio); //FIX APAGAR
             
             Formatar_Banco_de_Dados(inicio,5);
             for(char i=0;i<15;i++) 
                 {
                 //flag_main_loop_WDT=1;
                 ShowStaticValueGrid(i);             
                 }
             PROCULUS_OK();             
             ShowMessage("SUCESSO!",2000,SOUND_OK,FALSE);                                 
            }
         else
            { 
            PROCULUS_Popup(ACESSO_NEGADO);
            PROCULUS_NOK();                                 
            }         
         }                  

}// Pagina 19

//-----------------------------Pagina 23----------------------------------------
//Salva dados de Seguran�a.
void pagina_23(void)                    
{ 
    if(flag_senha_liberada)
      {  
        Seg_Condensador=PROCULUS_VP_Read_UInt16(210);
        Seg_Vacuo=PROCULUS_VP_Read_UInt16(211);
        Seg_Aq_cond=PROCULUS_VP_Read_UInt16(212);
        Seg_Aq_vacuo=PROCULUS_VP_Read_UInt16(213); 

        EEPROM_Write_Integer(0x01,Seg_Condensador);
        EEPROM_Write_Integer(0x03,Seg_Vacuo);                         
        EEPROM_Write_Integer(0x05,Seg_Aq_cond);
        EEPROM_Write_Integer(0x07,Seg_Aq_vacuo);
        PROCULUS_Popup(SALVO_COM_SUCESSO);
        PROCULUS_OK();
        Carregar_Parametros_de_Seguranca();
        
     }
    else
     {   
     PROCULUS_Popup(ACESSO_NEGADO);    
     PROCULUS_NOK();
     }
}   







//------------------------------- Pagina 25-------------------------------------
void pagina_25(void)                   
{

    if(flag_senha_liberada)                           
       { 
       char date[10];
       char time[10];           
       PROCULUS_VP_Read_String(20,date); 
       PROCULUS_VP_Read_String(30,time);                        
       PROCULUS_Write_RTC(date,time); 
       PROCULUS_Popup(SALVO_COM_SUCESSO);
       PROCULUS_OK();
       }   
    else
       {                           
       PROCULUS_Popup(ACESSO_NEGADO);
       PROCULUS_NOK();
       }                             

}



//---------------------------- pagina 29----------------------------------------
void pagina_29(void)    
  { 
    if(flag_senha_liberada)
         {
          EEPROM_Write_Integer(0x09,PROCULUS_VP_Read_UInt16(172));
          Carregar_tempo_de_datalog();
          PROCULUS_Popup(SALVO_COM_SUCESSO);
          PROCULUS_OK();
         }
     else
         {
         PROCULUS_Popup(ACESSO_NEGADO);
         PROCULUS_NOK();
         }                    
  } 


//----------------------------ALTERAR SENHA-----------------------------------
void pagina_31(void){
         unsigned long nova_senha, confirma_senha;
         char senhavetor[4];
         
         nova_senha    =PROCULUS_VP_Read_Double32(382);
         confirma_senha=PROCULUS_VP_Read_Double32(384);

            
         PROCULUS_VP_Read(380,senhavetor,4);  //L� registrador do teclado 4Bytes
         senha_atual=((unsigned long)senhavetor[0]<<24)+
                     ((unsigned long)senhavetor[1]<<16)+
                     ((unsigned long)senhavetor[2]<< 8)+
                     ((unsigned long)senhavetor[3]<< 0);                           

         PROCULUS_VP_Read(382,senhavetor,4);  //L� registrador do teclado 4Bytes
         nova_senha =((unsigned long)senhavetor[0]<<24)+
                     ((unsigned long)senhavetor[1]<<16)+
                     ((unsigned long)senhavetor[2]<< 8)+
                     ((unsigned long)senhavetor[3]<< 0);                          


         PROCULUS_VP_Read(384,senhavetor,4);  //L� registrador do teclado 4Bytes
         confirma_senha=((unsigned long)senhavetor[0]<<24)+
                        ((unsigned long)senhavetor[1]<<16)+
                        ((unsigned long)senhavetor[2]<< 8)+
                        ((unsigned long)senhavetor[3]<< 0);                          



         if((senha_atual==0) ||
            (nova_senha ==0) ||
            (confirma_senha==0))
            { 
            //PROCULUS_VP_Write_String(1660,"Nenhuma senha deve ser zero");                          
            PROCULUS_Popup(SENHA_ZERO_INVALIDA) ;
            }
         else
         if(senha_atual==EEPROM_Read_Long32(11))
            { 
            if(nova_senha==confirma_senha)
               {// Cadastrada com Sucesso
               EEPROM_Write_Long32(11,nova_senha) ;
               senha_atual=nova_senha;               
               Gravar_Status_da_Senha_Global();
               PROCULUS_VP_Write_Long32(380,0);
               PROCULUS_VP_Write_Long32(382,0);   //Nova senha cadastrada com sucesso!
               PROCULUS_VP_Write_Long32(384,0);
               PROCULUS_Popup(SENHA_CADASTRADA_SUCESSO) ;               
               } 
            else
               {
               //PROCULUS_VP_Write_String(1660,"A nova senha nao conferem "); 
               PROCULUS_Popup(SENHAS_DIFERENTES) ;
               }                             
            }
         else
            { 
            //Senha atual invalida 
            //PROCULUS_VP_Write_String(1660,"Senha atual invalida"); 
            PROCULUS_Popup(SENHA_INVALIDA) ;
            }

}



void Exibe_Hora_Data(char showseconds){
     char date[10];
     char time[10];     
     
     PROCULUS_Read_RTC(date,time);
     if(showseconds==FALSE) time[5]=0;
     PROCULUS_VP_Write_String(20,date);
     PROCULUS_VP_Write_String(30,time);            
}






//Inicializa parametros de seguran�a.
void Inicializar_Seguranca(void){
     Seg_Condensador=EEPROM_Read_Integer(0x01);
     Seg_Vacuo=EEPROM_Read_Integer(0x03);
     Seg_Aq_cond=EEPROM_Read_Integer(0x05);
     Seg_Aq_vacuo=EEPROM_Read_Integer(0x07);        
     PROCULUS_VP_Write_UInt16(210,Seg_Condensador); // Seguran�a Condensador
     PROCULUS_VP_Write_UInt16(211,Seg_Vacuo); // Seguran�a Vacuo
     PROCULUS_VP_Write_UInt16(212,Seg_Aq_cond); // Seguranca aquec. cond.
     PROCULUS_VP_Write_UInt16(213,Seg_Aq_vacuo); // Seguranca aquec. Vacuo
}





void Check_And_Send_Capture_Datalog(void){
     if(flag_global_datalog==1)
       { 
       if(flag_capture_datalog==1)
         {  
         flag_capture_datalog=0; 
         //PROCULUS_OK();
         save_datalog(0);         
         //processo.pontos++; //Totaliza os pontos capturados para indicar na Fat
                            //o tamanho do registro.
         }
       }
}





 


void Contagem_Tempo_de_Processo(char value){
    if(value)
      { 
      if(flag_time_process==FALSE) 
         { 
         processo_minuto=0;
         processo_hora=0;
         }
      PROCULUS_VP_Write_String(1970,"Executando Processo...");  
      flag_time_process=1;      
       //Write_Fat(TRUE);
      }
    else
      {  
      PROCULUS_VP_Write_String(1970,"");  
      flag_time_process=0;
      rtc.milisegundo=725;     
      //Write_Fat(FALSE);
      }        
        
}

void SaveBlackoutStatusRuning(void){
     if(processo_minuto%10==0)
       {
       if(flag_save_time==0)  
          {
          flag_save_time=1;  
          EEPROM_Write_Byte(17,processo_hora);     //Hora
          EEPROM_Write_Byte(18,processo_minuto);   //Minuto        
          }
       }
     else
       {  
       flag_save_time=0;  
       } 
}


void SaveBlackoutStatus(void){
     EEPROM_Write_Byte(16,statuspower.bits);  //Todos os Status
     EEPROM_Write_Byte(17,processo_hora);     //Hora
     EEPROM_Write_Byte(18,processo_minuto);   //Minuto
}




void RecallBlackoutStatus(void){
     statuspower.bits=EEPROM_Read_Byte(16);
     processo_hora=EEPROM_Read_Byte(17);
     processo_minuto=EEPROM_Read_Byte(18);

     if(flag_global_datalog) PROCULUS_VP_Write_UInt16(2,1);
     if(flag_global_condensador) PROCULUS_VP_Write_UInt16(3,1);
     if(flag_global_vacuo) PROCULUS_VP_Write_UInt16(4,1);
     if(flag_global_aquecimento) PROCULUS_VP_Write_UInt16(5,1);
     //flag_time_process = autometico ao carregar;
     
     flag_global_datalog=0;     
     flag_global_condensador=0;
     flag_global_vacuo=0; 
     flag_global_aquecimento=0;    
}





void Carregar_tempo_de_datalog(void){
     tempocapturaconstante=(EEPROM_Read_Integer(0x09)*1000);
     tempocaptura=tempocapturaconstante;     
}


void Gerenciador_de_Senha(void){
     unsigned long senha;
    //=================== ENTRAR COM A SENHA DO ADMINISTRADOR===============
    PROCULUS_VP_Read(155,senhavetor,4);  //L� registrador do teclado 4Bytes
    senha=((unsigned long)senhavetor[0]<<24)+
          ((unsigned long)senhavetor[1]<<16)+
          ((unsigned long)senhavetor[2]<< 8)+
          ((unsigned long)senhavetor[3]<< 0);  


    if(senha!=0)
        { 
        if(senha==senha_atual)
           { 
           senha=0; 
           senhacount=TIME_PASSWORD; 
           flag_senha_liberada=TRUE;
           PROCULUS_VP_Write_UInt16(154,1); //torna o icone da senha verde    
           PROCULUS_VP_Write_UInt16(155,0); //VP Duplo do teclado onde
           PROCULUS_VP_Write_UInt16(156,0); // eh armazenado a senha           
           PROCULUS_OK();
           }
        }


    if((flag_senha_liberada==TRUE)&&(senhacount<0))
        {           
        flag_senha_liberada=FALSE; 
        PROCULUS_VP_Write_UInt16(154,0); //torna o icone vermelho 
        PROCULUS_NOK(); 
        }              


    if(PROCULUS_TPFLAG_Read()==0x5A) //Se houver toque no display
       { 
       senhacount=TIME_PASSWORD;     //Reiniciar o contador
       PROCULUS_TPFLAG_Write(0);     //Reinicia o reg. de toque
       }   
    //----------------------------------------------------------------------
}




void Gerenciador_de_Senha_Global(void){
     unsigned long senha;
    //=================== ENTRAR COM A SENHA DO ADMINISTRADOR===============
    PROCULUS_VP_Read(179,senhavetor,4);  //L� registrador do teclado 4Bytes
    senha=((unsigned long)senhavetor[0]<<24)+
          ((unsigned long)senhavetor[1]<<16)+
          ((unsigned long)senhavetor[2]<< 8)+
          ((unsigned long)senhavetor[3]<< 0);  


    if(senha!=0)
        { 
        if(senha==senha_atual)
           { 
           senha=0; //Zera Senha a ser testada          
           PROCULUS_VP_Write_Long32(179,0); // Zerar o teclado
           if(PROCULUS_VP_Read_UInt16(178)==1) //Icone
              { 
              flag_senha_global_liberada=FALSE; 
              flag_senha_liberada=FALSE;
              senhacount=-1;
              PROCULUS_VP_Write_UInt16(178,0); //torna o icone da senha GLOBAL vermelho.
              PROCULUS_VP_Write_UInt16(154,0); //torna o icone da senha vermelho
              }        
           else if(PROCULUS_VP_Read_UInt16(178)==0)          
              { 
              flag_senha_global_liberada=TRUE; 
              flag_senha_liberada=TRUE;
              senhacount=10;
              PROCULUS_VP_Write_UInt16(178,1); //torna o icone da senha GLOBAL verde
              PROCULUS_VP_Write_UInt16(154,1); //torna o icone da senha verde
              }                
           Gravar_Status_da_Senha_Global();
           PROCULUS_OK();
           }
        }
    //----------------------------------------------------------------------
}




void Icones_de_alarmes(void){
     //-------------------------------------------------------------------------    
     if (Condensador<Seg_Condensador)       
         PROCULUS_VP_Write_UInt16(176,1);         
     else         
         PROCULUS_VP_Write_UInt16(176,0);
     //-------------------------------------------------------------------------    
     if (Vacuometro<Seg_Vacuo)         
         PROCULUS_VP_Write_UInt16(177,1);         
     else         
         PROCULUS_VP_Write_UInt16(177,0);
     //-------------------------------------------------------------------------
}

void Formatar_Lista_de_Receitas(void){
     t_receita receita;
     for(char i=0;i<8;i++) 
        { 
        //flag_main_loop_WDT=1; 
        receita.setpoint=0;
        receita.potenciaON=0;
        receita.potenciaOFF=0;
        receita.histerese=0;
        strcpy(receita.nome,"");           
        Grava_Receita(i, &receita);        
        } 
}

void Carregar_Parametros_de_Seguranca(void){     
     Seg_Condensador=EEPROM_Read_Integer(0x01);
     Seg_Vacuo=EEPROM_Read_Integer(0x03);
     Seg_Aq_cond=EEPROM_Read_Integer(0x05);
     Seg_Aq_vacuo=EEPROM_Read_Integer(0x07);  
     PROCULUS_VP_Write_UInt16(210,Seg_Condensador); // Seguran�a Condensador
     PROCULUS_VP_Write_UInt16(211,Seg_Vacuo); // Seguran�a Vacuo
     PROCULUS_VP_Write_UInt16(212,Seg_Aq_cond); // Seguranca aquec. cond.
     PROCULUS_VP_Write_UInt16(213,Seg_Aq_vacuo); // Seguranca aquec. Vacuo  
     
     PROCULUS_VP_Write_UInt16(172,EEPROM_Read_Integer(0x09)); //Tempo de captura do grafico       
}

void Formatar_Dados_de_Seguranca(void){
     EEPROM_Write_Integer(0x01,-150);    //Condensador
     EEPROM_Write_Integer(0x03,10000);    //Vacuo
     EEPROM_Write_Integer(0x05,-150);      //Condensador aquecimento
     EEPROM_Write_Integer(0x07,8000);    //Vacuo aquecimento
}

void Carregar_Status_da_Senha_Global(void){
     flag_senha_global_liberada=EEPROM_Read_Byte(15);
     if(flag_senha_global_liberada)
        { 
        PROCULUS_VP_Write_UInt16(178,1);//torna o icone da senha global verde 
        flag_senha_liberada=TRUE;
        senhacount=10;
        PROCULUS_VP_Write_UInt16(154,1); //torna o icone da senha verde
        }
     else
        { 
        PROCULUS_VP_Write_UInt16(178,0);//torna o icone da senha global vermelho  
        flag_senha_liberada=FALSE;
        senhacount=-1;
        PROCULUS_VP_Write_UInt16(154,0); //torna o icone da senha vermelho
        }
}

void Gravar_Status_da_Senha_Global(void){
     EEPROM_Write_Byte(15,flag_senha_global_liberada);
}


void Formatar_Datalog(void){
    int i;
    for(i=500;i<=720;i+=20)    
       {
       //flag_main_loop_WDT=1; 
       PROCULUS_VP_Write_String(i,""); 
       PROCULUS_VP_Write_String(i+3,""); 
       PROCULUS_VP_Write_String(i+13,"");       
       }
}



int Tupla_Log_Free(void){
    int i;
    for(i=500;i<=720;i+=20)
        {  
        PROCULUS_VP_Read_String(i,texto);
        if(strlen(texto)==0) break;
        my_delay_ms_CLRWDT(500);
        } 
    return i;
}


void Write_Fat(char value){
     char date[10];
     char time[10];    
     char index;
     unsigned long add_memo;
      
     
     PROCULUS_Read_RTC(date,time);
     if(value==TRUE)
       { 
       strcpy(processo.inicio.date,date);
       strcpy(processo.inicio.time,time);
       processo.pontos=0;
       }
     else
       {  
       strcpy(processo.fim.date,date);
       strcpy(processo.fim.time,time);
       processo.amostra=EEPROM_Read_Integer(0x09); //Amostra em captura por seg.
              
       //Grava efetivamente na mem�ria.       
       for(index=0;index<9; index++)
           {
           //flag_main_loop_WDT=1;
           add_memo=index*(sizeof(t_fat)+1);
           if(EEPROM_24C1025_Read_Byte(0,add_memo)==0xFF)
              {    
              processo.start_add=add_memo;
              processo.end_add=add_memo+processo.pontos;              
              }
           }
     
     
       }
}


void Memo2Graphic(char SlaveBoardAdd, char chipNumber, int add_24C1025, char LCDchannel){
     char bb[3];
     int  value;
     
     bb[0]=chipNumber;
     bb[1]=Hi(add_24C1025);
     bb[2]=Lo(add_24C1025);        
     
     value=Send_To_Slave(SlaveBoardAdd, COMMAND_EEE_R_INT, 3, bb);
     //itoa(value,texto,10);
     //print(texto);     
     PROCULUS_graphic_plot(LCDchannel, value);
}  

//Buffer de recebimento de dados pela serial. Pode receber at� 10 comandos que
//ficarao armazenados na memoria e serao processados.
void Buffer_Manager(void)
     {
     char i;
     if(flag_usart_rx==FALSE)
         {
         for(i=USART_LINE_BUFFER_SIZE;i>0;i--)
           {
           //flag_main_loop_WDT=1;  
           if(usart_buffer_fila[i-1][0]!=0) 
              { 
              for(char j=0;j<USART_BUFFER_SIZE;j++) 
                  {
                  //flag_main_loop_WDT=1;
                  usart_buffer[j]=usart_buffer_fila[i-1][j];
                  }          
              usart_buffer_fila[i-1][0]=0; 
              flag_usart_rx=TRUE;
              MonitorBuffer=i-1;
              break;                
              }
           }
         }
}  
     
     
     



//------------------------------------------------------------------------------
unsigned int Captura_Pagina(void){
    unsigned int pagina[5];    
    unsigned int retorno=0;   
    
    while(retorno==0)
       { 
       //flag_main_loop_WDT=1; 
       for(char i=0;i<5;i++) 
          pagina[i]=PROCULUS_Get_Page();

          retorno=pagina[0];
          for(char i=1;i<5;i++)
             {
             if(pagina[0]!=pagina[i]) 
                { 
                retorno=0;
                break;
                }
             }    
       }
    return retorno;
}


_Bool memory_test(char board, char chip, int value, int inicialadd, int finaladd)
     {    
     char txt[30];
     char bb[5];    
     int salva=0;
     int ler=0;
     //PROCULUS_Show_Screen(0); //35grafico , 0 Print
     strcpy(texto,"BOARD =");
     itoa(board,txt,16);
     strcat(texto,txt);
     print(texto);
             
             
             
             for(int add=inicialadd; add<=finaladd; add++)             
                {
                //flag_main_loop_WDT=1; 
                bb[0]=chip;  //chip
                bb[1]=Hi(add); //___ADD
                bb[2]=Lo(add); //      
                bb[3]=Hi(salva); //___VALUE
                bb[4]=Lo(salva); //              
                Send_To_Slave(board, COMMAND_EEE_W_INT, 5, bb); 
                
                bb[0]=chip;
                bb[1]=Hi(add);
                bb[2]=Lo(add);
                ler=Send_To_Slave(board, COMMAND_EEE_R_INT, 3, bb); 
                //ler=salva;
                        
                strcpy(texto,"W 0x");                
                itoa(add,txt,16);
                strcat(texto,txt);               
                strcpy(txt," = ");
                strcat(texto,txt); 
                itoa(value,txt,10);
                strcat(texto,txt);                              
                
                
                if(salva==ler)
                   { 
                   strcpy(txt," Iguais.");
                   }
                else
                   { 
                   strcpy(txt," diferentes.");
                   //return FALSE;
                   }
                strcat(texto,txt);
                print(texto);
                value++;
                }                       
     return TRUE;
     }        



char menorValorDisponivel(char * trendCurve){
     unsigned char icone; 
     char i,j,p;    
      //=====================================================
      icone=0;
      for(p=1;p<14;p++)
         {
         trendExist=FALSE;
         for(i=1;i<14;i++)
            { 
            for(j=i;j<14;j++)
               {
               //flag_main_loop_WDT=1; 
               if(trendCurve[j]==p) 
                  { 
                  trendExist=TRUE;
                  break;
                  }
               }
            if(trendExist==TRUE)
               {
               break;
               }
            }
         if(trendExist==FALSE)
           { 
           icone=p;  
           break;  
            }
         }
      //----------------------------------------------------
     return icone;
}      



void Exibe_Tempo_de_Processo(void){
     if(flag_time_process)
       {
       //update_time_process();
       char temp0,temp1;     

       temp0=processo_hora/10;
       temp1=processo_hora%10;
       PROCULUS_VP_Write_UInt16(40,temp0);
       PROCULUS_VP_Write_UInt16(41,temp1);

       PROCULUS_VP_Write_UInt16(42,1); 

       temp0=processo_minuto/10;
       temp1=processo_minuto%10;
       PROCULUS_VP_Write_UInt16(43,temp0);
       PROCULUS_VP_Write_UInt16(44,temp1);                          
      }
}



void Atualizar_Lista_de_Receitas(void){
     for(char i=0;i<8;i++)                         
        {
        //flag_main_loop_WDT=1; 
        Exibe_Receita(i);        
        }     
}





void TrendCurveFuncao(char funcao){
     char figura;
     char canal;
     int  cor;
     char i, index;
     switch(funcao)
           {
           case FORMAT:
                       for(i=0;i<13;i++)
                          { 
                          mapa.cor[i]=0xFFFF;
						  mapa.canal[i]=0x0A;                          
						  mapa.icone[i]=-1;                          
                          mapa.vpIcone[i]=-1;
                          mapa.entrada[i]=NULL;
                          mapa.fator[i]=0;
                          
                          EEPROM_Write_Byte(19+i,mapa.vpIcone[i]);
                          EEPROM_Write_Byte(0xEA+i,mapa.icone[i]);        
                          EEPROM_Write_Byte(36+i,mapa.canal[i]);
                          
                          PROCULUS_VP_Write_UInt16(0x0310+i,-1);           //Figura
                          PROCULUS_VP_Write_UInt16((i*10+PPCANAL),0x0A00); //Canal
                          PROCULUS_VP_Write_UInt16((i*10+PPCOR),0xFFFF);   //Cor 
                          }               
                       break;
           case   LOAD:
                      
                       EEPROM_Read_Buffer(19,mapa.vpIcone,15);
                       EEPROM_Read_Buffer(0xEA,mapa.icone,15);
                       EEPROM_Read_Buffer(36,mapa.canal,15);
                       


                       //-----------------------------------ICONE------------------------------------
                       for(i=0;i<13;i++)
                          {                          
                          figura=mapa.vpIcone[i];
                          if(figura!=255) 
                             {							   
                             PROCULUS_VP_Write_UInt16(0x0310+i, figura); //Figura -quadro colorido                             
                             }
                          else
                            {							
                            PROCULUS_VP_Write_UInt16(0x0310+i,-1);                            
                            }
                          }
                       
					   //------------------------------------COR-------------------------------------
                       for(i=0;i<13;i++)   
                          {
                          if(mapa.canal[i]<8)	
                             {                            
							 cor=TrendColor[mapa.icone[i]-1];								 
							 }
						  else
						     {
						     cor=0xFFFF;
						     } 						     
						  PROCULUS_VP_Write_UInt16((i*10+PPCOR),cor);
						  mapa.cor[i]=cor; 		
						  }
                          
                       //--------------------------------------CANAL------------------------------------------
                       for(i=0;i<13;i++)
                          { 
                          canal=mapa.canal[i];
						  if((canal>=0)&&(canal<=7))
						     {
                             PROCULUS_VP_Write_UInt16((i*10+PPCANAL),(canal<<8)|(0x0001)); //Canal Associado                             						     	
							 }
						  else	 
							 {							 
                             PROCULUS_VP_Write_UInt16((i*10+PPCANAL),0x0A00); //Canal                             
						     }
					      }
					      
                       
                       
                          //----------------------------------------VALOR-----------------------------------------------
                          for(index=0;index<8;index++)
                              {                                       
                              if(mapa.canal[index]!=0x0A)
                                 {                       
                                 mapa.entrada[mapa.canal[index]]=&leitura[saltaIndice4(mapa.icone[mapa.canal[index]]-1)];
                                 mapa.fator[mapa.canal[index]]=FATOR_PADRAO;                                                
                                 if(mapa.canal[index]==0)mapa.fator[mapa.canal[index]]=FATOR_TENSAO;
                                 if(mapa.canal[index]==1)mapa.fator[mapa.canal[index]]=FATOR_VACUO;                                                                
                                 }
                              }
                          				      
					      
                       break;
           case   SAVE:
                       for(i=0;i<14;i++)   
                           {
                           mapa.vpIcone[i]=PROCULUS_VP_Read_UInt16(0x0310+i);
						   mapa.canal[i]=PROCULUS_VP_Read_UInt16(i*10+PPCANAL)>>8;	
                           }
                       EEPROM_Write_Buffer(19,&mapa.vpIcone[0],15);
                       EEPROM_Write_Buffer(0xEA,&mapa.icone[0],15);
                       EEPROM_Write_Buffer(36,&mapa.canal[0],15);   
                       break;

                                                            
           } 
}

    


char buscaIndex(char *buffer,char valor)
{
	char i;
	for(i=0;i<13;i++)
	   {
	   if(buffer[i]==valor) break;
	   }
	return i;   
}




void AcordaFilha(){      
     flag_global_datalog=0;
     flag_global_condensador=0;
     flag_global_vacuo=0;     
     flag_global_aquecimento=0;         
}


void showTotalReset(void){
    clear_screen();
    print("-----RELATORIO TECNICO----");
    //ShowHardwareInfo();
    print("Reinicializacoes:");
    if(PROCULUS_VP_Read_UInt16(0x00AA)==1)                   
      {
      char bb[3];
      int  valor; 
      char placa;
      char vetor[3];                              
      PROCULUS_VP_Write_UInt16(0x00AA,0);      
      PROCULUS_Show_Screen(0);
      valor=EEPROM_Read_Integer(34);
      itoa(valor,buffer,10);
      strcpy(texto,"Placa 0 = ");
      strcat(texto,buffer);
      print(texto);
      for(placa=1;placa<=totalboard;placa++)
          {  
          //flag_main_loop_WDT=1;
          strcpy(texto,"Placa ");
          itoa(placa,buffer,10); 
          strcat(texto,buffer);
          strcpy(buffer," = ");
          strcat(texto,buffer);
          bb[0]=0x10;
          valor=Send_To_Slave(placa,COMMAND_IEE_R_INT,1,bb);
          itoa(valor,buffer,10);                                  
          strcat(texto,buffer);
          print(texto);                            
          }

      print("Desligar do Vacuo:");
      valor=EEPROM_Read_Integer(0xE8);
      itoa(valor,texto,10);
      strcat(texto," vez(es).");
      print(texto);      

      my_delay_ms_CLRWDT(15000);
      PROCULUS_Show_Screen(15);
      }
            
}


void ShowHardwareInfo(){
     char i;
     char destino; 
     char tipo;
     int  resposta;
     char versao[10];
     Send_To_Slave(destino, COMANDO_QUEM_EH_VOCE, 0, buffer);
     totalboard=0;
     strcpy(texto,"");
     strcat(texto,"* : Mother Board ");
     strcat(texto,FVERSION);
     print(texto);
     for(destino=1;destino<15;destino++)
        {
        my_delay_ms_CLRWDT(100); 
        Buffer_Manager();        
        resposta = Send_To_Slave(destino, COMANDO_QUEM_EH_VOCE, 0, buffer);  
        tipo=Hi(resposta);
        if(resposta!=-1)
            {            
            totalboard++;
            strcpy(texto,"");
            itoa(destino,texto,10); 
            strcat(texto," : ");             
            strcat(texto, boardtype[tipo]);
            strcat(texto," ");
            strcpy(buffer," ");
            Send_To_Slave(destino, COMMAND_VERSION, 0, buffer);
            strcat(texto,buffer);
            print(texto);
            }
        }
      

     //totalboard=5; //fix
     if(totalboard==0)
        { 
        print("Nenhuma Placa conectada!");        
        }
     else
        { 
        itoa(totalboard,texto,10);
        strcat(texto," placa(s) encontrada(s).");
        print(texto);
        }
     
    
}

char MenorCanalLivre()
     {
     unsigned char canal, canalLivre, canalBusca, busca, Exist;	
     canal=0x0A;	
     for(canalLivre=0;canalLivre<8;canalLivre++)
	    {	    	
	    //printf("Buscando existencia de canal %d\n",canalLivre);
	    Exist=FALSE;
	    for(canalBusca=0;canalBusca<13;canalBusca++)		
	       {	       
	       busca=mapa.canal[canalBusca];
	       //printf("Varrendo figura %d=%d    \n",canalLivre,busca);
		   if(canalLivre==busca)
			 {
			 //printf("Existe o canal %d\n",canalLivre);	
			 Exist=TRUE;
			 break;	
			 }  
		   }
		if(Exist==FALSE)
		  {
		  //printf("NAO Existe o canal %d\n",canalLivre);	
		  canal=canalLivre;	
		  break;
		  }		   
		}
	 //printf("Retornando canal %d\n",canal);	
	 return canal;	
	 }





void Incrementa_Contador_de_Repique_do_Vacuo(){
     int valor;
     if(flag_Vacuo_estava_ligado==1)
       {
       flag_Vacuo_estava_ligado=0;      
       valor=EEPROM_Read_Integer(0xE8);
       valor++;
       EEPROM_Write_Integer(0xE8,valor); 
       }
}
