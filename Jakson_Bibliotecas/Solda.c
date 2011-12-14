#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File  
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário
#include "Solda.h"

#define ATIVO		0x01
#define TERMINADO	0x00


Uint16 Estado = 0;
struct _solda Solda;

/**
*  Solda realmente ativa, podendo ser realizada
*  em SKT ou KIR
*/
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
	}
	//Fim controle semi-ciclos aplicados

	#ifdef DEBUG
	Namostras[k_ciclos] = Rms.FaseA.i;
	CorrenteSkt[k_ciclos] = Rms.FaseA.RMS120Hz(&Rms.FaseA);	//Calcula o valor RMS da corrente do semiciclo 
	k_ciclos++;
	#endif
		
	Rms.FaseA.RMS120Hz(&Rms.FaseA);	//Calcula o valor RMS da corrente do semiciclo 

	Solda.principal.iRmsMedia[Solda.principal.cont_impulsos] += Rms.FaseA.out120Hz;  //Soma os valores rms de cada ciclo (qdo a solda acabar, esta soma é dividida pelo número de semiciclos)
//	Solda.cont_ciclos++;  //Comentei, pois este campo fazia apenas a pova real do campo solda.cont_atual

	if(Solda.principal.cont_semiciclos == 1) {  //Descarta o primeiro pulso
		Solda.principal.iRmsMedia[Solda.principal.cont_impulsos] = 0;
	//	SciA_SendMsg("\n\rAqui ");
	}

	if(Solda.principal.modo == 1) //0->skt;   1->kir
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
		}
		else
		{
			Kir.i1 = Kir.i2;
			Kir.i2 = Kir.i_rms;
		}
	}
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
void ConfiguraRampas()
{
	float sktPrincipal = 0;
	float sktPre = 0;
	float sktPos = 0;
	

	if( Solda.principal.modo == MODO_KIR) 
		sktPrincipal = CalcularSkt(Solda.principal.iRef);
	else  
		sktPrincipal = Solda.principal.skt;


	if(Solda.preSolda.modo == MODO_KIR)
		sktPre = CalcularSkt(Solda.preSolda.iRef);

	if(Solda.posSolda.modo == MODO_KIR)
		Solda.posSolda.skt = CalcularSkt(Solda.posSolda.iRef);



	if(Solda.subida.total_semiciclos > 0) 
		Solda.subida.deltaSkt = (sktPrincipal-Solda.preSolda.skt) / (float)Solda.subida.total_semiciclos;	
	if(Solda.descida.total_semiciclos > 0)
		Solda.descida.deltaSkt = (sktPrincipal-Solda.posSolda.skt) / (float)Solda.descida.total_semiciclos;		
}



Uint16 Rampa(struct _rampa *r)
/**
*   Varia os valores de Skt para formar a rampa de subida ou descida
*/
{
	if( (r->total_semiciclos > 0) && (r->cont_semiciclos <= r->total_semiciclos) )
	{
		Habilita_Pulsos();
		r->cont_semiciclos++;
		r->skt += r->deltaSkt;
		SetaSkt(r->skt);
		
		return ATIVO;
	}
	else
	{
		Desabilita_Pulsos();
		return TERMINADO;
	}

	return TERMINADO;
}


Uint16 SoldaPrePos(struct _prepossolda *s)
/**
*   Realiza a pré o pós solda (em skt)
*/
{
	if( (s->total_semiciclos > 0) && (s->cont_semiciclos <= s->total_semiciclos) )
	{
		Habilita_Pulsos();
		s->cont_semiciclos += 1;
		SetaSkt(s->skt);
		return ATIVO;
	}
	else
	{
		Desabilita_Pulsos();
		return TERMINADO;
	}

	return TERMINADO;	
}

Uint16 Pausa(struct _pausa *p)
/**
*	Realiza um dos 3 tipos de pausa possíveis
*/
{
	if( (p->total_semiciclos > 0) && (p->cont_semiciclos <= p->total_semiciclos) )
	{
		p->cont_semiciclos += 1;
		return ATIVO;
	}
	else
		return TERMINADO;

	return TERMINADO;
}

Uint16 SoldaImpulsos()
/**
*	Quando este impulso (varios ciclos) de solda terminar Verificar 
*	se é o último Impulso.  
*	Caso seja o último termina a solda
*	Caso contrario, reabilita a soldagem e incremente o contador de impulsos
*/
{
	SetaSkt(Solda.principal.skt);	//Garante um skt valido para a solda principal
									//se o modo for KIR este valor setado é ignorado
	SoldaPrincipal();	
	
	if(Solda.principal.cont_semiciclos > Solda.principal.total_semiciclos) 
	{
		if(Solda.principal.cont_impulsos < Solda.principal.total_impulsos) 
		{
			Solda.principal.cont_impulsos += 1;
			Solda.principal.cont_semiciclos = 0;
			SoldaPrincipal();  //Impede o surgimento de um semiciclo nulo
			return ATIVO;
		}
		else {
			return TERMINADO;
		}
	}

	return ATIVO;
}


Uint16 MaquinaEstados()
{

	if( SoldaPrePos(&Solda.preSolda) ) 
		return ATIVO;
	if( Pausa(&Solda.pausa1) )
		return ATIVO;
	if( Rampa(&Solda.subida) )
		return ATIVO;
	if( SoldaImpulsos() )
		return ATIVO;
	if( Pausa(&Solda.pausaPrincipal) )
		return ATIVO;
	if( Rampa(&Solda.descida) )
		return ATIVO;
	if( Pausa(&Solda.pausa2) )
		return ATIVO;
	if( SoldaPrePos(&Solda.posSolda) ) 
		return ATIVO;
	
	
	return TERMINADO;
}

/*
void IniciaSolda()
{
	Solda.principal.total_impulsos = 1;
	Solda.subida.total_semiciclos = 0;
	Solda.descida.total_semiciclos = 0;
	Solda.preSolda.total_semiciclos = 0;
	Solda.posSolda.total_semiciclos = 0;
	Solda.pausa1.total_semiciclos = 0;
	Solda.pausa2.total_semiciclos = 0;
	Solda.pausaPrincipal.total_semiciclos = 0;
}

*/