#include<stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <curses.h>//这个就是我们要用到的额外的ui库
#include <stdlib.h>
 
#define MAX_X 70   //场地宽
#define MAX_Y 20  //场地长
#define CORNER_X 4  //左上角x坐标
#define CORNER_Y 2   //左上角y坐标
 
struct point {
 int x;
 int y;
};
struct point SnakeBody[50];
struct point food;
 
int Length = 4;  //初始蛇长
int life = 1;   //是否还活着
int input = 0;  //记录键盘按键的ascii
pthread_t id; //声明一个linux线程，按键等待线程
 
void FoodCheck();//检查是否吃到了食物
void FoodProduce();//生成一个食物
void Initializition();//初始化线程，进行蛇的初始设定，创建第一个食物
void SnakeHeadMovement();//移动蛇
void SnakeBodyMovement();//移动蛇辅助方法
 
void DeathCheck();//检查是否满足死亡条件
void Paint();//画社画场地画食物
void * waitForKey(void *);//这个是另一个线程的函数定义
void drawDot(int x, int y,char s);//画点喽
void clearDot(int x, int y);//清楚点喽
void end();//程序的结束工作
 
//主函数
 
int main(int argc, char** argv) {
 
 Initializition();
 refresh();//刷新画布
 while (life) {
 
 Paint();
 usleep(200000);//刷新频率是0.2秒
 SnakeHeadMovement();//移动蛇，在这个方法里执行了foodCheck方法。其实这里的逻辑稍微混乱了点
 DeathCheck();//判断是否死亡
 }
 end();
 return 0;
}
 
void * waitForKey(void *para) {
 while (1) {
usleep(1000);//为什么要加这个，不知道什么原因，在curses下，如果建了这个线程并且不加这句话的话就会出现花屏现象。很难看
 input = getch();
 }
}
 
void end() {
 
 move(1, 0);
 addstr("Press any key to quit!");
 refresh();
 getch();
 endwin();
}
 
//食物的随机产生
 
void FoodProduce() {
 int superposition = 0;
 int i;
 srand(time(NULL));
 do {
 food.x = (rand() % ((MAX_X-2) / 2))*2+2;  //2 to MAX_X-2  and is 偶数
 food.y = rand() % (MAX_Y-1)+1;   //1 to MAX_Y-1
 for (i = 0; i < Length; i++) {
 if (food.x == SnakeBody[i].x && food.y == SnakeBody[i].y)
 superposition = 1;
 }
 } while (superposition); /*直到没有重合*/
}
 
//蛇身和食物的初始化 初始化的蛇身为4节长度
 
void Initializition() {
 initscr();//curses初始化
 noecho();//默认不将输入的字符显示在屏幕上
 int i;
 for (i = 3; i <= 6; i++) {//初始化蛇
 SnakeBody[6 - i].x = 4;
 SnakeBody[6 - i].y = i;
 }
 FoodProduce();
 int ret;
 ret = pthread_create(&id, NULL, waitForKey, NULL);//创建线程
 if (ret != 0) {
 exit(1);
 }
 
 for ( i = 0; i <= MAX_X; i+=2) {  //画围墙
 drawDot(i, 0,'*');
 drawDot(i, MAX_Y,'*');
 }
 
 for (i = 0; i <= MAX_Y; i++) {
 drawDot(0, i,'*');
 drawDot(MAX_X, i,'*');
 }
 
}
 
//蛇移动，依次从尾巴到头赋值
void SnakeBodyMovement() {
 int i;
 for (i = Length - 1; i > 0; i--) {
 SnakeBody[i].x = SnakeBody[i - 1].x;
 SnakeBody[i].y = SnakeBody[i - 1].y;
 }
 
}
 
void SnakeHeadMovement() {
 
 clearDot(SnakeBody[Length - 1].x, SnakeBody[Length - 1].y);
 int directionX, directionY; /*定义原本蛇前进的方向，可通过蛇头坐标减去蛇的第二部分*/
 int newX, newY;
 newX = SnakeBody[0].x;
 newY = SnakeBody[0].y;
 
 directionX = SnakeBody[0].x - SnakeBody[1].x;
 directionY = SnakeBody[0].y - SnakeBody[1].y;
 
 if (input == 'w' && directionY<=0) //不走回头路
 newY--;
 else if (input == 's' && directionY>=0 )
 newY++;
 else if (input == 'a' && directionX<=0)
 newX -= 2; /*因为字符高是宽的两倍*/
 else if (input == 'd' && directionX>=0)
 newX += 2;
 else {
 newX += directionX;
 newY += directionY;
 }
 FoodCheck();
 SnakeBodyMovement();
 SnakeBody[0].x = newX;
 SnakeBody[0].y = newY;
 
}
//判断是否吃到食物,以及吃到后长度变长还有产生新的食物
 
void FoodCheck() {
 if (food.x == SnakeBody[0].x && food.y == SnakeBody[0].y) {
 Length = Length + 1;
 FoodProduce();
 }
}
//判断是否死亡
 
void DeathCheck() {
 int i;
 if (SnakeBody[0].x <=1 || SnakeBody[0].x >= MAX_X  || SnakeBody[0].y <= 0 || SnakeBody[0].y >=MAX_Y)
 life = 0;
 for (i = 4; i < Length; i++)
 if (SnakeBody[0].x == SnakeBody[i].x && SnakeBody[0].y == SnakeBody[i].y)
 life = 0;
}
 
//排序和打印
 
void Paint() {
 int i = 0;
 
 drawDot(SnakeBody[i].x, SnakeBody[i].y,'@');
 for (i=1; i < Length; i++) {
 drawDot(SnakeBody[i].x, SnakeBody[i].y,'*');
 }
 drawDot(food.x, food.y,'$');
 move(0,0);
 refresh();//刷新画布
}
 
void drawDot(int x, int y,char s) {
 move(y+CORNER_Y, x+CORNER_X);
 addch(s);
}
 
void clearDot(int x, int y) {
 move(y+CORNER_Y, x+CORNER_X);
 addch(' ');
}
