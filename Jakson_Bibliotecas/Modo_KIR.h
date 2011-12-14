/* Controle de corrente - KIR
	Desenvolvido por Jakson Bonaldo
	23/10/09 - Campinas - SP
									*/

// Equação da reta:  y = a*x + b

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File 


struct _kir {
	void (*Calcular)(struct _kir *);
	void (*EscolherPontos)(struct _kir *);
	void (*AdicionarPonto)(struct _kir *);
	void (*InicializarVariaveis)(struct _kir *);

	float skt1;
	float skt2;
	float skt_out;
	float i1;
	float i2;
	float coef_a;
	float coef_b;
	float i_ref;
	float i_rms;
	float erro_admissivel;  //Dado em kA

	

	float padrao_skt1;
	float padrao_skt2;
	float padrao_i1;
	float padrao_i2;
	float fDummySkt;
	Uint16 indice_pontos;
	Uint16 N;
	Uint16 indice1;
	Uint16 indice2;
	float vet_skt[10];
	float vet_kA[10];	
};


