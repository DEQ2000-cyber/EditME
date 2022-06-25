
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
*/

#include "editme.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Primero preparamos la textura la cual vamos a crear, editar y guardar. */

/* CLIPPING RECORTE. */
typedef struct{
	int X0; // <---,
	int Y0; // <------ PRIMER PUNTO.
	int X1; // <---,
	int Y1; // <------ SEGUNDO PUNTO.
} *P_CLIPPING, CLIPPING;

CLIPPING EDITME_recortar( const int X0, const int Y0, const int X1, const int Y1 ){
	static CLIPPING T;
	T.X0 = X0;
	T.Y0 = Y0;
	T.X1 = X1;
	T.Y1 = Y1;
	return T;
}

P_CLIPPING EDITME_Precortar( const int X0, const int Y0, const int X1, const int Y1 ){
	static CLIPPING T;
	T.X0 = X0;
	T.Y0 = Y0;
	T.X1 = X1;
	T.Y1 = Y1;
	return &T;
}

/* TAMAÑO. */
typedef struct{
	int ANCHO; // ANCHO.
	int ALTO;  // ALTO.
} *P_EDITME_ESPACIO, EDITME_ESPACIO;

/* DATO DE CANAL DE PIXELES. */
typedef struct{
	unsigned char B:8;  // CANAL AZUL.
	unsigned char G:8;  // CANAL VERDE.
	unsigned char R:8;  // CANAL ROJO.
	unsigned char NC:8; // NO CONECCION.
} *P_EDITME_RGBX, EDITME_RGBX;

#define COLOR_NULO   (EDITME_RGBX){0,0,0,0}
#define COLOR_NEGRO  (EDITME_RGBX){0,0,0,0xFF}
#define COLOR_ROJO   (EDITME_RGBX){0,0,0xFF,0xFF}
#define COLOR_VERDE  (EDITME_RGBX){0,0xFF,0,0xFF}
#define COLOR_AZUL   (EDITME_RGBX){0xFF,0,0,0xFF}
#define COLOR_BLANCO (EDITME_RGBX){0xFF,0xFF,0xFF,0xFF}

typedef union{
	unsigned int RGBA;
	struct{
		unsigned char B:8;
		unsigned char G:8;
		unsigned char R:8;
		unsigned char A:8;
	};
} *P_EDITME_RGBX_CANAL, EDITME_RGBX_CANAL;

/* ALMACENAMIENTO TEXTURA. */
typedef struct{
	unsigned int *IDENTIDAD; // IDENTIDaD.
	char ACTIVA;             // ACTIVO.
	P_CLIPPING CLIP;         // RECORTADO.
	P_EDITME_RGBX PIXELS;    // PIXELES.
	EDITME_ESPACIO WH;       // ESPACIO.
	unsigned int TAM;        // TAMAÑO ENTRE (ANCHO * ALTO * [CANALES]).
} *P_EDITME_textura, EDITME_textura;



//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////




/* Crear un FBO - Frame Buffer Object. */

/* FBO. VECTOR[]... */
typedef struct{
	P_EDITME_textura TEXTURA;  // VECTOR TEXTURA.
	unsigned int TAM;          // TAMAÑO FBO.
	unsigned int PASO;         // ULTIMO PASO.
	unsigned int PASO_ANTIGUO; // EL ANTIGUO PASO.
} *P_EDITME_FBO, EDITME_FBO;


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/* UTILIDADES */
#include <stdlib.h>
#include <string.h>

/* MODO TEXTURA. */
enum EDITME_MODOS_ACTIVO
{
	APAGADO   = 2,
	ENCENDIDO = 4
};

/* MODO CREACIÓN TEXTURA. */
enum EDITME_MODOS_CREACION
{
	RELLENO_SOLIDO = 2,
	RELLENO_IMAGEN = 4
};

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

/* CARGAR TEXTURA. */
unsigned char* __fastcall LoadTGA( const char *filename, unsigned int *ancho, unsigned int *alto, unsigned char *bpp ){

	FILE *f;
	/* CARGAMOS EL HEADER */
	unsigned char *header;
	unsigned int size;

	/* CREAMOS EL DATA */
	unsigned char *data = NULL;

	/* INICIADO */
	*ancho = 0;
	*alto = 0;
	*bpp = 0;

	header = (unsigned char*)malloc( 18 );

	/* ABRIMO ARCHIVO */
	f = fopen( filename, "rb" );

	if(!f) printf("ERROR LECTURA: %s\n", filename );


	/* LEEMOS EL HEADER */
	fread( header, 18, 1, f );

	/* CARGAMO ANCHO */
	*ancho = (header[12]&0xFF) | (header[13]<<8);

	/* CARGAMO ALTO */
	*alto  = (header[14]&0xFF) | (header[15]<<8);

	/* CARGAMO BYTE POR PIXEL */
	*bpp = header[16];

	/* LIBERAMOS EL HEADER */
	free(header),header=NULL;

	/* CARGAMO VALOR COMPLETO DEL BUFFER */
	size = ( (*ancho) * (*alto) * ((*bpp)/8) );

	/* CARGAMO EL BUFFER */
	data = (unsigned char*)malloc( size * sizeof(unsigned char) );

	fread( data, 1, size, f );

	/*CERRAMO ARCHIVO */
	size = 0;
	fclose(f);

	return data;
}


