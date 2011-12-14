/*
   PLL Monofásico
   Jakson Bonaldo  	Campinas 12/11/2009
*/

#include <math.h>

#define NUM_AMOSTRAS	1000

float vetor[NUM_AMOSTRAS];
float InvNAmostrasFiltro; 

void InicializaPLL()
{
	unsigned int i;
	
	for(i=0; i<NUM_AMOSTRAS; i++)
		vetor[i] = 0;
	
	InvNAmostrasFiltro = (float) 1/NUM_AMOSTRAS;
}

float PLL(float tensao)
{
	static float w = 0;			//omega					
	static float w_integral = 376.99991;
		   float w_proporcional = 0;
	static float produto_escalar = 0;
	static float produto_escalar_medio = 0;
	static float saida_filtro = 0;
	static float erro = 0;		//erro do produto escalar (produto escalar deve ser zero)
	static float tensao_90_graus = 0;  //sinal de tensao defasado em 90 graus da tensao de entrada
	static float teta = 0;
	const  float Ts = 0.000023;	//Período de amostragem; 43KkHz
	//const  float Ts = 0.0000153333;	//Período de amostragem; 43KkHz
	const  float kp = 35.35;
	const  float ki = 1000;
	static unsigned int i = 0;
	

	//tensao *= 1000;
//	if(tensao >  1) tensao =  1;
//	if(tensao < -1) tensao = -1;
	produto_escalar = tensao * tensao_90_graus;		//Calcula produto escalar da tensao de entrada com a tensao defasade de 90 graus
	
	produto_escalar = (float) produto_escalar * InvNAmostrasFiltro;  //Filtro para pegar o valor medio do produto escalar;
	saida_filtro += produto_escalar - vetor[i];		
	vetor[i] = produto_escalar;

	i++;
	if(i >= NUM_AMOSTRAS) i = 0;

	erro = 0 - saida_filtro;					//Controlador PI para manter a média do produto escalar igual a zero
	w_integral += erro * ki * Ts;
	w_proporcional = erro * kp;
	w = w_integral + w_proporcional;

	teta += w * Ts;								//Integral de teta
	if(teta >= 6.28318531) teta = 0;

	tensao_90_graus = sin(teta + 1.57079633);	//Defasegem de 90 graus (para fazer o produto escalar a cima)
	
	return sin(teta );
}

