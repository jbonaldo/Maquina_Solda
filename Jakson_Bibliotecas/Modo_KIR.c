/* Controle de corrente - KIR
	Desenvolvido por Jakson Bonaldo
	23/10/09 - Campinas - SP
									*/

// Equação da reta:  y = a*x + b
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário

#include "Solda.h" 
#include <float.h>

float Saida = 0;


#pragma CODE_SECTION(Calcular, "ramfuncs");
Calcular(struct _kir *kir)
{
	float a1, a2, a3;
	float b1, b2, b3;
	float temp, erro, saida;

	//kir->i2 = kir->i_rms;   //Corrente anterior, gerada apartirdo skt anterior

	if( ( Solda.principal.cont_semiciclos % 2 ) == 1 ) //Se o ciclo for ímpar entra no kir (que deve ser tratado a cada ciclo e nao semicilco)
	{
		erro = (float)(kir->i_ref - Kir.i_rms) / (float) kir->i_ref;
		if(erro < 0.0) erro = 0.0 - erro;
		//if(erro > kir->erro_admissivel  ||  -erro > kir->erro_admissivel)
		if(erro > kir->erro_admissivel)
		{
			if(Solda.principal.cont_semiciclos==1)
			{
				kir->skt1 = kir->padrao_skt1;
				kir->skt2 = kir->padrao_skt2;
				kir->i1 = kir->padrao_i1;
				kir->i2 = kir->padrao_i2;
			}
			else {
				kir->i1 = kir->i2;
				kir->i2 = kir->i_rms;   //Corrente anterior, gerada apartir do skt anterior

				kir->padrao_i1 = kir->i1;
				kir->padrao_i2 = kir->i2;
				kir->padrao_skt1 = kir->skt1;
				kir->padrao_skt2 = kir->skt2;
			}
			
			if(Solda.principal.cont_semiciclos < Solda.principal.total_semiciclos)  //Se o pacote estiver acabando, nao calcula um novo valor de skt
			{																		  //O proximo pacote terá o valor de skt o pacote anterior
				temp = kir->skt1 + kir->skt2;
				a1 = 2 * ( (kir->skt1 * kir->i1) + (kir->skt2 * kir->i2) );
				a2 = temp * (kir->i1 + kir->i2);
				a3 = 2 * ( (kir->skt1 * kir->skt1) + (kir->skt2 * kir->skt2) ) -  temp * temp;
				kir->coef_a = (a1 - a2) / a3;
				if(kir->coef_a > 0.0001)
				{
					kir->coef_b = 0.5 * ( (kir->i1 + kir->i2) - kir->coef_a * temp );
		
					saida = (kir->i_ref - kir->coef_b) / kir->coef_a;
					if(saida < 0) saida = 0;
					if(saida > 950) saida = 950;

					//Rotina que impede que apareçam valores discrepantes de corrente dentro do mesmo pacote
			  		if(saida > (Uint16)(1.1*kir->skt2) ) 
						saida = 1.1*kir->skt2;
					if(saida < (Uint16)(0.9*kir->skt2) )
						saida = 0.9*kir->skt2;
					//fim rotina q impede ...

					kir->skt1 = kir->skt2;
					Saida = saida;
					kir->skt2 = (float) saida;
			
					//kir->i1 = kir->i2;
					kir->skt_out = saida;

					
			
				}
			}
			
			
		}

		//vetores de debug
		//Pega o SKT e a corrente antigos
	/*	Skt[k] = kir->skt1;
		Corrente[k] = kir->i_rms;
		k++;
	*/	//fim vetores debug

		Kir.i_rms = 0;	
	}
			
}


void OrdenaPontos(float *P, float *V, Uint16 N)
//P -> vetor das potências
//V -> vetor dos índices das potências
{
	unsigned int i, j;
	float temp = 0;

	for(j=0; j<N; j++) {
		for(i=0; i<(N-1); i++) {
			if( P[i] > P[i+1]) {
				temp = P[i];
				P[i] = P[i+1];
				P[i+1] = temp;

				temp = V[i];
				V[i] = V[i+1];
				V[i+1] = temp;
			}
		}
	}
} 


void EscolherPontos(struct _kir *kir)
{
	Uint16 j;
	Uint16 indice;

	OrdenaPontos(kir->vet_skt, kir->vet_kA, kir->N);

	for(j=0; j<(kir->N-1); j++) 
		if(kir->i_ref > kir->vet_kA[j]  &&  kir->i_ref < kir->vet_kA[j+1])
		{
			indice = j;
			kir->padrao_skt1 = kir->vet_skt[j];
			kir->padrao_i1 = kir->vet_kA[j];

			
			while( ++j<(kir->N) )
				if( (kir->vet_skt[j]-kir->vet_skt[indice]) > 40 ) {
					j++; 
					break;
				}
				
			kir->padrao_skt2 = kir->vet_skt[j-1];
			kir->padrao_i2 = kir->vet_kA[j-1];
			break;	
		}
		else
			if(kir->i_ref > kir->vet_kA[Kir.N-1] )
			{
				kir->padrao_skt1 = kir->vet_skt[Kir.N-2];
				kir->padrao_i1 = kir->vet_kA[Kir.N-2];
				kir->padrao_skt2 = kir->vet_skt[Kir.N-1];
				kir->padrao_i2 = kir->vet_kA[Kir.N-1];
			}


} 

#pragma CODE_SECTION(AdicionarPonto, "ramfuncs");
void AdicionarPonto(struct _kir *kir)
{
	Uint16 i;
	Uint16 flag = 0;

	for(i=0;i<kir->N; i++) 
		if(kir->vet_skt[i] == kir->fDummySkt)
			flag = 1;
	
	if(flag == 0)
	{
		kir->indice_pontos += 1;
		if(kir->indice_pontos >= kir->N)
			kir->indice_pontos = 1;	

		kir->vet_skt[kir->indice_pontos] = kir->fDummySkt;
		kir->vet_kA[kir->indice_pontos] = kir->i_rms;
	}

}

void InicializarVariaveis(struct _kir *kir)
{
	Uint16 i;
	
	Kir.erro_admissivel = 0.03;	//Erro admitido (em kA)
	kir->N = 6;			//No. de pontos guardados na tabela de inicializacao
	kir->indice_pontos = 0;

	for(i=0;i<kir->N; i++) {
		kir->vet_skt[i] = 0;
		kir->vet_kA[i] = 0;
	}
}


struct _kir Kir = {Calcular, EscolherPontos, AdicionarPonto, InicializarVariaveis, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