unsigned char* __fastcall LoadBMP( const char* filename, unsigned int *ancho, unsigned int *alto, unsigned char *bpp ){

	/* Abrimos el archivo para leer */
	FILE *f = fopen( filename, "rb" );

	/*bytes del Header son 54 */
	unsigned char header[54];

	/*Leemo asta 54 del header*/ 
	fread( header, sizeof(unsigned char), 54, f );

	/*Leemos el ancho y alto*/
	/*y se lo pasamos a la variable*/ 
	const unsigned int ANCHO = *(int*)&header[18];
	const unsigned int ALTO  = *(int*)&header[22];

	/*Leemos el byte por pixel */
	const unsigned char BPP = *(int*)&header[28];

	/*Bpp nos dara 24 por eso dividimos entre 8*/
	/*para que nos de 3*/
	/*Creamos el tamaño esacto*/
	const unsigned int size = ( ANCHO * ALTO * ((BPP)/8) );

	/*Creamos el data a nulo*/
	unsigned char *data = NULL;

	/*Creamos el tamaño del data*/
	/* *3 por el tamaño de los 3 colores RGB*/
	data = (unsigned char*)malloc( size * (BPP/8) );

	/*Calculamos el padding. es una reserva de bytes basuras.*/
	const unsigned char padding = ( ( 4 - ( ANCHO * (BPP/8) ) % 4) % 4 );

	/*Nos volvemos al ultimo del header*/
	fseek( f, 54, SEEK_SET );

	/*Creamos el contenedor de los bytes basuras*/
	unsigned char BYTES_BAZURAS[3];

	/*Leemos la imagen de forma de matriz por sus tamaños*/
	int Y; /*alto de la imagen al pasar*/
	unsigned int X; /* ancho de la imagen al pasar*/

	/*pasamos por los bytes de color*/
	for ( Y = ALTO-1; Y >=0; --Y)
	{
		for ( X = 0; X < ANCHO; ++X)
		{
			/*Creamos el step para posicionar al data*/
			/*correctamente*/
			unsigned int STEP = ( ( Y * ANCHO + X ) * (BPP/8) );

			/*Leemos y cargamos los bytes de color al data*/
			fread( &data[ STEP ], (BPP/8), 1, f );
		}
		/*Leemos los bytes bazuras*/
		fread( BYTES_BAZURAS, padding, 1, f );
	}

	/*Cerramos el archivo */
	fclose(f),f=NULL;

	*ancho = ANCHO;
	*alto = ALTO;
	*bpp = BPP;

	return data;
}


void* __fastcall LoadTGAConverter( const char *filename, int *w, int *h, int *bpp ){

	/* Creamos el contenedor del data original */
	unsigned int *data=NULL;

	/* Creamos el contenedor del data original puntero arimetico */
	register unsigned int *pixels;

	/* Creamos los contenedores de ancho, alto, iterador del for y byte por pixel */
	unsigned int x,y,i;
	unsigned char n;

	/* Creamos el contenedor del data copia */
	unsigned char *temp = LoadTGA( filename, &x, &y, &n );

	/* Guardamos los datos correspondientes para el original */
	*w=x,*h=y,*bpp=(n/8);

	/* Cargamos el tamaño al data original */
	data=(unsigned int*)malloc( x*y*sizeof(unsigned int));

	pixels=data;

	/* Si BytePerPixel son iguales copiar todo de una. */
	if(*bpp==4){
		memcpy(pixels,temp,(x*y*sizeof(unsigned int)));
		*bpp=4;
		free( temp );
		return data;
	}

	/* Creamos la iteracion para guardar los datos/pixels */
	for (i=0;i<(x*y*(n/8));i+=(n/8))
		*(pixels++)=(temp[i]<<(0))|(temp[i+1]<<(8))|(temp[i+2]<<16)|(255<<24);
	*bpp=4;

	/* Liberamos la imagen copia de la memoria */
	free( temp );

	return data;
}

void* __fastcall LoadBMPConverter( const char *filename, int *w, int *h, int *bpp ){

	/*Creamos el contenedor del data original*/
	unsigned int *data=NULL;

	/*Creamos los contenedores de ancho, alto, iterador del for y byte por pixel*/
	unsigned int x,y,i;
	unsigned char n;

	/*Creamos el contenedor del data copia*/
	unsigned char *temp = LoadBMP( filename, &x, &y, &n );

	/*Guardamos los datos correspondientes para el original*/
	*w=x,*h=y,*bpp=(n/8);

	/*Cargamos el tamaño al data original*/
	data=(unsigned int*)malloc( x*y*sizeof(unsigned int));

	/*Creamos el contenedor del data original puntero arimetico*/
	register unsigned int *pixels=data;

	/* Si BytePerPixel son iguales copiar todo de una. */
	if(*bpp==4){
		memcpy(pixels,temp,(x*y*sizeof(unsigned int)));
		*bpp=4;
		free( temp );
		return data;
	}

	/*Creamos la iteracion para guardar los datos/pixels*/
	for (i=0;i<(x*y*(n/8));i+=(n/8))
		*(pixels++)=(temp[i]<<(0))|(temp[i+1]<<(8))|(temp[i+2]<<16)|(255<<24);
	*bpp=4;

	/*Liberamos la imagen copia de la memoria*/
	free( temp );

	return data;
}

#include <stdbool.h>

/* Funcion para agregar al vector FBO. */

