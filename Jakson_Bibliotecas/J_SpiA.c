/*
	Desenvolvido por Jakson Bonaldo   
	Campinas - SP 		19/08/09
	Rotinas referentes à SpiA

	Gpio16 -> MOSI
	Gpio17 -> MISO
	Gpio18 -> Clock
	Gpio19 -> Enable (logica invertida)
*/

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File 
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário

extern volatile struct GPADAT_BITS *portA;

Uint16 Adc_offset[] = {0, 0};

void SpiA_Configura()
{
// Initialize SPI FIFO registers

	InitSpiaGpio();
   SpiaRegs.SPICCR.bit.SPISWRESET=0; // Reset SPI
									 //0-3            4           6
   SpiaRegs.SPICCR.all=0x000F;       //No. bits = 15; Loopback=0; Polaridade=0;
   //SpiaRegs.SPICCR.all=0x004F;       //No. bits = 15; Loopback=0; Polaridade=1;
   //SpiaRegs.SPICCR.all=0x000F;       //No. bits = 15; Loopback=0; Polaridade=0;
									 //0			  1							2						3			 4			 
   //SpiaRegs.SPICTL.all=0x0006;       //Hablita int=1; habi transmisao (Talk=1); Master/Slave=1(Master); clk Phase=0; Overrum int = 0 (desab);
   SpiaRegs.SPICTL.all=0x000E;       //Hablita int=1; habi transmisao (Talk=1); Master/Slave=1(Master); clk Phase=1; Overrum int = 0 (desab);
   
   //clk Phase=0; Polaridade=0
   //Rising Edge Without Delay. The SPI transmits data on the rising edge of the SPICLK signal and
   //receives data on the falling edge of the SPICLK signal.
   
   //clk Phase=1; Polaridade=0
   //Rising Edge With Delay. The SPI transmits data one half-cycle ahead of the rising edge of the SPICLK
   //signal and receives data on the rising edge of the SPICLK signal.

   SpiaRegs.SPISTS.all=0x0000;
									 
   SpiaRegs.SPIBRR=0x18;           	 //BaudRate = LSPCLK/(SPIBRR+1); SPIBRR >= 3;
									 //0-4					  5					   13				   15
   SpiaRegs.SPIFFTX.all=0xC040;      //Profundidade TxFifo=2; TxFifo_int = 1(hab); TxFifo funcionando; reseta SPI;
									 //0-4					   5						6
   SpiaRegs.SPIFFRX.all=0x0071;      //Profundidade RxFifo=16; RxFifo_int = 1(hab); 	Clear RXFFINT flag = 1			   //P.RxFifo=32 para evitar interrupcoes
									 
   SpiaRegs.SPIFFCT.all=0x00;		 //Delay entre transmissoes da TxFifo (em ciclos SPI)
   SpiaRegs.SPIPRI.all=0x0010;		 //Acao quando o progrma é interrompido pelo emulador

   SpiaRegs.SPICCR.bit.SPISWRESET=1;  // Habilita SPI

   SpiaRegs.SPIFFTX.bit.TXFIFO=1;		//Reabilita TxFifo
   SpiaRegs.SPIFFRX.bit.RXFIFORESET=1;	//Realibita TxFifo

}

interrupt void spiTxFifoIsr(void)
{
	PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ack
}


interrupt void spiRxFifoIsr(void)
{
	Uint16 i, temp;
    for(i=0;i<2;i++)
    {
	   temp=SpiaRegs.SPIRXBUF;		// Read data
	}

	SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1;  // Clear Overflow flag
	SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 	// Clear Interrupt flag
	PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ack
}

