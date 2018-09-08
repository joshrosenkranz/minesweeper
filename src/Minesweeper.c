#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "SDL_image.h"
/*
To run, enter the following command into a terminal:
gcc Minesweeper.c -lSDL2_mixer -lSDL2_image -lSDL2_ttf `sdl2-config --cflags --libs` && ./a.out 
*/

typedef SDL_Texture* Texture;

typedef struct square_t {
    /*
    Squares are defined by their top left corner with (0,0) being the top 
    left corner
    */
    int x; //x-coordinate of the square 
    int y; //y-coordinate of the square
    int bomb; //0 if the square is clear, 1 if it is a bomb
    int bomb_neighbors; //Number of neighboring squares that are bombs
} Square;

Square makeSquare(int x, int y, int b, int n) {
    Square square = {x, y, b, n};
    return square;
}

void revealZeroes(int width, int height, Square board[][height], unsigned char hit[][height], int x, int y) {
    /*
    Helper function to reveal all adjacent zeroes if a zero square is found
    Modifies the board struct
    */
    hit[x][y] = true;
    if (board[x][y].bomb_neighbors == 0) {
        for (int xn = -1; xn < 2; xn++) {
            for (int yn = -1; yn < 2; yn++) {
                if (0 <= x+xn && x+xn < width && 0 <= y+yn && y+yn < height) {
                    if (hit[x+xn][y+yn] == false) {
                        revealZeroes(width, height, board, hit, x+xn, y+yn);
                    }
                }
            }
        }
    }
}


SDL_Renderer *renderer;

