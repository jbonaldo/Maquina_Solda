#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "Jakson_Bibliotecas/Solda.h"
#include "Jakson_Bibliotecas/Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário



#pragma CODE_SECTION(spiTxFifoIsr, "ramfuncs");
#pragma CODE_SECTION(spiRxFifoIsr, "ramfuncs");
#pragma CODE_SECTION(epwm1_isr, "ramfuncs");
#pragma CODE_SECTION(epwm5_isr, "ramfuncs");
#pragma CODE_SECTION(xInt3_isr, "ramfuncs");


//#define DEBUG

interrupt void spiTxFifoIsr(void);
interrupt void spiRxFifoIsr(void);
interrupt void cpu_timer0_isr(void);
interrupt void epwm1_isr(void);
interrupt void epwm2_isr(void);
interrupt void epwm3_isr(void);
interrupt void epwm4_isr(void);
interrupt void epwm5_isr(void);
//interrupt void xInt3_isr(void);
void Configura_EPwm1();
void Configura_EPwm2();
void Configura_EPwm3();
void Configura_EPwm4();
void Configura_EPwm5();
void Configura_EPwm6();
void Configura_GPIO();
void Dog_Configura();


void Ajusta_OffSet();
void Configura_ADC();


union _bytefloat {
	unsigned long num_float;
	char  num_4bytes[4];
};

/*union long2float {
	float nFloat;
	unsigned long unLong;
};
*/
union long2float f2l;



extern Uint16 ADC_OffSet;
extern Uint16 PerAmostragem;
extern Uint16 Per120Hz;
extern Uint16 HalfPerDAC_PWM;

extern Uint16 FlagSoldaConcluida;  //Solda_v1.c
  

extern void itoa(int n, char s[]);
char sTemp[15];
Uint16 i;

union _bytefloat Buffer1;
//unsigned long jtmp = 0;
float jfloat;

#ifdef DEBUG
float Skt[30], Corrente[30];   //debeug
unsigned int k = 0;				//debug
float CorrenteSkt[30];   //debeug
unsigned int k_ciclos = 0, Namostras[30];	 			//debug
#endif

/*
union _bytefloat {
	float num_float;
	char  num_4bytes[4];
};

union _bytefloat Buffer, Buffer2;
*/

