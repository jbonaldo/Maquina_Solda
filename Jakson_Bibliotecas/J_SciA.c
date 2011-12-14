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
#include "Solda.h"
#include <string.h>
#include <stdlib.h>



#define CPU_FREQ 	 150E6
#define LSPCLK_FREQ  CPU_FREQ/2
#define SCI_FREQ 	 57600
#define SCI_PRD 	 (LSPCLK_FREQ/(SCI_FREQ*8))-1

union _bytefloat {
	float num_float;
	char  num_4bytes[4];
};

//union long2float {
//	float nFloat;
//	unsigned long unLong;
//};



union _bytefloat Buffer;
union long2float Float2LongTx, Long2FloatRx;
unsigned long jtmp;

void itoa(int n, char s[]);
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
    SciaRegs.SCIFFRX.all=0x0001;	  // Gera interrup qdo chegarem 1 bytes
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; //Limpa flag de interrup Rx
	SciaRegs.SCIFFRX.bit.RXFFIENA = 0;  //Habilita interrupcao Rx
    SciaRegs.SCIFFCT.all=0x00;

    
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
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

/*
//recebe o pacote de 5 bytes, sendo:
//   byte1: identificador da ação
//byte 2-5: float começando pelo byte mais significativo
char SciA_Receber_5bytes()
{

	if(SciaRegs.SCIFFRX.bit.RXFFST >= 5)  // wait for XRDY =1 for empty state
	{
		Funcao = SciaRegs.SCIRXBUF.all;
		Buffer.num_4bytes[0] = SciaRegs.SCIRXBUF.all;
		Buffer.num_4bytes[1] = SciaRegs.SCIRXBUF.all;
		Buffer.num_4bytes[2] = SciaRegs.SCIRXBUF.all;
		Buffer.num_4bytes[3] = SciaRegs.SCIRXBUF.all;

		jtmp = 0;
		jtmp = Buffer.num_4bytes[3];
		jtmp = jtmp <<8;
		jtmp |= Buffer.num_4bytes[2];  
		jtmp = jtmp <<8;
		jtmp |= Buffer.num_4bytes[1];
		jtmp = jtmp <<8;
		jtmp |= Buffer.num_4bytes[0];

		Long2FloatRx.unLong = jtmp;
		SciA_Enviar_5bytes(Funcao, Long2FloatRx.nFloat);

		if(Funcao) 
			SciA_Comandos(Funcao, Long2FloatRx.nFloat);
		return SciaRegs.SCIRXBUF.all;
	}		

   // 
   return 0;
}
*/
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
	
    while(i < 5)
    {
    	SciA_Transmitir(msg[i]);
        i++;
    }
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




