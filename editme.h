#ifndef __EDTIME__DEQ__
#define __EDTIME__DEQ__



/*
───────────────────────────────────────────────────────────────────────────────────────────────
─██████████████─████████████───██████████─██████████████─██████──────────██████─██████████████─
─██░░░░░░░░░░██─██░░░░░░░░████─██░░░░░░██─██░░░░░░░░░░██─██░░██████████████░░██─██░░░░░░░░░░██─
─██░░██████████─██░░████░░░░██─████░░████─██████░░██████─██░░░░░░░░░░░░░░░░░░██─██░░██████████─
─██░░██─────────██░░██──██░░██───██░░██───────██░░██─────██░░██████░░██████░░██─██░░██─────────
─██░░██████████─██░░██──██░░██───██░░██───────██░░██─────██░░██──██░░██──██░░██─██░░██████████─
─██░░░░░░░░░░██─██░░██──██░░██───██░░██───────██░░██─────██░░██──██░░██──██░░██─██░░░░░░░░░░██─
─██░░██████████─██░░██──██░░██───██░░██───────██░░██─────██░░██──██████──██░░██─██░░██████████─
─██░░██─────────██░░██──██░░██───██░░██───────██░░██─────██░░██──────────██░░██─██░░██─────────
─██░░██████████─██░░████░░░░██─████░░████─────██░░██─────██░░██──────────██░░██─██░░██████████─
─██░░░░░░░░░░██─██░░░░░░░░████─██░░░░░░██─────██░░██─────██░░██──────────██░░██─██░░░░░░░░░░██─
─██████████████─████████████───██████████─────██████─────██████──────────██████─██████████████─
───────────────────────────────────────────────────────────────────────────────────────────────
________________________________________DEQ2000-cyber__________________________________________
VIERNES : 17 : JUNIO : 2022
*/


#ifdef __cplusplus
extern "C" {
#endif

/* PREPARAR EDITME. */
#define EDI_SC_NORMAL      2
#define EDI_SC_FRAMEBUFFER 4
#define EDI_SC_DEPTH       8

/* MODO DE VACIAR. */
#define EDI_COLOR 2
/* PARA CREAR EL ID DE TEXTURA. */
#define EDI_ID(X) int X=0

/* EFECTOS. */
#define EDITME_GFX_1 "escala gris"
#define EDITME_GFX_2 "escala rojo"
#define EDITME_GFX_3 "escala verde"
#define EDITME_GFX_4 "escala azul"
#define EDITME_GFX_5 "escala invertir"
#define EDITME_GFX_6 "escala posicion"

void EDI_PREPARAR( int MODO ); /* USA EL PREPARADO. */
void EDI_CREARBUFFR( int *ID, const int ANCHO, const int ALTO );
void EDI_CARGAR( int *ID, const char *NOMBRE );
void EDI_GUARDARTEXTURA( int *ID, const char NOMBRE[] );
int  EDI_ANCHO( int *ID );
int  EDI_ALTO( int *ID );
void EDI_P_ANCHO( int *ID, int *X );
void EDI_P_ALTO( int *ID, int *X );
void EDI_COLOR_RGB( int R, int G, int B );
void EDI_COLOR_RGBA( int R, int G, int B, int A );
void EDI_COLOR_RGB_INT( unsigned int X );
void EDI_COLOR_RGBA_INT( unsigned int X );
void EDI_VACIAR( int *ID, int MODO ); /* MODO DE VACIOS. */
void EDI_PIXEL( int *ID, int X, int Y );
unsigned int EDI_GETPIXEL( int *ID, int X, int Y );

void EDI_GFX( int *ID, const char EFECTOS[] ); /* USA LOS EFECTOS. */



#ifdef __cplusplus
	}
#endif
#endif