/*
#pragma CODE_SECTION(SpiA_ConversaoAD, "ramfuncs");
void SpiA_ConversaoAD(Uint16 *valor_amostrado)
{
	//Amostra a corrente
	//SpiaRegs.SPICCR.bit.SPICHAR = 0x0E;		  //No. de bits (15 bits)

	portA->GPIO19 = 0;						  //Inicia conversao
	SpiaRegs.SPITXBUF = 0xFF;				  //Fornce os pulsos de clock
	while(SpiaRegs.SPIFFRX.bit.RXFFST != 1);  //Espera até o dado ser recebido
	portA->GPIO19 = 1;						  //Termina conversao
	*valor_amostrado = (SpiaRegs.SPIRXBUF >> 1);				  //Le o dado da porta SPI
	*valor_amostrado &= 0x0FFF;    //Elimina bits indesejados (importante sao os LSB)

	valor_amostrado++;

	//Amostra a Tensao
	portA->GPIO27 = 0;						  //Inicia conversao
	SpiaRegs.SPITXBUF = 0xFF;				  //Fornce os pulsos de clock
	while(SpiaRegs.SPIFFRX.bit.RXFFST != 1);  //Espera até o dado ser recebido
	portA->GPIO27 = 1;
	*valor_amostrado = (SpiaRegs.SPIRXBUF >> 1);				  //Le o dado da porta SPI
	*valor_amostrado &= 0x0FFF;    //Elimina bits indesejados (importante sao os LSB)	
}
*/

#pragma CODE_SECTION(SpiA_AquisicaoCorrente, "ramfuncs");
Uint16 SpiA_AquisicaoCorrente()
{
	Uint16 valorCorrente = 0;
	//Amostra a corrente
	//SpiaRegs.SPICCR.bit.SPICHAR = 0x0E;		  //No. de bits (15 bits)

	//portA->GPIO19 = 0;						  //Inicia conversao
	CONV_CORRENTE_INICIAR;
	SpiaRegs.SPITXBUF = 0xFF;				  //Fornce os pulsos de clock
	while(SpiaRegs.SPIFFRX.bit.RXFFST != 1);  //Espera até o dado ser recebido
	//portA->GPIO19 = 1;						  //Termina conversao
	CONV_CORRENTE_TERMINAR;
	valorCorrente = SpiaRegs.SPIRXBUF;				  //Le o dado da porta SPI
	//valorCorrente = SpiaRegs.SPIRXBUF ;				  //Le o dado da porta SPI
	valorCorrente = 0x0FFF & (valorCorrente >> 2);    //Elimina bits indesejados (importante sao os LSB)

	return  valorCorrente;
}

#pragma CODE_SECTION(SpiA_AquisicaoTensao, "ramfuncs");
Uint16 SpiA_AquisicaoTensao()
{
	
	Uint16 valorTensao = 0;
	
	//Amostra a Tensao
	portA->GPIO27 = 0;						  //Inicia conversao
	SpiaRegs.SPITXBUF = 0xFF;				  //Fornce os pulsos de clock
	while(SpiaRegs.SPIFFRX.bit.RXFFST != 1);  //Espera até o dado ser recebido
	portA->GPIO27 = 1;
	valorTensao = (SpiaRegs.SPIRXBUF >> 2);				  //Le o dado da porta SPI
//	valorTensao = SpiaRegs.SPIRXBUF;				  //Le o dado da porta SPI
	valorTensao &= 0x0FFF;    //Elimina bits indesejados (importante sao os LSB)
	
	//debug para verificar se os pulsos estao ok (o erro pode ser devido a detecao de zero do pll)
	//valorTensao = 0;
	

	return  valorTensao;
}


#pragma CODE_SECTION(SpiA_CalibracaoAD, "ramfuncs");
void SpiA_CalibracaoAD()
{
	Uint16 valor[2];
	unsigned long acumulador[] = {0, 0};
	Uint16 i;


	RESETA_INTEGRADOR;
	DELAY_US(10000);	
	CpuTimer0Regs.TCR.bit.TIE = 0;      // 0 = Disable/ 1 = Enable Timer Interrupt

	for(i=0;i<10000;i++)
	{
		while(!CpuTimer0Regs.TCR.bit.TIF);
		CpuTimer0Regs.TCR.bit.TIF = 1;  //Limpa flag
		valor[0] = SpiA_AquisicaoCorrente();
		valor[1] = SpiA_AquisicaoTensao();
		acumulador[0] += valor[0];
		acumulador[1] += valor[1];
	}
	Adc_offset[0] = acumulador[0] / 10000;  //offset corrente
	Adc_offset[1] = acumulador[1] / 10000;  //offset tensao

	CpuTimer0Regs.TCR.bit.TIE = 1;      // 0 = Disable/ 1 = Enable Timer Interrupt
}



