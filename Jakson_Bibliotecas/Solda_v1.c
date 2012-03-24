#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File  
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário
#include "Solda.h"
#include "Comandos.h"

#define ATIVO		0x01
#define TERMINADO	0x00

Uint16 Fim_pacote = 0;   
Uint16 FlagSoldar = 0;
Uint16 FlagSoldaConcluida = 0;
Uint16 FlagEmergencia = 0;

Uint16 Estado = 0;
extern Uint16 Fim_pacote;
struct _solda Solda;
struct _compensacaoTensao CompensacaoTensao = {0.0, 0.0, 1.0};	//fator de compensacao = 1.0;

void InicializarParametrosSolda();
void Gambiarra_MudaKirParaSkt();
void verificaEmergencia();

//#pragma CODE_SECTION(pegaCorrente, "ramfuncs");
void pegaCorrente()
{
	Rms.FaseA.RMS120Hz(&Rms.FaseA);	//Calcula o valor RMS da corrente do semiciclo 

	Solda.principal.iRmsMedia[Solda.principal.cont_impulsos] += Rms.FaseA.out120Hz;  //Soma os valores rms de cada ciclo (qdo a solda acabar, esta soma é dividida pelo número de semiciclos)
	//Solda.principal.iRmsMedia[Solda.principal.cont_impulsos] += 15.7 * (1+Solda.principal.cont_impulsos);
	


	if(Solda.principal.cont_semiciclos == 1) {  //Descarta o primeiro pulso
	//	if(Solda.principal.cont_impulsos == 0)		
		Solda.principal.iRmsMedia[Solda.principal.cont_impulsos] = 0;
	}

	if( (Solda.principal.modo == 1) && Solda.principal.estado) //0->skt;   1->kir
	{
		Kir.i_rms += (float) Rms.FaseA.out120Hz * 0.5;


		#ifdef JDEBUG //usado para debug
		if( ( Solda.principal.cont_semiciclos % 2 ) == 1 ) //Se o ciclo for ímpar entra no kir (que deve ser tratado a cada ciclo e nao semicilco)
		{
			Skt[k] = Kir.skt2;
			Corrente[k] = Kir.i_rms;
			k++;
		}
		#endif
		//fim debug

		if(Solda.principal.cont_semiciclos <= Solda.principal.total_semiciclos)
		{
			Kir.Calcular(&Kir);
			SetaSkt(Kir.skt_out);
			Solda.principal.skt = Kir.skt_out;
		}
		else
		{
			Kir.i1 = Kir.i2;
			Kir.i2 = Kir.i_rms;
		}
	} 
}

/**
*  Solda realmente ativa, podendo ser realizada
*  em SKT ou KIR
*/
//#pragma CODE_SECTION(SoldaPrincipal, "ramfuncs");
void SoldaPrincipal()
{
	Solda.principal.cont_semiciclos++;
	
	//Controla o numero de semi-ciclos aplicados
	if( Solda.soldar) {
		Habilita_Pulsos();
		if(Solda.principal.cont_semiciclos > Solda.principal.total_semiciclos) {
			Desabilita_Pulsos();
			//Solda.soldar = 0;
		}
	}
	else {
		Desabilita_Pulsos();
		RESETA_INTEGRADOR;	
	}
	//Fim controle semi-ciclos aplicados

/*	#ifdef DEBUG
	Namostras[k_ciclos] = Rms.FaseA.i;
	CorrenteSkt[k_ciclos] = Rms.FaseA.RMS120Hz(&Rms.FaseA);	//Calcula o valor RMS da corrente do semiciclo 
	k_ciclos++;
	#endif
*/
	pegaCorrente();
}


float CalcularSkt(float iRef)
{
	float m = 0;
	float di = 0;
	float dt = 0;
	float sktFinal = 0;

	di = Kir.padrao_i2 - Kir.padrao_i1;
	dt = Kir.padrao_skt2 - Kir.padrao_skt1;
	m = (float)di/dt;
	sktFinal = ( (float)(iRef - Kir.padrao_i1) / m ) + Kir.padrao_skt1;

	return sktFinal;
}

