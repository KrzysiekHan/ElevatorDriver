//#################################################################################################################
///                                           	/* BIBLIOTEKI */
//#################################################################################################################

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

//#################################################################################################################
///                                           	/* MAKRODEFINICJE */
//#################################################################################################################
/* definicje pinów dla wejœæ przycisków */
#define pinp1 (1<<PD0)
#define pinp2 (1<<PD1)
#define pinp3 (1<<PD2)
#define pinp4 (1<<PD3)

/* makra przycisk wciœniêty */
#define	P1	!(PIND&pinp1)
#define	P2	!(PIND&pinp2)
#define	P3	!(PIND&pinp3)
#define	P4	!(PIND&pinp4)

/* definicje pinów dla wejœæ wy³¹czników krañcowych */
#define pinwk1 (1<<PD4)
#define pinwk2 (1<<PD5)
#define pinwk3 (1<<PD6)
#define pinwk4 (1<<PD7)

/* makra krañcowy przesterowany */
#define	WK1	!(PIND&pinwk1)
#define	WK2	!(PIND&pinwk2)
#define	WK3	!(PIND&pinwk3)
#define	WK4	!(PIND&pinwk4)

/* definicje pinów dla wyjsc */
#define dir    		(1<<PC1)
#define dirdol		PORTC &=~ dir
#define dirgora		PORTC |= dir

#define enable 		(1<<PC2)
#define enable_OFF  PORTC &=~ enable
#define enable_ON	PORTC |= enable

#define clk			(1<<PC3)

/* makra do sterowania silnika*/
#define stop    0
#define kgora 	1
#define kdol  	2
#define ON 		1
#define OFF 	0

//#################################################################################################################
///                                           	/* DEKLARACJE GLOBALNE */
//#################################################################################################################

///F U N K C J E
void silnik (uint8_t strona);
void generator (uint8_t stan);
void obsluga_przyciskow (void);
void obsluga_krancowych (void);
void stany_maksymalne (void);
void logika_windy (void);
void wysw_lcd (void);

volatile uint8_t status_generatora;								// zmienna wlaczajaca clk (volatile dla przerwania)
uint8_t blokp1 , blokp2 , blokp3 , blokp4;  					// zmienne filtrujace dzialanie przyciskow
uint8_t blokw1 , blokw2 , blokw3 , blokw4 , blokw; 			 	// zmienne filtrujace dzialanie wylacznikow krancowych
uint8_t zadanie1 , zadanie2 , zadanie3 , zadanie4; 				// pamiec poszczegolnych wezwan
uint8_t pamwk1 , pamwk2 , pamwk3 , pamwk4; 						// pamiec zadzialania wylacznikow krancowych
uint8_t zabezpieczenie_dol , zabezpieczenie_gora;				// zmienne blokujace prace silnika w skrajnych polozeniach
uint8_t pozycja_windy , cel_windy , ilosc_zadan , cel_posredni;	// zmienne pomocnicze obslugujace logike windy
uint8_t tempkier;												// zmienna pomocnicza wyboru kierunku


int main(void)
{
	/*Inicjalizacja wejœæ/wyjœæ */
	DDRC |= dir | enable | clk;														// kierunek pinow wyjsciowych
	DDRD &=~ (pinp1 | pinp2 | pinp3 | pinp4 | pinwk1 | pinwk2 | pinwk3 | pinwk4);	// kierunek pinow wejsciowych
	PORTD |= pinp1 | pinp2 | pinp3 | pinp4 | pinwk1 | pinwk2 | pinwk3 | pinwk4 ;    // podciagniecie pinow do VCC

//TIMER2
 //Przerwanie dla generatora silnika
TCCR2 	= (1<<WGM21);						// tryb pracy CTC
TCCR2 	= (1<<CS20) | (1<<CS21);			// preskaler
OCR2 	= 255;								// wartosc poczatkowa OCR
TIMSK 	= (1<<OCIE2);						// odblokowanie przerwania CompareMatch
sei();               					    // globalne za³¹czenie przerwañ


//#################################################################################################################
///                                           	/* INICJALIZACJA ZMIENNYCH*/
//#################################################################################################################
//wylacznie generatora
status_generatora = 0;
//inicjowanie zmiennych wezwan (winda domyslnie zjezdza na poziom 1)
zadanie1 = 1;
zadanie2 = 0;
zadanie3 = 0;
zadanie4 = 0;
//zerowanie pamieci zadzialania wk
pamwk1 = 0;
pamwk2 = 0;
pamwk3 = 0;
pamwk4 = 0;
enable_OFF;



//#################################################################################################################
///                                           	/* PÊTLA NIESKOÑCZONA WHILE(1) */
//#################################################################################################################
	while(1)
	{
		obsluga_przyciskow();
		obsluga_krancowych();
		stany_maksymalne();
		logika_windy();
	}//end_while(1)
}//end_main(void)