void main(void)
{
	Uint16 i;

   InitSysCtrl();
   

// Step 3. Initialize PIE vector table:
// Disable and clear all CPU interrupts
   DINT;
   IER = 0x0000;
   IFR = 0x0000; 

// Initialize PIE control registers to their default state:
// This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
// This function is found in DSP2833x_PieVect.c.
   InitPieVectTable();

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
   EALLOW;	// This is needed to write to EALLOW protected registers
   PieVectTable.SPIRXINTA = &spiRxFifoIsr;
   PieVectTable.SPITXINTA = &spiTxFifoIsr;
   PieVectTable.TINT0 = &cpu_timer0_isr;
   PieVectTable.EPWM1_INT = &epwm1_isr;
   PieVectTable.EPWM2_INT = &epwm2_isr;
   PieVectTable.EPWM3_INT = &epwm3_isr;

   PieVectTable.XINT3 = &xInt3_isr;
   //PieVectTable.SCIRXINTA = &sciA_Rx_isr;
   EDIS;   // This is needed to disable write to EALLOW protected registers


   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
   EDIS;

    
   InitEPwm1Gpio();;
   InitEPwm2Gpio();
   InitEPwm3Gpio();
   InitEPwm4Gpio();
//   InitEPwm5Gpio();
//   InitEPwm6Gpio();

//  InitAdc();
//  Ajusta_OffSet();
//	Configura_ADC();

	//Move as funcoes (marcadas com #pragma) da Flash para a RAM     
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	InitFlash();

   Configura_EPwm1();
   Configura_EPwm2();
   Configura_EPwm3();
   Configura_EPwm4();
//   Configura_EPwm5();
//   Configura_EPwm6();

   Desabilita_Pulsos();

   InitAdc();
   Ajusta_OffSet();
   Configura_ADC();

   xInt_Configura();
   Configura_SciA();

   SpiA_Configura();
   Configura_GPIO();
   
   DELAY_US(1000000);

   SpiA_CalibracaoAD();
   init_mcbsp_spi();
   
   Inicializa_Variaveis();
   
   InitCpuTimers(); //Timer q gera a taxa de amostragem
   ConfigCpuTimer(&CpuTimer0, 150, 23);  //Timer0, 100MHz, periodo=80us
   CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
//   CpuTimer0Regs.TCR.bit.TIE = 0;      // 0 = Disable/ 1 = Enable Timer Interrupt
   CpuTimer0Regs.TCR.bit.TIE = 1;      // 0 = Disable/ 1 = Enable Timer Interrupt	
   	

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
   EDIS;


// Enable interrupts required for this example
  // PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
   PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
   PieCtrlRegs.PIEIER6.bit.INTx1=1;     // Enable PIE Group 6, INT 1  (spi-a)
   PieCtrlRegs.PIEIER6.bit.INTx2=1;     // Enable PIE Group 6, INT 2  (spi-a)
   
   PieCtrlRegs.PIEIER1.bit.INTx4 = 1;   // Enable (xInt1) PIE Group 1 INTx.4
   PieCtrlRegs.PIEIER1.bit.INTx5 = 1;   // Enable (xInt2) PIE Group 1 INTx.5
   PieCtrlRegs.PIEIER12.bit.INTx1 = 1;   // Enable (xInt3) PIE Group 12 INTx.1
   PieCtrlRegs.PIEIER12.bit.INTx2 = 1;   // Enable (xInt4) PIE Group 12 INTx.2
   PieCtrlRegs.PIEIER12.bit.INTx3 = 1;   // Enable (xInt5) PIE Group 12 INTx.3
   PieCtrlRegs.PIEIER12.bit.INTx4 = 1;   // Enable (xInt6) PIE Group 12 INTx.4
   PieCtrlRegs.PIEIER12.bit.INTx5 = 1;   // Enable (xInt7) PIE Group 12 INTx.5

   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;    // Enable (TMR0) PIE Group 1 INTx.7
   PieCtrlRegs.PIEIER9.bit.INTx1=1;      // RxSciA - PIE Group 9, int1
   IER=0x20;                             // Enable CPU INT6 //spi

   IER |= 0x0800;      //Habilita interrup dp grupo12 - XInt3-XInt7
   IER |= M_INT1;	   //Habilita interrup do grupo1  - xInt1 e xInt2
   IER |= M_INT3;      //Enable CPU INT3 which is connected to EPWM1-6 INT:

   // Enable EPWM INTn in the PIE: Group 3 interrupt 1-6
   PieCtrlRegs.PIEIER3.bit.INTx1 = 1;   //Habilita interrupcao PWM1
   PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
   PieCtrlRegs.PIEIER3.bit.INTx3 = 1;
//   PieCtrlRegs.PIEIER3.bit.INTx4 = PWM4_INT_ENABLE;
//   PieCtrlRegs.PIEIER3.bit.INTx5 = 1;
//   PieCtrlRegs.PIEIER3.bit.INTx6 = PWM6_INT_ENABLE;

   EINT;   // Enable Global interrupt INTM
   ERTM;   // Enable Global realtime interrupt DBGM


	//Dog_Configura();
/*
	Kir.vet_skt[0] = 800.0;
	Kir.vet_kA[0] = 8.0;

	Kir.vet_skt[1] = 120.0;
	Kir.vet_kA[1] = 1.2;

	Kir.vet_skt[2] = 195.0;
	Kir.vet_kA[2] = 1.95;

	Kir.vet_skt[3] = 0.0;
	Kir.vet_kA[3] = 0.0;

	Kir.vet_skt[4] = 500;
	Kir.vet_kA[4] = 5.0;

	Kir.i_ref = 7.0;

*/

/*
	jfloat = 100.25;
	f2l.nFloat = jfloat;

	Buffer1.num_4bytes[0] = (f2l.unLong & 0x000000FF);
	Buffer1.num_4bytes[1] = (f2l.unLong & 0x0000FF00) >> 8;
	Buffer1.num_4bytes[2] = (f2l.unLong & 0x00FF0000) >> 16;
	Buffer1.num_4bytes[3] = (f2l.unLong & 0xFF000000) >> 24;

*/

	DetecaoRogowiski();

    while(1) 
	{
	//	Dog_Reset();
	
		//SciA_Comandos_Hiperterminal();	//Usa o Hyperterminal
		GerenciarSolda();
        RecebePacote();
	}


}