void ConfigurarRampas()
{
	float sktPrincipal;
//	float sktPre = 0;
//	float sktPos = 0;

	sktPrincipal = Solda.principal.skt;

	//Se a pre e a pos solda nao estiverem presentes, o skt deve ser o valor minimo, para nao interferir na rampa
	if(Solda.preSolda.total_semiciclos <= 0)		
		Solda.preSolda.skt = 10;
	if(Solda.posSolda.total_semiciclos <= 0)
		Solda.posSolda.skt = 10;
	if(Solda.preSolda.skt < 10) Solda.preSolda.skt = 10;	//garante o valor minimo
	if(Solda.posSolda.skt < 10) Solda.posSolda.skt = 10;	//garante o valor minimo	

	if(Solda.subida.total_semiciclos > 0) {
		if(Solda.pausa1.total_semiciclos > 0)
			Solda.subida.deltaSkt = (sktPrincipal-10) / (float)Solda.subida.total_semiciclos;	
		else
			Solda.subida.deltaSkt = (sktPrincipal-Solda.preSolda.skt) / (float)Solda.subida.total_semiciclos;	

		if(Solda.pausa1.total_semiciclos > 0)
			Solda.subida.skt = 10;
		else
			Solda.subida.skt = Solda.preSolda.skt;
	}

	if(Solda.descida.total_semiciclos > 0) {
		if(Solda.pausa2.total_semiciclos > 0)
			Solda.descida.deltaSkt = (sktPrincipal-10) / (float)Solda.descida.total_semiciclos;		
		else
			Solda.descida.deltaSkt = (sktPrincipal-Solda.posSolda.skt) / (float)Solda.descida.total_semiciclos;		

		Solda.descida.skt = Solda.principal.skt;
	}
}


void ConfiguraRampasKIR() 
{
	Solda.principal.skt = CalcularSkt(Kir.i_ref);
 
//	if(Solda.preSolda.modo == MODO_KIR)
		Solda.preSolda.skt = CalcularSkt(Solda.preSolda.iRef);

//	if(Solda.posSolda.modo == MODO_KIR)
		Solda.posSolda.skt = CalcularSkt(Solda.posSolda.iRef); 

	ConfigurarRampas();
}


void ConfiguraRampasSKT() 
{
	ConfigurarRampas();
}



//#pragma CODE_SECTION(RampaSubida, "ramfuncs");
Uint16 RampaSubida()
/**
*   Varia os valores de Skt para formar a rampa de subida ou descida
*/
{
	Uint16 skt;

	if( (Solda.subida.total_semiciclos > 0) && (Solda.subida.cont_semiciclos < Solda.subida.total_semiciclos) )
	{
		Habilita_Pulsos();
		

	//	if(Solda.subida.skt < Solda.principal.skt)		//se ainda nao chegou no valor desejado
		skt =  Solda.subida.skt + (Solda.subida.deltaSkt * Solda.subida.cont_semiciclos);
		Solda.subida.cont_semiciclos++;
		SetaSkt(skt);
		Solda.subida.estado = ATIVO;
		
		return ATIVO;
	}
	else
	{
		Solda.subida.estado = TERMINADO;
		return TERMINADO;
	}

	//Solda.subida.estado = TERMINADO;
	//return TERMINADO;
}

//#pragma CODE_SECTION(RampaDescida, "ramfuncs");
Uint16 RampaDescida()
/**
*   Varia os valores de Skt para formar a rampa de descida 
*/
{
	Uint16 skt;

	if( (Solda.descida.total_semiciclos > 0) && (Solda.descida.cont_semiciclos < Solda.descida.total_semiciclos) )
	{
		Habilita_Pulsos();
		Solda.descida.cont_semiciclos++;

		//if(Solda.descida.skt > Solda.descida.deltaSkt)  //impede que a variavel se torne negativa
		//	Solda.descida.skt -= Solda.descida.deltaSkt;
		skt = Solda.descida.skt - (Solda.descida.deltaSkt * Solda.descida.cont_semiciclos);
		SetaSkt(skt);
		Solda.descida.estado = ATIVO;
		
		return ATIVO;
	}
	else
	{
		Solda.descida.estado = TERMINADO;
		return TERMINADO;
	}

	//Solda.descida.estado = TERMINADO;
	//return TERMINADO;
}


