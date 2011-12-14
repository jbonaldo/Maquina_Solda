#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File
#include "Jakson_Bibliotecas/Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário
#include "Jakson_Bibliotecas/Solda.h"
#include <math.h>  //debug

///#pragma CODE_SECTION(InitFlash, "ramfuncs");

#define PER_AMOSTRAGEM 	2084
#define FREQ_AMOSTRAGEM 43478 	// aprox: 120Hz * 360 pontos
#define NUM_PI 3.141592

Uint16 PerAmostragem = PER_AMOSTRAGEM;		//100Mhz/4/Freq_Amostr.
#ifdef CRISTAL_20MHZ
Uint16 Per120Hz = 52083 + 100;		//Set timer period = 100Mhz/1/16/120Hz (se deseja freq de 120Hz)
Uint16 PerTremPulsos = 125;
Uint16 HalfPerDAC_PWM = 2314 / 2;   // Set timer period = 100Mhz/1/1/43200kHz (se deseja freq de 43200KHz = FreqAmostragem
#else
Uint16 Per120Hz = 39063 + 100;		//Set timer period = 100Mhz/2/16/120Hz (se deseja freq de 120Hz)
Uint16 PerTremPulsos = 188;
Uint16 HalfPerDAC_PWM = 3472 / 2;   // Set timer period = 150Mhz/1/1/43200kHz (se deseja freq de 43200KHz = FreqAmostragem)
#endif

//Uint16 Duty1A = 1000;

//Variaveis utilizadas para DAC e ADC
#define Vmax   1
#define Vbase  1
Uint16 ADC_OffSet = 0;
float corrente_pu = 0;
float tensao_pu = 0;
float Const_ADC_corrente = (float)Vmax*2/4096/Vbase;
float Const_ADC_tensao = (float)Vmax*2/4096/Vbase;

float Const_DAC = (float)Vbase/3.3; //Mudanca de base
float Temp = 0;

volatile Uint16 FlagDetectandoBobina = 0;
Uint16 FlagResultadoDetecaoBobina = 0;
void MaquinaEstadoDetecaoRogowiski(Uint16 valorIn);

struct _disparo_corte_scr Controle_SCRs;


volatile struct GPADAT_BITS *portA = &GpioDataRegs.GPADAT.bit;  //atalho para acessar a porta A
volatile struct GPBDAT_BITS *portB = &GpioDataRegs.GPBDAT.bit;  //atalho para acessar a porta A


Uint16 Alpha_LiMax;
Uint16 Alpha_LiMin;

float temp_dac = 0;
Uint16 ii = 0;

//#pragma CODE_SECTION(AquisitaCanal_1, "ramfuncs");
void AquisitaCanal_1()
{
	static float seno1 = 0, seno2 = 0;
	Uint16 tempTensao;
	Uint16 tempCorrente;
	//SpiA_ConversaoAD(valor_amostrado);
	tempTensao = SpiA_AquisicaoTensao();
	tempCorrente = SpiA_AquisicaoCorrente();
	corrente_pu = Const_ADC_corrente*((float)tempCorrente-Adc_offset[0]); //Vo=Vref/4096*(Vi-2047)

	tensao_pu = Const_ADC_tensao*((float)tempTensao-Adc_offset[1]); //Vo=Vref/4096*(Vi-2047)
	temp_dac = corrente_pu/Const_ADC_corrente + 2048;


	


//	McBsp_DAC(ii*4, 2);
//	if(ii++ > 360) ii = 0;
//	McBsp_DAC(tensao_pu*2048 + 2048, 3);
//	McBsp_DAC(corrente_pu*4096 + 2048, 2);
	


/*	//Aquisita Dados do ADC interno
	AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;		//Inicia conversao AD;
	while (AdcRegs.ADCST.bit.INT_SEQ1== 0) {} // Wait for interrup
	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
	//Fim aquisicao interna
	//tensao_pu = Const_ADC_tensao*((float)(AdcRegs.ADCRESULT0>>4)-ADC_OffSet); //Vo=Vref/4096*(Vi-2047)
	//corrente_pu = Const_ADC_corrente*((float)(AdcRegs.ADCRESULT0>>4)-ADC_OffSet);
*/
	seno2 = PLL(tensao_pu);
	if( (seno2>=0 && seno1<0) || (seno2<=0 && seno1>0))
	{
		DetecaoZeroPLL();
	}
	seno1 = seno2;
	EPwm2Regs.CMPA.half.CMPA = (seno2 * HalfPerDAC_PWM) + HalfPerDAC_PWM; //debug, dac de tensao
//	EPwm3Regs.CMPA.half.CMPA = ((float)(corrente_pu/2.0) * 1150) + 1150; //debug, dac de tensao
    EPwm3Regs.CMPA.half.CMPA = ((float)(tensao_pu) * HalfPerDAC_PWM) + HalfPerDAC_PWM; //debug, dac de tensao


	
	Rms.Tensao.in = tensao_pu;
	Rms.Tensao.Calcular(&Rms.Tensao);
 
	if(Solda.soldar) 
	{
		Rms.FaseA.in = corrente_pu; 
		Temp = Rms.FaseA.Calcular(&Rms.FaseA);
	//	portA->GPIO23 = 0;						  //Inicia conversao
	//	SpiA_DAC(Temp*100);
	//	portA->GPIO23 = 1;						  //Termina conversao
	}	

	MaquinaEstadoDetecaoRogowiski(tempCorrente);
}