void InitMcbspaGpio(void)
{
	EALLOW;

/* Configure McBSP-A pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be McBSP functional pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 2;	// GPIO20 is MDXA pin
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 2;	// GPIO22 is MCLKXA pin

/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user.
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPAPUD.bit.GPIO20 = 0;     // Enable pull-up on GPIO20 (MDXA)
	GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0;     // Enable pull-up on GPIO22 (MCLKXA)

/* Set qualification for selected input pins to asynch only */
// This will select asynch (no qualification) for the selected pins.
// Comment out other unwanted lines.


    GpioCtrlRegs.GPAQSEL2.bit.GPIO22 = 3;   // Asynch input GPIO22 (MCLKXA)

	EDIS;
}

void init_mcbsp_spi()
{
	//Configura McBsp como SPI
	InitMcbspaGpio();
     // McBSP-A register settings
    McbspaRegs.SPCR2.all=0x0000;		 // Reset FS generator, sample rate generator & transmitter
	McbspaRegs.SPCR1.all=0x0000;		 // Reset Receiver, Right justify word, Digital loopback dis.
    
    
    McbspaRegs.PCR.all=0x0F08;           //(CLKXM=CLKRM=FSXM=FSRM= 1, FSXP = 1)
	McbspaRegs.PCR.bit.SCLKME = 0;
	McbspaRegs.PCR.bit.CLKRP = 0;
	McbspaRegs.PCR.bit.CLKXP = 0;
	McbspaRegs.PCR.bit.CLKXM = 1;		//SPI como Master

	McbspaRegs.SPCR1.bit.CLKSTP = 2;     // Together with CLKXP/CLKRP determines clocking scheme
    McbspaRegs.SPCR1.bit.DLB = 1;
    
    McbspaRegs.RCR2.bit.RDATDLY=01;      // FSX setup time 1 in master mode. 0 for slave mode (Receive)
    McbspaRegs.XCR2.bit.XDATDLY=01;      // FSX setup time 1 in master mode. 0 for slave mode (Transmit)

	McbspaRegs.RCR1.bit.RWDLEN1=2;     // 32-bit word
    McbspaRegs.XCR1.bit.XWDLEN1=2;     // 32-bit word

    McbspaRegs.SRGR2.all=0x0000; 	 	 // CLKSM=1, FPER = 1 CLKG periods
	McbspaRegs.SRGR2.bit.CLKSM = 1;
    McbspaRegs.SRGR1.all= 0x0000;	     // Frame Width = 1 CLKG period, CLKGDV=16
	McbspaRegs.SRGR1.bit.CLKGDV = 0x02;	// clock_msbsp = HSPCLK/CLKGV (SCLKME=0; CLKSM=1 )

    McbspaRegs.SPCR2.bit.GRST=1;         // Enable the sample rate generator
//	delay_loop();                        // Wait at least 2 SRG clock cycles
	DELAY_US(10);
	McbspaRegs.SPCR2.bit.XRST=1;         // Release TX from Reset
	McbspaRegs.SPCR1.bit.RRST=1;         // Release RX from Reset
    McbspaRegs.SPCR2.bit.FRST=1;         // Frame Sync Generator reset
}

void McBsp_DAC(Uint16 valor, Uint16 enable)
{
	unsigned long i;
    //Inicia conversao
	McbspaRegs.DXR1.all = valor & 0x0FFF;
//	McbspaRegs.DXR2.all = 0;
	
//	while( McbspaRegs.SPCR2.bit.XRDY == 0 ) {}         // Master waits until RX data is ready
//	for(i=0;i<500;i++);

	if(enable == 1) portA->GPIO15 = 0;	
	if(enable == 2) portA->GPIO21 = 0;	
	if(enable == 3) portA->GPIO23 = 0;	

	DELAY_US(1);

	portA->GPIO15 = 1;	
	portA->GPIO21 = 1;
	portA->GPIO23 = 1;


	/*
	SpiaRegs.SPICCR.bit.SPICHAR = 0x0F;		  //No. de bits (16 bits)
	portA->GPIO15 = 0;				  //Inicia conversao
	SpiaRegs.SPITXBUF = valor;				  //Fornce os pulsos de clock
	while(SpiaRegs.SPIFFRX.bit.RXFFST != 1);  //Espera até o dado ser recebido
	SpiaRegs.SPIRXBUF;
	//DELAY_US(4);
	portA->GPIO15 = 1;

	*/
}


//