interrupt void sciA_Rx_isr(void)
{
    Uint16 temp;
	temp = ScibRegs.SCIRXBUF.all;	 // Read data

	ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag
	ScibRegs.SCIFFRX.bit.RXFFINTCLR=1; 	// Clear Interrupt flag
	PieCtrlRegs.PIEACK.all|=0x100;  	// Issue PIE ack
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









void SciA_Comandos_Hiperterminal()
{
	char *msg_menu;
	char msg[30];
	char temp;
	Uint16 i, j;
	float tmp;
	float calc;
	Uint16 ciclos_soldando;

	msg_menu = "\r\n\n \r\n\ 1 - Modo\n\r 2 - Alterar SKT\n\r 3 - Alterar KIR\n\r 4 - No. Ciclos\n\r 5 - Soldar\r\n 6 - Para\r\n" ;
	SciA_SendMsg(msg_menu);
	
	temp = SciA_Receber();


	if(temp == '1')
	{	//Modo de soldagem
		i = 0;
		SciA_SendMsg("\r\n\n0) SKT\n\r1) KIR\n\r2)Kir Config\n\r Modo:  ");
		while( (temp = SciA_Receber()) !='\r')
			msg[i++] = temp;

		if(msg[0] == '0') {
			SciA_SendMsg("\n\rModo SKT selecionado");
			Solda.principal.modo = 0;
		}
		if(msg[0] == '1') {
			SciA_SendMsg("\n\rModo KIR selecionado");
			Solda.principal.modo = 1;
		}
		if(msg[0] == '2') {
			SciA_SendMsg("\n\rModo Kir Config selecionado");
			Solda.principal.modo = 2;
		}
	}
		

	if(temp == '2')
	{	//Altera SKT
		i = 0;
		SciA_SendMsg("\r\n\nSKT: ");
		while( (temp = SciA_Receber()) !='\r')
			msg[i++] = temp;

		msg[i] = '\0';
		SciA_SendMsg("\r\nSKT: ");
		SciA_SendMsg(msg);
		Solda.principal.skt = atof(msg);
		//SetaSkt(Controle_SCRs.SKT);
	}


	if(temp == '3')
	{	//Altera KIR
		i = 0;
		SciA_SendMsg("\r\n\nKIR: ");
		while( (temp = SciA_Receber()) !='\r')
			msg[i++] = temp;

		msg[i] = '\0';
		SciA_SendMsg("\r\nKIR: ");
		SciA_SendMsg(msg);
		Kir.i_ref = atof(msg);
		Kir.EscolherPontos(&Kir);
	}
	
	

	if(temp == '4')
	{	//No. Ciclos Ativos (soldando)
		i = 0;
		SciA_SendMsg("\r\n\nNo. Ciclos: ");
		while( (temp = SciA_Receber()) !='\r')
			msg[i++] = temp;

		msg[i] = '\0';
		SciA_SendMsg("\r\nNo. Ciclos: ");
		SciA_SendMsg(msg);
		ciclos_soldando = atoi(msg);
		Solda.principal.total_semiciclos =  2 * ciclos_soldando ;  //Um pulso a mais, pois o primeiro eh descartado
		SciA_SendMsg("\r\nOk");				  //O tiristor recebera atoi(msg) pulsos.	
	}


	if(temp == '5')
	{	//Inicia soldagem
		Rms.FaseA.i = 0;
		//Solda.cont_ciclos = 0;


		//alteracoes novas
		IniciarSolda();
		//SciA_SendMsg("\nSoldando!");


		while(Solda.soldar);

		
		/*//Debug Skt e Corrente no modo SKT
		for(j=0;j<k_ciclos;j++)
		{
		
			SciA_SendMsg("\n\rIrms[kA]: ");
			CorrenteSkt[j] *= 1000;
			itoa(CorrenteSkt[j], msg);
			SciA_SendMsg(msg);

			SciA_SendMsg("    N_amostras: ");	
			itoa(Namostras[j], msg);
			SciA_SendMsg(msg);
		}
		k_ciclos = 0;
		//Fim Debug Skt e Corrente no modo SKT
		*/

		//Solda.ciclo_atual--;
		calc = (float)Solda.principal.iRmsMedia[0]/(Solda.principal.cont_semiciclos - 1);
		calc *= 1000;  //25000 -> fator de escala
		SciA_SendMsg("\n\rRms Medio: ");
		itoa( (int)calc, msg);
		SciA_SendMsg(msg);
		SciA_SendMsg(" A\n\rSemi-ciclos Usados: ");
		itoa( (Solda.principal.cont_semiciclos ), msg);
		SciA_SendMsg(msg);
		Fim_pacote = 1;

		

	/*	//Debug Skt e Corrente no modo KIR
		for(j=0;j<k;j++)
		{
			SciA_SendMsg("\n\rSkt: ");
			itoa(Skt[j], msg);
			SciA_SendMsg(msg);

			SciA_SendMsg("    kA: ");
			Corrente[j] *= 1000;	
			itoa((int)Corrente[j], msg);
			SciA_SendMsg(msg);
		}
		k = 0;
		//Fim Debug Skt e Corrente no modo KIR
	*/	
		 
	}	

	if(temp == '6')
	{	//Interromper Soldgem
		Desabilita_Pulsos();
		Solda.soldar = 0;
		SciA_SendMsg("Ok!");
	}



}
		


void Reportar_Erro_Ilegal() {};