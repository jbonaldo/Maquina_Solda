/*
	Desenvolvido por Jakson Bonaldo   
	Campinas - SP 		19/08/09
	Rotinas referentes à SciA
	
	Como o DSP tem um buffer para dados transmitidos e recebidos de 16 posicoes
	não se torna necessário implementar buffers via software.

	Gpio28 -> Rx
	Gpio29 -> Tx
*/

#include "Comandos.h"

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File 
#include "Jakson_Prototipos_Funcoes.h"
#include <string.h>
#include <stdlib.h>



#define CPU_FREQ 	 150E6
#define LSPCLK_FREQ  CPU_FREQ/2
#define SCI_FREQ 	 57600
#define SCI_PRD 	 (LSPCLK_FREQ/(SCI_FREQ*8))-1
#define TAM_FIFO    7


union _bytefloat {
	float num_float;
	char  num_4bytes[4];
};

union _bytefloat Buffer;
union long2float Float2LongTx, Long2FloatRx;
//unsigned long jtmp;   //Comentada no serviço




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
	SciaRegs.SCIFFCT.bit.FFTXDLY = 0x40;//0x29;	//500 us//Delay entre transmissoes de bytes. (0xFF=256 ciclos do baund rate) 

    
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}

/*
float TrataComoFloat()
{
	Buffer.num_4bytes[0] = SciaRegs.SCIRXBUF.all;  //LSB
	Buffer.num_4bytes[1] = SciaRegs.SCIRXBUF.all;extern Uint16 Fim_pacote;		    //utilizado pela versao da sci q usa Hyperterminal
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






char buffer[TAM_FIFO];
#pragma CODE_SECTION(RecebePacote, "ramfuncs");
void RecebePacote(void)
{
	Uint16 funcao = 0;
	Uint16 i;

	Uint16 checksum = 0, checksum_recebido;
	Uint32 jtmp = 0;
	Uint16 temp;
	static Uint16 indice = 0;
		

//	if(SciaRegs.SCIFFRX.bit.RXFFST >= TAM_FIFO)  // wait for XRDY =1 for empty state
//	{
//	    for(i=0; i<TAM_FIFO; i++) 
//	       buffer[i] = SciaRegs.SCIRXBUF.all;
//	    SciaRegs.SCIRXBUF.all;
//	          
//	    //realiza o cheksum dos dados recebidos
//	    for(i=0; i<(TAM_FIFO-1); i++) 
//	       checksum ^= buffer[i];
//	    //fim cheksum
//	    
//		sinc = buffer[0];	
//		checksum_recebido = buffer[TAM_FIFO-1];

/////////////////////////////////////////////////////

	if(SciaRegs.SCIFFRX.bit.RXFFST >= 1)  // wait for XRDY =1 for empty state
	{
		temp = SciaRegs.SCIRXBUF.all;
		if(temp == SINC) {
			indice = 0;
		}
		
		buffer[indice++] = temp;
		
		if(indice >= TAM_FIFO) 
		{
		    //realiza o cheksum dos dados recebidos
		    for(i=0; i<(TAM_FIFO-1); i++) 
		       checksum ^= buffer[i];
		    //fim cheksum
		    
			checksum_recebido = buffer[TAM_FIFO-1];

 			funcao = buffer[1];
 			jtmp = buffer[5];   //MSB
			jtmp = jtmp <<8;
			jtmp |= buffer[4];  
			jtmp = jtmp <<8;
			jtmp |= buffer[3];
			jtmp = jtmp <<8;
			jtmp |= buffer[2];  //LSB
			
			Long2FloatRx.unLong = jtmp;

			Comandos(funcao, jtmp);
		}
	}

//	for(i=0;i<16;i++)  //16 ->tamanho da fifo
//		sinc = SciaRegs.SCIRXBUF.all;

	ScibRegs.SCIFFRX.bit.RXFIFORESET = 0;	//Zera a FIFO
//	DELAY_US(1);
	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;	//Zera a FIFO

	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag

}




