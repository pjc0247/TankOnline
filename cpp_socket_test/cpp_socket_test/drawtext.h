#ifndef M_DRAWTEXT
#define M_DRAWTEXT

#include <sdl_ttf.h>
#include <sdl.h>

void han2unicode(char *src, Uint16 *dest);


class FONT{
public:
	FONT();
	~FONT();

	bool InitFont(const char *font,int size=10);
	void Close();
	void Alpha(int a);

	void SetStyle(int style);
	void SetColor(int r,int g,int b);
	void SetSize(int size);

	SDL_Surface* Render(char *str);
	void Draw(SDL_Renderer *screen,char *str,int x,int y,...);

	SDL_Rect PeekSize(char *str);

	int alpha;
//private:
	TTF_Font *p_font;
	int p_size;
	SDL_Color p_color;
	char p_font_path[256];
	int rr,gg,bb;
	
};

#endif