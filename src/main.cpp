#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <fmodex/fmod.h>

struct Enemy
{
    SDL_Surface *s_enemy;
    SDL_Rect    r_enemy;
    int         t_size;
    int         x_spawn;
    int         t_speed;
};

struct Button
{
    SDL_Surface *s_button;
    SDL_Surface *s_button_r;
    SDL_Surface *s_button_ct;
    SDL_Rect    r_button;
    SDL_Rect    r_button_r;
    SDL_Rect    r_button_ct;
};

char *nb_to_str(int nb)
{
    char str[128];
    sprintf(str, "%d", nb);
    return str;
}

char *nb_to_str_encode(int nb)
{
    char str[128];
    sprintf(str, "%d", nb);
    for(int i=0;i<strlen(str);i++){str[i]+=8;}
    return str;
}

int nb_to_str_decode(int nb, FILE *f)
{
    char str[128];
    fgets(str,sizeof(str),f);
    for(int i=0;i<strlen(str);i++){str[i]-=8;}
    return (nb = atoi(str));
}

struct  Enemy   e_enemy_1;
struct  Enemy   e_enemy_2;
struct  Button  b_play;
struct  Button  b_quit;
struct  Button  b_retry;
struct  Button  b_menu;
int width = 800, height = 600;

int isSelected(SDL_Rect r)
{
    int x, y;
    SDL_GetMouseState(&x,&y);
    if(x >= r.x && x <= r.x + 250 + 30 && y >= r.y && y <= r.y + 50 + 30)
    {
        return 1;
    }
    return 0;
}

void recreate_e_1()
{
    e_enemy_1.r_enemy.x = e_enemy_1.x_spawn;
    e_enemy_1.t_speed = rand() % 4 + 1;
    e_enemy_1.t_size = rand() % 30 + 19;
    e_enemy_1.r_enemy.y = (4*height)/5 - e_enemy_1.t_size;
    e_enemy_1.s_enemy = SDL_CreateRGBSurface(SDL_HWSURFACE, e_enemy_1.t_size, e_enemy_1.t_size, 32 ,0,0,0,0);
    SDL_FillRect(e_enemy_1.s_enemy, NULL, SDL_MapRGB((*e_enemy_1.s_enemy).format,128,0,0));
}

void recreate_e_2()
{
    e_enemy_2.r_enemy.x = e_enemy_2.x_spawn;
    e_enemy_2.t_speed = rand() % 4 + 1;
    e_enemy_2.t_size = rand() % 30 + 19;
    e_enemy_2.r_enemy.y = (4*height)/5 - e_enemy_2.t_size;
    e_enemy_2.s_enemy = SDL_CreateRGBSurface(SDL_HWSURFACE, e_enemy_2.t_size, e_enemy_2.t_size, 32 ,0,0,0,0);
    SDL_FillRect(e_enemy_2.s_enemy, NULL, SDL_MapRGB((*e_enemy_2.s_enemy).format,128,0,0));
}

int check_collision(int x, int y, int w, int h, int i, int j, int w_2, int h_2)
{
    int dx = abs(x - i);
    int dy = y + h;
    //int dd = dy - h_2;

    if(dx < w && i + w_2 > x && dy < j + 5 && dy > j - 5)
    {
        return 2;
    }
    if(dx < w && i + w_2 > x && dy > j)
    {
        return 1;
    }
    return 0;
}

