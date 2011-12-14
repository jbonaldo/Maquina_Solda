/*
	J_PI.h

	Controlador Proporcional Integral - PI
	Desenvolvido por Jakson Bonaldo
	Campinas - SP     13/08/2009
*/
#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File

#define PI_CONFIG { PI_Calcular, PI_Inicializar, 0, 0, 0, 0, 0, 0.9, 0, 0, 0, 0}




struct _PI {
	float (*Calcular)(struct _PI *);
	void (*Inicializar)(struct _PI *);
	float ref;
	float Kp;
	float Ki;
	float T_amostragem;
	float Ti;
	float limite;
	float Up;
	float Ui;
	float in;
	Uint16 Habilitar;
};



struct _Pi3Fases {
	void (*Inicializar)(struct _Pi3Fases *);
	struct _PI FaseA;
	struct _PI FaseB;
	struct _PI FaseC;
};








              