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
#define TAM_FIFO    7


// Constantes utilizados no protocolo de comunicaçao
#define CMD_SOLDAR    0x01
#define CMD_CICLOS    0x02
#define CMD_SKT       0x03
#define CMD_KIR       0x04
#define CMD_TESTE	  0x41
#define CMD_ERRO	  0xFF


#define	TIPO_SOLDA			0x01

#define PRESOLDA_CICLOS		0x02
#define PRESOLDA_SKT		0x03
#define PRESOLDA_KIR		0x04

#define TEMPO_FRIO1_CICLOS	0x05

#define SUBIDA_CICLOS		0x06

#define PRINCIPAL_CICLOS	0x07
#define PRINCIPAL_SKT		0x08
#define PRINCIPAL_KIR		0x09
#define PRINCIPAL_IMPULSOS	0x0A
#define PRINCIPAL_PAUSA		0x0B

#define DESCIDA_CICLOS		0x0C

#define TEMPO_FRIO2_CICLOS  0x0D

#define POSSOLDA_CICLOS		0x0E
#define POSSOLDA_SKT		0x0F
#define POSSOLDA_KIR		0x10

#define TEMPO_FRIO3_CICLOS	0x11

#define MANUTENCAO_CICLOS	0x12

#define PAUSA4_CICLOS		0x13

#define SOLDAR				0x50

#define REPORTAR_ERROS		0x60

#define SELECAO_MUX         0x80




#define SINC		  0x1B
// fim constantes comunicaçao


extern Uint16 FlagSoldar;

union _bytefloat {
	float num_float;
	char  num_4bytes[4];
};





union _bytefloat Buffer;
union long2float Float2LongTx, Long2FloatRx;
//unsigned long jtmp;   //Comentada no serviço

void itoa(int n, char s[]);
void SciA_Comandos(char funcao, union long2float parametro);
char SciA_Enviar_5bytes(char funcao, float parametro);



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
	SciaRegs.SCIFFCT.bit.FFTXDLY = 0xFF;//0x29;	//500 us//Delay entre transmissoes de bytes. (0xFF=256 ciclos do baund rate) 

    
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}

/*
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
*/


#pragma CODE_SECTION(SciA_Transmitir, "ramfuncs");
void SciA_Transmitir(int a)
{
    //while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}

    SciaRegs.SCITXBUF=a;
	DELAY_US(20);

}

char SciA_Receber()
{

	while(SciaRegs.SCIFFRX.bit.RXFFST !=1) { } // wait for XRDY =1 for empty state
    return SciaRegs.SCIRXBUF.all;
}

#pragma CODE_SECTION(SciA_Enviar_5bytes, "ramfuncs");
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