//#pragma CODE_SECTION(SoldaPrePos, "ramfuncs");
Uint16 SoldaPrePos(struct _prepossolda *s)
/**
*   Realiza a pré o pós solda (em skt)
*/
{	// GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;
	if( (s->total_semiciclos > 0) && (s->cont_semiciclos < s->total_semiciclos) )
	{
		
		Habilita_Pulsos();
		s->cont_semiciclos += 1;
		SetaSkt(s->skt);
		s->estado = ATIVO;
		return ATIVO;
	}
	else
	{
		s->estado = TERMINADO;
		return TERMINADO;
	}

	//s->estado = TERMINADO;
	//return TERMINADO;	
}

//#pragma CODE_SECTION(Pausa, "ramfuncs");
Uint16 Pausa(struct _pausa *p)
/**
*	Realiza um dos 3 tipos de pausa possíveis
*/
{
	if( (p->total_semiciclos > 0) && (p->cont_semiciclos < p->total_semiciclos) )
	{
		p->cont_semiciclos += 1;
		return ATIVO;
	}
	else
		return TERMINADO;

	//return TERMINADO;
}

////#pragma CODE_SECTION(SoldaImpulsos, "ramfuncs");
Uint16 SoldaImpulsos()
/**
*	Quando este impulso (varios ciclos) de solda terminar Verificar 
*	se é o último Impulso.  
*	Caso seja o último termina a solda
*	Caso contrario, reabilita a soldagem e incrementa o contador de impulsos
*/
{
	static Uint16 flag = 0;	

	if(Solda.principal.cont_semiciclos >= Solda.principal.total_semiciclos) 
	{
		if(Solda.principal.cont_impulsos < Solda.principal.total_impulsos) 
		{	
			
			if(Solda.pausaPrincipal.cont_semiciclos == 0)
				//pegaCorrente();
				SoldaPrincipal();

			Solda.principal.estado = TERMINADO;
			if( Pausa(&Solda.pausaPrincipal) )
			{
				//Solda.principal.cont_semiciclos = 0;
				if(Solda.principal.cont_impulsos == Solda.principal.total_impulsos-1)
				{
					Solda.principal.cont_impulsos += 1;

					//verificaEmergencia();
					if(FlagEmergencia == 1) 
						Solda.soldar = 0;	//Se estiver em estado de emergencia, desabilita a solda;

					return TERMINADO;
				}
				return ATIVO;
			}
			else {
				Solda.principal.cont_impulsos += 1;
				Solda.principal.cont_semiciclos = 0;
				Solda.principal.estado = ATIVO;
				SoldaPrincipal();
				Solda.pausaPrincipal.cont_semiciclos = 0;
				return ATIVO;
			}
		}
		else {
		//	if(flag) 	SoldaPrincipal();//pegaCorrente();
		//	flag = 0;
			return TERMINADO;
		}
	}

	SetaSkt(Solda.principal.skt);	//Garante um skt valido para a solda principal
							//se o modo for KIR este valor setado é ignorado
	Solda.principal.estado = ATIVO;
	SoldaPrincipal();
	flag = 1;

	return ATIVO;
}

void SoldaContinua()
{
	static Uint16 cont = 0;
	float corrente;

	SetaSkt(Solda.principal.skt);	//Garante um skt valido para a solda principal
									//se o modo for KIR este valor setado é ignorado
	Solda.principal.estado = ATIVO;
	SoldaPrincipal();

	cont++;
	if(cont > 20) {
		cont = 0;
		Solda.principal.cont_semiciclos = 0;
		corrente = (Solda.principal.iRmsMedia[0] / 10);
	//	InicializarParametrosSolda();
		comandos_enviar_corrente(corrente);		//Envia o valor da corrente de solda

	}
	
	if(PinoSoldar == PIN_ATIVO)
		Solda.soldar = 1;
	else
		Solda.soldar = 0;
		

	
}