void EDITME_FBO_AGREGAR( P_EDITME_FBO X, unsigned int MODO_CREACION, const char *NOMBRE, const int *ANCHO, const int *ALTO, int *ID ){

	static bool Primera = false;
	if (Primera==false)
	{
		(*X).PASO = 0;
		(*X).PASO_ANTIGUO = 0;
		Primera=true;
	}

	////////////////////////////////////////

	// MARCAMOS NULO TODO.
	if((*X).TEXTURA[ (*X).PASO ].PIXELS){
		free((*X).TEXTURA[ (*X).PASO ].PIXELS);
	}
	(*X).TEXTURA[ (*X).PASO ].IDENTIDAD = NULL;
	(*X).TEXTURA[ (*X).PASO ].ACTIVA    = APAGADO;
	(*X).TEXTURA[ (*X).PASO ].CLIP      = NULL;
	(*X).TEXTURA[ (*X).PASO ].PIXELS    = NULL;
	(*X).TEXTURA[ (*X).PASO ].WH.ANCHO  = 0;
	(*X).TEXTURA[ (*X).PASO ].WH.ALTO   = 0;
	(*X).TEXTURA[ (*X).PASO ].TAM       = 0;
	////////////////////////////////////////

	/* RELLENADO POR SOLIDO. */
	if ( (MODO_CREACION == RELLENO_SOLIDO) || (NOMBRE == NULL) )
	{
		/* CONFIGURAMO TEXTURA. */
		(*X).TEXTURA[ (*X).PASO ].IDENTIDAD = &(*X).PASO; /* IDENTIDAD. */
		(*X).TEXTURA[ (*X).PASO ].ACTIVA    = 1;          /* ACTIVA. */
		(*X).TEXTURA[ (*X).PASO ].CLIP      = EDITME_Precortar(0,0,*ANCHO,*ALTO);                                /* RECORTE. */
		/* CREAMOS LOS PIXELES. */
		(*X).TEXTURA[ (*X).PASO ].PIXELS    = (P_EDITME_RGBX)malloc( (*ANCHO) * (*ALTO) * sizeof(EDITME_RGBX) ); /* DATO PIXELES. */
		(*X).TEXTURA[ (*X).PASO ].WH.ANCHO  = *ANCHO;                                                            /* ANCHO. */
		(*X).TEXTURA[ (*X).PASO ].WH.ALTO   = *ALTO;                                                             /* ALTO. */
		(*X).TEXTURA[ (*X).PASO ].TAM       = ( (*ALTO) * (*ANCHO) );                                            /* TAMAÑO. */
		/* CREAMOS UNA TEXTURA SOLIDA. */
		for (register unsigned int I=0; I<(*X).TEXTURA[ (*X).PASO ].TAM; ++I)
		{
			/* UNICO COLOR -> NULO/NEGRO TRASPARENTE. */
			(*X).TEXTURA[ (*X).PASO ].PIXELS[I] = COLOR_NULO;
		}
	}

	if ( (MODO_CREACION == RELLENO_IMAGEN) )
	{
		int BPP;
		(*X).TEXTURA[ (*X).PASO ].IDENTIDAD = &(*X).PASO; /* IDENTIDAD. */
		(*X).TEXTURA[ (*X).PASO ].ACTIVA    = 1;          /* ACTIVA. */
		if ( strstr( NOMBRE, ".tga" ) || strstr( NOMBRE, ".TGA" ) )
			(*X).TEXTURA[ (*X).PASO ].PIXELS    = (P_EDITME_RGBX)LoadTGAConverter(
				NOMBRE,
				&(*X).TEXTURA[ (*X).PASO ].WH.ANCHO,
				&(*X).TEXTURA[ (*X).PASO ].WH.ALTO,
				&BPP
			);
		if ( strstr( NOMBRE, ".bmp" ) || strstr( NOMBRE, ".BMP" ) )
			(*X).TEXTURA[ (*X).PASO ].PIXELS    = (P_EDITME_RGBX)LoadBMPConverter(
				NOMBRE,
				&(*X).TEXTURA[ (*X).PASO ].WH.ANCHO,
				&(*X).TEXTURA[ (*X).PASO ].WH.ALTO,
				&BPP
			);
		(*X).TEXTURA[ (*X).PASO ].CLIP = EDITME_Precortar(0,0,(*X).TEXTURA[ (*X).PASO ].WH.ANCHO,(*X).TEXTURA[ (*X).PASO ].WH.ALTO);
		(*X).TEXTURA[ (*X).PASO ].TAM = ( ((*X).TEXTURA[ (*X).PASO ].WH.ALTO) * ((*X).TEXTURA[ (*X).PASO ].WH.ANCHO) ); 
	}

	/* DAMOS PASO A LA SIGUIENTE. */
	*ID = *(*X).TEXTURA[ (*X).PASO ].IDENTIDAD;
	(*X).PASO_ANTIGUO = (*X).PASO;
	(*X).PASO++;

	/* PASO ES MAS GRANDE QUE EL TAMAÑO CREA UN NUEVO VECTOR RETENIENDO SU CONTENIDO. */
	if ( (*X).PASO >= (*X).TAM )
	{
		const unsigned int TEMP_TAM = (*X).TAM;
		(*X).TAM *= 2;
		P_EDITME_textura TEMP_TEX2 = malloc( sizeof(EDITME_textura) * (*X).TAM );
		memset( TEMP_TEX2, 0x0, ( sizeof(EDITME_textura) * (*X).TAM ) );
		memcpy( TEMP_TEX2, (*X).TEXTURA, sizeof(EDITME_textura) * TEMP_TAM );
		free( (*X).TEXTURA );
		(*X).TEXTURA = NULL;
		(*X).TEXTURA = TEMP_TEX2;
	}

}


/*void EDITME_FBO_REESCALAR( P_EDITME_FBO X, int MODO, const char *NOMBRE, const int *ANCHO, const int *ALTO, int *ID ){
	unsigned int *Pixels = (*X).TEXTURA[ *ID ].PIXELS;
}*/


