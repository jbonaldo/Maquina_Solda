#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "Jakson_Prototipos_Funcoes.h"  //Prototipo das funcoes programadas pelo usuário

#define N_BITS_SPI  14
#define SPI_CLK_HIGH    GpioDataRegs.GPASET.bit.GPIO18
#define SPI_CLK_LOW     GpioDataRegs.GPACLEAR.bit.GPIO18
#define SPI_DATA_IN     GpioDataRegs.GPADAT.bit.GPIO17    
    
typedef struct {
    Uint16 in[32];
    Uint16 n_samples;
    Uint16 flag;
} debug_spi_t;

debug_spi_t debug_spi;     
    
Uint16 debug_spi_aquisicao(void);
    
void debug_spi_init(void)
{
   EALLOW;

    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;   // Enable pull-up on GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;   // Enable pull-up on GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;   // Enable pull-up on GPIO18 (SPICLKA)

    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 (SPICLKA)

    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0; // Configure GPIO16 as SPISIMOA
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 0; // Configure GPIO17 as SPISOMIA
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 0; // Configure GPIO18 as SPICLKA
    
    GpioCtrlRegs.GPADIR.bit.GPIO17 = 0; //OUTPUT    //DATA IN
    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1; //OUTPUT    //CLK
    
    EDIS;
    
    
    debug_spi.n_samples = 20;
    debug_spi.flag = 1;
}



Uint16 debug_spi_aquisicao(void)
{
    Uint16 i;
    Uint16 temp = 0;
    
    CONV_CORRENTE_INICIAR;
    for(i=0; i<N_BITS_SPI; i++)
    {
        SPI_CLK_HIGH;
        
        if( SPI_DATA_IN )
            temp |= (1 << i);
            
        SPI_CLK_LOW;
    } 
    CONV_CORRENTE_TERMINAR;
    return temp;
}


void debug_spi_run(void)
{
    Uint16 i;
    
    while(debug_spi.flag)
    {
        debug_spi.in[i] = debug_spi_aquisicao();
        if(++i > debug_spi.n_samples)
            i = 0;
    }
}