//#pragma CODE_SECTION(SetaSkt, "ramfuncs");
// Alpha -> angulo de disparo dos tiristores
#define ALPHA_MIM	            30	//em graus -> corresponde à máxima corrente
#define ALPHA_MAX	            150	//em graus -> corresponde à mínima corrente
#define ALPHA_RANGE	            (ALPHA_MAX - ALPHA_MIM)	//Os graus válidos estao entre ALPHA_MIM e ALPHA_MAX
#define GRAUS_ANTES_ZERO_CROSS	5	//Número de graus até o próximo cruzamento por zero, que ocorre em 180 graus.
void SktUpdate(Uint16 skt)
{
	Uint16 temp;
	float alpha;

	if(skt > 990) skt = 990;

	alpha = (float) ALPHA_RANGE*((float)(1000-skt)/1000) + ALPHA_MIM;
	
	temp = (Uint16) Per120Hz*(alpha/180);
	if( temp>Alpha_LiMax )
	{
		Controle_SCRs.ticks_disparo = Alpha_LiMax;
		//EPwm1Regs.CMPA.half.CMPA = Alpha_LiMax;
	}
	else
		if( temp<Alpha_LiMin )
		{
			Controle_SCRs.ticks_disparo = Alpha_LiMin;
		//	EPwm1Regs.CMPA.half.CMPA = Alpha_LiMin;
		}
		else {
			Controle_SCRs.ticks_disparo = temp;	
		//	EPwm1Regs.CMPA.half.CMPA = temp;
		}

	Controle_SCRs.Alpha = alpha;

	EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
	EPwm2Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
	EPwm3Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
}



#pragma CODE_SECTION(Habilita_Pulsos, "ramfuncs");
void Habilita_Pulsos()
{
	EPwm1Regs.ETSEL.bit.INTEN = 1;         // Enable INT
	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;     //Habilita Pulsos FaseA (PWM1 dispara PWM4)
	EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;    // Generate INT on 1st event  
/*
	EPwm2Regs.ETSEL.bit.INTEN = 1;         // Enable INT
	EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;     //Habilita Pulsos FaseA (PWM1 dispara PWM4)
	EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;    // Generate INT on 1st event  


	EPwm3Regs.ETSEL.bit.INTEN = 1;         // Enable INT
	EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;     //Habilita Pulsos FaseA (PWM1 dispara PWM4)
	EPwm3Regs.ETPS.bit.INTPRD = ET_1ST;    // Generate INT on 1st event  
*/
}

#pragma CODE_SECTION(Desabilita_Pulsos, "ramfuncs");
void Desabilita_Pulsos()
{
	EPwm1Regs.ETSEL.bit.INTEN = 0;		    // Disanable INT
	EPwm1Regs.ETPS.bit.INTPRD = ET_DISABLE; // Nao gera mais interrupcoes
	EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM4)
	//EPwm4Regs.AQCTLA.bit.PRD = AQ_SET;      //Desabilita Pulsos FaseA (PWM4)
	EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM1 dispara PWM4)
/*
	EPwm2Regs.ETSEL.bit.INTEN = 0;		    // Disanable INT
	EPwm2Regs.ETPS.bit.INTPRD = ET_DISABLE; // Nao gera mais interrupcoes
	EPwm5Regs.AQCTLA.bit.ZRO = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM4)
	EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM1 dispara PWM4)

	EPwm3Regs.ETSEL.bit.INTEN = 0;		    // Disanable INT
	EPwm3Regs.ETPS.bit.INTPRD = ET_DISABLE; // Nao gera mais interrupcoes
	EPwm6Regs.AQCTLA.bit.ZRO = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM4)
	EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;    //Desabilita Pulsos FaseA (PWM1 dispara PWM4)
*/
}

void Parar_Solda()
{
	Desabilita_Pulsos();
}