#include <stdio.h>

/* GUARDAR TEXTURA. */
void __fastcall SaveTGA( const char *filename, unsigned char *data, unsigned int ancho, unsigned int alto, unsigned char bpp ){

	/*Verificamos si bpp es un valor bajo, si lo es corregirlo al valor esacto al usar*/
	if ( bpp<24 )bpp*=8;


	/* ABRIMO ARCHIVO */

	FILE *f = fopen( filename, "wb" );

	if (!f){return;}

	/* CARGAMO HEADER */
	unsigned char *header = (unsigned char*)malloc(sizeof(unsigned char)*18);

	header[0] = 0;
	header[1] = 0;
	header[2] = 2;
	header[3] = 0;
	header[4] = 0;
	header[5] = 0;
	header[6] = 0;
	header[7] = 0;
	header[8] = 0;
	header[9] = 0;
	header[10] = 0;
	header[11] = 0;

	/* CARGAMO ANCHO */
	header[12] = (unsigned char)(ancho&0xFF);
	header[13] = (unsigned char)(ancho>>8)&0xFF;

	/* CARGAMO ALTO */
	header[14] = (unsigned char)(alto&0xFF);
	header[15] = (unsigned char)(alto>>8)&0xFF;

	/* CARGAMO BYTE POR PIXEL */
	header[16] = bpp;

	header[17] = 32;

	/* ESCRIBIMOS EL HEADER */
	fwrite( header, 1, 18, f );

	/* LIBERAMOS HEADER */
	free(header);

	bpp = (bpp/8);

	/* ESCRIBIMOS EL DATA */
	fwrite( data, 1, ( ancho * alto * bpp ), f );

	/* CERRAMO ARCHIVO */
	fclose(f);
}



void __fastcall SaveBMP( const char* filename, unsigned char *data, unsigned int ancho, unsigned int alto, unsigned char bpp ){


	/*Verificamos si bpp es un valor bajo, si lo es corregirlo al valor esacto al usar*/
	if ( bpp<24 )bpp*=8;

	/*Verificamos si data contiene algo en verdad.*/
	if ( !data )
	{ return; }

	/*Calculamos el padding. es una reserva de bytes basuras.*/
	const unsigned char padding = ( ( 4 - ( ancho * (bpp/8) ) % 4) % 4 );

	/*Creamos el contenedor header de la imagen*/
	unsigned char header[14];

	/*tamaño del header*/
	const unsigned char sizeheader = 14;

	/*cantidad del header*/
	const unsigned int infoheadersize = 40;
	unsigned char informacionheader[ infoheadersize ];

	/*Calculamos el tamaño del archivo*/
	const unsigned int filesize = sizeheader + infoheadersize + ancho * alto * (bpp/8) + padding * ancho;

	/*typo del archivo*/
	header[0] = 'B';
	header[1] = 'M';

	/*tamaño del archivo*/
	header[2] = filesize;
	header[3] = filesize >> 8;
	header[4] = filesize >> 16;
	header[5] = filesize >> 24;

	/*Reserva no usada*/
	header[6] = 0;
	header[7] = 0;
	
	/*Reserva no usada*/
	header[8] = 0;
	header[9] = 0;

	/*dato offset del pixel*/
	header[10] = sizeheader + infoheadersize;
	header[11] = 0;
	header[12] = 0;
	header[13] = 0;
	
	/*tamaño header*/
	informacionheader[0] = infoheadersize;
	informacionheader[1] = 0;
	informacionheader[2] = 0;
	informacionheader[3] = 0;

	/*imagen ancho*/
	informacionheader[4] = ancho;
	informacionheader[5] = ancho >> 8;
	informacionheader[6] = ancho >> 16;
	informacionheader[7] = ancho >> 24;
	
	/*imagen alto*/
	informacionheader[8]  = alto;
	informacionheader[9]  = alto >> 8;
	informacionheader[10] = alto >> 16;
	informacionheader[11] = alto >> 24;
	
	/*planos*/
	informacionheader[12] = 1;
	informacionheader[13] = 0;

	/*bites por pixel RGB*/
	informacionheader[14] = bpp;
	informacionheader[15] = 0;

	/*compresion*/
	informacionheader[16] = 0;
	informacionheader[17] = 0;
	informacionheader[18] = 0;
	informacionheader[19] = 0;

	/*compresion de tamaño de imagen*/
	informacionheader[20] = 0;
	informacionheader[21] = 0;
	informacionheader[22] = 0;
	informacionheader[23] = 0;

	/*pixeles en X por meter*/
	informacionheader[24] = 0;
	informacionheader[25] = 0;
	informacionheader[26] = 0;
	informacionheader[27] = 0;

	/*pixeles en Y por meter*/
	informacionheader[28] = 0;
	informacionheader[29] = 0;
	informacionheader[30] = 0;
	informacionheader[31] = 0;

	/*total de colores*/
	informacionheader[32] = 0;
	informacionheader[33] = 0;
	informacionheader[34] = 0;
	informacionheader[35] = 0;

	/*colores importantes*/
	informacionheader[36] = 0;
	informacionheader[37] = 0;
	informacionheader[38] = 0;
	informacionheader[39] = 0;

	/*Abrimos el archivo para leer */
	FILE *f = fopen( filename, "wb" );

	/*Creamos el contenedor de los bytes basuras*/
	unsigned char BYTES_BAZURAS[3];

	/*escribimos el header*/
	fwrite( header, sizeheader, 1, f );

	/*escribimos los datos del header*/
	fwrite( informacionheader, infoheadersize, 1, f );

	/*Escribimos la imagen de forma de matriz por sus tamaños*/
	int Y; /*alto de la imagen al pasar*/
	unsigned int X; /*ancho de la imagen al pasar*/

	unsigned char BPP = (bpp/8); /*Dato del Byte Per Pixel/Byte Por Pixel*/

	/*pasamos por los bytes de color*/
	for ( Y = alto-1; Y >=0 ; --Y)
	{
		for ( X = 0; X < ancho; ++X)
		{

			/*Creamos el step para posicionar al data*/
			/*correctamente*/
			unsigned int STEP = ( Y * ancho + X ) * BPP;

			/*Escribimos los colores*/
			fwrite( &data[ STEP ], BPP, 1, f );

		}

		/*Escribimos los bytes bazuras*/
		fwrite( BYTES_BAZURAS, padding, 1, f );

	}

	/*Cerramos el archivo */
	fclose(f),f=NULL;


	return;
}