////#pragma CODE_SECTION(MaquinaEstados, "ramfuncs");
Uint16 MaquinaEstados()
{
	GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;



	if(PinoSoldar != PIN_ATIVO)
		if( (Solda.tipo == COSTURA)  || (Solda.tipo == CONTINUA) )
			return TERMINADO;

	if(Solda.tipo == CONTINUA) {
		SoldaContinua();
		return ATIVO;
	}

	if( SoldaPrePos(&Solda.preSolda) ) 
		return ATIVO;
	if( Pausa(&Solda.pausa1) )
		return ATIVO;
	if( RampaSubida() )
		return ATIVO;
	if( SoldaImpulsos() )
		return ATIVO;
	if( RampaDescida() )
		return ATIVO;
	if( Pausa(&Solda.pausa2) )
		return ATIVO;
	if( SoldaPrePos(&Solda.posSolda) ) 
		return ATIVO;
	if( Pausa(&Solda.pausa3) )
		return ATIVO;
	if( Pausa(&Solda.manutencao) )
		return ATIVO;
	if( Pausa(&Solda.pausa4) )
		return ATIVO;
	
	if(Solda.tipo == COSTURA) {
		Solda.subida.cont_semiciclos = 0;
		RampaSubida();
	}

		
	return TERMINADO;
}


////#pragma CODE_SECTION(InicializarParametrosSolda, "ramfuncs");
void InicializarParametrosSolda()
{
	Uint16 i;

	Rms.FaseA.i = 0;
	Solda.principal.cont_impulsos = 0;
	Solda.principal.cont_semiciclos = 0;
	Solda.principal.estado = 0;
	Solda.pausaPrincipal.cont_semiciclos = 0;
//	ConfigurarRampas();
	if(Solda.tipo != COSTURA)
		Solda.subida.cont_semiciclos = 0;
	Solda.descida.cont_semiciclos = 0;
	Solda.preSolda.cont_semiciclos = 0;
	Solda.posSolda.cont_semiciclos = 0;
	Solda.pausa1.cont_semiciclos = 0;
	Solda.pausa2.cont_semiciclos = 0;
	Solda.pausa3.cont_semiciclos = 0;
	Solda.pausa4.cont_semiciclos = 0;
	Solda.manutencao.cont_semiciclos = 0;

	
	EPwm1Regs.ETCLR.bit.INT = 1;          	// Limpa os pedidos de interrupcao que por ventura estiverem pendentes 
	Solda.soldar = 1;
	
	for(i=0; i<10; i++)
		Solda.principal.iRmsMedia[i] = 0;

	if(Solda.tipo==CONTINUA) {
		Solda.preSolda.total_semiciclos = 0;
		Solda.posSolda.total_semiciclos = 0;
		Solda.subida.total_semiciclos = 0;
		Solda.descida.total_semiciclos = 0;
		Solda.pausa1.total_semiciclos = 0;
		Solda.pausa2.total_semiciclos = 0;
		Solda.pausaPrincipal.total_semiciclos = 1;
		Solda.principal.total_semiciclos = 30;
	}


/*	if(Solda.tipo==COSTURA) {
		Solda.preSolda.total_semiciclos = 0;
		Solda.posSolda.total_semiciclos = 0;
		Solda.pausa1.total_semiciclos = 0;
		Solda.pausa2.total_semiciclos = 0;
	}
*/


}


//float RmsMedio

float CalcularMediaRms()
{
	Uint16 i;
	float rmsMedio = 0;

	/*
	CompensacaoTensao.TensaoRmsMedia = CompensacaoTensao.Acumulador /  CompensacaoTensao.i;
	CompensacaoTensao.Acumulador = 0;
	CompensacaoTensao.i = 0;
	*/

	for(i=0; i<Solda.principal.total_impulsos; i++)
		rmsMedio = rmsMedio + (Solda.principal.iRmsMedia[i] / Solda.principal.total_semiciclos);

	rmsMedio = (float)rmsMedio/Solda.principal.total_impulsos;
	Solda.principal.iMediaImpulsos = rmsMedio;

	return rmsMedio;
}