void Iniciar_Solda()
{
	Habilita_Pulsos();
}

#pragma CODE_SECTION(DetecaoZeroPLL, "ramfuncs");
void DetecaoZeroPLL()
{
	//Pulso de detecao do zero da fase1
	TogglePinoDetectaZero = 1;	//mostra o instante em q ocorreu o sincronismo
	TogglePinoDetectaZero_errado = 1;

	//if(EPwm1Regs.TBCTR > 15000) 
	//{
		EPwm1Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		EPwm2Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		EPwm3Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		PieCtrlRegs.PIEIFR3.bit.INTx1 = 0;
		EPwm1Regs.TBCTL.bit.SWFSYNC = 1;		//Força sincronizacao do PWM1 (Zera o counter)
	//}

	//Rms.Tensao.RMS120Hz(&Rms.Tensao);

	if(Solda.soldar)
	{
		EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_min;
		Habilita_Pulsos();
		
		CompensacaoTensao.Acumulador += Rms.Tensao.out120Hz;
		CompensacaoTensao.i += 1;
	}
	else {
		Desabilita_Pulsos();
		//GpioDataRegs.GPADAT.bit.GPIO25 = 0;  //Reseta Integrador

		if(FlagDetectandoBobina == 0)
			RESETA_INTEGRADOR;
	}

	
}


void Inicializa_Variaveis()
{
	long teste;
	
	Alpha_LiMax = Per120Hz * (float)(ALPHA_MAX)/180.0;  		//CMPA * (grau/180)
	Alpha_LiMin = Per120Hz * (float)(ALPHA_MIM)/180.0;		//CMPA * (grau/180)

	Controle_SCRs.ticks_corta_pulsos = Per120Hz *((float)(180 - GRAUS_ANTES_ZERO_CROSS) / 180.0); //Termina os pulsos aprox 5 graus antes do cruzamento por zero
	Controle_SCRs.ticks_min = Alpha_LiMin;   //Ticks correspondetes de 0 a AlphaMim graus

	Solda.principal.modo = 0;		//0->skt;   1->kir;
	Solda.principal.total_semiciclos = 10;
	Solda.principal.cont_semiciclos = 0;
	Solda.principal.cont_impulsos = 0;
	Solda.soldar = 0;

	//Valores default
	Solda.principal.total_impulsos = 0;
	Solda.principal.estado = 0;
	Solda.subida.total_semiciclos = 0;
	Solda.subida.cont_semiciclos = 0;
	Solda.subida.estado = 0;
	Solda.descida.total_semiciclos = 0;
	Solda.descida.cont_semiciclos = 0;
	Solda.descida.estado = 0;
	Solda.preSolda.total_semiciclos = 0;
	Solda.preSolda.skt = 0;
	Solda.preSolda.estado = 0;
	Solda.posSolda.total_semiciclos = 0;
	Solda.posSolda.skt = 0;
	Solda.posSolda.estado = 0;
	Solda.pausa1.total_semiciclos = 0;
	Solda.pausa2.total_semiciclos = 0;
	Solda.pausaPrincipal.total_semiciclos = 0;
	Solda.pausa3.cont_semiciclos = 0;
	Solda.pausa3.total_semiciclos = 0;
	Solda.pausa4.cont_semiciclos = 0;
	Solda.pausa4.total_semiciclos = 0;
	Solda.manutencao.cont_semiciclos = 0;
	Solda.manutencao.total_semiciclos = 0;
	
	//fim valores default

	//Valores de teste
	Solda.pausaPrincipal.total_semiciclos = 2;
	Solda.principal.total_impulsos = 3;
	Solda.principal.total_semiciclos = 4;
	Solda.principal.skt = 100;
	Solda.subida.total_semiciclos = 4;
	Solda.descida.total_semiciclos = 4;
	Solda.preSolda.total_semiciclos = 4;
	Solda.preSolda.skt = 300;
	Solda.posSolda.total_semiciclos = 4;
	Solda.posSolda.skt = 300;
	Solda.pausa1.total_semiciclos = 2;
	Solda.pausa2.total_semiciclos = 2;
	
	//Fim valores de teste

	CompensacaoTensao.fator = 1.0;	//Fator de compensacao de tensao padra (ñão faz compensacao)

	Rms.Inicializar(&Rms);
	Rms.FaseA.fator_escala = (float)CORRENTE_FUNDO_ESCALA;	//Fator que multiplicado por Irms resulta em kA
	Rms.Tensao.fator_escala = (float)TENSAO_FUNDO_ESCALA;	//Fator que multiplicado por Irms resulta em kA
	set_mux_corrente(GANHO_CORRENTE_0_25_A);
	Rms.FaseA.Inicializar(&Rms.FaseA);
//	Rms.FaseB.Inicializar(&Rms.FaseB);
//	Rms.FaseC.Inicializar(&Rms.FaseC);

	Kir.InicializarVariaveis(&Kir);

	InicializaPLL();
	
	RESETA_INTEGRADOR;
	
}