/* CONVERTIR Y GUARDAR TEXTURA. */
void EDITME_SaveTGAConverter( const char *filename, void *data, unsigned int w, unsigned int h, unsigned int bpp ){

	SaveTGA( filename, (unsigned char*)data, w, h, bpp );
}


void EDITME_SaveBMPConverter( const char *filename, void *data, unsigned int w, unsigned int h, unsigned int bpp ){

	SaveBMP( filename, (unsigned char*)data, w, h, bpp );

}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


/* Crear un VBO - Vertice Buffer Object. */


typedef struct{
	float X;
	float Y;
	EDITME_RGBX C;
} *P_EDITME_VO, EDITME_VO;


typedef struct{
	P_EDITME_VO VO;
	unsigned int TAM;
	unsigned int PASO;
	unsigned int PASO_ANTIGUO;
} *P_EDITME_VBO, EDITME_VBO;

/* AGREGAR VERTICE AL VECTOR VERTICE BUFFER OBJECT. */
void EDITME_VBO_AGREGAR( P_EDITME_VBO X, P_EDITME_VO V ){

	(*X).VO[ (*X).PASO ] = *V;
	(*X).PASO_ANTIGUO = (*X).PASO;
	(*X).PASO++;

	if( (*X).PASO >= (*X).TAM ){
		const unsigned int TEMP_TAM = (*X).TAM;
		(*X).TAM *= 2;
		P_EDITME_VO TEMP_VO2 = malloc( sizeof(EDITME_VO) * (*X).TAM );
		memset( TEMP_VO2, 0x0, ( sizeof(EDITME_VO) * (*X).TAM ) );
		memcpy( TEMP_VO2, (*X).VO, sizeof(EDITME_VO) * TEMP_TAM );
		free( (*X).VO );
		(*X).VO = NULL;
		(*X).VO = TEMP_VO2;
	}

}

/* BORRAR VECTOR DE VERTICE BUFFER OBJECT. */
void EDITME_VBO_BORRAR_ALL( P_EDITME_VBO X ){

	register unsigned int I = 0;

	for (;I<X->TAM; ++I){
		X->VO[ I ].X = 0;
		X->VO[ I ].Y = 0;
		X->VO[ I ].C = COLOR_NULO;
	}
	X->PASO = 0;
	X->PASO_ANTIGUO = 0;
	X->TAM = 2;
	if ( X->VO ){
		free( X->VO );
		X->VO = NULL;
	}

}



/* Crear un VAO - Vertice Array Object. */


typedef struct{
	P_EDITME_VBO VO;
	unsigned int TAM;
	unsigned int PASO;
	unsigned int PASO_ANTIGUO;
} *P_EDITME_VAO, EDITME_VAO;


void EDITME_VAO_BORRAR_ALL( P_EDITME_VAO X ){

	register unsigned int I = 0;

	for (;I<X->TAM; ++I){
		if( X->VO[ I ].VO ){
			X->VO[ I ].TAM = 2;
			X->VO[ I ].PASO = 0;
			X->VO[ I ].PASO_ANTIGUO = 0;
			free( X->VO[ I ].VO );
			X->VO[ I ].VO = NULL;
		}
	}
	X->PASO = 0;
	X->PASO_ANTIGUO = 0;
	X->TAM = 2;
}


void EDITME_VAO_PREPARAR( P_EDITME_VAO X, const unsigned int *TAM ){

	X->TAM = *TAM;
	X->PASO = 0;
	X->PASO_ANTIGUO = 0;
	if ( X->VO ) free( X->VO );
	X->VO = NULL;
	X->VO = (P_EDITME_VBO)malloc( sizeof( EDITME_VBO ) * X->TAM );
	memset( X->VO, 0x0, ( sizeof( EDITME_VBO ) * X->TAM ) );

}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static EDITME_RGBX EDITME_COLOR_ALL = { 0,0,0,0xFF };
static EDITME_VO EDITME_VO_ALL = { 0,0, {0,0,0,0} };
static EDITME_FBO EDIFBO; /* FRAME BUFFER OBJECT. */
static EDITME_VBO EDIVBO; /* VERTICE BUFFER OBJECT. */
static EDITME_VAO EDIVAO; /* VERTICE ARRAY OBJECT. */