//#pragma CODE_SECTION(GerenciarSolda, "ramfuncs");
void GerenciarSolda()
{
	float corrente;

	if(FlagSoldar  &&  FlagEmergencia==0  ) {
		FlagSoldar = 0;
		InicializarParametrosSolda();
	}

	if(FlagSoldaConcluida) {
		FlagSoldaConcluida = 0;
		corrente = CalcularMediaRms();	//Calcula o valor RMS médio durante a soldagem
		//GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;		
		//tmp.nFloat = Adc_offset[0];
		comandos_enviar_corrente(corrente);
	//	GpioDataRegs.GPBTOGGLE.bit.GPIO32 = 1;
		Fim_pacote = 1;		
		if(Solda.tipo==COSTURA  ||  Solda.tipo==CONTINUA) {
			if(PinoSoldar == PIN_ATIVO) {
				if(FlagEmergencia == 0)
					FlagSoldar = 1;	
			}
		}
		/////////// Gera os pontos de referencia para o KIR//////////////////////
		//Modo configuracao de kir.  Só funciona se a solda for individual
		if( (Solda.principal.modo == 2) && (Solda.tipo == INDIVIDUAL) )	
		{
			Kir.fDummySkt = Solda.principal.skt;
			Kir.i_rms = Solda.principal.iMediaImpulsos;  
		
			Kir.AdicionarPonto(&Kir);
		}	
		////////////////////Fim geracao dos 2 pontos de ref p/ o KIR/////////////////
		
		///////////////////  Compensacao da Tensao  //////////////////////////////////
		#ifdef COMPENSAR_TENSAO
		/// se estiver operando em modo Kir ou Skt ////
		if(Solda.principal.modo == MODO_SKT  ||  Solda.principal.modo == 1)	
			CompensarTensao();	
		#endif
		//////////////////////////////////////////////////////////////////////////////	

		Gambiarra_MudaKirParaSkt();
	}
}


void CompensarTensao()
{
	float difTensao; 
	float difTensaoAbs; 
	

	difTensao = (CompensacaoTensao.RefTensao - CompensacaoTensao.TensaoRmsMedia) / CompensacaoTensao.RefTensao;
	difTensaoAbs = fabs(difTensao);

	if(difTensaoAbs > 0.02) {
		CompensacaoTensao.fator = 1.0 + difTensao;
	}
	else {
		CompensacaoTensao.fator = 1.0;
	}
}


void SetaSkt(Uint16 sktIn)
{
	Uint16 sktOut;

	#ifdef COMPENSAR_TENSAO
	sktOut = (float)sktIn * CompensacaoTensao.fator;
	if(sktOut > SKT_MAX) sktOut = SKT_MAX - 1;
	//TODO:
	//Avisar que situaçao aconteceu, gerando um erro e parar de soldar
	#else
	sktOut = sktIn;
	#endif

	SktUpdate(sktOut);
}


void Gambiarra_MudaKirParaSkt()
{
	float erro;
	static Uint16 cnt = 0;	

	if(Solda.principal.modo == MODO_KIR) 
	{		
		erro = Kir.i_ref - Solda.principal.iMediaImpulsos;
		erro = erro / Kir.i_ref;
		if(erro < 0.0) erro = 0 - erro;

		cnt++;

		if( (erro < Kir.erro_admissivel)  &&  cnt > 3)
		{
			Solda.principal.modo = MODO_SKT;
			Solda.principal.skt = Kir.skt_out;
			ConfiguraRampasSKT();
			cnt = 0; 
		}		
	}
}
  

#define ATIVAR_EMERGENCIA_THRESHOLD		10
#define DESATIVAR_EMERGENCIA_THRESHOLD	10
void verificaEmergencia()
{
	static Uint16 cntAtivarEmergencia = 0;
	static Uint16 cntDesativarEmergencia = 0;

	if(PinoEmergencia == PIN_ATIVO) {
		if(FlagEmergencia == 0) cntAtivarEmergencia++;	//se flag está zerada, incrementa
		cntDesativarEmergencia = 0;
	}
	else {
		if(FlagEmergencia == 1) cntDesativarEmergencia++;	//se a flag está setada, incremeta
		cntAtivarEmergencia = 0;
	}

	if(cntAtivarEmergencia >= ATIVAR_EMERGENCIA_THRESHOLD) 
		FlagEmergencia = 1;

	if(cntDesativarEmergencia >= DESATIVAR_EMERGENCIA_THRESHOLD)
		FlagEmergencia = 0;
}
