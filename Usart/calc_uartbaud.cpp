#include <stdio.h>
#include <math.h>
int calc_uartdivide(double mhz,double baud){
	float decimal=(mhz*1000000)/(baud*16);
    int Mantissa=(int)decimal;
    int Fraction  =ceil((decimal-Mantissa)*16);
	return ((Mantissa<<4)+Fraction);	
}

main()
{	
printf("%X",calc_uartdivide(84,9600));		

}
