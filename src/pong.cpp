#include<GLFW/glfw3.h>
#include<stdio.h>
#include<math.h>


const int WIDTH = 800;
const int HEIGHT = 600;
float PADDLE_SPEED = 500.0f;
float BALL_SPEED = 400.0f;

typedef struct{
    float x,y;
    float width,height;
} Paddle;

typedef struct{
    float x,y;
    float size;
    float speedX,speedY;
} Ball;

Paddle leftPaddle = {0,HEIGHT/2 - 500,20,100};
Paddle rightPaddle = {WIDTH - 20, HEIGHT/2 - 50, 20,100};
Ball ball = {WIDTH/2,HEIGHT/2,15,BALL_SPEED,BALL_SPEED};

void reset_ball(int goRight){
    ball.x=WIDTH/2;
    ball.y=HEIGHT/2;
    ball.speedX=goRight ? BALL_SPEED : -BALL_SPEED;
    ball.speedY=BALL_SPEED;
}
void draw_rect(float x, float y,float width, float height){
    glBegin(GL_QUADS);
    glVertex2f(x,y);
    glVertex2f(x+width,y);
    glVertex2f(x+width,y+height);
    glVertex2f(x,y+height);
    glEnd();
}

int main(){
    if(!glfwInit()){
        fprintf(stderr,"failed init glfw\n");
        return -1;
    }
    GLFWwindow* scrin = glfwCreateWindow(WIDTH,HEIGHT,"pong",NULL,NULL);

    if(!scrin){
        fprintf(stderr,"failed create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(scrin);
    glfwSwapInterval(1);

    // 2d projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,WIDTH,HEIGHT,0,-1,1);
    glMatrixMode(GL_MODELVIEW);

    double lastTime=glfwGetTime();


    while(!glfwWindowShouldClose(scrin)){
        double currentTime=glfwGetTime();
        float deltaTime = (float)(currentTime-lastTime);
        lastTime=currentTime;

        if(glfwGetKey(scrin,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(scrin,1);

        if(glfwGetKey(scrin,GLFW_KEY_W) == GLFW_PRESS && leftPaddle.y>0)
            leftPaddle.y-=PADDLE_SPEED*deltaTime;

        if(glfwGetKey(scrin,GLFW_KEY_S)== GLFW_PRESS && leftPaddle.y < HEIGHT - leftPaddle.height)
            leftPaddle.y += PADDLE_SPEED * deltaTime;

        if(glfwGetKey(scrin,GLFW_KEY_UP) == GLFW_PRESS && rightPaddle.y >0)
            rightPaddle.y -= PADDLE_SPEED * deltaTime;



    }
}
