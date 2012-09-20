// patcher.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include <SDL.h>
#include <sdl_gdiplus.h>
#include <sdl_ttf.h>

#include "drawtext.h"
#include "sprite.h"

#pragma comment (lib,"sdl")
#pragma comment (lib,"sdl_gdiplus")
#pragma comment (lib,"sdl_ttf")


SDL_Window *window;
SDL_Renderer *renderer;


void ScenePatch(){
	bool quit = false;
	SDL_Event event;

	Sprite *bgi = new Sprite("resource\\bgi_patcher.jpg",1,1);
	Sprite *title = new Sprite("resource\\title.png",1,1);

	FONT gulim;

	float progress_total = 50;
	float progress_current = 30;

	gulim.InitFont("c:\\windows\\fonts\\gulim.ttc",12);
	gulim.SetColor(255,255,255);
	gulim.SetStyle(TTF_STYLE_BOLD);

	while(!quit){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_QUIT:
				break;
			}
		}
		{
			SDL_RenderClear(renderer);

			bgi->Draw(0,0,480,272);
			title->Draw(20,20,440,170);


			SDL_Rect rtProgressTotalFrame = {20,200,440,25};
			SDL_Rect rtProgressCurrentFrame = {20,235,440,25};
			SDL_Rect rtProgressTotal = {25,205,progress_total * 4.4,15};
			SDL_Rect rtProgressCurrent = {25,240,progress_current * 4.4,15};

			SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
			SDL_SetRenderDrawColor(renderer,64,64,64,255);
			SDL_RenderFillRect(renderer,&rtProgressTotalFrame);
			SDL_RenderFillRect(renderer,&rtProgressCurrentFrame);

			SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(renderer,64,255,64,196);
			SDL_RenderFillRect(renderer,&rtProgressTotal);
			SDL_RenderFillRect(renderer,&rtProgressCurrent);

			gulim.Draw(renderer,"전체 진행률",25,190);
			gulim.Draw(renderer,"현재 진행률",25,225);

			gulim.Draw(renderer,"Tank Online Patcher",343,258);

			SDL_RenderPresent(renderer);
		}

		SDL_Delay(1);
	}

	delete bgi;
}

void SetupRC(){
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(
						"Patcher",
						SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
						480,272,
						SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(
						window,
						-1,
						0);

	TTF_Init();
}
void Quit(){
	TTF_Quit();
	SDL_Quit();
}

int _tmain(int argc, _TCHAR* argv[])
{
	SetupRC();

	ScenePatch();

	Quit();
	return 0;
}


