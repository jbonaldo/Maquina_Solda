/*
	Desenvolvido por Jakson Bonaldo   
	Campinas - SP 		19/08/09
	Rotinas referentes à SciA

	Gpio28 -> Rx
	Gpio29 -> Tx
*/

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File 
#include "Jakson_Prototipos_Funcoes.h"
#include <string.h>
#include <stdlib.h>



#define CPU_FREQ 	 100E6
#define LSPCLK_FREQ  CPU_FREQ/2
#define SCI_FREQ 	 57600
#define SCI_PRD 	 (LSPCLK_FREQ/(SCI_FREQ*8))-1


// Constantes utilizados no protocolo de comunicaçao
#define CMD_SOLDAR    0x01
#define CMD_CICLOS    0x02
#define CMD_SKT       0x03
#define CMD_KIR       0x04
#define CMD_TESTE	  0x41

#define MODO_SKT_PIC      0x01
#define MODO_KIR_PIC      0x02

#define SINC		  0x1B
// fim constantes comunicaçao




union _bytefloat {
	float num_float;
	char  num_4bytes[4];
};

union long2float {
	float nFloat;
	unsigned long unLong;
};



union _bytefloat Buffer;
union long2float Float2LongTx, Long2FloatRx;
unsigned long jtmp;

void itoa(int n, char s[]);
void SciA_Comandos(char funcao, union long2float parametro);
char SciA_Enviar_5bytes(char funcao, float parametro);

char Funcao = 0;
float sciAtmp = 55.6;

extern float Skt[30], Corrente[30];   //debeug kir.c
extern unsigned int k;				//debug kir.c
extern Uint16 Fim_pacote;		    //utilizado pela versao da sci q usa Hyperterminal

void Configura_SciA()
{

    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                  // No parity,8 char bits,
                                  // async mode, idle-line protocol
    SciaRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
                                  // Disable RX ERR, SLEEP, TXWAKE
	SciaRegs.SCICTL2.all =0x0003;
    SciaRegs.SCICTL2.bit.TXINTENA =0;
    SciaRegs.SCICTL2.bit.RXBKINTENA =1;
    SciaRegs.SCIHBAUD = 0x0000;
    SciaRegs.SCILBAUD = SCI_PRD;
	
	SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset

    SciaRegs.SCIFFTX.all=0xC040;      //Reseta Sci; Habilita SCi; Desabilita Iterrup;
    SciaRegs.SCIFFRX.all=0x0007;	  // Gera interrup qdo chegarem 1 bytes
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; //Limpa flag de interrup Rx
	SciaRegs.SCIFFRX.bit.RXFFIENA = 0;  //Habilita interrupcao Rx
    SciaRegs.SCIFFCT.all=0x00;
	SciaRegs.SCIFFCT.bit.FFTXDLY = 29;	//500 us//Delay entre transmissoes de bytes. (0xFF=256 ciclos do baund rate) 

    
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}


float TrataComoFloat()
{
	Buffer.num_4bytes[0] = SciaRegs.SCIRXBUF.all;  //LSB
	Buffer.num_4bytes[1] = SciaRegs.SCIRXBUF.all;
	Buffer.num_4bytes[2] = SciaRegs.SCIRXBUF.all;
	Buffer.num_4bytes[3] = SciaRegs.SCIRXBUF.all;  //MSB

	jtmp = 0;
	jtmp = Buffer.num_4bytes[3];
	jtmp = jtmp <<8;
	jtmp |= Buffer.num_4bytes[2];  
	jtmp = jtmp <<8;
	jtmp |= Buffer.num_4bytes[1];
	jtmp = jtmp <<8;
	jtmp |= Buffer.num_4bytes[0];

	Long2FloatRx.unLong = jtmp;	
	
	return  Long2FloatRx.nFloat;
}



void SciA_Transmitir(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF=a;

}

char SciA_Receber()
{

	while(SciaRegs.SCIFFRX.bit.RXFFST !=1) { } // wait for XRDY =1 for empty state
    return SciaRegs.SCIRXBUF.all;
}


