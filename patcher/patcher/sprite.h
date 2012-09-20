#ifndef SPRITE_H
#define SPRITE_H


#include <vector>
using namespace std;

#define SDL_SRCCOLORKEY 1
#define SDL_SRCALPHA 1

extern SDL_Renderer *renderer;

class Sprite{
public:

	Sprite(int w,int h){
		if ( w == -1 ){
			return;
		}
		else{
			texture.push_back(SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_ARGB8888,
				SDL_TEXTUREACCESS_TARGET,
				w,h));

			int pitch;
			char *pix;
			pix = (char *)malloc(sizeof(char) * w * h * 4);
			memset(pix,0,sizeof(char) * w * h * 4);
			SDL_UpdateTexture(texture[0],NULL,pix,4);
			free(pix);

			SDL_SetTextureBlendMode(texture[0],SDL_BLENDMODE_BLEND);

			this->w = 1;
			this->h = 1;
			this->step = 0;
			this->e = 0;
			this->angle = 0;
			Rect(0,0,w,h);
			repeat = true;
			aa = false;
			surface = NULL;

			range_min = 0;
			range_max = w*h;
		}
	}
	Sprite(const char *file,int w,int h,int r=255,int g=255,int b=255){
		surface = IMG_Load(file);;

		if(surface == NULL)
			return;

		SDL_Texture *t;
		SDL_SetColorKey(surface,SDL_SRCCOLORKEY,SDL_MapRGB(surface->format,r,g,b));
		t = SDL_CreateTextureFromSurface(renderer,surface);
		if(t == NULL){
			printf("failed to load texture : %s\n", file);
		}
		texture.push_back(t);
		this->w = w;
		this->h = h;
		this->step = 0;
		this->e = 0;
		this->angle = 0;
		Rect(0,0,surface->w,surface->h);
		repeat = true;
		aa = false;

		range_min = 0;
		range_max = w*h;
	}
	Sprite(SDL_Surface *surface,int w,int h,int r=255,int g=255,int b=255){
		this->surface = surface;
		SDL_SetColorKey(surface,SDL_SRCCOLORKEY,SDL_MapRGB(surface->format,r,g,b));
		texture.push_back(SDL_CreateTextureFromSurface(renderer,surface));
		this->surface = surface;
		this->w = w;
		this->h = h;
		this->step = 0;
		this->e = 0;
		this->angle = 0;
		Rect(0,0,surface->w,surface->h);
		repeat = true;
		aa = false;

		range_min = 0;
		range_max = w*h;
	}
	Sprite(bool a,const char *file,int r=255,int g=255,int b=255){
		for(int i=0;;i++){
			char path[256];
			sprintf(path,"%s%d%d%d.png",file,i/100,(i%100)/10,i%10);
			FILE*fp = fopen(path,"r");
			if(fp == NULL)
				break;
			fclose(fp);
			surface = IMG_Load(path);
			SDL_SetColorKey(surface,SDL_SRCCOLORKEY,SDL_MapRGB(surface->format,r,g,b));
			texture.push_back(SDL_CreateTextureFromSurface(renderer,surface));
			SDL_FreeSurface(surface);
		}
		this->w = texture.size();
		this->h = 1;
		this->step = 0;
		this->e = 1;
	}
	~Sprite(){
		if(surface != NULL)
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture[0]);

		printf("delete sprite %x\n", this);
	}

	void Colorkey(int r,int g,int b){
		printf("recolorkey %d %d %d\n",r,g,b);
		SDL_SetColorKey(surface,SDL_SRCCOLORKEY,SDL_MapRGB(surface->format,r,g,b));
		SDL_DestroyTexture(texture[0]);
		texture.pop_back();
		texture.push_back(SDL_CreateTextureFromSurface(renderer,surface));
		printf("%d\n", texture.size());
		cr = r;
		cg = g;
		cb = b;
	}
	void Rect(int x,int y,int w,int h){
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		use_rectmap = false;
	}
	void RectMap(const char *file){
		rectmap.clear();
		FILE *fp = fopen(file,"r");
		for(int i=0;i<w*h;i++){
			SDL_Rect r;
			fscanf(fp,"%d%d%d%d\n", &r.x,&r.y,&r.w,&r.h);
			rectmap.push_back(r);
		}
		fclose(fp);
		use_rectmap = true;
	}
	void Color(int r,int g,int b){
		if(this->e)
			for(int i=0;i<w*h;i++)
				SDL_SetTextureColorMod(texture[i],r,g,b);
		else
			SDL_SetTextureColorMod(texture[0],r,g,b);
		rr = r;
		gg = g;
		bb =b;
	}
	void Alpha(int a){
		if(this->e)
			for(int i=0;i<w*h;i++)
				SDL_SetTextureAlphaMod(texture[i],a);
		else
			SDL_SetTextureAlphaMod(texture[0],a);
	}
	void Blend(SDL_BlendMode blend){
		if(this->e)
			for(int i=0;i<w*h;i++){
				SDL_SetTextureBlendMode(texture[i],blend);
			}
		else{
			SDL_SetTextureBlendMode(texture[0],blend);
		}
	}
	void Set(int s){
		step = s;
		if(s >= w*h)
			step = w*h-1;
	}
	void Repeat(bool f){
		repeat = f;
	}
	bool Step(){
		if(! (step >= range_min && step <= range_max)){
			step = range_min;
		}
		step++;
		if(step == range_max){
			if(repeat)
				step = range_min;
			else step--;
			return 1;
		}
		else return 0;
	}
	int W(){
		int w;
		SDL_QueryTexture(texture[step],NULL,NULL,&w,NULL);
		return w;
	}
	int H(){
		int h;
		SDL_QueryTexture(texture[step],NULL,NULL,NULL,&h);
		return h;
	}

	void Draw(int x,int y,int sw=-1,int sh=-1){
		SDL_Rect dst,src;
		dst.x = x;
		dst.y = y;
	//	printf("tsize   %d\n", texture.size());
		if(this->e){
			SDL_QueryTexture(texture[step],NULL,NULL,&dst.w,&dst.h);
			SDL_RenderCopyEx(renderer,texture[step],NULL,&dst,angle,NULL,SDL_FLIP_NONE);
		}
		else if(aa == true)
		{
			SDL_RenderCopy(renderer,texture[0],NULL,NULL);
		}
		else{
			src.x = rect.x + (rect.w/w) * (step%w);
			src.y = rect.y + (rect.h/h) * (step/w);
			
			if(sw == -1){
				dst.w = src.w = (rect.w)/w;
				dst.h = src.h = (rect.h)/h;
				
			}
			else{
				src.w = (rect.w)/w;
				src.h = (rect.h)/h;
				dst.w = sw;
				dst.h = sh;
			}
			
			SDL_Point center = {dst.w/2,dst.h/2};
		//	SDL_RenderCopy(renderer,texture[0],&src,&dst);
			SDL_RenderCopyEx(renderer,texture[0],&src,&dst,angle,&center,SDL_FLIP_NONE);
		}
	}

	vector<SDL_Rect> rectmap;
	bool use_rectmap;

	int range_min;
	int range_max;
	bool aa;
	float angle;
	vector<SDL_Texture *> texture;
	SDL_Surface *surface;
	int w,h;
	SDL_Rect rect;
	int step;
	int e;
	int cr,cb,cg;
	int rr,gg,bb;
	bool repeat;
	int sw,wh;
};


#endif