#pragma CODE_SECTION(SciA_Enviar_Pacote, "ramfuncs");
char SciA_Enviar_Pacote(char funcao, union long2float varL2F)
{
	 unsigned int i = 0;
	 char msg[7];
	 Uint16 checksum = 0;
	 Uint32 parametro;
	 
	 parametro = varL2F.unLong;



	
	 msg[0] = SINC;
	 msg[1] = funcao;
	 msg[2] = (parametro & 0x000000FF);
	 msg[3] = (parametro & 0x0000FF00) >> 8;
	 msg[4] = (parametro & 0x00FF0000) >> 16;
	 msg[5] = (parametro & 0xFF000000) >> 24;
	 //msg[6] = 0xFF;
	
	
	 //Realiza o cheksum dos campos 6 campos (0:5)
	 for(i=0; i<(TAM_FIFO-1); i++)
	    checksum ^= msg[i];
   //fim cheksum
   
	 msg[6] = checksum;
	
//	DELAY_US(20000);
	 //transmite os dados
   	for(i=0; i<TAM_FIFO; i++)
    	SciA_Transmitir(msg[i]);
   //fim transmissão

		
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



void Reportar_Erro_Ilegal()
{
	union long2float vartemp;
	vartemp.unLong = 0xAAFFFFFF;
	SciA_Enviar_Pacote( (Uint16) CMD_ERRO, vartemp);
}

void DevolveACK(Uint16 funcao)
{
	union long2float vartemp;
	vartemp.unLong = 0x000000AA;
	SciA_Enviar_Pacote(funcao, vartemp);
}

void SciReportarErro(Uint32 erro)
{
	union long2float vartemp;
	vartemp.unLong = erro;
	SciA_Enviar_Pacote(REPORTAR_ERROS, vartemp);
}


#pragma CODE_SECTION(SciA_Comandos, "ramfuncs");
void SciA_Comandos(char funcao, union long2float parametro)
{
	Uint16 ciclos_soldando = 0;
	float temp = 0;
	union long2float tmpUL2F;
	Uint32 vartemp;
	Uint16 i;

	switch(funcao) 
	{
		case CMD_TESTE: 
			/*	if(parametro.unLong == 0x41414141)
				GpioDataRegs.GPADAT.bit.GPIO31 = 0;
			//	if(Funcao == 0x03)
				if(parametro.unLong == 0x42424242)
				GpioDataRegs.GPADAT.bit.GPIO31 = 1;
			*/
	
			tmpUL2F.unLong = 0x44434241;
			SciA_Enviar_Pacote(funcao, tmpUL2F);
			break;
			
		case SELECAO_MUX:
			set_mux_corrente( (unsigned char)parametro.unLong );
			break; 

		/////////////////////////
		case TIPO_SOLDA:
			Solda.tipo = (Uint16) parametro.unLong;	
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		//////////////////////////
		case PRESOLDA_CICLOS:
			SciA_Enviar_Pacote(funcao, parametro);
			Solda.preSolda.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			break;

		case PRESOLDA_SKT:
			Solda.preSolda.skt =  (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case PRESOLDA_KIR:
			//Ajusta a referencia de corrente do KIR
			Solda.preSolda.iRef = parametro.nFloat;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		///////////////////////
		case TEMPO_FRIO1_CICLOS:
			SciA_Enviar_Pacote(funcao, parametro);
			Solda.pausa1.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			break;

		//////////////////////
		case SUBIDA_CICLOS:
			Solda.subida.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;


		//////////////////////////
		case PRINCIPAL_CICLOS:
			//No. Ciclos Ativos (soldando)
			ciclos_soldando = (unsigned int) parametro.unLong;
			Solda.principal.total_semiciclos = 2 * ciclos_soldando ;  //Um pulso a mais, pois o primeiro eh descartado
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case PRINCIPAL_SKT:
			//Altera SKT	
			Solda.principal.skt = (unsigned int) parametro.unLong;
			SetaSkt(Solda.principal.skt);	
			//DevolveACK(funcao);			
			SciA_Enviar_Pacote(funcao, parametro);
			break;


		case PRINCIPAL_KIR:
			//Altera referência de corrente do KIR
			if( (Kir.i_ref > 1.02*parametro.nFloat) || (Kir.i_ref < 0.98*parametro.nFloat) )
			{	
				Kir.i_ref = parametro.nFloat;
				Kir.EscolherPontos(&Kir);
			}
			//DevolveACK(funcao);	
			SciA_Enviar_Pacote(funcao, parametro);
			break;
		
		case PRINCIPAL_IMPULSOS:
			//Altera o numero de impulsos na solda principal (varios impulsos com varios ciclos)
			Solda.principal.total_impulsos = (unsigned int) parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;
		
		case PRINCIPAL_PAUSA:
			Solda.pausaPrincipal.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;	


		//////////////////
		case DESCIDA_CICLOS:
			Solda.descida.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		///////////////
		case TEMPO_FRIO2_CICLOS:
			Solda.pausa2.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		/////////////////////
		case POSSOLDA_CICLOS:
			Solda.posSolda.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case POSSOLDA_SKT:
			Solda.posSolda.skt =  (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case POSSOLDA_KIR:
			//Ajusta a referencia de corrente do KIR
			Solda.posSolda.iRef = parametro.nFloat;
			//DevolveACK(funcao);			
			SciA_Enviar_Pacote(funcao, parametro);
			break;


		///////////////////////
		case TEMPO_FRIO3_CICLOS:
			Solda.pausa3.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case MANUTENCAO_CICLOS:
			Solda.manutencao.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		case PAUSA4_CICLOS:
			Solda.pausa4.total_semiciclos = 2 * (unsigned int)parametro.unLong;
			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);
			break;

		////////////////////////		


		case SOLDAR:
			//Inicia soldagem

			//DevolveACK(funcao);
			SciA_Enviar_Pacote(funcao, parametro);

		if( DetecaoRogowiski() == 1 ) 
		{

		  	if(parametro.unLong == 0) {
		   		Solda.principal.modo = 0;  //modo SKT
				ConfiguraRampasSKT();
			}
	  		if(parametro.unLong == 1) {
	    		Solda.principal.modo = 1;	//Modo KIR
				ConfiguraRampasKIR();
			//	Kir.EscolherPontos(&Kir);
			}
			if(parametro.unLong == 2) {
		   		Solda.principal.modo = 2;  //modo KirConfig (preenche tabela)
				ConfiguraRampasSKT();
			}

			

			Solda.subida.cont_semiciclos = 0;

			//WorkAround - Antonio
	/*		if(Solda.tipo == CONTINUA) {
				Solda.principal.total_semiciclos = 0;
				Solda.pausaPrincipal.total_semiciclos = 1;
			}
	*/
			//WorkAround - elimina pausa "residual" na solda principal, qdo se deseja 0 pausa 
			if(Solda.pausaPrincipal.total_semiciclos==0) {
				Solda.pausaPrincipal.total_semiciclos = 1;
				Solda.principal.total_semiciclos = Solda.principal.total_semiciclos * Solda.principal.total_impulsos;
				Solda.principal.total_impulsos = 1;
			}
			//Fim WorkAround
		
			//Se a bobina estiver conectada e o Pino de solda estiver ativo...//
			//if( DetecaoRogowiski() == 1 ) {
				if(PinoSoldar == PIN_ATIVO )	//Além do comando Soldar é necessario que este pino esteja no nídel SOLDAR
					FlagSoldar = 1;
				GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;
			//}
			}
			else {
				DELAY_US(100000);
				tmpUL2F.nFloat = -100.0;
				SciA_Enviar_Pacote(0x55, tmpUL2F );
			}

			break;
		} 	
}


#pragma CODE_SECTION(RecebePacote, "ramfuncs");
void RecebePacote(void)
{
	Uint16 sinc = 0;
	Uint16 funcao = 0;
	Uint16 i;
	Uint16 buffer[TAM_FIFO];
	Uint16 checksum = 0, checksum_recebido;
	Uint32 jtmp = 0;
		

	if(SciaRegs.SCIFFRX.bit.RXFFST >= TAM_FIFO)  // wait for XRDY =1 for empty state
	{
	    for(i=0; i<TAM_FIFO; i++) 
	       buffer[i] = SciaRegs.SCIRXBUF.all;
	    SciaRegs.SCIRXBUF.all;
	          
	    //realiza o cheksum dos dados recebidos
	    for(i=0; i<(TAM_FIFO-1); i++) 
	       checksum ^= buffer[i];
	    //fim cheksum
	    
		sinc = buffer[0];	
		checksum_recebido = buffer[TAM_FIFO-1];
		
		if( (sinc==SINC) /*&& (cheksum==checksum_recebido)*/ )
		{
 			funcao = buffer[1];
 			jtmp = buffer[5];   //MSB
			jtmp = jtmp <<8;
			jtmp |= buffer[4];  
			jtmp = jtmp <<8;
			jtmp |= buffer[3];
			jtmp = jtmp <<8;
			jtmp |= buffer[2];  //LSB
			
			Long2FloatRx.unLong = jtmp;

			SciA_Comandos(funcao, Long2FloatRx);
		}
	
	}

//	for(i=0;i<16;i++)  //16 ->tamanho da fifo
//		sinc = SciaRegs.SCIRXBUF.all;

	ScibRegs.SCIFFRX.bit.RXFIFORESET = 0;	//Zera a FIFO
//	DELAY_US(1);
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












		


