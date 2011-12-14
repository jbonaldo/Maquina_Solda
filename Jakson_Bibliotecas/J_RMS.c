//#include "J_RMS.h"  
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário

#include <math.h>

#pragma CODE_SECTION(RMS_Calcular, "ramfuncs");
float RMS_Calcular(struct _RMS *RMS)
{
	float temp;

//	RMS->buffer[RMS->i] = RMS->in;	
//	RMS->aux += RMS->in;

	temp = RMS->in * RMS->in;
	RMS->out += temp;
	RMS->i++;
	
	return (0);
}

void RMS_Inicializar(struct _RMS *rms)
{
/*	Uint16 i;
	
//	rms->invNAmostras = (float) 1/rms->NAmostras;

	rms->buffer = (float *) malloc(FILTRO_TAM*sizeof(float));
	for(i=0; i < rms->NAmostras; i++)
  		rms->buffer[i] = (float) 0;	
*/
}

#pragma CODE_SECTION(RMS_120Hz, "ramfuncs");
float RMS_120Hz(struct _RMS *rms)
{
	rms->out120Hz = (float) rms->out / rms->i;
	rms->i = 0;
	rms->out = 0;

	rms->out120Hz = sqrt(rms->out120Hz);
	rms->out120Hz *= rms->fator_escala; //25 fator de escala para kA
	
	return rms->out120Hz;

}


void RMS_Inicializar_Estrutura(struct _Rms3Fases *rms)
{
	struct _RMS a = RMS_CONFIG;
	struct _RMS b = RMS_CONFIG ;
	struct _RMS c = RMS_CONFIG;
	struct _RMS d = RMS_CONFIG;
	

	rms->FaseA = a;
	rms->FaseB = b;
  	rms->FaseC = c;
	rms->Tensao = d;
}

struct _Rms3Fases Rms = {RMS_Inicializar_Estrutura, 0, 0, 0, 0} ;






 

		