char SciA_Enviar_5bytes(char funcao, float parametro) 
{
	unsigned int i = 0;
	char msg[5];

	Float2LongTx.nFloat = parametro;

	msg[0] = funcao;
	msg[1] = (Float2LongTx.unLong & 0x000000FF);
	msg[2] = (Float2LongTx.unLong & 0x0000FF00) >> 8;
	msg[3] = (Float2LongTx.unLong & 0x00FF0000) >> 16;
	msg[4] = (Float2LongTx.unLong & 0xFF000000) >> 24;
	
    for(i=0; i<5; i++)
    {
    	SciA_Transmitir(msg[i]);
		//DELAY_US(1000);
    }
}


char SciA_Enviar_Pacote(char funcao, float parametro)
{
	unsigned int i = 0;
	char msg[7];

	Float2LongTx.nFloat = parametro;
	
	msg[0] = SINC;
	msg[1] = funcao;
	msg[2] = (Float2LongTx.unLong & 0x000000FF);
	msg[3] = (Float2LongTx.unLong & 0x0000FF00) >> 8;
	msg[4] = (Float2LongTx.unLong & 0x00FF0000) >> 16;
	msg[5] = (Float2LongTx.unLong & 0xFF000000) >> 24;
	msg[6] = 0xFF;
	
    for(i=0; i<7; i++)
    	SciA_Transmitir(msg[i]);

	DELAY_US(2000);	
} 

void SciA_SendMsg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        SciA_Transmitir(msg[i]);
        i++;
    }
}

void EnviaValorRmsMedio()
{
	float calc;
	
	calc = (float)Solda.I_rms_media/(Solda.ciclos_ativos-1);
	SciA_Enviar_5bytes(4, calc );	
}


void SciA_Comandos(char funcao, union long2float parametro)
{
	Uint16 ciclos_soldando = 0;
	float temp = 0;
	union long2float tmpFUL;

	if(funcao == CMD_TESTE)
	{
	/*	if(parametro.unLong == 0x41414141)
			GpioDataRegs.GPADAT.bit.GPIO31 = 0;
	//	if(Funcao == 0x03)
		if(parametro.unLong == 0x42424242)
			GpioDataRegs.GPADAT.bit.GPIO31 = 1;
	*/
	
	tmpFUL.unLong = 0x44434241;
	SciA_Enviar_Pacote(funcao, tmpFUL.nFloat);

	}

	if(funcao == CMD_SKT)
	{	//Altera SKT
	
		Solda.principal.skt = (unsigned int) parametro.unLong;
		seta_alpha(Solda.principal.skt);
	
		//Devolve o ACK 
		tmpFUL.unLong = 0x000000AA;
		SciA_Enviar_Pacote(funcao, tmpFUL.nFloat);
	}


	if(funcao == CMD_KIR)
	{	//Altera referência de corrente do KIR
	
		Kir.i_ref = parametro.nFloat;
		Kir.EscolherPontos(&Kir);
		
		//devolve o ACK	
		tmpFUL.unLong = 0x000000AA;
		SciA_Enviar_Pacote(funcao, tmpFUL.nFloat);	
	}
		

	if(funcao == CMD_CICLOS)
	{	//No. Ciclos Ativos (soldando)
		
		ciclos_soldando = (unsigned int) parametro.unLong;
		Solda.ciclos_ativos = 2 * ciclos_soldando ;  //Um pulso a mais, pois o primeiro eh descartado
		
		//Devolve o ACK
		tmpFUL.unLong = 0x000000AA;
		SciA_Enviar_Pacote(funcao, tmpFUL.nFloat);
	}


	if(funcao == CMD_SOLDAR)
	{	//Inicia soldagem

		//Devolve o ACK
		

	  	if(parametro.unLong == MODO_SKT_PIC) 
	   		Solda.modo = 0;  //modo KirConfig (preenche tabela)
	 	if(parametro.unLong == MODO_KIR_PIC) 
	    	Solda.modo = 1;

	//	if(Solda.modo < 3)  //executa solda, somente se modo==1 ou modo==2
		{
			
			//tmpFUL.unLong = 0x000000AA;
			

		  	Rms.FaseA.i = 0;
			Solda.cont_ciclos = 0;
			Solda.ciclo_atual = 0;
			EPwm1Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
			Solda.soldar = 1;
			Solda.I_rms_media = 0;

			while(Solda.soldar);                  //Espera até concluir a solda

			Solda.ciclo_atual--;
		  	//temp = (float)Solda.I_rms_media/(Solda.ciclo_atual);
			temp = 157.42;
			//SciA_Enviar_Pacote(funcao, temp);    //Envia o valor da corrente de solda
	 		Fim_pacote = 1;

			
		}

		tmpFUL.unLong = 0x000000AA;
		SciA_Enviar_Pacote(funcao, tmpFUL.nFloat);
	}	
}