/* PREPARAMOS. */
void EDI_PREPARAR( int MODO ){

	if ( EDIFBO.TEXTURA ) free( EDIFBO.TEXTURA );
	EDIFBO.TAM = 2;
	EDIFBO.TEXTURA = NULL;
	EDIFBO.TEXTURA = (P_EDITME_textura)malloc( sizeof(EDITME_textura) * EDIFBO.TAM );
	memset( EDIFBO.TEXTURA, 0, (sizeof(EDITME_textura) * EDIFBO.TAM) );
	EDIFBO.PASO = 0;
	EDIFBO.PASO_ANTIGUO = 0;

	if ( EDIVBO.VO ) free( EDIVBO.VO );
	EDIVBO.TAM = 2;
	EDIVBO.VO = NULL;
	EDIVBO.VO = (P_EDITME_VO)malloc( sizeof(EDITME_VO) * EDIVBO.TAM );
	memset( EDIVBO.VO, 0, (sizeof(EDITME_VO) * EDIVBO.TAM) );
	EDIVBO.PASO = 0;
	EDIVBO.PASO_ANTIGUO = 0;

	EDITME_VAO_PREPARAR( &EDIVAO, &EDIVBO.TAM );

}

/* CREAR TEXTURA BUFFER CON TAMAÑO. */
void EDI_CREARBUFFR( int *ID, const int ANCHO, const int ALTO ){

	/*if ( *ID > EDIFBO.PASO ){
		EDITME_FBO_REESCALAR(
		&EDIFBO,
		RELLENO_SOLIDO,
		"NULO",
		&ANCHO,
		&ALTO,
		ID
		);
		return;
	}*/

	EDITME_FBO_AGREGAR(
		&EDIFBO,
		RELLENO_SOLIDO,
		"NULO",
		&ANCHO,
		&ALTO,
		ID
	);
}

void EDI_CARGAR( int *ID, const char *NOMBRE ){

	EDITME_FBO_AGREGAR(
		&EDIFBO,
		RELLENO_IMAGEN,
		NOMBRE,
		NULL,
		NULL,
		ID
	);

}

/* GUARDAR TEXTURA BUFFER CON NOMBRE. */
void EDI_GUARDARTEXTURA( int *ID, const char NOMBRE[] ){

	if( !EDIFBO.TEXTURA[ *ID ].ACTIVA ) return;
	if ( strstr( NOMBRE, ".tga" ) || strstr( NOMBRE, ".TGA" ) ) EDITME_SaveTGAConverter( NOMBRE, EDIFBO.TEXTURA[ *ID ].PIXELS, EDIFBO.TEXTURA[ *ID ].WH.ANCHO, EDIFBO.TEXTURA[ *ID ].WH.ALTO, 32 );
	if ( strstr( NOMBRE, ".bmp" ) || strstr( NOMBRE, ".BMP" ) ) EDITME_SaveBMPConverter( NOMBRE, EDIFBO.TEXTURA[ *ID ].PIXELS, EDIFBO.TEXTURA[ *ID ].WH.ANCHO, EDIFBO.TEXTURA[ *ID ].WH.ALTO, 32 );

}

/* COLOR CANAL 3 [RGB]. */
void EDI_COLOR_RGB( int R, int G, int B ){
	EDITME_COLOR_ALL.R = R;
	EDITME_COLOR_ALL.G = G;
	EDITME_COLOR_ALL.B = B;
	EDITME_COLOR_ALL.NC = 0xFF;
}

/* COLOR CANAL 4 [RGBA]. */
void EDI_COLOR_RGBA( int R, int G, int B, int A ){
	EDITME_COLOR_ALL.R = R;
	EDITME_COLOR_ALL.G = G;
	EDITME_COLOR_ALL.B = B;
	EDITME_COLOR_ALL.NC = A;
}


/* COLOR CANAL 3 ENTERO SIN SIGNO [RGB]. */
void EDI_COLOR_RGB_INT( unsigned int X ){
	EDITME_COLOR_ALL.R = (X>>16);
	EDITME_COLOR_ALL.G = (X>>8);
	EDITME_COLOR_ALL.B = (X>>0);
	EDITME_COLOR_ALL.NC = 0xFF;
}

/* COLOR CANAL 4 ENTERO SIN SIGNO [RGBA]. */
void EDI_COLOR_RGBA_INT( unsigned int X ){
	EDITME_COLOR_ALL.R = (X>>16);
	EDITME_COLOR_ALL.G = (X>>8);
	EDITME_COLOR_ALL.B = (X>>0);
	EDITME_COLOR_ALL.NC = (X>>24);
}


/* MODO DE VACIADO. */
void EDI_VACIAR( int *ID, int MODO ){

	register EDITME_RGBX *P = EDIFBO.TEXTURA[ *ID ].PIXELS;
	register unsigned int STOP = ( EDIFBO.TEXTURA[ *ID ].TAM );

	switch( MODO ){

		case EDI_COLOR:{
			while( STOP ){
				*P = EDITME_COLOR_ALL;
				P++;
				STOP--;
			}
			return;
		}

		default:{ return; }

	}

}

/* OBTIENE EL ANCHO DE TEXTURA. */
int EDI_ANCHO( int *ID ){
	if ( EDIFBO.TEXTURA[ *ID ].ACTIVA ) return EDIFBO.TEXTURA[ *ID ].WH.ANCHO;
	return 0;
}

/* OBTIENE EL ALTO DE TEXTURA. */
int EDI_ALTO( int *ID ){
	if ( EDIFBO.TEXTURA[ *ID ].ACTIVA ) return EDIFBO.TEXTURA[ *ID ].WH.ALTO;
	return 0;
}