interrupt void cpu_timer0_isr(void)
{
//	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1;
	if(!Solda.soldar)
		Rms.FaseA.i = 1;
	AquisitaCanal_1();

   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge this interrupt to receive more interrupts from group 3

//	GpioDataRegs.GPADAT.bit.GPIO24 = 0;
}


//Timer que gera o angulo de disparo dos tiristores
interrupt void epwm1_isr(void)
{	
	EINT;	//Permite q esta interrup seja interrompida
				//qdo esta interrup terminar, 
				//INTM voltará para o estado antigo

	if( EPwm1Regs.TBCTR < Controle_SCRs.ticks_disparo )
	{
		//Alpha = minimo angulo de disparo;
		EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
		
		GpioDataRegs.GPADAT.bit.GPIO15 = 1; //debug
	
		RESETA_INTEGRADOR;       //Reset do integrador externo (suspende operacao do integrador);
		
	


	if ( !MaquinaEstados() ) {
			Solda.soldar = 0;
			FlagSoldaConcluida = 1;
		//	GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1; //DEBUG
		}			
					

				
		GpioDataRegs.GPADAT.bit.GPIO15 = 0;		//debug

	} 
	else {
		if(EPwm1Regs.TBCTR >= Controle_SCRs.ticks_corta_pulsos)
		{
			//Alpha ~= 189 graus
			EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_min;
			//EPwm4Regs.AQCTLA.bit.PRD = AQ_CLEAR;      //Desabilita ePWM4  // Retirar pulsos do SCR
			EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;      //Desabilita ePWM4  // Retirar pulsos do SCR
			EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;
			GpioDataRegs.GPADAT.bit.GPIO15 = 0;
		}
	
		if( (EPwm1Regs.TBCTR) < Controle_SCRs.ticks_corta_pulsos)
		{
			//Alpha = angulo disparo dos tiristores
    		EPwm1Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_corta_pulsos;

			GpioDataRegs.GPADAT.bit.GPIO15 = 1;
			
			
			//Gera pulsos para a solda principal
			if( Solda.principal.estado//(Solda.principal.cont_semiciclos <= Solda.principal.total_semiciclos)   //Apenas conclui a amostragem do ultimo semi-ciclo (nao incia outro)
				|| Solda.preSolda.estado || Solda.posSolda.estado
				|| Solda.subida.estado || Solda.descida.estado )
			{
				LIBERA_INTEGRADOR;   //Termina reset (integrador volta a operar)
				//Gera pulsos para fase1
				EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;      //Habilita ePWM4
				EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
				EPwm4Regs.TBCTR = 0x0000;
			}


		}
	}
		
	EPwm1Regs.ETCLR.bit.INT = 1;            // Clear INT flag for this timer  
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3; // Acknowledge this interrupt to receive more interrupts from group 3
	//GpioDataRegs.GPADAT.bit.GPIO15 = 0;	
}

interrupt void epwm2_isr(void)
{
/*
	EINT;	//Permite q esta interrup seja interrompida
				//qdo esta interrup terminar, 
				//INTM voltará para o estado antigo
					
	if(EPwm2Regs.TBCTR >= Controle_SCRs.ticks_corta_pulsos)
	{
		EPwm2Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
		EPwm5Regs.AQCTLA.bit.ZRO = AQ_CLEAR;      //Desabilita ePWM4  // Retirar pulsos do SCR
//		GpioDataRegs.GPADAT.bit.GPIO15 = 1;
	}
	
	if( (EPwm2Regs.TBCTR) < Controle_SCRs.ticks_corta_pulsos)
	{
    	EPwm2Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_corta_pulsos;
		if(Solda.ciclo_atual != Solda.ciclos_ativos)  //Apenas conclui a amostragem do ultimo semi-ciclo (nao incia outro)
		{		
			EPwm5Regs.AQCTLA.bit.ZRO = AQ_SET;      //Habilita ePWM4  //Gera pulsos para fase1
		}
	//	GpioDataRegs.GPADAT.bit.GPIO15 = 0;
	}
		
	EPwm2Regs.ETCLR.bit.INT = 1;            // Clear INT flag for this timer  	
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3; // Acknowledge this interrupt to receive more interrupts from group 3	
*/
}

