#include <sdl.h>
#include <sdl_gdiplus.h>
SDL_Surface *LoadIMG(char* filename);
void DrawBitmap(SDL_Renderer *surface, int x,int y,SDL_Surface *bitmap);
void Flip();
Uint32 SDL_RGB(SDL_Surface* surface,int R,int G,int B);
Uint32 GetColorKey(SDL_Surface * surface);
void SetColorKey(SDL_Surface * surface);
void DrawBitmap(SDL_Surface *surface, int x,int y,SDL_Surface *bitmap,int sx,int sy,int sw,int sh);
SDL_Surface* MkSurface();
void DrawBitmap(SDL_Surface *surface, int x,int y,SDL_Surface *bitmap,SDL_Rect *sprite);
SDL_Surface* DrawTextS(char str[],int size,SDL_Color color);
SDL_Surface* DrawTextS(char str[],int size);
SDL_Surface* DrawTextSA(char str[],int size);
void DrawText_SetFont(int size);
SDL_Surface* DrawText_Draw(char str[]);
void DrawText_SetFont2(int size);
SDL_Surface* DrawText_Draw2(char str[]);
SDL_Surface* DrawTextC(char fontname[],char str[],int size);
SDL_Surface* DrawTextC(char fontname[],char str[],int size,SDL_Color color);
void DrawPixel(SDL_Surface *screen, int x, int y,Uint8 R, Uint8 G, Uint8 B);
void line_to(SDL_Surface*screen ,int x1, int y1, int x2, int y2 );
void DrawCircle(SDL_Surface*screen,int x, int y, int radius,bool fill);
SDL_Surface* DrawText_Draw(int n);
SDL_Surface* DrawText_Draw2(char str[],SDL_Color color);
void SetColorKey(SDL_Surface * surface,int r,int g,int b);