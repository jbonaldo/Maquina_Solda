#include "stdlib.h"
#include "J_RMS.h" 
#include "Modo_KIR.h" 

extern struct _Rms3Fases Rms;
extern struct _kir  Kir;

//#define COMPENSAR_TENSAO



#pragma CODE_SECTION(AquisitaCanal_1, "ramfuncs");
//#pragma CODE_SECTION(Rotina_Teclado, "ramfuncs");


union long2float {
	float nFloat;
	unsigned long unLong;
};

/*
struct _solda {
	Uint16 modo; //0->skt;   1->kir
	Uint16 cont_ciclos;
	Uint16 ciclo_atual;
	Uint16 ciclos_ativos;
	Uint16 ciclos_total;
	Uint16 soldar;
	float I_rms_media;
	
};
*/

//extern struct _solda Solda;

struct _disparo_corte_scr { 
	Uint16 ticks_min;           
	Uint16 ticks_disparo;		//valor do counter onde os pulsos de disparo do scr sao aplicados
	Uint16 ticks_corta_pulsos;  //valor do counter onde os pulsos de disparo do scr sao retirados	
	Uint16 SKT;
	float Alpha;
};



extern struct _disparo_corte_scr Controle_SCRs;


#define T_AMOSTRAGEM_US	40
#define T_AMOSTRAGEM    40e-6;
#define FREQ_AMOSTRAGEM 25000 	// aprox: 120Hz * 208 pontos


#define CORRENTE_FUNDO_ESCALA   (25.0)	//fundo escala correspondente a entrada do Mux com menor ganho
#define TENSAO_FUNDO_ESCALA     1.0


#define MODO_SKT      	0x00
#define MODO_KIR      	0x01
#define MODO_CONFIG_KIR	0x02


#define GANHO_CORRENTE_0_25_A	  2
#define GANHO_CORRENTE_25_50_A	  1
#define GANHO_CORRENTE_50_100_A	  0
#define GANHO_CORRENTE_100_200_A  3

#define RESETA_INTEGRADOR       GpioDataRegs.GPACLEAR.bit.GPIO25 = 1
#define LIBERA_INTEGRADOR       GpioDataRegs.GPASET.bit.GPIO25 = 1	

#define PinMuxAnalogico_bit0_set    GpioDataRegs.GPASET.bit.GPIO8 = 1
#define PinMuxAnalogico_bit0_clear  GpioDataRegs.GPACLEAR.bit.GPIO8 = 1
#define PinMuxAnalogico_bit1_set    GpioDataRegs.GPASET.bit.GPIO10 = 1
#define PinMuxAnalogico_bit1_clear  GpioDataRegs.GPACLEAR.bit.GPIO10 = 1
#define PinMuxAnalogico_bit2_set    GpioDataRegs.GPASET.bit.GPIO16 = 1
#define PinMuxAnalogico_bit2_clear  GpioDataRegs.GPACLEAR.bit.GPIO16 = 1

#define PinoSoldar				    GpioDataRegs.GPBDAT.bit.GPIO61
#define PinoEmergencia			    GpioDataRegs.GPADAT.bit.GPIO30
#define PinoDetecaoRogowiski	    GpioDataRegs.GPADAT.bit.GPIO24
#define PinoDetecaoRogowiski_ligaFonteC	    GpioDataRegs.GPASET.bit.GPIO24 = 1
#define PinoDetecaoRogowiski_deslFonteC	    GpioDataRegs.GPACLEAR.bit.GPIO24 = 1		
#define TogglePinoDetectaZero	      GpioDataRegs.GPBTOGGLE.bit.GPIO58 //este eh o toggle correto, utilizando pelo programa no pic tambem
#define TogglePinoDetectaZero_errado	    GpioDataRegs.GPBTOGGLE.bit.GPIO59
#define	PIN_ATIVO		1
//#define	PARAR		0

#define CONV_CORRENTE_INICIAR        GpioDataRegs.GPACLEAR.bit.GPIO19 = 1
#define CONV_CORRENTE_TERMINAR       GpioDataRegs.GPASET.bit.GPIO19 = 1
#define CONV_TENSAO_INICIAR          GpioDataRegs.GPACLEAR.bit.GPIO27 = 1
#define CONV_TENSAO_TERMINAR         GpioDataRegs.GPASET.bit.GPIO27 = 1


#define DEBUG_GPIO12	GpioDataRegs.GPADAT.bit.GPIO12
#define DEBUG_GPIO13	GpioDataRegs.GPADAT.bit.GPIO13
#define DEBUG_GPIO14	GpioDataRegs.GPADAT.bit.GPIO14	

#define INDIVIDUAL	0x01
#define SERIE		0x02
#define COSTURA		0x03
#define CONTINUA	0x04



#define SKT_MAX		999
//#define LIMIAR_BOBINA	0.05	//Limiar que indica a presena ou ausencia da bobina
#define ERRO_AUSENCIA_BOBINA	0x00000001

//#define CRISTAL_20MHZ	//Se o crista da placa do DSP for de 30 MHz, comentar esta linha

Uint16 DetecaoRogowiski();
Uint16 DetectaRogowiski(void);
extern void AquisitaCanal_1();
void SetaSkt(Uint16 skt);
void SktUpdate(Uint16 skt);
extern void Rotina_Teclado();
extern void Inicializa_Variaveis();
void Habilita_Pulsos();
void Desabilita_Pulsos();
void Iniciar_Solda();
void Parar_Solda();
void DetecaoZeroPLL();
void set_mux_corrente(unsigned int in);
extern struct _compensacaoTensao CompensacaoTensao;

void trem_pulsos_tiristor_acionar(void);
void trem_pulsos_tiristor_remover(void);

void Dog_Reset();

//Rotinas SciA
extern void Configura_SciA();
interrupt void sciA_Rx_isr(void);
extern void SciA_TelaInicial();
extern void SciA_Comandos_Hiperterminal();
void RecebePacote(void);
void Reportar_Erro_Ilegal();
void SciReportarErro(Uint32 erro);



//Rotinas SpiA
void SpiA_Configura();
void SpiA_CalibracaoAD();
//void SpiA_ConversaoAD(Uint16 *valor_amostrado);
Uint16 SpiA_AquisicaoTensao();
Uint16 SpiA_AquisicaoCorrente();
void McBsp_DAC(Uint16 valor, Uint16 enable);
interrupt void spiTxFifoIsr(void);
interrupt void spiRxFifoIsr(void);
extern Uint16 Adc_offset[];



//Rotinas xInt
extern void xInt_Configura();
extern void xInt_ConfiguraIO();
extern interrupt void xInt3_isr(void);

//Rotinas PLL
void InicializaPLL();
float PLL(float tensao);







//Debug
extern float CorrenteSkt[30];   //debeug
extern unsigned int k_ciclos, Namostras[30];	 			//debug