interrupt void epwm3_isr(void)
{	

/*
	EINT;	//Permite q esta interrup seja interrompida
				//qdo esta interrup terminar, 
				//INTM voltará para o estado antigo

	if(EPwm3Regs.TBCTR >= Controle_SCRs.ticks_corta_pulsos)
	{
		EPwm3Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_disparo;
		EPwm6Regs.AQCTLA.bit.ZRO = AQ_CLEAR;      //Desabilita ePWM4  // Retirar pulsos do SCR
//		GpioDataRegs.GPADAT.bit.GPIO15 = 1;
	}
	
	if( (EPwm3Regs.TBCTR) < Controle_SCRs.ticks_corta_pulsos)
	{
    	EPwm3Regs.CMPA.half.CMPA =  Controle_SCRs.ticks_corta_pulsos;
		if(Solda.ciclo_atual != Solda.ciclos_ativos)  //Apenas conclui a amostragem do ultimo semi-ciclo (nao incia outro)
		{		
			EPwm6Regs.AQCTLA.bit.ZRO = AQ_SET;      //Habilita ePWM4  //Gera pulsos para fase1
		}
		//GpioDataRegs.GPADAT.bit.GPIO15 = 0;
	}
		
	EPwm3Regs.ETCLR.bit.INT = 1;            // Clear INT flag for this timer  	
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3; // Acknowledge this interrupt to receive more interrupts from group 3	
*/
}


void Configura_EPwm1()
{
   //Pulsos de disparo do SCR  Fase A	
   EPwm1Regs.TBPRD = Per120Hz;              // Set timer period = 100Mhz/16/120Hz (se deseja freq de 120Hz)
   EPwm1Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm1Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm1Regs.CMPA.half.CMPA = Per120Hz/2;			      //Duty_Cicle

   // Setup TBCLK
   EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm1Regs.TBCTL.bit.PHSEN = TB_ENABLE;      	    // Habilita Sincronizacao
   #ifdef CRISTAL_20MHZ
   EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0x00;         	// dividido por 1; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/16
   #else
   EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0x01;         	// dividido por 2; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/16
   #endif
   EPwm1Regs.TBCTL.bit.CLKDIV = 0x04;  			    //dividido por 16;
   EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Habilita sincronismo por software ou externo 
    

   EPwm1Regs.CMPCTL.bit.SHDWAMODE =  CC_IMMEDIATE; //TB_SHADOW;    //Registrador do comparador sem sombra 
   EPwm1Regs.CMPCTL.bit.SHDWBMODE =  CC_IMMEDIATE; //TB_SHADOW;
   EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;	  //Carrega CMPA qdo CTR=PRD 
   EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

   // Set actions
   EPwm1Regs.AQCTLA.bit.ZRO = AQ_CLEAR; //AQ_SET;             // Set PWM1A on Zero
   EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;

 
   // Config. da interrupcao
   EPwm1Regs.ETSEL.bit.INTSEL = ET_CTRU_CMPA;     // Gera interrup qdo CTR=CMPA
   EPwm1Regs.ETSEL.bit.INTEN = 0;                // Disaable INT
   EPwm1Regs.ETPS.bit.INTPRD = ET_DISABLE;       // Inicia o timer nao gerando interrupcoes
   EPwm1Regs.ETCLR.bit.INT = 1;          	     // Clear INT flag for this timer 

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
   EDIS;
}

/**
 * Utilizado apenas na versão trifásica
 */
void Configura_EPwm2()
{
   //DAC PWM
   EPwm2Regs.TBPRD = 2 * HalfPerDAC_PWM;          // Set timer period = 150Mhz/1/1/43200kHz (se deseja freq de 5KHz)
   EPwm2Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm2Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm2Regs.CMPA.half.CMPA = HalfPerDAC_PWM;     //Duty_Cicle

   // Setup TBCLK
   EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;      	    // Habilita Sincronizacao
   EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0x0;         	// dividido por 1; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/160
   EPwm2Regs.TBCTL.bit.CLKDIV = 0x0;  			    //dividido por 1;
   EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_DISABLE;       // Desativa sincronismo externo ou por software
    
   // Set actions
   EPwm2Regs.AQCTLA.bit.ZRO = AQ_CLEAR;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
   EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
}

/**
 * Utilizado apenas na versão trifásica
 */