/* OBTIENE EL ANCHO DE TEXTURA POR PUNTERO. */
void EDI_P_ANCHO( int *ID, int *X ){
	if ( EDIFBO.TEXTURA[ *ID ].ACTIVA ) *X=EDIFBO.TEXTURA[ *ID ].WH.ANCHO; else *X=0x0;
}

/* OBTIENE EL ALTO DE TEXTURA POR PUNTERO. */
void EDI_P_ALTO( int *ID, int *X ){
	if ( EDIFBO.TEXTURA[ *ID ].ACTIVA ) *X=EDIFBO.TEXTURA[ *ID ].WH.ALTO; else *X=0x0;
}

static P_EDITME_RGBX MEZCLA_ALPHA;

/*static inline double transparencia( double inColor, double a, double or, double og, double ob ) {
	long long inCol=inColor, av=a, rv=or, gv=og, bv=ob, r,g,b;
	int blNew, blOrg;
	inCol &= 0xFFffff;
	
	blNew=av & 0xFF; blOrg=255-blNew;
	b=((((inCol & 0xFF) * blOrg / 256) + ((bv & 0xFF) * blNew / 256)));
	g=(((((inCol>>8) & 0xFF) * blOrg / 256) + ((gv & 0xFF) * blNew / 256)));
	r=(((((inCol>>16) & 0xFF) * blOrg / 256) + ((rv & 0xFF) * blNew / 256)));
	return b | (g<<8) | (r<<16);
}*/

/* INTERPOLACION. */
static inline float EDITME_LERP(
	const unsigned char A,
	const unsigned char B,
	const float P
	)
{
	return ( (float)( 1.0f - P / 0xFF ) * A + (float)( P / 0xFF ) * B );
}



#define NC(X)(!(X)?1:(X))
#define MASK8_IN <<8
#define MASK8_OUT >>8

static unsigned int SL_D[6];

/* CREAR ALPHA */
static void EDITME_ALPHA( P_EDITME_RGBX X0, P_EDITME_RGBX X1 ){
	SL_D[0] = (0xFF MASK8_IN) / NC(( X1->R - X0->R ));
	SL_D[1] = (0xFF MASK8_IN) / NC(( X1->G - X0->G ));
	SL_D[2] = (0xFF MASK8_IN) / NC(( X1->B - X0->B ));
	SL_D[3] = X0->R MASK8_IN;
	SL_D[4] = X0->G MASK8_IN;
	SL_D[5] = X0->B MASK8_IN;
	SL_D[3] += ( SL_D[0] * X1->NC );
	SL_D[4] += ( SL_D[1] * X1->NC );
	SL_D[5] += ( SL_D[2] * X1->NC );
	X0->R = SL_D[3] MASK8_OUT;
	X0->G = SL_D[4] MASK8_OUT;
	X0->B = SL_D[5] MASK8_OUT;
}


/*static void EDITME_ALPHA( P_EDITME_RGBX X0, P_EDITME_RGBX X1 ){
	int slR = (0xFF<<8) / NC(( X1.R - X0->R ));
	int slG = (0xFF<<8) / NC(( X1.G - X0->G ));
	int slB = (0xFF<<8) / NC(( X1.B - X0->B ));
	int dR = X0->R<<8;
	int dG = X0->G<<8;
	int dB = X0->B<<8;
	dR += ( slR * X1.NC );
	dG += ( slG * X1.NC );
	dB += ( slB * X1.NC );
	X0->R = dR>>8;
	X0->G = dG>>8;
	X0->B = dB>>8;
}*/