//#################################################################################################################
///                                           	/* DEFINICJE FUNKCJI */
//#################################################################################################################


//generowanie przebiegu prostokatnego 500Hz dla wyjscia clk
void generator (uint8_t stan)
{
	if (stan == 1)
	{
			status_generatora = 1;
	} else
	{
		status_generatora = 0;
	}
}

//funkcja sterujaca silnikiem windy w zaleznosci od przekazanego statusu
void silnik (uint8_t status)
{
	//-----stop---------
	if (status==0)
	{
		generator(OFF);
		enable_ON;
		tempkier = 0;
	}

	//-----gora---------
	if (status==1 && !zabezpieczenie_gora)
	{
		generator(ON);	//sygnal prostokatny clk
		enable_ON;		//zalaczenie sterowania ena
		dirgora; 		//kierunek silnika dir
		tempkier = 1;
	}

	//-------dol---------
	if (status==2 && !zabezpieczenie_dol)
	{
		generator(ON);	//sygnal prostokatny clk
		enable_ON;		//zalaczenie sterowania ena
		dirdol; 		//kierunek silnika dir
		tempkier = 2;
	}
}

//obsluga przyciskow z programowa eliminacja drgan stykow
void obsluga_przyciskow (void)
{
	//obsluga przycisku P1 (PD0)
	if( !blokp1 && P1 ) {
	   blokp1=1;
	   zadanie1 = 1;
	} else if( blokp1 && !P1 ) blokp1++;

	//obsluga przycisku P2 (PD1)
	if( !blokp2 && P2 ) {
		blokp2=1;
		zadanie2 = 1;
	} else if( blokp2 && !P2 ) blokp2++;

	//obsluga przycisku P3 (PD2)
	if( !blokp3 && P3 ) {
		blokp3=1;
		zadanie3 = 1;
	} else if( blokp3 && !P3 ) blokp3++;

	//obsluga przycisku P4 (PD3)
	if( !blokp4 && P4 ) {
		blokp4=1;
		zadanie4 = 1;
	} else if( blokp4 && !P4 ) blokp4++;

}

//obsluga wylacznikow krancowych z programowa eliminacja drgan stykow
void obsluga_krancowych (void)
{
	//obsluga WK1(PD4)
	if( !blokw1 && WK1 ) {
	   blokw1=1;
	   pamwk1 = 1;
	   pamwk2 = 0;
	   pamwk3 = 0;
	   pamwk4 = 0;
	} else if( blokw1 && !WK1 ) blokw1++;

	//obsluga WK2(PD5)
	if( !blokw2 && WK2 ) {
		blokw2=1;
	   pamwk1 = 0;
	   pamwk2 = 1;
	   pamwk3 = 0;
	   pamwk4 = 0;
	} else if( blokw2 && !WK2 ) blokw2++;

	//obsluga WK3(PD6)
	if( !blokw3 && WK3 ) {
		blokw3=1;
	   pamwk1 = 0;
	   pamwk2 = 0;
	   pamwk3 = 1;
	   pamwk4 = 0;
	} else if( blokw3 && !WK3 ) blokw3++;

	//obsluga WK4(PD7)
	if( !blokw4 && WK4 ) {
		blokw4=1;
	   pamwk1 = 0;
	   pamwk2 = 0;
	   pamwk3 = 0;
	   pamwk4 = 1;
	} else if( blokw4 && !WK4 ) blokw4++;

}