void Configura_EPwm3()
{
   //DAC PWM
   EPwm3Regs.TBPRD = 2 * HalfPerDAC_PWM;          // Set timer period = 150Mhz/1/1/43200kHz (se deseja freq de 5KHz)
   EPwm3Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm3Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm3Regs.CMPA.half.CMPA = HalfPerDAC_PWM;     //Duty_Cicle

   // Setup TBCLK
   EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;      	    // Habilita Sincronizacao
   EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0x0;         	// dividido por 1; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/160
   EPwm3Regs.TBCTL.bit.CLKDIV = 0x0;  			    //dividido por 1;
   EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_DISABLE;       // Desativa sincronismo externo ou por software
    
   // Set actions
   EPwm3Regs.AQCTLA.bit.ZRO = AQ_CLEAR;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
   EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;
}



void Configura_EPwm4()
{
   //Pulsos de disparo do SCR Fase A	
   EPwm4Regs.TBPRD = 125;                         // Set timer period = 100Mhz/160/5kHz (se deseja freq de 5KHz)
   EPwm4Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm4Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm4Regs.CMPA.half.CMPA = 100;                 //Duty_Cicle

   // Setup TBCLK
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;      	    // Habilita Sincronizacao
   EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0x05;         	// dividido por 10; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/160
   EPwm4Regs.TBCTL.bit.CLKDIV = 0x04;  			    //dividido por 16;
   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_DISABLE;       // Desativa sincronismo externo ou por software
    
   // Set actions
//   EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
//   EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
     EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
     EPwm4Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
     EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;
}

/**
 * Não utilizar ePWM5-A
 * 
 * O GPIO-8  esta sendo usado no mux analógico
 * Este gpio está configurado como i/o
 */
void Configura_EPwm5()
{
   EPwm5Regs.TBPRD = 2 * HalfPerDAC_PWM;          // Set timer period = 150Mhz/1/1/43200kHz (se deseja freq de 5KHz)
   EPwm5Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm5Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm5Regs.CMPA.half.CMPA = HalfPerDAC_PWM;     //Duty_Cicle

   // Setup TBCLK
   EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;      	    // Habilita Sincronizacao
   EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0x00;         	// dividido por 10; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/160
   EPwm5Regs.TBCTL.bit.CLKDIV = 0x00;  			    //dividido por 16;
   EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_DISABLE;       // Desativa sincronismo externo ou por software
    
   // Set actions
   EPwm5Regs.AQCTLA.bit.ZRO = AQ_CLEAR;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
   EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;
}

/**
 * Não utilizar ePWM6-A
 * 
 * O GPIO-10  esta sendo usado no mux analógico
 * Este gpio está configurado como i/o
 */
void Configura_EPwm6()
{
   //DAC PWM
   EPwm6Regs.TBPRD = 2 * HalfPerDAC_PWM;          // Set timer period = 150Mhz/1/1/43200kHz (se deseja freq de 5KHz)
   EPwm6Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
   EPwm6Regs.TBCTR = 0x0000;                      // Clear counter
   EPwm6Regs.CMPA.half.CMPA = HalfPerDAC_PWM;     //Duty_Cicle

   // Setup TBCLK
   EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// Count up
   EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;      	    // Habilita Sincronizacao
   EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0x0;         	// dividido por 1; ePWM1_Clock=SYSCLKOUT/HSPCLKDIV/CLKDIV = SYSCLKOUT/160
   EPwm6Regs.TBCTL.bit.CLKDIV = 0x0;  			    //dividido por 1;
   EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_DISABLE;       // Desativa sincronismo externo ou por software
    
   // Set actions
   EPwm6Regs.AQCTLA.bit.ZRO = AQ_CLEAR;   //Inicia desabilitado AQ_SET;             // Set PWM1A on Zero
   EPwm6Regs.AQCTLA.bit.CAU = AQ_SET;
}