/*
//recebe o pacote de 5 bytes, sendo:
//   byte1: identificador da ação
//byte 2-5: float começando pelo byte mais significativo
interrupt void sciA_Rx_isr(void)
{
	Uint16 sinc;
	Uint16 check;
	Uint16 i;
		
	
		for(i=0;i<10;i++) 		//metade do tamanho da fifo + 1
		{
			sinc = SciaRegs.SCIRXBUF.all;
			if(sinc == SINC)
				break;
		}
		
		if(sinc == SINC)
		{
			Funcao = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[0] = SciaRegs.SCIRXBUF.all;   //LSB
			Buffer.num_4bytes[1] = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[2] = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[3] = SciaRegs.SCIRXBUF.all;   //MSB
			check = SciaRegs.SCIRXBUF.all;

			jtmp = 0;
			jtmp = Buffer.num_4bytes[3];
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[2];  
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[1];
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[0];

			Long2FloatRx.unLong = jtmp;

	//	if(Funcao) 
			SciA_Comandos(Funcao, Long2FloatRx);
		}
	

	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;	//Zera a FIFO

	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag
	SciaRegs.SCIFFRX.bit.RXFFINTCLR=1; 	// Clear Interrupt flag
//	PieCtrlRegs.PIEACK.all|=0x100;  	// Issue PIE ack
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}


*/



void RecebePacote(void)
{
	Uint16 sinc;
	Uint16 check;
	Uint16 i;
		
	/*
		for(i=0;i<10;i++) 		//metade do tamanho da fifo + 1
		{
			sinc = SciaRegs.SCIRXBUF.all;
			if(sinc == SINC)
				break;
		}
	*/

	if(SciaRegs.SCIFFRX.bit.RXFFST >= 7)  // wait for XRDY =1 for empty state
	{
		sinc = SciaRegs.SCIRXBUF.all;	
		if(sinc == SINC)
		{
			Funcao = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[0] = SciaRegs.SCIRXBUF.all;   //LSB
			Buffer.num_4bytes[1] = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[2] = SciaRegs.SCIRXBUF.all;
			Buffer.num_4bytes[3] = SciaRegs.SCIRXBUF.all;   //MSB
			check = SciaRegs.SCIRXBUF.all;

			jtmp = 0;
			jtmp = Buffer.num_4bytes[3];
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[2];  
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[1];
			jtmp = jtmp <<8;
			jtmp |= Buffer.num_4bytes[0];

			Long2FloatRx.unLong = jtmp;

	//	if(Funcao) 
			SciA_Comandos(Funcao, Long2FloatRx);
		}
	
	}

//	for(i=0;i<16;i++)  //16 ->tamanho da fifo
//		sinc = SciaRegs.SCIRXBUF.all;

	ScibRegs.SCIFFRX.bit.RXFIFORESET = 0;	//Zera a FIFO
	DELAY_US(1);
	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;	//Zera a FIFO

	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag

}






void itoa(int n, char s[])
{
    int i, sign;
	int j, c;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;

    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */

    if (sign < 0)
        s[i++] = '-';

    s[i] = '\0';

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
	}
}












		


