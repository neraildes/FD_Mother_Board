/* 
 * File:   Liofilizador Placa Mae.h
 * Author: Nera
 *
 * Created on 7 de Abril de 2019, 12:04
 */

#ifndef LIOFILIZADOR_PLACA_MAE_H
#define	LIOFILIZADOR_PLACA_MAE_H

#define saltaIndice4(arg1) (arg1<3)?(arg1):((arg1)+1)

typedef struct{
	          int  *entrada[15];
	          float   fator[15];
	          char    canal[15];
	          char    icone[15];
	          char  vpIcone[15];
			  int       cor[15];	          
              } T_mapa;

//------------------------------------------------------------------------------
// Estrutura utilizada na primeira p�gina, onde exibe aquecimento em tempo real.
typedef struct{ 
        unsigned char plataforma;   //0
        //       int  temperatura;  //none
                 int  setpoint;     //1e2
        unsigned char tempoON;      //3
        unsigned char tempoOFF;     //4
        unsigned char histerese;    //5
                 char receita[9];   //6
        unsigned int  status ;      //13
} t_liofilizador;


//------------------------------------------------------------------------------
//Estrutura utilizada para edi��o de parametros de aquecimento.
typedef struct{
    unsigned char plataforma;
    int           setpoint;
    unsigned int  potenciaON;
    unsigned int  potenciaOFF;
    unsigned char histerese;
    unsigned char receita[10];
    unsigned int  status0;        
} t_aquecimento;

typedef struct{
    int           setpoint;
    unsigned int  potenciaON;
    unsigned int  potenciaOFF;
    unsigned char histerese;
             char nome[10];    
} t_receita;

typedef struct{
    char date[10];
    char time[10];
} t_tempo;

typedef struct{
    t_tempo      inicio;
    unsigned int start_add;
    t_tempo fim;
    unsigned int end_add;
    unsigned int amostra;
    unsigned int pontos;
} t_fat;


unsigned char countboard(void);
int  Send_To_Slave(char destino, char comando, char size, char * buffer); //Comando padrao para conversar com escravo.
int  Send_To_Slave_EMULA(char destino, char comando, char size, char * buffer); //Comando padrao para conversar com escravo.
void ShowSensorRealTimeHS(void);
void showTotalReset(void);
void AcordaFilha(void);


void SaveLiofilizadorOnMemory(char index,t_liofilizador *liofilizador); //Salva dados de aquecimento na mem�ria.
void ShowStaticValueGrid(unsigned char tupla); //Exibe at� 15 placas em tempo real. 

void ShowAndSetSlaveParameters(unsigned char tupla);//Exibe e seta parametros dos escravos
void Comando_Protocolo_Serial(void); //Comunicacao com o computador
void Send_to_PC(unsigned char size); //Envia resposta para o PC

void Upload_Data_to_Slave(void); //Transfere dados da placa m�e para placa filha
void Decodify_Command(void);  //Decodifica comandos vindo do PC (Delphi)
void SEND_REPLY_OK(void); //Envia Ok para o PC


void global_datalog(void);      //Liga/Desliga datalog
void global_condensador(void);  //Liga/Desliga condensador
void global_vacuo(void);        //Liga/Desliga vacuo
void global_aquecimento(void);  //Liga/Desliga aquecimento

void Exibe_Hora_Data(char showseconds);     //Faz leitura de tempo do RTC do Display
void Exibe_Tempo_de_Processo(void);

void DataBaseBackupMain(unsigned char tupla); //Grava dados na Placa M�e.
void Formatar_Banco_de_Dados(char inicio, char total); //Formata banco de dados
void Salva_Seguranca(void);
void Formatar_Datalog(void);
void ShowMessage(char mensagem[30],unsigned int delay, char SoundType, char retem); // Tela de mensagem, comunicacao com usuario

void Comando_Microcomputador(void); //Interpreta comando do microcomputador
void Comando_Display(void); //Interpreta comando do display


void Inicializar_Seguranca(void);  // Parametros de seguran�a sendo inicializados

void SaveBlackoutStatus(void);
void SaveBlackoutStatusRuning(void);
void RecallBlackoutStatus(void);

void Check_And_Send_Capture_Datalog(void); //Analisa condi��o para salvar datalog
void save_datalog(unsigned int add); //Envia comando de captura para todas as placas simultaneamente

void Contagem_Tempo_de_Processo(char value);
void Carregar_tempo_de_datalog(void);

void Carrega_Tupla_Receita(char index, t_receita *receita);

void ShowHardwareInfo(void);

void Global_Aquecimento_Switch(unsigned char estado);
void Gerenciador_de_Senha(void);
void Gerenciador_de_Senha_Global(void);
void Icones_de_alarmes(void);

void Condensador_Switch(unsigned char estado);
void Vaccum_Switch(unsigned char estado);
void TrendCurveFuncao(char funcao);
char buscaIndex(char *buffer,char valor);

void Exibe_Receita(int add_receita);
void Grava_Receita(char index, t_receita *receita);
void Formatar_Lista_de_Receitas(void);
void Formatar_Dados_de_Seguranca(void);
void Carregar_Parametros_de_Seguranca(void);

void Gravar_Status_da_Senha_Global(void);
void Carregar_Status_da_Senha_Global(void);

void Atualizar_Lista_de_Receitas(void);

int  Tupla_Log_Free(void);
void Write_Fat(char value);
void Memo2Graphic(char SlaveBoardAdd, char chipNumber, int add_24C1025, char LCDchannel);
//void Teclas_da_Tela_de_Aquecimento(void);
void Buffer_Manager(void);


unsigned int Captura_Pagina(void);
_Bool memory_test(char board, char chip, int value, int inicialadd, int finaladd);
char menorValorDisponivel(char * trendCurve);

 
 void Set_Receita(unsigned char index, char status);

void pagina_15(void);  //Primeira p�gina
void pagina_19(void);  // Tela de edi��o de funcionamento 10 tuplas     
void pagina_23(void);
void pagina_25(void);  // Editar e Salvar Data e Hora
void pagina_29(void);  //Captura de Datalog, altera��o de valor   
void pagina_31(void);  //Captura de Datalog, altera��o de valor  
void pagina_47(void);  //Receita Lista
void pagina_49(void);  //Receita Edi��o

char MenorCanalLivre();

void Incrementa_Contador_de_Repique_do_Vacuo();

#endif	/* LIOFILIZADOR_PLACA_MAE_H */
