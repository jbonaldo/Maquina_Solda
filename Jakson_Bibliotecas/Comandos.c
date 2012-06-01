

#include "Comandos.h"
#include "Jakson_Prototipos_Funcoes.h"
#include "Solda.h"

extern Uint16 FlagSoldar;
extern Uint16 Fim_pacote;		    //utilizado pela versao da sci q usa Hyperterminal
void itoa(int n, char s[]);


void SciA_Comandos(char funcao, union long2float parametro);
char SciA_Enviar_5bytes(char funcao, float parametro);



#pragma CODE_SECTION(Comandos, "ramfuncs");
void Comandos(char funcao, unsigned long int param)
{
	union long2float parametro;
	Uint16 ciclos_soldando = 0;
	union long2float tmpUL2F;

	
	parametro.unLong = param;

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

		if( DetectaRogowiski() == 1 ) 
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
			
			//////////// tirado para debug ///////////
			//////// ATENCAO ////////////////
				//if(PinoSoldar == PIN_ATIVO )	//Além do comando Soldar é necessario que este pino esteja no nídel SOLDAR
		   //////////// COLCOAR DE VOLTA ////////////////////
				FlagSoldar = 1;
				GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;
			}
			else {
				DELAY_US(100000);
				tmpUL2F.nFloat = -100.0;
				SciA_Enviar_Pacote(0x55, tmpUL2F );
			}

			break;
		} 	
}

//void EnviaValorRmsMedio()
//{
//	float calc;
//	
//	calc = (float)Solda.I_rms_media/(Solda.principal.total_semiciclos);
//	SciA_Enviar_5bytes(4, calc );	
//}


void comandos_enviar_corrente(float corrente)
{
	union long2float tmp;
	
	tmp.nFloat = corrente;
	SciA_Enviar_Pacote(0x55, tmp);    //Envia o valor da corrente de solda para a gerencia
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
		//IniciarSolda();
		FlagSoldar = 1;
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



