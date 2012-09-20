#include "stdafx.h"
#include "image.h"

#include <math.h>

#define FONTS "fonts\\"

#define Color(x) 

#define SDL_SRCCOLORKEY 1

SDL_Surface *LoadIMG(char * filename)
{
	// 불러온이미지를잠시저장할공간
	SDL_Surface * loadedImage = NULL;
	
	// 다양한용도로사용할공간
	SDL_Surface * optimizedImage = NULL;
	
	loadedImage = IMG_Load( filename);
	

	// 이미지가정상적으로불러와졌다면
	if(loadedImage != NULL)
	{	
		optimizedImage = loadedImage;
		
	}
	else {
		Color(9);
		printf(" Load Failed , %s \n", filename);
	//	MessageBox(NULL,"이미지 로드에 실패하였습니다",filename,MB_OK | MB_ICONERROR);
		
	}
	Color(15);
	printf (" Image loaded ( %s , %x ) \n", filename, optimizedImage);
	return optimizedImage;
}

void DrawBitmap(SDL_Renderer *surface, int x,int y,SDL_Surface *bitmap){
	
    SDL_Rect offset;

	SDL_Texture *texture = SDL_CreateTextureFromSurface(surface,bitmap);
    offset.x = x;
    offset.y = y;
	SDL_QueryTexture(texture,NULL,NULL,&offset.w,&offset.h);
	SDL_RenderCopy(surface,texture,NULL,&offset);
	SDL_DestroyTexture(texture);
	//SDL_BlitSurface( bitmap, NULL , surface, &offset);
}

void DrawBitmap(SDL_Surface *surface, int x,int y,SDL_Surface *bitmap,SDL_Rect *sprite){
	
    SDL_Rect offset;
	
    offset.x = x;
    offset.y = y;
	
	SDL_BlitSurface( bitmap, sprite , surface, &offset);
}


void DrawBitmap(SDL_Surface *surface, int x,int y,SDL_Surface *bitmap,int sx,int sy,int sw,int sh){
	
    SDL_Rect offset,sprite;
	
    offset.x = x;
    offset.y = y;
	
	sprite.x = sx;
	sprite.y = sy;
	sprite.w = sw;
	sprite.h = sh;
	
	SDL_BlitSurface( bitmap, &sprite , surface, &offset);
}


Uint32 SDL_RGB(SDL_Surface* surface,int R,int G,int B){
	return SDL_MapRGB(surface->format, R, G, B);
}
Uint32 GetColorKey(SDL_Surface * surface){
	return SDL_MapRGB(surface->format, 255, 255, 255);
}
void SetColorKey(SDL_Surface * surface){
	SDL_SetColorKey( surface, SDL_RLEACCEL | SDL_SRCCOLORKEY, GetColorKey(surface));
}
void SetColorKey(SDL_Surface * surface,int r,int g,int b){
	SDL_SetColorKey( surface, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_RGB(surface,r,g,b));
}


void DrawPixel(SDL_Surface *screen, int x, int y,
			   Uint8 R, Uint8 G, Uint8 B)
{
	Uint32 color = SDL_MapRGB(screen->format, R, G, B);
	switch (screen->format->BytesPerPixel)
	{
	case 1: // Assuming 8-bpp
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
			*bufp = color;
		}
		break;
	case 2: // Probably 15-bpp or 16-bpp
		{
			Uint16 *bufp;
			bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
			*bufp = color;
		}
		break;
	case 3: // Slow 24-bpp mode, usually not used
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
			if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
			{
				bufp[0] = color;
				bufp[1] = color >> 8;
				bufp[2] = color >> 16;
			} else {
				bufp[2] = color;
				bufp[1] = color >> 8;
				bufp[0] = color >> 16;
			}
		}
		break;
	case 4: // Probably 32-bpp
		{
			Uint32 *bufp;
			bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
			*bufp = color;
		}
		break;
	}
}

void line_to(SDL_Surface*screen ,int x1, int y1, int x2, int y2 )
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    double a  = (double)dy/dx;
    double b  = y1 - a*x1;
    int  stepx=1, stepy=1;
	
    if ( dx < 0 )
        stepx = -1;
	
    if ( dy < 0 )
        stepy = -1;
	
	
    if( abs(dx) > abs(dy) )
    {
        while( x1 != x2 )
        {
			DrawPixel(screen,x1,(int)(a*x1+ b + 0.5),0,0,0);
            x1 += stepx;
        }
		DrawPixel(screen,x1,(int)(a*x1+ b + 0.5),0,0,0);
    }
    else
    {
        while( y1 != y2 )
        {  
			DrawPixel(screen,(int)( (y1-b)/a + 0.5),y1,0,0,0);
            y1 += stepy;
        }
		DrawPixel(screen,(int)( (y1-b)/a  + 0.5),y1,0,0,0);
    }
	
}
void DrawCircle(SDL_Surface* screen,int x, int y, int radius,bool fill)
{
   signed int a, b, P;
   a = 0;
   b = radius;
   P = 1 - radius;
   do
   {
      if(fill)
      {
         line_to(screen,x-a, y+b, x+a, y+b);
         line_to(screen,x-a, y-b, x+a, y-b);
         line_to(screen,x-b, y+a, x+b, y+a);
         line_to(screen,x-b, y-a, x+b, y-a);
      }
      else
      {
         DrawPixel(screen,a+x, b+y, 0,0,0);
         DrawPixel(screen,b+x, a+y, 0,0,0);
         DrawPixel(screen,x-a, b+y, 0,0,0);
         DrawPixel(screen,x-b, a+y, 0,0,0);
         DrawPixel(screen,b+x, y-a, 0,0,0);
         DrawPixel(screen,a+x, y-b, 0,0,0);
         DrawPixel(screen,x-a, y-b, 0,0,0);
         DrawPixel(screen,x-b, y-a, 0,0,0);
      }
      if(P < 0)
         P+= 3 + 2*a++;
      else
         P+= 5 + 2*(a++ - b--);
    } while(a <= b);
}