/* ELEMENTO DE IMAGEN. */
void EDI_PIXEL( int *ID, int X, int Y ){
	if (
		(unsigned int)X <= (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ANCHO &&
		(unsigned int)Y <= (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ALTO
	)
	{
		MEZCLA_ALPHA = ( EDIFBO.TEXTURA[ *ID ].PIXELS + Y * EDIFBO.TEXTURA[ *ID ].WH.ANCHO + X );
		EDITME_ALPHA( MEZCLA_ALPHA, &EDITME_COLOR_ALL );
	}
}


/* OBTENER ELEMENTO DE IMAGEN. */
unsigned int EDI_GETPIXEL( int *ID, int X, int Y ){
	if (
		(unsigned int)X <= (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ANCHO &&
		(unsigned int)Y <= (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ALTO
	){
		MEZCLA_ALPHA = ( EDIFBO.TEXTURA[ *ID ].PIXELS + Y * EDIFBO.TEXTURA[ *ID ].WH.ANCHO + X );
		return *(unsigned int*)MEZCLA_ALPHA;
	}
	return 0;
}


/* AGREGA VERTICE. */
void EDI_VERTICE_2D_INT( int X, int Y ){
	EDITME_VO_ALL.X = X;
	EDITME_VO_ALL.Y = Y;
	EDITME_VO_ALL.C = EDITME_COLOR_ALL;
	EDITME_VBO_AGREGAR(
		&EDIVBO,
		&EDITME_VO_ALL
	);
}



#define EDI_ABS(X)(X)<0?(X)*-1:(X)
#define EDI_MAX(A,B)(A)>(B)?(A):(B)
#define EDI_MIN(A,B)(A)<(B)?(A):(B)
float EDI_FLOAT_SWAP;
#define EDI_SWAP_F(A,B)EDI_FLOAT_SWAP=(A),(A)=(B),(B)=EDI_FLOAT_SWAP;
int EDI_INT_SWAP;
#define EDI_SWAP_I(A,B)EDI_INT_SWAP=(A),(A)=(B),(B)=EDI_INT_SWAP;

void (*EDITME_PIPELINE)(
	P_EDITME_FBO Fbo,
	P_EDITME_VBO Vbo,
	P_EDITME_VAO Vao,
	const int MODO
	);



void EDI_EMPEZAR( int MODO ){MODO=0;}
void EDI_FINAL( void ){}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


/* Apartado GFX ---- EDITME-GFX */

void EDITME_GFX_ESCALA( int *ID, const int MODO );

void EDI_GFX( int *ID, const char EFECTOS[] ){

	/* BUSCANDO CUAL EFECTO USAR. */
	if ( strstr( EFECTOS, "escala" ) ){
		if ( strstr( EFECTOS, "gris" )     ) EDITME_GFX_ESCALA( ID, 1 );
		if ( strstr( EFECTOS, "rojo" )     ) EDITME_GFX_ESCALA( ID, 2 );
		if ( strstr( EFECTOS, "verde" )    ) EDITME_GFX_ESCALA( ID, 3 );
		if ( strstr( EFECTOS, "azul" )     ) EDITME_GFX_ESCALA( ID, 4 );
		if ( strstr( EFECTOS, "invertir" ) ) EDITME_GFX_ESCALA( ID, 5 );
		if ( strstr( EFECTOS, "posicion" ) ) EDITME_GFX_ESCALA( ID, 6 );
	}

}

void EDITME_GFX_ESCALA( int *ID, const int MODO ){

	switch( MODO ){

		/* ########################### */
		case 1:{
			for (register unsigned int I=0; I<EDIFBO.TEXTURA[ *ID ].TAM; ++I){
				unsigned char GRAY = ( ( EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R + EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G + EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B ) / 3 );
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R = GRAY;
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G = GRAY;
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B = GRAY;
			}
			return;
		}
		/* ########################### */

		/* ########################### */
		case 2:{
			for (register unsigned int I=0; I<EDIFBO.TEXTURA[ *ID ].TAM; ++I){
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G = 0;
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B = 0;
			}
			return;
		}
		/* ########################### */

		/* ########################### */
		case 3:{
			for (register unsigned int I=0; I<EDIFBO.TEXTURA[ *ID ].TAM; ++I){
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R = 0;
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B = 0;
			}
			return;
		}
		/* ########################### */

		/* ########################### */
		case 4:{
			for (register unsigned int I=0; I<EDIFBO.TEXTURA[ *ID ].TAM; ++I){
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R = 0;
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G = 0;
			}
			return;
		}
		/* ########################### */

		/* ########################### */
		case 5:{
			for (register unsigned int I=0; I<EDIFBO.TEXTURA[ *ID ].TAM; ++I){
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R = ( 0xFF - EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].R );
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G = ( 0xFF - EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].G );
				EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B = ( 0xFF - EDIFBO.TEXTURA[ *ID ].PIXELS[ I ].B );
			}
			return;
		}
		/* ########################### */

		/* ########################### */
		case 6:{
			register unsigned int Y=0,X=0;
			const unsigned int W = (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ANCHO;
			const unsigned int H = (unsigned int)EDIFBO.TEXTURA[ *ID ].WH.ALTO;
			for (;Y<H;++Y){
				for (;X<W;++X){
					EDIFBO.TEXTURA[ *ID ].PIXELS[ Y * W + X ].R = ( 0xFF * NC(X) / W );
					EDIFBO.TEXTURA[ *ID ].PIXELS[ Y * W + X ].G = ( 0xFF * NC(Y) / H );
				}X=0;
			}
			return;
		}
		/* ########################### */

	}

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



#define EDITME_CONSTRUCTOR __attribute__((constructor))
#define EDITME_DESTRUCTOR  __attribute__((destructor))

void EDITME_CONSTRUCTOR EDI_PREPARAR_ESCENAS(void){

	EDI_PREPARAR( EDI_SC_NORMAL );
	const int W=2,H=2;
	int ID;
	EDI_CREARBUFFR(
		&ID,
		W,H
	);

}

void EDITME_DESTRUCTOR EDI_BORRAR_ESCENAS(void){

	printf("EDITME LIBERA. [%i]\n--------------\n", EDIFBO.TAM );
	for (unsigned int I=0; I<EDIFBO.TAM; ++I)
	{
		if (EDIFBO.TEXTURA[ I ].PIXELS)
		printf("TEXTURA [%3i] - %ix%i\t : ACTIVO [%i]\n", I, EDIFBO.TEXTURA[ I ].WH.ANCHO, EDIFBO.TEXTURA[ I ].WH.ALTO, EDIFBO.TEXTURA[ I ].ACTIVA );
		EDIFBO.TEXTURA[ I ].IDENTIDAD = NULL;
		free( EDIFBO.TEXTURA[ I ].PIXELS );
		EDIFBO.TEXTURA[ I ].PIXELS   = NULL;
		EDIFBO.TEXTURA[ I ].WH.ANCHO = 0;
		EDIFBO.TEXTURA[ I ].WH.ALTO  = 0;
		EDIFBO.TEXTURA[ I ].TAM      = 0;
		EDIFBO.TEXTURA[ I ].ACTIVA   = 0;
		EDIFBO.TEXTURA[ I ].CLIP     = NULL;
	}
	EDIFBO.PASO = 0;
	EDIFBO.PASO_ANTIGUO = 0;
	EDIFBO.TAM = 0;
	free( EDIFBO.TEXTURA );
	EDIFBO.TEXTURA = NULL;

	EDITME_VBO_BORRAR_ALL( &EDIVBO );
	EDITME_VAO_BORRAR_ALL( &EDIVAO );

}


#ifdef __cplusplus
	}
#endif