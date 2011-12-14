#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File
//#include "Jakson_Bibliotecas/Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário
#include "stdlib.h"

#define FILTRO_TAM		600
#define FILTRO_TAM_DIV2 FILTRO_TAM/2
#define INV_FILTRO_TAM	1/(float)FILTRO_TAM;

#define RMS_CONFIG { RMS_Calcular, RMS_Inicializar, RMS_120Hz, FILTRO_TAM,  0, 0, 0, 0, 0, 0, 0, 0, 0}


struct _RMS
{
	float (*Calcular)(struct _RMS *);
	void (*Inicializar)(struct _RMS *);
	float (*RMS120Hz)(struct _RMS *);
	Uint16 NAmostras;

	Uint16 NAsemi;
	Uint16 i;
	Uint16 j;
	Uint16 aux;
	float in;
	float out;
	float out120Hz;
	float fator_escala;
	float *buffer;
};

struct _Rms3Fases {
	void (*Inicializar)(struct _Rms3Fases *);
	struct _RMS FaseA;
	struct _RMS FaseB;
	struct _RMS FaseC;
	struct _RMS Tensao;
};

