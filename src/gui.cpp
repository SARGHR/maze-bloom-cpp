#include "Maze.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <iostream>

struct Color { Uint8 r,g,b,a=255; };
static void color(SDL_Renderer*r,Color c){SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);}
static void rect(SDL_Renderer*r,int x,int y,int w,int h,Color c){color(r,c);SDL_Rect z{x,y,w,h};SDL_RenderFillRect(r,&z);}
static void outline(SDL_Renderer*r,int x,int y,int w,int h,Color c){color(r,c);SDL_Rect z{x,y,w,h};SDL_RenderDrawRect(r,&z);}
static void circle(SDL_Renderer*r,int cx,int cy,int radius,Color c){color(r,c);for(int dy=-radius;dy<=radius;++dy){int dx=(int)std::sqrt(radius*radius-dy*dy);SDL_RenderDrawLine(r,cx-dx,cy+dy,cx+dx,cy+dy);}}
static void txt(SDL_Renderer*r,TTF_Font*f,const std::string&s,int x,int y,Color c){if(!f)return;SDL_Color col{c.r,c.g,c.b,c.a};SDL_Surface*surf=TTF_RenderUTF8_Blended(f,s.c_str(),col);if(!surf)return;SDL_Texture*t=SDL_CreateTextureFromSurface(r,surf);if(t){SDL_Rect dst{x,y,surf->w,surf->h};SDL_RenderCopy(r,t,nullptr,&dst);SDL_DestroyTexture(t);}SDL_FreeSurface(surf);}
static TTF_Font*font(int n){const char*paths[]={"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf","/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf","/System/Library/Fonts/Supplemental/Arial.ttf"};for(auto p:paths){auto*f=TTF_OpenFont(p,n);if(f)return f;}return nullptr;}
struct Button{SDL_Rect box;std::string label;};
static bool inside(int x,int y,SDL_Rect b){return x>=b.x&&y>=b.y&&x<b.x+b.w&&y<b.y+b.h;}
class Game{
  SDL_Window*win=nullptr;SDL_Renderer*r=nullptr;TTF_Font*large=nullptr,*regular=nullptr,*small=nullptr;
  std::mt19937 rng{std::random_device{}()};
  int level=0,player=0,hearts=3,moves=0,score=0,bonus=0,coinsTaken=0;bool won=false,lost=false,showHint=false,selectLevel=true;
  Uint32 hitAt=0,startedAt=0,wonAt=0;std::vector<int>hazards,hint,coins;Maze maze{9,7};
  const int widths[3]={9,13,18},heights[3]={7,10,13},hazardCounts[3]={3,8,17};
  std::vector<Button> buttons={{{40,545,174,44},"NEW MAZE"},{{226,545,174,44},"HINT (BFS)"},{{412,545,174,44},"NEXT LEVEL"}};
  void reset(int n){selectLevel=false;level=n;coinsTaken=0;maze=Maze(widths[n],heights[n]);maze.generate();player=0;hearts=3;moves=0;won=false;lost=false;showHint=false;hitAt=0;wonAt=0;bonus=0;startedAt=SDL_GetTicks();hint=maze.solveBFS();hazards.clear();
    // Avoid the start, finish, and their adjacent cells; keep the guaranteed maze route playable.
    std::vector<int>candidates;for(int i=0;i<maze.width()*maze.height();++i){int x=i%maze.width(),y=i/maze.width();if(x+y<=1 || (maze.width()-1-x)+(maze.height()-1-y)<=1)continue;
      // Never put a monster on the unique route from start to exit.
      if(std::find(hint.begin(),hint.end(),i)!=hint.end())continue;
      candidates.push_back(i);}
    std::shuffle(candidates.begin(),candidates.end(),rng);int count=std::min(hazardCounts[n],(int)candidates.size());hazards.assign(candidates.begin(),candidates.begin()+count);
    // Collectible coins are on the guaranteed safe route, never on start/exit.
    coins.clear();int coinCount=3+n*2;
    for(int k=1;k<=coinCount;++k){int at=(int)((long long)k*(hint.size()-1)/(coinCount+1));if(at>0&&at+1<(int)hint.size()&&std::find(coins.begin(),coins.end(),hint[at])==coins.end())coins.push_back(hint[at]);}
  
  }
  void move(int dir){if(selectLevel||won||lost)return;int x=player%maze.width(),y=player/maze.width();if(maze.wall(x,y,dir))return;int nx=x+(dir==1)-(dir==3),ny=y+(dir==2)-(dir==0);if(nx<0||ny<0||nx>=maze.width()||ny>=maze.height())return;player=ny*maze.width()+nx;++moves;
    if(std::find(hazards.begin(),hazards.end(),player)!=hazards.end()){
      --hearts;hitAt=SDL_GetTicks();if(hearts<=0){lost=true;}else{player=0;}return;
    }
    auto coin=std::find(coins.begin(),coins.end(),player);
    if(coin!=coins.end()){coins.erase(coin);++coinsTaken;score+=50;}
    if(player==maze.width()*maze.height()-1){
      won=true;wonAt=SDL_GetTicks();
      const int shortest=(int)hint.size()-1;
      bonus=(moves==shortest)?500:0;
      score+=hearts*100+std::max(0,300-moves*2)+(level+1)*100+bonus;
    }
  }
  // Monsters move between safe side corridors, but never enter the guaranteed BFS path.
  void moveMonsters(){if(selectLevel||won||lost)return;
    std::vector<int> next=hazards;
    for(size_t i=0;i<hazards.size();++i){
      int pos=hazards[i],x=pos%maze.width(),y=pos/maze.width();
      std::vector<int>choices{pos};
      for(int d=0;d<4;++d){if(maze.wall(x,y,d))continue;
        int nx=x+(d==1)-(d==3),ny=y+(d==2)-(d==0),v=ny*maze.width()+nx;
        if(nx<0||ny<0||nx>=maze.width()||ny>=maze.height())continue;
        if(std::find(hint.begin(),hint.end(),v)!=hint.end())continue;
        if(std::find(next.begin(),next.end(),v)!=next.end())continue;
        choices.push_back(v);
      }
      std::uniform_int_distribution<int> pick(0,(int)choices.size()-1);
      next[i]=choices[pick(rng)];
    }
    hazards=std::move(next);
    if(std::find(hazards.begin(),hazards.end(),player)!=hazards.end()){
      --hearts;hitAt=SDL_GetTicks();if(hearts<=0)lost=true;else player=0;
    }
  }
  void draw(){int W=980,H=650;Uint32 ticks=SDL_GetTicks();float t=ticks/1000.f;
    rect(r,0,0,W,H,{17,19,40});
    for(int i=0;i<24;++i){int x=(i*137+29)%W,y=(i*83+19)%H;int rad=1+(i%3);circle(r,x,y+(int)(4*std::sin(t*.6f+i)),rad,{82,77,133});}
    rect(r,24,20,932,92,{29,31,60});rect(r,24,20,6,92,{250,119,168});
    txt(r,large,"MAZE BLOOM",46,31,{255,203,222});txt(r,small,"Find the exit. Dodge the traps. Save your hearts.",48,74,{183,189,219});
    txt(r,regular,"LEVEL " +std::to_string(level+1)+" / 3",720,32,{249,206,139});
    txt(r,small,level==0?"EASY":level==1?"MEDIUM":"HARD",722,69,{175,217,248});
    rect(r,24,127,610,398,{31,34,64});rect(r,646,127,310,398,{31,34,64});
    txt(r,regular,"YOUR ADVENTURE",669,147,{250,190,215});
    txt(r,small,"LIVES",669,195,{180,185,213});
    for(int i=0;i<3;++i){Color c=i<hearts?Color{255,100,148}:Color{78,79,108};int x=685+i*43,y=241;circle(r,x-6,y-6,9,c);circle(r,x+6,y-6,9,c);circle(r,x,y+2,11,c);}
    txt(r,small,"MOVES",669,277,{180,185,213});txt(r,regular,std::to_string(moves),669,301,{249,238,247});
    txt(r,small,"TOTAL SCORE",798,277,{180,185,213});txt(r,regular,std::to_string(score),798,301,{249,238,247});
    txt(r,small,"SHORTEST PATH: "+std::to_string((int)hint.size()-1)+" moves",669,335,{255,217,132});
    txt(r,small,"CONTROLS",669,360,{180,185,213});txt(r,small,"Arrow keys or ZQSD / WASD",669,390,{239,228,247});
    txt(r,small,"Pink: player   Gold: exit",669,424,{239,228,247});txt(r,small,"Red monsters: -1 heart",669,449,{250,157,173});
    txt(r,small,"Coins: "+std::to_string(coinsTaken)+" (+50 each)",669,468,{255,217,132});
    txt(r,small,"N: restart   H: hint   1/2/3: level",669,493,{178,186,217});
    int cell=std::min(34,std::min(574/maze.width(),354/maze.height()));int ox=24+(610-maze.width()*cell)/2,oy=127+(398-maze.height()*cell)/2;
    for(int y=0;y<maze.height();++y)for(int x=0;x<maze.width();++x){int i=y*maze.width()+x,px=ox+x*cell,py=oy+y*cell;rect(r,px+1,py+1,cell-2,cell-2,(x+y)%2?Color{48,51,85}:Color{53,55,92});
      if(showHint&&std::find(hint.begin(),hint.end(),i)!=hint.end()&&i!=player)circle(r,px+cell/2,py+cell/2,std::max(2,cell/7),{95,211,220});
      if(i==maze.width()*maze.height()-1){circle(r,px+cell/2,py+cell/2,std::max(5,cell/3),{251,204,105});circle(r,px+cell/2,py+cell/2,std::max(2,cell/7),{255,244,194});}
      if(std::find(coins.begin(),coins.end(),i)!=coins.end()){
        int cx=px+cell/2,cy=py+cell/2+(int)(2*std::sin(t*5.f+i));
        circle(r,cx,cy,std::max(4,cell/4),{255,203,77});
        circle(r,cx,cy,std::max(2,cell/8),{255,244,164});
      }
      if(std::find(hazards.begin(),hazards.end(),i)!=hazards.end()){
        // Animated little monster: floating body, ears, eyes and blinking.
        int mx=px+cell/2,my=py+cell/2+(int)(2.5f*std::sin(t*4.f+i));
        int radius=std::max(5,cell/3);
        circle(r,mx,my,radius+2,{109,42,105});
        circle(r,mx,my,radius,{255,98,133});
        circle(r,mx-radius/2,my-radius+1,std::max(2,radius/3),{255,98,133});
        circle(r,mx+radius/2,my-radius+1,std::max(2,radius/3),{255,98,133});
        bool blink=std::fmod(t+i*.73f,3.7f)<.15f;
        if(blink){rect(r,mx-radius/2-2,my-1,4,2,{44,25,58});rect(r,mx+radius/2-2,my-1,4,2,{44,25,58});}
        else{circle(r,mx-radius/2,my-1,2,{37,24,51});circle(r,mx+radius/2,my-1,2,{37,24,51});}
        rect(r,mx-2,my+radius/3,4,2,{84,29,63});
      }
      Color wall={166,133,208};color(r,wall);if(maze.wall(x,y,0))SDL_RenderDrawLine(r,px,py,px+cell,py);if(maze.wall(x,y,1))SDL_RenderDrawLine(r,px+cell,py,px+cell,py+cell);if(maze.wall(x,y,2))SDL_RenderDrawLine(r,px,py+cell,px+cell,py+cell);if(maze.wall(x,y,3))SDL_RenderDrawLine(r,px,py,px,py+cell);
    }
    int px=ox+(player%maze.width())*cell+cell/2,py=oy+(player/maze.width())*cell+cell/2;
    bool flash=hitAt&&ticks-hitAt<650&&((ticks/90)%2==0);if(!flash){circle(r,px,py,std::max(5,cell/3)+(int)(1.5f*std::sin(t*5)),{255,131,190});circle(r,px-2,py-2,std::max(2,cell/9),{255,233,244});}
    int mx,my;SDL_GetMouseState(&mx,&my);for(int i=0;i<3;++i){auto b=buttons[i];bool hover=inside(mx,my,b.box);Color c=hover?Color{218,111,170}:Color{102,77,139};if(i==2&&level==2)c={85,89,116};rect(r,b.box.x,b.box.y,b.box.w,b.box.h,c);outline(r,b.box.x,b.box.y,b.box.w,b.box.h,{198,166,220});txt(r,small,b.label,b.box.x+17,b.box.y+13,{255,247,251});}
    txt(r,small,"Press 1, 2 or 3 to choose difficulty",633,561,{183,190,219});
    if(won||lost){rect(r,185,260,585,126,{25,25,48});outline(r,185,260,585,126,won?Color{120,230,187}:Color{251,130,154});txt(r,large,won?"LEVEL COMPLETE!":"GAME OVER",220,277,won?Color{143,242,197}:Color{255,147,167});txt(r,small,won?(bonus>0?"PERFECT PATH! +500 stars!":"Level cleared! Try the shortest path for +500 stars."):"Press N to try again",222,338,{242,228,246});
      if(won&&bonus>0){for(int j=0;j<18;++j){float a=j*6.2831853f/18.f+t*.5f;int sx=490+(int)(225*std::cos(a)),sy=320+(int)(52*std::sin(a));circle(r,sx,sy,2+(j%3),{255,214,98});}}}
    if(selectLevel){
      rect(r,120,110,740,430,{25,24,52});outline(r,120,110,740,430,{255,171,208});
      txt(r,large,"CHOOSE YOUR ADVENTURE",220,148,{255,202,225});
      txt(r,small,"Move with arrows or ZQSD. Collect golden coins, dodge monsters!",185,203,{220,212,243});
      for(int j=0;j<3;++j){int x=194+j*207;rect(r,x,265,186,145,j==0?Color{61,119,103}:j==1?Color{143,102,66}:Color{128,64,99});
        outline(r,x,265,186,145,{247,210,228});txt(r,regular,j==0?"1  EASY":j==1?"2  MEDIUM":"3  HARD",x+18,292,{255,244,250});
        txt(r,small,j==0?"3 monsters":j==1?"8 monsters":"17 monsters",x+18,332,{250,225,240});
        txt(r,small,j==0?"3 coins":j==1?"5 coins":"7 coins",x+18,360,{255,233,157});
      }
      txt(r,small,"Click a level or press 1 / 2 / 3 to start",290,460,{230,218,244});
    }
    SDL_RenderPresent(r);
  }
public:
  int run(){if(SDL_Init(SDL_INIT_VIDEO)!=0){std::cerr<<SDL_GetError()<<'\n';return 1;}if(TTF_Init()!=0){std::cerr<<TTF_GetError()<<'\n';SDL_Quit();return 1;}
    win=SDL_CreateWindow("Maze Bloom | C++ Maze Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,980,650,SDL_WINDOW_SHOWN);r=win?SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC):nullptr;if(!r&&win)r=SDL_CreateRenderer(win,-1,SDL_RENDERER_SOFTWARE);if(!r){std::cerr<<SDL_GetError()<<'\n';return 1;}
    large=font(27);regular=font(19);small=font(14);if(!large||!regular||!small)std::cerr<<"Font not found: text may be missing.\n";reset(0);selectLevel=true;bool running=true;Uint32 lastMonsterMove=SDL_GetTicks();while(running){SDL_Event e;while(SDL_PollEvent(&e)){if(e.type==SDL_QUIT)running=false;if(e.type==SDL_KEYDOWN&&!e.key.repeat){SDL_Keycode k=e.key.keysym.sym;if(k==SDLK_ESCAPE)running=false;else if(k==SDLK_n)reset(level);else if(k==SDLK_m)selectLevel=true;else if(k==SDLK_h)showHint=!showHint;else if(k==SDLK_1)reset(0);else if(k==SDLK_2)reset(1);else if(k==SDLK_3)reset(2);else if(k==SDLK_UP||k==SDLK_z||k==SDLK_w)move(0);else if(k==SDLK_RIGHT||k==SDLK_d)move(1);else if(k==SDLK_DOWN||k==SDLK_s)move(2);else if(k==SDLK_LEFT||k==SDLK_q||k==SDLK_a)move(3);}if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT){int x=e.button.x,y=e.button.y;if(selectLevel){for(int j=0;j<3;++j)if(x>=194+j*207&&x<380+j*207&&y>=265&&y<410)reset(j);}
        else if(inside(x,y,buttons[0].box))reset(level);else if(inside(x,y,buttons[1].box))showHint=!showHint;else if(inside(x,y,buttons[2].box)&&level<2)reset(level+1);}}
      Uint32 now=SDL_GetTicks();if(now-lastMonsterMove>=(Uint32)(900-level*180)){moveMonsters();lastMonsterMove=now;}
      draw();SDL_Delay(16);}
    TTF_CloseFont(large);TTF_CloseFont(regular);TTF_CloseFont(small);SDL_DestroyRenderer(r);SDL_DestroyWindow(win);TTF_Quit();SDL_Quit();return 0;
  }
};
int main(){Game g;return g.run();}
