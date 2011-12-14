
/*
	Desenvolvido por Jakson Bonaldo   
	Campinas - SP 		20/08/09
	Rotinas referentes à Interrupcoes Externas

	Gpio16 -> MOSI
	Gpio17 -> MISO
	Gpio18 -> Clock
	Gpio19 -> Enable (logica invertida)
*/

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File 
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário


void xInt_ConfiguraIO()
{
	EALLOW;
//	GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;   // Disaable pullup on GPIO58
	GpioCtrlRegs.GPBPUD.bit.GPIO59 = 1;   // Disaable pullup on GPIO59
//    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 1;   // Disaable pullup on GPIO60
//	GpioCtrlRegs.GPBPUD.bit.GPIO61 = 1;   // Disaable pullup on GPIO61
    GpioCtrlRegs.GPBPUD.bit.GPIO62 = 1;   // Disaable pullup on GPIO62
//	GpioCtrlRegs.GPBPUD.bit.GPIO63 = 1;   // Disaable pullup on GPIO62

	//escolhe a funcao de cada pino
//    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;  // GPIO58 = i/o 
//	GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0;  // GPIO59 = i/o
//	GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 0;  // GPIO60 = i/o
//	GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 0;  // GPIO61 = i/o
	GpioCtrlRegs.GPBMUX2.bit.GPIO62 = 0;  // GPIO62 = i/o
//	GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 0;  // GPIO63 = i/o

//	GpioCtrlRegs.GPBDIR.bit.GPIO58 = 0;   // GPIO58 = input
//	GpioCtrlRegs.GPBDIR.bit.GPIO59 = 0;   // GPIO60 = input
//	GpioCtrlRegs.GPBDIR.bit.GPIO60 = 0;   // GPIO60 = input
//	GpioCtrlRegs.GPBDIR.bit.GPIO61 = 0;   // GPIO60 = input
	GpioCtrlRegs.GPBDIR.bit.GPIO62 = 0;   // GPIO62 = input
//	GpioCtrlRegs.GPBDIR.bit.GPIO63 = 0;   // GPIO62 = input
	
	//Interrupcao externa 
	GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 0;     // sincronizado com SYSCLKOUT
	GpioCtrlRegs.GPAQSEL2.bit.GPIO31 = 0;     // sincronizado com SYSCLKOUT
//	GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 0;     // sincronizado com SYSCLKOUT
//	GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 0;     // sincronizado com SYSCLKOUT
//	GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 0;     // sincronizado com SYSCLKOUT
//	GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 0;     // sincronizado com SYSCLKOUT
	GpioCtrlRegs.GPBQSEL2.bit.GPIO62 = 0;     // sincronizado com SYSCLKOUT

    GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0;    //Periodo de amostragem igual ao de SYSCLKOUT
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD3 = 0;    //Periodo de amostragem igual ao de SYSCLKOUT
											  //QUALPRD3 afeta GPIO 56 a 63		
	EDIS;
}

void xInt_Configura()
{
	xInt_ConfiguraIO();

	EALLOW;                                                                                                                                                                                                           
//	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 30;   //xInt1 -> Gpio30	//Emergencia
	GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 31;   //xInt2 -> Gpio31
	GpioIntRegs.GPIOXINT3SEL.bit.GPIOSEL = 26;   //xInt3 -> Gpio58 //Deteccao zero da rede
	//GpioIntRegs.GPIOXINT4SEL.bit.GPIOSEL = 27;   //xInt4 -> Gpio59
	GpioIntRegs.GPIOXINT5SEL.bit.GPIOSEL = 28;   //xInt5 -> Gpio60
	//GpioIntRegs.GPIOXINT6SEL.bit.GPIOSEL = 29;   //xInt6 -> Gpio61
	GpioIntRegs.GPIOXINT7SEL.bit.GPIOSEL = 30;   //xInt7 -> Gpio62
	EDIS;

//	XIntruptRegs.XINT1CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	XIntruptRegs.XINT2CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	XIntruptRegs.XINT3CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	//XIntruptRegs.XINT4CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	XIntruptRegs.XINT5CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
//	XIntruptRegs.XINT6CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	XIntruptRegs.XINT7CR.bit.POLARITY = 1 ;      // Gera interrup na borda de Subida
	
//	XIntruptRegs.XINT1CR.bit.ENABLE = 0;        // Enable Xint1
	XIntruptRegs.XINT2CR.bit.ENABLE = 0;        // Enable Xint2
	XIntruptRegs.XINT3CR.bit.ENABLE = 1;        // Enable Xint3
	//XIntruptRegs.XINT4CR.bit.ENABLE = 0;        // Enable Xint4
	XIntruptRegs.XINT5CR.bit.ENABLE = 0;        // Enable Xint5
//	XIntruptRegs.XINT6CR.bit.ENABLE = 0;        // Enable Xint6
	XIntruptRegs.XINT7CR.bit.ENABLE = 0;        // Enable Xint7
} 


interrupt void xInt3_isr(void)
{
/*	//Pulso de detecao do zero da fase1
	GpioDataRegs.GPBTOGGLE.bit.GPIO61 = 1;	//mostra o instante em q ocorreu o sincronismo

	if(EPwm1Regs.TBCTR > 15000) 
	{
		EPwm1Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		EPwm2Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		EPwm3Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
		PieCtrlRegs.PIEIFR3.bit.INTx1 = 0;
		EPwm1Regs.TBCTL.bit.SWFSYNC = 1;		//Força sincronizacao do PWM1 (Zera o counter)
	}

	if(Solda.soldar)
	{
		EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_min;
		Habilita_Pulsos();
	}

	Rms.Tensao.RMS120Hz(&Rms.Tensao);
*/
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;    // Acknowledge this interrupt to get more from group 1
}


