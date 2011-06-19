#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define DISPLAY_X 80
#define DISPLAY_Y 23

// ----------------------------------
// Function prototype
// 関数宣言
// ----------------------------------
void do_something(void);
void init_screen(void);
void str2screen(int, int, char[], int);
int arr_length(int[]);
void pay(void);
void *screen_view(void *arg);
void *real_view(void *arg);
void *game_model(void *arg);
void *help_model(void *arg);
void *slot_model(void *arg);
void *real1_model(void *arg);
void *real2_model(void *arg);
void *real3_model(void *arg);

// ----------------------------------
// Global variable
// グローバル変数
// ----------------------------------
char screen[DISPLAY_Y][DISPLAY_X];
char input='w';
char state[16] = "title";
int credit = 50;
int bet = 3;
int real1 = 0;
int real2 = 0;
int real3 = 0;

int main(void){
  struct termios save_term;
  struct termios temp_term;
  pthread_t draw;
  pthread_t title;
  char ch;


  /* 現在の端末設定を取得 */
  errno = 0;
  if(tcgetattr(fileno(stdin), &save_term) == -1){
    perror("tcgetattr failure");
    exit(EXIT_FAILURE);
  }else{
    temp_term = save_term;
  }

  /* 端末設定 */
  temp_term.c_iflag &= IGNCR;   /* 受信したCRを無視 */
  temp_term.c_lflag &= ~ICANON; /* カノニカルモードを外す */
  temp_term.c_lflag &= ~ECHO;   /* 入力をエコーしない */
  temp_term.c_lflag &= ~ISIG;   /* シグナルを無効化 */
  temp_term.c_cc[VMIN] = 1;     /* 何文字受け取ったらreadが返るか */
  temp_term.c_cc[VTIME] = 0;    /* 何秒経ったらreadが返るか */

  errno = 0;
  if(tcsetattr(fileno(stdin), TCSANOW, &temp_term) == -1){
    perror("tcsetattr(temp_term) failure");
    exit(EXIT_FAILURE);
  }else{
    init_screen();

    // 描画開始
    if( pthread_create( &draw, NULL, screen_view, NULL)){
      printf("error creating thread.");
      abort();
    }

    // タイトル
    if( pthread_create( &title, NULL, game_model, NULL)){
      printf("error creating thread.");
      abort();
    }

    while(ch != 'q'){
      ch = fgetc(stdin);
      /* 入力がCRとLFの場合は何もしない */
      if(ch != '\r' && ch != '\n'){
        input = ch;
      }
    }
  }

  /* 端末設定をもとに戻す */
  errno = 0;
  if(tcsetattr(fileno(stdin), TCSANOW, &save_term) == -1){
    perror("tcsetattr(save_term) failure");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

// ----------------------------------
// Function
// 関数
// ----------------------------------

/* 画面初期化 */
void init_screen(void){
  int i, j;
  for(i = 0; i < DISPLAY_Y; i++){
    for(j = 0; j < DISPLAY_X; j++){
      screen[i][j] = ' ';
    }
  }
}

/* 文字列表示 */
void str2screen(int y, int x, char str[], int space){
  int i, j;
  int length = (int)strlen(str);

  for(i = 0; i < length; i++){
    if(x+i >= DISPLAY_X) break;
    screen[y][x + i] = str[i];
    for(j = 0; j < space; j++){
      if(x+i >= DISPLAY_X) break;
      x++;
    }
  }
}

/* スロットの判定をして，払い出しを行う */
void pay(void){
  char real1eye = screen[12][25];
  char real2eye = screen[12][40];
  char real3eye = screen[12][55];
  int get = 0;

  if (real1eye == real2eye && real2eye == real3eye) {
    switch (real1eye) {
    case '1':
      get = 1*bet;
      break;
    case '2':
      get = 2*bet;
      break;
    case '3':
      get = 3*bet;
      break;
    case '7':
      get = 7*bet;
      break;
    case 'R':
      get = 110;
      break;
    }
    sprintf(&screen[20][5], "\x1b[31mYou got %d credit!!", get);
    sprintf(&screen[21][5], "push [space] key to start game.\x1b[39m");
    credit += get;
  }
  else {
    sprintf(&screen[20][5], "\x1b[34mYou lost your money!", get);
    sprintf(&screen[21][5], "push [space] key to start game.\x1b[39m");
    credit -= bet;

    if (credit <= 0 ){
      init_screen;
      sprintf(&screen[10][36], "\x1b[31mG A M E\x1b[39m");
      sprintf(&screen[15][36], "\x1b[31mO V E R\x1b[39m");
      usleep(100000);

      exit(EXIT_SUCCESS);
    }
  }

  if (credit < 3){
    bet = credit;
  }

}
// ----------------------------------
// Thread
// スレッド
// ----------------------------------
void *screen_view(void *arg){
  int i, j;
  int count = 0;

  while(input != 'q'){
    sprintf(&screen[0][0], "flame: %6d", count);
    sprintf(&screen[0][18], "credit: %5d", credit);
    sprintf(&screen[0][35], "bet: %d", bet);
    sprintf(&screen[0][50], "input: %c", input);
    sprintf(&screen[0][64], ":: DEBUG-MODE ::");
    for(i = 0; i < DISPLAY_Y; i++){
      for(j = 0; j < DISPLAY_X; j++){
        fprintf(stdout, "%c", screen[i][j]);
      }
      fprintf(stdout, "\n");
    }
    usleep(50000);
    fprintf(stdout, "\x1b[2J");
    count++;
  }
}

void *game_model(void *arg){
  pthread_t help;
  pthread_t slot;

  init_screen();

  while(input != 'q'){

    str2screen(4, 18, "the", 0);
    str2screen(5, 23, "SLOT", 9);

    str2screen(10, 18, "[s]. Start slot", 0);
    str2screen(12, 18, "[h]. How to play this game", 0);

    switch(input){
    case 's':
      init_screen();
      if(pthread_create( &slot, NULL, slot_model, NULL)){
        printf("error creating thread.");
        abort();
      }
      if (pthread_join (slot, NULL)) {
        printf("error joining thread.");
        abort();
      }
      break;

    case 'h': //Draw How to play
      init_screen();
      if(pthread_create( &help, NULL, help_model, NULL)){
        printf("error creating thread.");
        abort();
      }
      if (pthread_join (help, NULL)) {
        printf("error joining thread.");
        abort();
      }
      break;
    }
    usleep(20000);
  }
}
void *help_model(void *arg){
  str2screen(2, 18, "the", 0);
  str2screen(3, 23, "SLOT", 9);

  sprintf(&screen[6][10], "* How to play");

  sprintf(&screen[8][13], "1. Push down space bar and start slot machine.");
  sprintf(&screen[9][13], "2. Push down 'j', 'k' and 'l' key to stop each real.");

  sprintf(&screen[20][10], "push [space] to back to the title");

  while(1){
    if(input == ' ') break;
    usleep(20000);
  }
  init_screen();
}

void *slot_model(void *arg){
  pthread_t real1;
  pthread_t real2;
  pthread_t real3;

  str2screen(2, 18, "the", 0);
  str2screen(3, 23, "SLOT", 9);

  sprintf(&screen[16][24], "[j]");
  sprintf(&screen[16][39], "[k]");
  sprintf(&screen[16][54], "[l]");

  while(1){

    if(pthread_create( &real1, NULL, real1_model, NULL)){
      printf("error creating thread.");
      abort();
    }
    if(pthread_create( &real2, NULL, real2_model, NULL)){
      printf("error creating thread.");
      abort();
    }
    if(pthread_create( &real3, NULL, real3_model, NULL)){
      printf("error creating thread.");
      abort();
    }
    if (pthread_join (real3, NULL)) {
      printf("error joining thread.");
      abort();
    }
    if (pthread_join (real2, NULL)) {
      printf("error joining thread.");
      abort();
    }
    if (pthread_join (real1, NULL)) {
      printf("error joining thread.");
      abort();
    }

    pay();

    while(1){
      if(input == ' ') break;
      usleep(20000);
    }

    sprintf(&screen[20][5], "                                          ");
    sprintf(&screen[21][5], "                                          ");
  }
  init_screen();
}

void *real1_model(void *arg){
  char real[]={'1', '2', '3', '1', 'R', '7', '2', '1', '1', 'R', '3'};
  int count = 0;
  static int i;
  int length = sizeof(real)/sizeof(real[0]);

  while(1){
    if(input == 'j') break;

    if(count % 13 == 0){
      sprintf(&screen[7][23], "real1");
      sprintf(&screen[8][21], "#running#");
      sprintf(&screen[11][25], "%c ", real[abs((i+2)%length)]);
      sprintf(&screen[12][25], "%c ", real[abs((i+1)%length)]);
      sprintf(&screen[13][25], "%c ", real[abs(i%length)]);

      i++;
    }
    usleep(20000);
    count++;
  }
  sprintf(&screen[8][21], "#killed!#");
}

void *real2_model(void *arg){
  char real[]={'2', '1', '3', '2', 'R', '7', '1', '2', '1', '1', '3', '1'};
  int count = 0;
  static int i;
  int length = sizeof(real)/sizeof(real[0]);

  while(1){
    if(input == 'k') break;
    if(count % 12 == 0){
      sprintf(&screen[7][38], "real2");
      sprintf(&screen[8][36], "#running#");
      sprintf(&screen[11][40], "%c ", real[abs((i+2)%length)]);
      sprintf(&screen[12][40], "%c ", real[abs((i+1)%length)]);
      sprintf(&screen[13][40], "%c ", real[abs(i%length)]);

      i++;
    }
    usleep(20000);
    count++;
  }
  sprintf(&screen[8][36], "#killed!#");
}

void *real3_model(void *arg){
  char real[]={'3', '2', '1', '1', '2', '1', '2', '7', 'R', '1', '3', '2'};
  int count = 0;
  static int i;
  int length = sizeof(real)/sizeof(real[0]);


  while(1){
    if(input == 'l') break;

    if(count % 13 == 0){
      sprintf(&screen[7][53], "real3");
      sprintf(&screen[8][51], "#running#");

      sprintf(&screen[11][55], "%c ", real[abs((i+2)%length)]);
      sprintf(&screen[12][55], "%c ", real[abs((i+1)%length)]);
      sprintf(&screen[13][55], "%c ", real[abs(i%length)]);

      i++;
    }
    usleep(20000);
    count++;
  }
  sprintf(&screen[8][51], "#killed!#");
}