void Configura_GPIO()
{
	EALLOW;

	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;  // GPIO12 = i/o  
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;  // GPIO13 = i/o   //Enable1 da spiA
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;  // GPIO14 = i/o   //Enable2 da spiA
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;  // GPIO15 = i/o   //Enable3 da spiA
	GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;  // GPIO25 = i/o
	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;  // GPIO31 = i/o

	GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;   // GPIO12 = output
	GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;   // GPIO13 = output
	GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;   // GPIO14 = output
	GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;   // GPIO15 = output
	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;   // GPIO25 = output
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;   // GPIO31 = output

	GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1;    // Enable pull-up on GPIO12 
	GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1;    // enable pull-up on GPIO13 
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = 1;    // enable pull-up on GPIO14 
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;    // enable pull-up on GPIO15 
	GpioCtrlRegs.GPAPUD.bit.GPIO25 = 1;    // enable pull-up on GPIO25
	GpioCtrlRegs.GPBPUD.bit.GPIO34 = 1;    // enable pull-up on GPIO31

	GpioDataRegs.GPADAT.bit.GPIO13 = 0;		//inicia pino com zero
	GpioDataRegs.GPADAT.bit.GPIO14 = 0;		//inicia pino com zero
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;		//inicia pino com zero
	GpioDataRegs.GPADAT.bit.GPIO25 = 0;		//inicia pino com 1		 //Reset do integrador externo
	GpioDataRegs.GPBDAT.bit.GPIO34 = 0;		//inicia pino com zero
	//fim configura i/o diversos

    // Pinos de configuraçao do Mux analógico (das correntes)
    // pino mux: 0 - lsb
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;   // GPIO08 = i/o
    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1;   // enable pullup on GPIO08
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1;   // GPIO08 = output
    GpioDataRegs.GPADAT.bit.GPIO8 = 0;   // Inicializa com zero
    // pino mux: 1
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;   // GPIO10 = i/o
    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;   // enable pullup on GPIO10
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;   // GPIO10 = output
    GpioDataRegs.GPADAT.bit.GPIO10 = 0;   // Inicializa com zero
    // pino mux: 2 - msb    
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0;   // GPIO16 = i/o
    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 1;   // enable pullup on GPIO16
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;   // GPIO16 = output
    GpioDataRegs.GPADAT.bit.GPIO16 = 0;   // Inicializa com zero
    // Fim - Pinos de configuraçao do Mux analógico

	//Enables da SPIA
	GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;  // GPIO19 = i/o   //Enable da spiA do ADC Corrente
	GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;  // GPIO27 = i/o   //Enable da spiA do ADC Tensao
	GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;   // GPIO19 = output
	GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;   // GPIO27 = output
	GpioCtrlRegs.GPAPUD.bit.GPIO19 = 1;    // enable pull-up on GPIO19 
	GpioCtrlRegs.GPAPUD.bit.GPIO27 = 1;    // enable pull-up on GPIO27 
	GpioDataRegs.GPADAT.bit.GPIO19 = 1;		//inicia pino com zero	 //Select da SPI de  Tensao
	GpioDataRegs.GPADAT.bit.GPIO27 = 1;		//inicia pino com zero	 //Select da SPI de  Tensao
	//Fim
		
	//SciA
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;	   // Enable pull-up for GPIO29 (SCITXDA)
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
	//Fim SciA


	//PinoSoldar
	GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 0;  // GPIO61 = i/o
	GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;   // enable pullup on GPIO61
	GpioCtrlRegs.GPBDIR.bit.GPIO61 = 0;   // GPIO61 = input	//PinoSoldar
	GpioDataRegs.GPBDAT.bit.GPIO61;
	//Fim PinoSoldar


	//PinoDetecaoRogowiski
	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;  // GPIO24 = i/o
	GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;   // disable pull-up on GPIO24
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;   // GPIO24 = output
	GpioDataRegs.GPADAT.bit.GPIO24 = 0;
	//Fim PinoDetecaoRogowiski

	
	//PinoEmergencia
	GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 0;  // GPIO61 = i/o
	GpioCtrlRegs.GPAPUD.bit.GPIO30 = 1;   // enable pullup on GPIO61
	GpioCtrlRegs.GPADIR.bit.GPIO30 = 0;   // GPIO61 = input	//Emergencia
	//Fim PinoEmergencia




	//Pino que mostra o instante da detecao de zero do PLL
	GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;  // GPIO61 = i/o
	GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;   // enable pullup on GPIO58
	GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;   // GPIO61 = output	//PassagemZero
	GpioDataRegs.GPBDAT.bit.GPIO58;
	GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0;  // GPIO61 = i/o
	GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0;   // enable pullup on GPIO58
	GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;   // GPIO61 = output	//PassagemZero
	GpioDataRegs.GPBDAT.bit.GPIO59;
	//Fim PinoDetectaZero

	//Pinos de i/o configurados como saída
//	GpioCtrlRegs.GPBPUD.bit.GPIO59 = 1;   // enable pullup on GPIO60
	GpioCtrlRegs.GPBPUD.bit.GPIO63 = 1;   // enable pullup on GPIO62
	GpioCtrlRegs.GPBPUD.bit.GPIO32 = 1;   // enable pullup on GPIO32
	GpioCtrlRegs.GPBPUD.bit.GPIO34 = 1;   // enable pullup on GPIO34	
	GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0;    // disable pull-up on GPIO21 
//	GpioCtrlRegs.GPAPUD.bit.GPIO22 = 1;    // enable pull-up on GPIO22 / McBsp clk
	GpioCtrlRegs.GPAPUD.bit.GPIO23 = 1;    // enable pull-up on GPIO23 



//	GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0;  // GPIO60 = i/o	
	GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 0;  // GPIO20 = i/o
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;  // GPIO32 = i/o
	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;  // GPIO34 = i/o
	GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;   //pino trabalhado como i/o  
//	GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 1;   //pino trabalhado como i/o 
	GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;   //pino trabalhado como i/o 


//	GpioCtrlRegs.GPBDIR.bit.GPIO59 = 0;   // GPIO58 = input //alta impdancia
	GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;   // GPIO62 = output
	GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;   // GPIO32 = output
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;   // GPIO34 = output
	GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;   // GPIO como output
//	GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;   // GPIO como output
	GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;   // GPIO como output


//	GpioDataRegs.GPBDAT.bit.GPIO59 = 0;   
	GpioDataRegs.GPBDAT.bit.GPIO63 = 1;
	GpioDataRegs.GPBDAT.bit.GPIO32 = 0;	 
	GpioDataRegs.GPBDAT.bit.GPIO34 = 0;	  //Enable2 da spiA
	GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;   // GPIO inicia com nivel alto
//	GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;   // GPIO inicia com nivel alto
	GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;   // GPIO inicia com nivel alto
	//fim Enable da SPIA
	EDIS;
}

