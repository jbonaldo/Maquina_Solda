/*
	J_PI.c

	Controlador Proporcional Integral - PI
	Desenvolvido por Jakson Bonaldo
	Campinas - SP     13/08/2009
*/

#include "J_PI.h" 

//#pragma CODE_SECTION(PI_Calcular, "ramfuncs");
float PI_Calcular(struct _PI *pi) 
{
	float tmp = 0; 
	float erro;

	if(pi->Habilitar)
	{
		erro = pi->ref - pi->in;
		pi->Up = pi->Kp * erro;
		pi->Ui = pi->Ui + pi->Ki * erro;
		if(pi->Ui > pi->limite)  
			pi->Ui = pi->limite;
		if(pi->Ui < -pi->limite)
			pi->Ui = -pi->limite;
		tmp = pi->Up + pi->Ui;
		if(tmp>pi->limite)  
			tmp = pi->limite;
		if(tmp<-pi->limite) 
			tmp = -pi->limite;
	}
	return (tmp);
}

void PI_Inicializar(struct _PI *pi) 
{
	pi->limite = 888;
	if(! pi->Ki)
		pi->Ki = pi->Ti * pi->T_amostragem / pi->Kp;

	if(! pi->Ti)
		pi->Ti = pi->Ti * pi->Kp / pi->T_amostragem;
}

void PI_Inicializar_Estrutura(struct _Pi3Fases *estrutura)
{
	struct _PI a = PI_CONFIG;
	struct _PI b = PI_CONFIG;
	struct _PI c = PI_CONFIG;
	estrutura->FaseA = a;
	estrutura->FaseB = b;
	estrutura->FaseC = c;
}

struct _Pi3Fases Pi = {PI_Inicializar_Estrutura, 0,0,0};

