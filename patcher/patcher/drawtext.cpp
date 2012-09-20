#include "stdafx.h"

#ifdef NDEBUG
   #ifdef _DEBUG
     #undef _DEBUG
   #endif
#endif

#include <sdl.h>
#include <sdl_ttf.h>

#include "drawtext.h"
#include "image.h"

#include "han2unicode.h"

#include <vector>
#include <string>
using namespace std;

#define Color(x)

FONT::FONT(){
	alpha = 255;
	p_font = NULL;
}

FONT::~FONT(){
	if(p_font != NULL)
		TTF_CloseFont(p_font);
}

bool FONT::InitFont(const char *font,int size){
	sprintf(p_font_path,font);

	//if(p_font != NULL)
		//TTF_CloseFont(p_font);
	p_font = TTF_OpenFont(font, size);
	
	if(p_font == NULL){
		Color(12);
		printf(" load failed ( %s , %x )\n", font,p_font);
		return false;
	}
	else {
		Color(15);
		printf("Load Font ( %s , %x )\n", font,p_font);
		return true;
	}
		
}
void FONT::SetStyle(int style){
	TTF_SetFontStyle(p_font,style);
}
void FONT::SetSize(int size){
	InitFont(p_font_path,size);
	p_size = size;
}
void FONT::SetColor(int r,int g,int b){
	p_color.r = r;
	p_color.g = g;
	p_color.b = b;
	
}

SDL_Surface* FONT::Render(char *str){
	Uint16 string_han[256];

	han2unicode(str, string_han);

	return TTF_RenderUNICODE_Blended(p_font, string_han, p_color);
}

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " "){
	if(str.size() == 0) return;

    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos){
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}
void FONT::Draw(SDL_Renderer *renderer,char *str,int x,int y,...){
	SDL_Surface *text;

	char buf[256];
	vector<string> tok;
	int offsetY = 0;

	va_list   argptr;
	va_start(argptr,str);
	vsprintf(buf, str, argptr);  
	va_end(argptr);

	Tokenize(string(buf),tok,string("\n"));
	for(int i=0;i<tok.size();i++){
		char buf[256];
		sprintf(buf,"%s",tok[i].c_str());

		text = Render(buf);
		SDL_SetSurfaceAlphaMod(text,alpha);
		if(text != NULL){
			DrawBitmap(renderer,x,y+offsetY,text);
			offsetY += text->h*1.1;
			SDL_FreeSurface(text);
		}
	}
}
SDL_Rect FONT::PeekSize(char *str){
	SDL_Rect rt;
	TTF_SizeText(p_font,str,&rt.w,&rt.h);
	return rt;
}
void FONT::Close(){
	TTF_CloseFont(p_font);
}
void FONT::Alpha(int a){
	alpha = a;
}