void set_mux_corrente(unsigned int in)
{
	switch( in )
	{
		case GANHO_CORRENTE_50_100_A:
			PinMuxAnalogico_bit0_set;
			PinMuxAnalogico_bit1_set;
			PinMuxAnalogico_bit2_set;
			Rms.FaseA.fator_escala = (float)CORRENTE_FUNDO_ESCALA * 1.0;
			break;
		
		case GANHO_CORRENTE_25_50_A:
			PinMuxAnalogico_bit0_clear;
			PinMuxAnalogico_bit1_set;
			PinMuxAnalogico_bit2_set;
			Rms.FaseA.fator_escala = (float)CORRENTE_FUNDO_ESCALA * 2.0;
			break;
		
		case GANHO_CORRENTE_0_25_A:
			PinMuxAnalogico_bit0_set;
			PinMuxAnalogico_bit1_clear;
			PinMuxAnalogico_bit2_set;		
			Rms.FaseA.fator_escala = (float)CORRENTE_FUNDO_ESCALA * 4.0;
			break;
		
		case GANHO_CORRENTE_100_200_A:
			PinMuxAnalogico_bit0_clear;
			PinMuxAnalogico_bit1_clear;
			PinMuxAnalogico_bit2_set;
			Rms.FaseA.fator_escala = (float)CORRENTE_FUNDO_ESCALA * 8.0;
			break;
	}	
}


/**
*	Se a tensao lida, quando a fonte de corrente estiver injetando corrente,
*  estiver abaixo do LIMIAR, então, a bobina está presente.
* 
* ATENÇÃO: utilizar o maior ganho (0 a 25 A) no mux de corrente (entrada 3)
* pois esta posicao da a maior relacao sinal ruido
*  
*/
//Se a tensao for maior que 500mV a bobina está ausente	
//se a tensao for menor que 100mV a bobina está presente	
#define LIMIAR_SUPERIOR_BOBINA	200		//500mV;  2048 = 5V => 500mV = 204 .:. limiar+ = 2048 * 0.5V / 5V = 204; 		
#define LIMIAR_INFERIOR_BOBINA	100		//100mV;  2048 = 5V => 100mV = 40  .:. limiar- = 2048 * 0.1 / 5V

void MaquinaEstadoDetecaoRogowiski(Uint16 valorIn)
{
	static Uint16 i = 0;
	static float tensao = 0;

	Uint16 valor;	
	
	//return 1;	//enquanto o circuito nao eh ajeitado, mantem desabilitada a funcionalidade
	//CpuTimer0Regs.TCR.bit.TIE = 0;      // 0 = Disable/ 1 = Enable Timer Interrupt 

	if(FlagDetectandoBobina == 1)
	{	
		if(PinoDetecaoRogowiski == 0) {
			//LIBERA_INTEGRADOR;
			RESETA_INTEGRADOR;
			DELAY_US(2);
			i = 0;
			tensao = 0;
			PinoDetecaoRogowiski_ligaFonteC;		//Habilita fonte de corrente 
		}

		tensao += (float)valorIn - Adc_offset[0];
	
		if(++i > 100)
		{
			tensao = tensao / 100.0;
			tensao *= 2.0;

			if(tensao >= LIMIAR_SUPERIOR_BOBINA) {
				GpioDataRegs.GPBDAT.bit.GPIO34 = 0;
				//SciReportarErro(ERRO_AUSENCIA_BOBINA);		
				FlagResultadoDetecaoBobina = 0;	//Bobina Ausente
			}
			if(tensao < LIMIAR_SUPERIOR_BOBINA) {
				GpioDataRegs.GPBDAT.bit.GPIO34 = 1;
				FlagResultadoDetecaoBobina = 1;	//Bobina Presente
			}

			PinoDetecaoRogowiski_deslFonteC;		//Desabilita fonte de corrente
			DELAY_US(2);
			RESETA_INTEGRADOR;
			DELAY_US(2);

			FlagDetectandoBobina = 0;	//Termina algoritmo de detecao
		}
	}
}


Uint16 DetecaoRogowiski() 
{
	FlagDetectandoBobina = 1;
	while(FlagDetectandoBobina==1);

	FlagDetectandoBobina = 0;
	
	return FlagResultadoDetecaoBobina;
}