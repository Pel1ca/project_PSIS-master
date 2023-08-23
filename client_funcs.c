#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "coms/coms.h"
#include "data.h"
#include "UI_library.h"
#include "board_func.h"
#include "game_rules.h"
#include "server.h"
#include "pac_lib.h"
#include "client_funcs.h"
#include "graphics.h"

static int running = 1;

void stop_running()
{
	running = 0;
}
void game_loop(int fd, color_t pl_color)
{
        SDL_Event event;
        cord_t end;
        cord_t beg = {-1, -1};
        int flag = 0;
        int ptype = MONSTER;

        while(running)
        {
                SDL_PollEvent(&event);
                //get user input
                switch(event.type)
                {
                        //exiting
                        case SDL_QUIT:
                                running = 0;
                                break;
                        case SDL_MOUSEMOTION:
                                get_board_place(event.motion.x, event.motion.y, &end.x, &end.y);
                                //corrects problem in UI library where coordinates cannot be negative
                                if (event.motion.x == 0 && event.motion.xrel < 0)
                                        end.x = -1;
                                if (event.motion.y == 0 && event.motion.yrel < 0)
                                        end.y = -1;
                                ptype = MONSTER;
                                flag = 0;
                                break;
                        case SDL_KEYDOWN:
                                flag = 1;
                                ptype = -1;
                                break;
                        case SDL_KEYUP:
                                if (flag == 1)
                                        handle_key_pres(event.key.keysym.sym, &end, &ptype);
                                else 
                                        ptype = -1;
                                flag = 0;
				break;  
                }
                if(ptype != -1)
                        send_mv(fd, ptype, beg, end, pl_color);
                SDL_Delay(2);
        }
}

void handle_key_pres(SDL_Keycode k_pressed, cord_t *end, int  *ptype)
{
        switch(k_pressed)
        {
                case SDLK_DOWN:
                        end->x = 0;
                        end->y = 1;     
                        *ptype = PACMAN;
                        break;
                case SDLK_UP:
                        end->x = 0;
                        end->y = -1;
                        *ptype = PACMAN;
                        break;
                case SDLK_LEFT:
                        end->x = -1;
                        end->y = 0;
                        *ptype = PACMAN;
                        break ; 
                case SDLK_RIGHT:
                        end->x = 1;
                        end->y = 0;
                        *ptype = PACMAN;
                        break ;
                default:
                        *ptype = -1;
        }
} 