Texture loadImage(char* image){
    //Loads
    SDL_Surface *loadedImage = IMG_Load(image);
    if(!loadedImage) {
        printf("Failed to load image: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(1);
    }
    Texture texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
    SDL_FreeSurface(loadedImage);
    if(!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(1);
    }
    return texture;
}

void displayTexture(Texture t, unsigned x, unsigned y, unsigned width, unsigned height, SDL_RendererFlip flip){
    SDL_Rect tex_size;
    tex_size.x = 0;
    tex_size.y = 0;
    SDL_Rect toplace;
    toplace.x = x;
    toplace.y = y;
    toplace.w = width;
    toplace.h = height;
    SDL_QueryTexture(t, NULL, NULL, &tex_size.w, &tex_size.h);
    SDL_RenderCopyEx(renderer,t,&tex_size,&toplace,0,NULL,flip);
}

int main () {
    //Ask user to choose easy (9x9 with 10 mines), medium (16x16 with 40 mines),
    //  or hard (25x25 with 99 mines)
    printf("Choose a level (Easy, Medium or Hard): ");
    char level[8]; //Length of eight because longest answer is 7 characters (medium + newline) + new line
    char* input;
    srand((unsigned)time(NULL));
    int width;
    int height;
    int num_bombs;
    int valid_input = 0;
    //Take user input to determine difficulty
    while (valid_input == 0) {
        fflush(stdin);
        input = fgets(level, sizeof(level), stdin);
        if (strchr(input, '\n') != NULL) {
            if (strcmp(level, "Easy\n") == 0 || strcmp(level, "easy\n") == 0 || strcmp(level, "E\n") == 0 || strcmp(level, "e\n") == 0) {
                printf("You have chosen easy.\n");
                width = 9;
                height = 9;
                num_bombs = 10;
                valid_input = 1;
            }
            else if (strcmp(level, "Medium\n") == 0 || strcmp(level, "medium\n") == 0 || strcmp(level, "M\n") == 0 || strcmp(level, "m\n") == 0) {
                printf("You have chosen medium.\n");
                width = 16;
                height = 16;
                num_bombs = 40;
                valid_input = 1;
            }
            else if (strcmp(level, "Hard\n") == 0 || strcmp(level, "hard\n") == 0 || strcmp(level, "H\n") == 0 || strcmp(level, "h\n") == 0) {
                printf("You have chosen hard.\n");
                width = 25;
                height = 25;
                num_bombs = 99;
                valid_input = 1;
            }
            else {
                printf("Please choose either Easy, Medium, or Hard: ");
            }
        }
    }

    //Create a board with no bombs
    Square board[width][height];
    for (int m = 0; m < width; m++) {
        for (int n = 0; n < height; n++) {
            board[m][n] = makeSquare(m,n,0,0);
        }
    }

    //Add bombs to random squares, while making sure that multiple bombs are not placed on the same square
    for (int bomb_count = 0; bomb_count < num_bombs; bomb_count++) {
        int x_coord = rand() % width;
        int y_coord = rand() % height;
        while (board[x_coord][y_coord].bomb == 1) {
            x_coord = rand() % width;
            y_coord = rand() % height;
        }
        board[x_coord][y_coord] = makeSquare(x_coord, y_coord, 1, 0);
    }

    //Count the number of neighbors with bombs
    int a, b, x_neighbor, y_neighbor; //a determines row number, b determines column number
    for (a = 0; a < height; a++) {
        for (b = 0; b < width; b++) {
            for (x_neighbor = -1; x_neighbor < 2; x_neighbor++) {
                for (y_neighbor = -1; y_neighbor < 2; y_neighbor++) {
                    if (a+y_neighbor < height && a+y_neighbor > -1 && b+x_neighbor < width && b+x_neighbor > -1) {
                        if (board[a+y_neighbor][b+x_neighbor].bomb == 1) {
                            board[a][b].bomb_neighbors++;
                        }
                    }
                }
            }
        }
    }

    //Remove a bomb from the bomb count if the square has a bomb (because it was counted earlier)
    for (a = 0; a < height; a++) {
        for (b = 0; b < width; b++) {
            if (board[a][b].bomb == 1) {
                board[a][b].bomb_neighbors--;
            }
        }
    }

    //Print out the board in the terminal
    int print_board = 0;
    if (print_board == 1) {
        for (int a = 0; a < height; a++) {
            for (int b = 0; b < width; b++) {
                if (board[b][a].bomb == 1) {
                    printf("!");
                }
                else {
                    printf(" ");
                }
                printf("%d ", board[b][a].bomb_neighbors);
            }
            printf("\n");
        }
    }

    //GRAPHICS
    //Create window and renderer
    int window_scale = 25;
    unsigned window_x = width * window_scale;
    unsigned window_y = height * window_scale;
    unsigned window_start_x = 200;
    unsigned window_start_y = 100;
    char* window_name = "Minesweeper";
    SDL_Window *window = SDL_CreateWindow(window_name,window_start_x,window_start_y, window_x,window_y,SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    //Create an array of all images
    Texture text_array[] = {loadImage("../images/0.png"), loadImage("../images/1.png"), loadImage("../images/2.png"), loadImage("../images/3.png"), loadImage("../images/4.png"), loadImage("../images/5.png"), loadImage("../images/6.png"), loadImage("../images/7.png"), loadImage("../images/8.png"), loadImage("../images/flag.png"), loadImage("../images/bomb.png"), loadImage("../images/gameover.png"), loadImage("../images/win.png")};
    //Deal with mouse clicks
    SDL_Event e;
    int quit = 0;
    //Initialize arrays to store which squares have been chosen and are bombs, not bombs, or flagged
    unsigned char hit[width][height];
    //int *hit_p = hit[0];
    unsigned char bomb[width][height];
    unsigned char flag[width][height];
    for (int a = 0; a < height; a++) {
        for (int b = 0; b < width; b++) {
            hit[a][b] = false;
            bomb[a][b] = false;
            flag[a][b] = false;
        }
    }
    //Draw a blank background
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,0,124,124,SDL_ALPHA_OPAQUE);
    SDL_Rect background_rectangle;
    background_rectangle.x = 0;
    background_rectangle.y = 0;
    background_rectangle.w = window_x;
    background_rectangle.h = window_y;
    SDL_RenderFillRect(renderer, &background_rectangle);
    SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
    //Add lines to the background
    for (int a = 0; a < width; a++) {
        SDL_RenderDrawLine(renderer,a*window_scale,0,a*window_scale,25*window_scale);
    }
    for (int b = 0; b < height; b++) {
        SDL_RenderDrawLine(renderer,0,b*window_scale,25*window_scale,b*window_scale);
    }
    //Iterate through the hit, bomb, and flag arrays to create the board, then present the rendered image
    for (int square_y = 0; square_y < height; square_y++) {
        for (int square_x = 0; square_x < width; square_x++) {
            if(hit[square_x][square_y]) {
                char num = board[square_x][square_y].bomb_neighbors;
                Texture text = text_array[num];
                displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
            }
            else if (bomb[square_x][square_y]) {
                Texture text = text_array[10];
                displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
            }
            else if (flag[square_x][square_y]) {
                Texture text = text_array[9];
                displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
            }
        }
    }
    SDL_RenderPresent(renderer);
    //Create variables to store whether the user has won and how many non-bomb squares they have revealed
    int win;
    int end_screen = false;
    while (!quit) {
        if(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
                break;
            }
            //Take in the location of the mouse click and record the result of the click
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (win == 11 || win == 12) {
                    end_screen = true;
                }
                SDL_MouseButtonEvent mb = e.button;
                int square_x = mb.x/window_scale;
                int square_y = mb.y/window_scale;
                if (mb.button == SDL_BUTTON_LEFT) {
                    if (board[square_x][square_y].bomb == 1) {
                        bomb[square_x][square_y] = true;
                        win = 11; //Lose
                        //break;
                    }
                    else {
                        hit[square_x][square_y] = true;
                        if (board[square_x][square_y].bomb_neighbors == 0) {
                            revealZeroes(width, height, board, hit, square_x, square_y);                        
                        }
                    }   
                }
                else if (mb.button == SDL_BUTTON_RIGHT) {
                    flag[square_x][square_y] = !flag[square_x][square_y];
                }
                //Draw a blank background
                SDL_RenderClear(renderer);
                if (win == 11) {
                    SDL_SetRenderDrawColor(renderer,240,0,0,SDL_ALPHA_OPAQUE);
                } else {
                    SDL_SetRenderDrawColor(renderer,0,124,124,SDL_ALPHA_OPAQUE);
                }
                SDL_Rect background_rectangle;
                background_rectangle.x = 0;
                background_rectangle.y = 0;
                background_rectangle.w = window_x;
                background_rectangle.h = window_y;
                SDL_RenderFillRect(renderer, &background_rectangle);
                SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
                //Add lines to the background
                for (int a = 0; a < width; a++) {
                    SDL_RenderDrawLine(renderer,a*window_scale,0,a*window_scale,25*window_scale);
                }
                for (int b = 0; b < height; b++) {
                    SDL_RenderDrawLine(renderer,0,b*window_scale,25*window_scale,b*window_scale);
                }
                //Iterate through the hit, bomb, and flag arrays to create the board, then present the rendered image
                for (int square_y = 0; square_y < height; square_y++) {
                    for (int square_x = 0; square_x < width; square_x++) {
                        if(hit[square_x][square_y]) {
                            char num = board[square_x][square_y].bomb_neighbors;
                            Texture text = text_array[num];
                            displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
                        }
                        else if (bomb[square_x][square_y]) {
                            Texture text = text_array[10];
                            displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
                        }
                        else if (flag[square_x][square_y]) {
                            Texture text = text_array[9];
                            displayTexture(text, square_x*window_scale, square_y*window_scale, window_scale, window_scale, SDL_FLIP_NONE);
                        }
                    }
                }
                int hit_count = 0;
                for (int square_y = 0; square_y < height; square_y++) {
                    for (int square_x = 0; square_x < width; square_x++) {
                        if (hit[square_x][square_y]) {
                            hit_count += 1;
                        }
                    }
                }
                //Check after each click whether the user has won
                if (hit_count == width * height - num_bombs) {
                    win = 12; //Winner
                    SDL_SetRenderDrawColor(renderer,0,124,0,SDL_ALPHA_OPAQUE);
                }
                if (!end_screen) {
                    SDL_RenderPresent(renderer);
                }
            }
        }
        //Show the won or lost screen
        if (end_screen) {
            Texture text = text_array[win];
            displayTexture(text, 0, 0, width*window_scale, height*window_scale, SDL_FLIP_NONE);
            SDL_RenderPresent(renderer);
        }
    }
    return 0;
}