void Dog_Configura()
{
	EALLOW;
	SysCtrlRegs.SCSR = 0x0000;		//Habilita reset do dispositivo 
	EDIS; 							//caso o Dog nao seja realimentado

	Dog_Reset();

	EALLOW;
  	SysCtrlRegs.WDCR = 0x0028;      //O counter do dog vai até 256
  	EDIS;							//entao reseta o DSP (realimentar antes de estourar) 
									//DogClk = 20Mhz/512/WDCR[0:2]
									//DogClk = 20M/52/64 ->WDCR[0:2]=7


   	 
}

void Dog_Reset()
{
	EALLOW;
    SysCtrlRegs.WDKEY = 0x0055;
    SysCtrlRegs.WDKEY = 0x00AA;
    EDIS;
}


void Configura_ADC()
{
   
    AdcRegs.ADCTRL1.bit.ACQ_PS = 0x01;       // S/H width in ADC module periods = 2 ADC clocks 
    AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x02;     // ADC module clock = HSPCLK/(2*ADC_CKPS) = 100MHz/(2*2) = 25MHz  
    AdcRegs.ADCTRL1.bit.CPS = 0;			 // ADC clock = HSPCLK/(2*ACQ_CKPS)/(2^CPS)    
										 	//3 ciclos do adclock para amostrar 1 sinal; 
										   	//total = 9 ciclos; 9*(1/adcloc) = 9/12.5MHz = 720ns 

	AdcRegs.ADCTRL1.bit.CONT_RUN = 0;		 // Desabilita conversao permanente
	AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;		 // desabilita amostragem simultanea Ax e Bx
    AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 0;  // disable SOCA from ePWM to start SEQ1
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 0;    // Disable SEQ1 interrupt a cada (EOS-fim de sequencia)
    AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0;
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 0;        // 1  Cascaded mode (desabilitado==0)
  
	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x00;  // 1 Conversoes SEQ1;  
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;   // Converte: A0/B0; Resultado:  ADCRESULT0 e 1
    
   	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;   
}

void Ajusta_OffSet()
{
	unsigned long i;
	long long temp = 0;
	ADC_OffSet = 0;

	AdcRegs.ADCTRL1.bit.ACQ_PS = 0x2;
    AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x1E;
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;        // 1  Cascaded mode
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;
    AdcRegs.ADCTRL1.bit.CONT_RUN = 1;       // Setup continuous run

    AdcRegs.ADCTRL2.all = 0x2000;			// Start SEQ1
    for( i=0; i<1000000; i++) {	
        while (AdcRegs.ADCST.bit.INT_SEQ1== 0) {} // Wait for interrupt
        AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
		temp += (AdcRegs.ADCRESULT0 >>4);
     }
	 ADC_OffSet = (Uint16)(temp/1000000);
}