int main (int argc, char** argv)
{
    if(SDL_Init(SDL_INIT_VIDEO))
        fprintf(stderr,"Error on: %s\n", SDL_GetError());
    if(TTF_Init() == -1)
    {
        fprintf(stderr, "Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    FMOD_SYSTEM *sys;
    FMOD_System_Create(&sys);
    FMOD_System_Init(sys, 6, FMOD_INIT_NORMAL, NULL);

    //OUR DIFFERENTS SOUNDS : jump / score++ / life-- / buttons selection / button clic / background music
    FMOD_SOUND *jmp = NULL;
    FMOD_SOUND *inc = NULL;
    FMOD_SOUND *life_less = NULL;
    FMOD_SOUND *select = NULL;
    FMOD_SOUND *clic = NULL;
    FMOD_SOUND *music = NULL;

    // WE LOAD OUR SOUNDS
    FMOD_System_CreateSound(sys, "music.mp3", FMOD_2D | FMOD_CREATESTREAM | FMOD_SOFTWARE | FMOD_LOOP_NORMAL , NULL, &music);
    FMOD_CHANNEL *chan = NULL;
    FMOD_System_GetChannel(sys, 0, &chan);
    //FMOD_Channel_SetVolume(chan, 0.5f);
    //FMOD_Sound_SetLoopCount(music, -1);

    FMOD_System_CreateSound(sys, "jmp.wav", FMOD_CREATESAMPLE, 0, &jmp);
    FMOD_System_CreateSound(sys, "score.wav", FMOD_CREATESAMPLE, 0, &inc);
    FMOD_System_CreateSound(sys, "hit.wav", FMOD_CREATESAMPLE, 0, &life_less);
    FMOD_System_CreateSound(sys, "select.wav", FMOD_CREATESAMPLE, 0, &select);
    FMOD_System_CreateSound(sys, "clic.wav", FMOD_CREATESAMPLE, 0, &clic);


    SDL_Surface *s_screen = NULL; // main screen (== background)
    s_screen = SDL_SetVideoMode(width,height,32,SDL_HWSURFACE);
    // 50 50 200 are the color of the background
    int color1 = 50;
    int color2 = 50;
    int color3 = 200;
    int max_c1 = 75;
    int max_c2 = 75;
    int max_c3 = 225;
    int min_c1 = 25;
    int min_c2 = 25;
    int min_c3 = 175;
    SDL_FillRect(s_screen, NULL, SDL_MapRGB((*s_screen).format,color1,color2,color3));

    // LE SOL
    SDL_Surface *s_ground = NULL;
    s_ground = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height/5, 32 ,0,0,0,0);
    SDL_FillRect(s_ground, NULL, SDL_MapRGB((*s_ground).format, 0,128,0));
    SDL_Rect r_ground;
    r_ground.x = 0;
    r_ground.y = (4*height)/5;
    SDL_BlitSurface(s_ground, NULL, s_screen, &r_ground);

    // LA TERRE
    SDL_Surface *s_dirt = NULL;
    s_dirt = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height/5 + 50, 32 ,0,0,0,0);
    SDL_FillRect(s_dirt, NULL, SDL_MapRGB((*s_dirt).format, 120,70,40));
    SDL_Rect r_dirt;
    r_dirt.x = 0;
    r_dirt.y = (4*height)/5 + 50;
    SDL_BlitSurface(s_dirt, NULL, s_screen, &r_dirt);

    // OUR MAIN CHARACTER
    SDL_Surface *s_square = NULL;
    s_square = SDL_CreateRGBSurface(SDL_HWSURFACE, width/10, height/10, 32, 0,0,0,0);
    SDL_FillRect(s_square, NULL, SDL_MapRGB((*s_square).format,0,255,0));
    SDL_Rect r_square;
    r_square.x = width/2 - width/20;
    r_square.y = (4*height)/5 - height/10;
    SDL_Rect o_r_square;
    o_r_square.x = r_square.x;
    o_r_square.y = r_square.y;
    SDL_BlitSurface(s_square, NULL, s_screen, &r_square);

    // SUN
    SDL_Surface *s_sun = NULL;
    s_sun = SDL_LoadBMP("sun.bmp");
    SDL_Rect r_sun;
    r_sun.x = 0;
    r_sun.y = 0;
    SDL_SetColorKey(s_sun, SDL_SRCCOLORKEY, SDL_MapRGB((*s_sun).format,255,255,255));
    SDL_BlitSurface(s_sun, NULL, s_screen, &r_sun);
    int a_sun = 253;

    // MOON
    SDL_Surface *s_moon = NULL;
    s_moon = SDL_LoadBMP("moon.bmp");
    SDL_Rect r_moon;
    r_moon.x = 0;
    r_moon.y = 0;
    SDL_SetColorKey(s_moon, SDL_SRCCOLORKEY, SDL_MapRGB((*s_moon).format,255,255,255));
    SDL_BlitSurface(s_moon, NULL, s_screen, &r_moon);
    int a_moon = 1;

    // ENEMY LEFT SIDE
    e_enemy_1.s_enemy = NULL;
    e_enemy_1.t_size = rand() % 30 + 19;
    e_enemy_1.s_enemy = SDL_CreateRGBSurface(SDL_HWSURFACE, e_enemy_1.t_size, e_enemy_1.t_size, 32 ,0,0,0,0);
    SDL_FillRect(e_enemy_1.s_enemy, NULL, SDL_MapRGB((*e_enemy_1.s_enemy).format,128,0,0));
    e_enemy_1.r_enemy.x = -e_enemy_1.t_size;
    e_enemy_1.r_enemy.y = (4*height)/5 - e_enemy_1.t_size;
    e_enemy_1.x_spawn = e_enemy_1.r_enemy.x;
    e_enemy_1.t_speed = rand() % 3 + 1;

    // ENEMY RIGHT SIDE
    e_enemy_2.s_enemy = NULL;
    e_enemy_2.t_size = rand() % 30 + 19;
    e_enemy_2.s_enemy = SDL_CreateRGBSurface(SDL_HWSURFACE, e_enemy_2.t_size, e_enemy_2.t_size, 32 ,0,0,0,0);
    SDL_FillRect(e_enemy_2.s_enemy, NULL, SDL_MapRGB((*e_enemy_2.s_enemy).format,128,0,0));
    e_enemy_2.r_enemy.x = width;
    e_enemy_2.r_enemy.y = (4*height)/5 - e_enemy_2.t_size;
    e_enemy_2.x_spawn = e_enemy_2.r_enemy.x;
    e_enemy_2.t_speed = rand() % 3 + 1;

    //LIFE
    int unsigned life = 10;
    SDL_Surface *s_life = NULL;
    // life * (width - width/10)/10
    s_life = SDL_CreateRGBSurface(SDL_HWSURFACE, life * (width - width/10)/10 , height/50, 32 ,0,0,0,0);
    SDL_FillRect(s_life, NULL, SDL_MapRGB((*s_life).format, 0,255,0));
    SDL_Rect r_life;
    r_life.x = width/20;
    r_life.y = height/50;
    SDL_BlitSurface(s_life, NULL, s_screen, &r_life);

    //DEAD LIFE
    SDL_Surface *s_dead_life = NULL;
    s_dead_life = SDL_CreateRGBSurface(SDL_HWSURFACE, width - width/10, height/50, 32 ,0,0,0,0);
    SDL_FillRect(s_dead_life, NULL, SDL_MapRGB((*s_dead_life).format, 255,0,0));
    SDL_Rect r_dead_life;
    r_dead_life.x = width/20;
    r_dead_life.y = height/50;
    SDL_BlitSurface(s_dead_life, NULL, s_screen, &r_dead_life);

    //Contour LIFE
    SDL_Surface *s_contour_life = NULL;
    s_contour_life = SDL_CreateRGBSurface(SDL_HWSURFACE, width - width/10 + width/50, height/50 + height/50, 32 ,0,0,0,0);
    SDL_FillRect(s_contour_life, NULL, SDL_MapRGB((*s_contour_life).format, 0,0,0));
    SDL_Rect r_contour_life;
    r_contour_life.x = width/20 - width/100;
    r_contour_life.y = height/50 - height/100;
    SDL_BlitSurface(s_contour_life, NULL, s_screen, &r_contour_life);

    // SCORE
    SDL_Surface *s_score = NULL;
    SDL_Rect r_score;
    r_score.x = width/2;
    r_score.y = height/15;
    TTF_Font *ttf_police = NULL;
    ttf_police = TTF_OpenFont("police.ttf", 50);

    // GAME OVER TEXT
    SDL_Color black = {255, 255, 255};
    SDL_Surface *s_game_over = NULL;
    SDL_Rect r_game_over;
    r_game_over.x = 2*width/9 + 5;
    r_game_over.y = height/2;
    s_game_over = TTF_RenderText_Solid(ttf_police, "GAME OVER", black);

    // NEW TEXT
    SDL_Color white = {255, 255, 255};
    SDL_Surface *s_new = NULL;
    SDL_Rect r_new;
    r_new.x = r_score.x + 50;
    r_new.y = r_score.y + 50;
    s_new = TTF_RenderText_Solid(ttf_police, "NEW", white);

    // CADRE NEW
    SDL_Surface *s_new_ct = NULL;
    s_new_ct = SDL_CreateRGBSurface(SDL_HWSURFACE, 165, 55, 32 ,0,0,0,0);
    SDL_FillRect(s_new_ct, NULL, SDL_MapRGB((*s_new_ct).format, 255,0,0));
    SDL_Rect r_new_ct;
    r_new_ct.x = r_score.x + 45;
    r_new_ct.y = r_score.y + 45;
    SDL_BlitSurface(s_new_ct, NULL, s_screen, &r_new_ct);

    // GAME OVER TEXT CONTOUR
    SDL_Color real_black = {0, 0, 0};
    SDL_Surface *s_game_over2 = NULL;
    SDL_Rect r_game_over2;
    r_game_over2.x = 2*width/9 + 10;
    r_game_over2.y = height/2;
    SDL_Rect r_game_over2_max;
    r_game_over2_max.x = 2*width/9 - 15;
    r_game_over2_max.y = height/2 - 15;
    SDL_Rect r_game_over2_min;
    r_game_over2_min.x = 2*width/9 + 15;
    r_game_over2_min.y = height/2 + 15;
    s_game_over2 = TTF_RenderText_Solid(ttf_police, "GAME OVER", real_black);

    // ALL THE BUTTONS
    // RETRY
    b_retry.s_button = TTF_RenderText_Solid(ttf_police, "RETRY", white);
    b_retry.r_button.x = width/8;
    b_retry.r_button.y = height/3;
    // BOX
    b_retry.s_button_r = SDL_CreateRGBSurface(SDL_HWSURFACE, b_retry.r_button.x + 20 + 3*50, 70, 32 ,0,0,0,0);
    SDL_FillRect(b_retry.s_button_r, NULL, SDL_MapRGB((*b_retry.s_button_r).format, 244,153,51));
    b_retry.r_button_r.x = b_retry.r_button.x - 10;
    b_retry.r_button_r.y = b_retry.r_button.y - 10;
    // CT BOX
    b_retry.s_button_ct = SDL_CreateRGBSurface(SDL_HWSURFACE, b_retry.r_button.x + 30 + 3*50, 80, 32 ,0,0,0,0);
    SDL_FillRect(b_retry.s_button_ct, NULL, SDL_MapRGB((*b_retry.s_button_ct).format, 0,0,0));
    b_retry.r_button_ct.x = b_retry.r_button.x - 15;
    b_retry.r_button_ct.y = b_retry.r_button.y - 15;

    // QUIT
    b_quit.s_button = TTF_RenderText_Solid(ttf_police, "QUIT", white);
    b_quit.r_button.x = width/2 - 100;
    b_quit.r_button.y = 2*height/5;
    // BOX
    b_quit.s_button_r = SDL_CreateRGBSurface(SDL_HWSURFACE, b_quit.r_button.x/5 + 20 + 3*50, 70, 32 ,0,0,0,0);
    SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244,153,51));
    b_quit.r_button_r.x = b_quit.r_button.x - 10;
    b_quit.r_button_r.y = b_quit.r_button.y - 10;
    // CT BOX
    b_quit.s_button_ct = SDL_CreateRGBSurface(SDL_HWSURFACE, b_quit.r_button.x/5 + 30 + 3*50, 80, 32 ,0,0,0,0);
    SDL_FillRect(b_quit.s_button_ct, NULL, SDL_MapRGB((*b_quit.s_button_ct).format, 0,0,0));
    b_quit.r_button_ct.x = b_quit.r_button.x - 15;
    b_quit.r_button_ct.y = b_quit.r_button.y - 15;

    // PLAY
    b_play.s_button = TTF_RenderText_Solid(ttf_police, "PLAY", white);
    b_play.r_button.x = width/2 - 100;
    b_play.r_button.y = height/5;
    // BOX
    b_play.s_button_r = SDL_CreateRGBSurface(SDL_HWSURFACE, b_play.r_button.x/5 + 20 + 3*50, 70, 32 ,0,0,0,0);
    SDL_FillRect(b_play.s_button_r, NULL, SDL_MapRGB((*b_play.s_button_r).format, 244,153,51));
    b_play.r_button_r.x = b_play.r_button.x - 10;
    b_play.r_button_r.y = b_play.r_button.y - 10;
    // CT BOX
    b_play.s_button_ct = SDL_CreateRGBSurface(SDL_HWSURFACE, b_play.r_button.x/5 + 30 + 3*50, 80, 32 ,0,0,0,0);
    SDL_FillRect(b_play.s_button_ct, NULL, SDL_MapRGB((*b_play.s_button_ct).format, 0,0,0));
    b_play.r_button_ct.x = b_play.r_button.x - 15;
    b_play.r_button_ct.y = b_play.r_button.y - 15;

    int jump_is_on = 0;
    int jump_is_up = 0;
    int jump_height = 200;
    int now = 105;
    int before = 100;
    int interval = 10;
    int interval2 = 100;
    int before2 = 100;
    int now2 = 105;
    int speed = 5;
    int speed_jump = 7;
    int left = 0;
    int right = 0;
    int score = 0;
    int isKeyUp = 0;
    int isKeyDown = 0;
    int isOldKeyUp = 0;
    int isOldKeyDown = 0;

    int offset_sun = 2;
    int offset_moon = -2;

    int isMouseOn_1 = 0;
    int isMouseOff_1 = 0;
    int isOldMouseOn_1 = 0;
    int isOldMouseOff_1 = 0;

    int isMouseOn_2 = 0;
    int isMouseOff_2 = 0;
    int isOldMouseOn_2 = 0;
    int isOldMouseOff_2 = 0;

    int loose = 0;

    FILE *save_file = NULL;
    save_file = fopen("save.theoricien", "a+");
    int old_score = nb_to_str_decode(old_score,save_file);

    FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, music, 0, NULL);

    SDL_Event event;

    // Main Loop
    int choice = 0;
    while(1)
    {
        // MENU
        while(choice == 0)
        {
            now = SDL_GetTicks();

            SDL_SetAlpha(s_sun, SDL_SRCALPHA, a_sun);
            SDL_SetAlpha(s_moon, SDL_SRCALPHA, a_moon);

            SDL_FillRect(s_screen, NULL, SDL_MapRGB((*s_screen).format,color1,color2,color3));
            SDL_BlitSurface(s_moon, NULL, s_screen, &r_moon);
            SDL_BlitSurface(s_sun, NULL, s_screen, &r_sun);
            SDL_BlitSurface(s_square, NULL, s_screen, &r_square);
            SDL_BlitSurface(s_ground, NULL, s_screen, &r_ground);
            SDL_BlitSurface(s_dirt, NULL, s_screen, &r_dirt);
            // buttons
            SDL_BlitSurface(b_play.s_button_ct, NULL, s_screen, &b_play.r_button_ct);
            SDL_BlitSurface(b_play.s_button_r, NULL, s_screen, &b_play.r_button_r);
            SDL_BlitSurface(b_play.s_button, NULL, s_screen, &b_play.r_button);

            SDL_BlitSurface(b_quit.s_button_ct, NULL, s_screen, &b_quit.r_button_ct);
            SDL_BlitSurface(b_quit.s_button_r, NULL, s_screen, &b_quit.r_button_r);
            SDL_BlitSurface(b_quit.s_button, NULL, s_screen, &b_quit.r_button);

            SDL_Flip(s_screen);

            SDL_PollEvent(&event);
            if(event.type == SDL_QUIT)
            {
                choice = 42;
            }

            if(isSelected(b_play.r_button_ct) == 1)
            {
                isMouseOn_1 = 1;
                isMouseOff_1 = 0;
                if(isMouseOn_1 == 1 && isOldMouseOff_1 == 1)
                    FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, select, 0, NULL);
                SDL_FillRect(b_play.s_button_r, NULL, SDL_MapRGB((*b_play.s_button_r).format, 244 - 30,153 - 30,51 - 30));
                if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_FillRect(b_play.s_button_r, NULL, SDL_MapRGB((*b_play.s_button_r).format, 244 - 50,153 - 50,51 - 50));
                    choice = 1;
                }
            }
            else
            {
                isMouseOn_1 = 0;
                isMouseOff_1 = 1;
                SDL_FillRect(b_play.s_button_r, NULL, SDL_MapRGB((*b_play.s_button_r).format, 244,153,51));
            }

            if(isSelected(b_quit.r_button_ct) == 1)
            {
                isMouseOn_2 = 1;
                isMouseOff_2 = 0;
                if(isMouseOn_2 == 1 && isOldMouseOff_2 == 1)
                    FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, select, 0, NULL);
                SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244 - 30,153 - 30,51 - 30));
                if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    choice = 42; // END THE PROGRAM
                }
            }
            else
            {
                isMouseOn_2 = 0;
                isMouseOff_2 = 1;
                SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244,153,51));
            }


            isOldMouseOff_1 = isMouseOff_1;
            isOldMouseOn_1 = isMouseOn_1;

            isOldMouseOff_2 = isMouseOff_2;
            isOldMouseOn_2 = isMouseOn_2;

            if(now-before<interval)
            {
                SDL_Delay(interval-(now-before));
            }
            before = now;
        }

        // GAME
        if(choice == 1)
        {
            life = 10;
            s_life = SDL_CreateRGBSurface(SDL_HWSURFACE, life * (width - width/10)/10 , height/50, 32 ,0,0,0,0);
            SDL_FillRect(s_life, NULL, SDL_MapRGB((*s_life).format, 0,255,0));
            r_square.x = width/2 - width/20;
            left = 0;
            right = 0;
            score = 0;
        }
        while(choice == 1)
        {
            FMOD_Channel_SetPaused(chan, 1);
            FMOD_Channel_SetVolume(chan, 1.0f);
            FMOD_Channel_SetPaused(chan, 0);

            // Refresh our screen so put blit
            now = SDL_GetTicks();
            now2 = SDL_GetTicks();

            SDL_SetAlpha(s_sun, SDL_SRCALPHA, a_sun);
            SDL_SetAlpha(s_moon, SDL_SRCALPHA, a_moon);

            // THE GAME
            SDL_FillRect(s_screen, NULL, SDL_MapRGB((*s_screen).format,color1,color2,color3));
            SDL_BlitSurface(s_moon, NULL, s_screen, &r_moon);
            SDL_BlitSurface(s_sun, NULL, s_screen, &r_sun);
            SDL_BlitSurface(s_square, NULL, s_screen, &r_square);
            SDL_BlitSurface(s_ground, NULL, s_screen, &r_ground);
            SDL_BlitSurface(s_dirt, NULL, s_screen, &r_dirt);
            SDL_BlitSurface(e_enemy_1.s_enemy, NULL, s_screen, &e_enemy_1.r_enemy);
            SDL_BlitSurface(e_enemy_2.s_enemy, NULL, s_screen, &e_enemy_2.r_enemy);
            SDL_BlitSurface(s_contour_life, NULL, s_screen, &r_contour_life);
            SDL_BlitSurface(s_dead_life, NULL, s_screen, &r_dead_life);
            SDL_BlitSurface(s_life, NULL, s_screen, &r_life);
            SDL_BlitSurface(s_score, NULL, s_screen, &r_score); /* Blit du texte */
            SDL_Flip(s_screen);

            // Event Session
            SDL_Event event;
            SDL_PollEvent(&event);
            if(event.type == SDL_QUIT)
                choice = 42;
            if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.sym == SDLK_RIGHT)
                    right = 1;
                if(event.key.keysym.sym == SDLK_LEFT)
                    left = 1;
                if(event.key.keysym.sym == SDLK_SPACE && jump_is_up == 0)
                {
                    jump_is_on = 1;
                    isKeyDown = 1;
                    isKeyUp = 0;
                }
            }
            if(event.type == SDL_KEYUP)
            {
                if(event.key.keysym.sym == SDLK_RIGHT)
                    right = 0;
                if(event.key.keysym.sym == SDLK_LEFT)
                    left = 0;
                if(event.key.keysym.sym == SDLK_SPACE)
                {
                    isKeyDown = 0;
                    isKeyUp = 1;
                }
            }


            if(isKeyDown == 1 && isOldKeyUp == 1)
            {
                FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, jmp, 0, NULL);
            }

            if(left == 1 && r_square.x >= 0)
            {
                r_square.x -= speed;
            }
            if(right == 1 && r_square.x <= width - width/10)
            {
                r_square.x += speed;
            }

            // I jump
            if(jump_is_on == 1)
            {
                r_square.y -= speed_jump;
                if(r_square.y <= o_r_square.y - jump_height)
                {
                    jump_is_on = 0;
                    jump_is_up = 1;
                }
            }
            if(jump_is_up == 1)
            {
                r_square.y += speed_jump;
                if(r_square.y > o_r_square.y)
                {
                    jump_is_up = 0;
                }
            }

            switch(check_collision(r_square.x, r_square.y, width/10, height/10, e_enemy_1.r_enemy.x, e_enemy_1.r_enemy.y, e_enemy_1.t_size, e_enemy_1.t_size))
            {
            case 1:
                recreate_e_1();
                life--;
                s_life = SDL_CreateRGBSurface(SDL_HWSURFACE, life * (width - width/10)/10 , height/50, 32 ,0,0,0,0);
                SDL_FillRect(s_life, NULL, SDL_MapRGB((*s_life).format, 0,255,0));
                FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, life_less, 0, NULL);
                break;
            case 2:
                recreate_e_1();
                score++;
                FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, inc, 0, NULL);
                break;
            }

            switch(check_collision(r_square.x, r_square.y, width/10, height/10, e_enemy_2.r_enemy.x, e_enemy_2.r_enemy.y, e_enemy_2.t_size, e_enemy_2.t_size))
            {
            case 1:
                recreate_e_2();
                life--;
                s_life = SDL_CreateRGBSurface(SDL_HWSURFACE, life * (width - width/10)/10 , height/50, 32 ,0,0,0,0);
                SDL_FillRect(s_life, NULL, SDL_MapRGB((*s_life).format, 0,255,0));
                FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, life_less, 0, NULL);
                break;
            case 2:
                recreate_e_2();
                score++;
                FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, inc, 0, NULL);
                break;
            }

            if(e_enemy_1.r_enemy.x >= width)
            {
                recreate_e_1();
            }

            if(e_enemy_2.r_enemy.x <= 0)
            {
                recreate_e_2();
            }

            // ENEMYS ARE MOVING
            e_enemy_1.r_enemy.x += e_enemy_1.t_speed;
            e_enemy_2.r_enemy.x -= e_enemy_2.t_speed;

            if(now-before<interval)
            {
                SDL_Delay(interval-(now-before));
            }
            if(now2 - before2 > interval2)
            {
                if(a_sun == 253 || a_sun == 1)
                    offset_sun = -offset_sun;
                if(a_moon == 253 || a_moon == 1)
                    offset_moon = -offset_moon;
                a_sun += offset_sun;
                a_moon += offset_moon;
                if(offset_sun < 0 && color1 != max_c1)
                {
                    color1++;
                }
                if(offset_sun < 0 && color2 != max_c2)
                {
                    color2++;
                }
                if(offset_sun < 0 && color3 != max_c3)
                {
                    color3++;
                }
                if(offset_sun > 0 && color1 != min_c1)
                {
                    color1--;
                }
                if(offset_sun > 0 && color2 != min_c2)
                {
                    color2--;
                }
                if(offset_sun > 0 && color3 != min_c3)
                {
                    color3--;
                }
                before2 = now2;
            }
            before = now;

            if(life == 0)
            {
                loose = 1;
                break;
            }
            s_score = TTF_RenderText_Solid(ttf_police, nb_to_str(score), black);
            isOldKeyUp = isKeyUp;
            isOldKeyDown = isKeyDown;
        }

        // IM LOOSING
        if(loose == 1)
        {
            int q = 0;
            int now = 105;
            int before = 100;
            int interval = 10;
            int sx = 1, sy = 1;

            isMouseOn_1 = 0;
            isMouseOff_1 = 0;
            isOldMouseOn_1 = 0;
            isOldMouseOff_1 = 0;

            isMouseOn_2 = 0;
            isMouseOff_2 = 0;
            isOldMouseOn_2 = 0;
            isOldMouseOff_2 = 0;

            // quit buttons
            b_quit.r_button.x = 5*width/8 ;
            b_quit.r_button.y = height/3;
            b_quit.r_button_r.x = b_quit.r_button.x - 10;
            b_quit.r_button_r.y = b_quit.r_button.y - 10;
            b_quit.r_button_ct.x = b_quit.r_button.x - 15;
            b_quit.r_button_ct.y = b_quit.r_button.y - 15;

            if(score > old_score)
                fprintf(save_file, nb_to_str_encode(score));

            SDL_WarpMouse(width/2, height/2);

            while(loose == 1)
            {
                SDL_SetAlpha(s_sun, SDL_SRCALPHA, a_sun);
                SDL_SetAlpha(s_moon, SDL_SRCALPHA, a_moon);

                SDL_FillRect(s_screen, NULL, SDL_MapRGB((*s_screen).format,color1,color2,color3));
                SDL_BlitSurface(s_moon, NULL, s_screen, &r_moon);
                SDL_BlitSurface(s_sun, NULL, s_screen, &r_sun);
                SDL_BlitSurface(s_square, NULL, s_screen, &r_square);
                SDL_BlitSurface(s_ground, NULL, s_screen, &r_ground);
                SDL_BlitSurface(s_dirt, NULL, s_screen, &r_dirt);
                SDL_BlitSurface(e_enemy_1.s_enemy, NULL, s_screen, &e_enemy_1.r_enemy);
                SDL_BlitSurface(e_enemy_2.s_enemy, NULL, s_screen, &e_enemy_2.r_enemy);
                SDL_BlitSurface(s_contour_life, NULL, s_screen, &r_contour_life);
                SDL_BlitSurface(s_dead_life, NULL, s_screen, &r_dead_life);
                SDL_BlitSurface(s_life, NULL, s_screen, &r_life);
                SDL_BlitSurface(s_score, NULL, s_screen, &r_score); /* Blit du texte */
                // BUTTONS
                SDL_BlitSurface(b_retry.s_button_ct, NULL, s_screen, &b_retry.r_button_ct);
                SDL_BlitSurface(b_retry.s_button_r, NULL, s_screen, &b_retry.r_button_r);
                SDL_BlitSurface(b_retry.s_button, NULL, s_screen, &b_retry.r_button);

                SDL_BlitSurface(b_quit.s_button_ct, NULL, s_screen, &b_quit.r_button_ct);
                SDL_BlitSurface(b_quit.s_button_r, NULL, s_screen, &b_quit.r_button_r);
                SDL_BlitSurface(b_quit.s_button, NULL, s_screen, &b_quit.r_button);

                if(score > old_score)
                {
                    SDL_BlitSurface(s_new_ct, NULL, s_screen, &r_new_ct);
                    SDL_BlitSurface(s_new, NULL, s_screen, &r_new);
                }
                SDL_BlitSurface(s_game_over2, NULL, s_screen, &r_game_over2); /* Blit du texte */
                SDL_BlitSurface(s_game_over, NULL, s_screen, &r_game_over); /* Blit du texte */
                SDL_Flip(s_screen);

                if(r_game_over2.x == r_game_over2_max.x)
                    sx = -sx;
                if(r_game_over2.y == r_game_over2_max.y)
                    sy = -sy;
                if(r_game_over2.x == r_game_over2_min.x)
                    sx = -sx;
                if(r_game_over2.y == r_game_over2_min.y)
                    sy = -sy;

                r_game_over2.x += sx;
                r_game_over2.y += sy;

                SDL_PollEvent(&event);
                if(event.type == SDL_QUIT)
                {
                    choice = 42;
                    loose = 0;
                }

                if(isSelected(b_quit.r_button_ct) == 1)
                {
                    isMouseOn_1 = 1;
                    isMouseOff_1 = 0;
                    if(isMouseOn_1 == 1 && isOldMouseOff_1 == 1)
                        FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, select, 0, NULL);
                    SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244 - 30,153 - 30,51 - 30));
                    if(event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244 - 50,153 - 50,51 - 50));
                        choice = 42;
                        loose = 0;
                    }
                }
                else
                {
                    isMouseOn_1 = 0;
                    isMouseOff_1 = 1;
                    SDL_FillRect(b_quit.s_button_r, NULL, SDL_MapRGB((*b_quit.s_button_r).format, 244,153,51));
                }

                if(isSelected(b_retry.r_button_ct) == 1)
                {
                    isMouseOn_2 = 1;
                    isMouseOff_2 = 0;
                    if(isMouseOn_2 == 1 && isOldMouseOff_2 == 1)
                        FMOD_System_PlaySound(sys, FMOD_CHANNEL_FREE, select, 0, NULL);
                    SDL_FillRect(b_retry.s_button_r, NULL, SDL_MapRGB((*b_retry.s_button_r).format, 244 - 30,153 - 30,51 - 30));
                    if(event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        SDL_FillRect(b_retry.s_button_r, NULL, SDL_MapRGB((*b_retry.s_button_r).format, 244 - 50,153 - 50,51 - 50));
                        choice = 1;
                        loose = 0;
                    }
                }
                else
                {
                    isMouseOn_2 = 0;
                    isMouseOff_2 = 1;
                    SDL_FillRect(b_retry.s_button_r, NULL, SDL_MapRGB((*b_retry.s_button_r).format, 244,153,51));
                }

                isOldMouseOff_1 = isMouseOff_1;
                isOldMouseOn_1 = isMouseOn_1;

                isOldMouseOff_2 = isMouseOff_2;
                isOldMouseOn_2 = isMouseOn_2;

                if(now-before<interval)
                {
                    SDL_Delay(interval-(now-before));
                }
                before = now;
            }
        }
        if(choice == 42)
            break;
    }

    if(score > old_score)
        fprintf(save_file, nb_to_str_encode(score));

    // Free our memory
    TTF_CloseFont(ttf_police);
    SDL_FreeSurface(s_screen);
    SDL_FreeSurface(e_enemy_1.s_enemy);
    SDL_FreeSurface(e_enemy_2.s_enemy);
    SDL_FreeSurface(s_life);
    SDL_FreeSurface(s_dead_life);
    SDL_FreeSurface(s_contour_life);
    SDL_FreeSurface(s_dirt);
    SDL_FreeSurface(s_ground);
    SDL_FreeSurface(s_moon);
    SDL_FreeSurface(s_square);
    SDL_FreeSurface(s_sun);
    SDL_FreeSurface(s_score);
    SDL_FreeSurface(s_game_over2);
    SDL_FreeSurface(s_game_over);
    SDL_FreeSurface(s_new);
    SDL_FreeSurface(s_new_ct);
    SDL_FreeSurface(b_retry.s_button);
    SDL_FreeSurface(b_retry.s_button_r);
    SDL_FreeSurface(b_retry.s_button_ct);
    FMOD_Sound_Release(life_less);
    FMOD_Sound_Release(jmp);
    FMOD_Sound_Release(inc);
    FMOD_Sound_Release(select);
    FMOD_Sound_Release(clic);
    FMOD_Sound_Release(music);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