void stany_maksymalne (void)
{
	if (pamwk1) {
	zabezpieczenie_dol = 1;
	} else {
	zabezpieczenie_dol = 0;
	}

	if (pamwk4) {
	zabezpieczenie_gora = 1;
	} else {
	zabezpieczenie_gora = 0;
	}
}

void logika_windy (void)
{
	//przygotowanie zmiennych pomocniczych

	//aktualna pozycja windy
	if (pamwk1) pozycja_windy=1;
	if (pamwk2) pozycja_windy=2;
	if (pamwk3) pozycja_windy=3;
	if (pamwk4) pozycja_windy=4;

	//sprawdzenie obecnosci wezwan posrednich
	ilosc_zadan = zadanie1 + zadanie2 + zadanie3 + zadanie4;

//------------------ILOSC ZADAN <= 1 --------------------------
if (ilosc_zadan <= 1)
{
	//zmienna wezwania konca trasy
	if (zadanie1) cel_windy=1;
	if (zadanie2) cel_windy=2;
	if (zadanie3) cel_windy=3;
	if (zadanie4) cel_windy=4;

	if (cel_windy > pozycja_windy)
	{
		silnik(kgora);
	}

	if (cel_windy < pozycja_windy)
	{
		silnik(kdol);
	}

	//zatrzymanie windy po osiagnieciu celu
	if (cel_windy == pozycja_windy)
	{
		silnik(stop); //zerowanie zmiennych zadan po osiagnieciu celu
		zadanie1 = 0;
		zadanie2 = 0;
		zadanie3 = 0;
		zadanie4 = 0;
	}

}

//------------------ILOSC ZADAN >= 2 --------------------------
if (ilosc_zadan >= 2)
{
	//zapamietanie jednego celu posredniego
	if (ilosc_zadan == 2)
	{
		//zmienna wezwania konca trasy
		if (zadanie1 && (cel_windy != 1)) cel_posredni=1;
		if (zadanie2 && (cel_windy != 2)) cel_posredni=2;
		if (zadanie3 && (cel_windy != 3)) cel_posredni=3;
		if (zadanie4 && (cel_windy != 4)) cel_posredni=4;
	}

	//zatrzymywanie windy na pietrze posrednim 1
	if(  (cel_posredni==1) && WK1 )
	{
			silnik(stop);
			cel_posredni=0;
			_delay_ms(3000);
			zadanie1 = 0;
	}
	//zatrzymywanie windy na pietrze posrednim 2
	if( (cel_posredni==2) && WK2 )
	{

			silnik(stop);
			cel_posredni=0;
			_delay_ms(3000);
			zadanie2 = 0;
	}
	//zatrzymywanie windy na pietrze posrednim 3
	if( (cel_posredni==3) && WK3)
	{
			silnik(stop);
			cel_posredni=0;
			_delay_ms(3000);
			zadanie3 = 0;
	}
	//zatrzymywanie windy na pietrze posrednim 4
	if( (cel_posredni==4) && WK4 )
	{
			silnik(stop);
			cel_posredni=0;
			_delay_ms(3000);
			zadanie4 = 0;
	}
}


//zatrzymanie windy po osiagnieciu celu
if (cel_windy == pozycja_windy)
{
	silnik(stop); //zerowanie zmiennych zadan po osiagnieciu celu
	zadanie1 = 0;
	zadanie2 = 0;
	zadanie3 = 0;
	zadanie4 = 0;
	cel_posredni = 0;
}

}//koniec funkcji

//#################################################################################################################
///                                           	/* PROCEDURA OBS£UGI PRZERWANIA */
//#################################################################################################################
// procedura obs³ugi przerwania dla timerów systemowych
ISR(TIMER2_COMP_vect)
{

	//generator dla pinu clk
	if (status_generatora == 1)
		{
		PORTC ^= clk;
		}

}




