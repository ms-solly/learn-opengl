#include <GL/glew.h>
#include <GLFW/glfw3.h>

float vcube[] = {
    -1,-1,-1,  0,0,-1,  1,-1,-1,  0,0,-1,  1, 1,-1,  0,0,-1,  -1, 1,-1,  0,0,-1, // back
    -1,-1, 1,  0,0, 1,  1,-1, 1,  0,0, 1,  1, 1, 1,  0,0, 1,  -1, 1, 1,  0,0, 1, // front
    -1,-1,-1, -1,0,0,  -1, 1,-1, -1,0,0,  -1, 1, -1, -1,0,0,  -1,-1,-1, -1,0,0, // left
     1,-1,-1,  1,0,0,   1, 1,-1,  1,0,0,   1, 1,1,  1,0,0,   1,-1, 1, 1,0,0, // right
    -1, 1,-1,  0,1,0,   1, 1,-1,  0,1,0,   1, 1, 1,  0,1,0,  -1, 1, 1, 0,1,0, // top
    -1,-1,-1,  0,-1,0,  1,-1,-1, 0,-1,0,   1,-1, 1,  0,-1,0, -1,-1, 1, 0,-1,0  // bottom
};
unsigned char qcube[] = {
    0,1,2,3, 3,4,5,0, 6,7,8,9, 9,10,11,6, 12,13,14,15, 15,16,17,12,
    18,19,20,21, 21,22,23,18, 24,25,26,27, 27,28,29,24, 30,31,32,33,
    33,34,35,30,0
};

float vplane[] = {
    -5,-5,1.0, 0,0,1,  5,-5,1.0, 0,0,1,  5,5,1.0, 0,0,1,  -5,5,1.0, 0,0,1,
};
unsigned char qplane[] = { 0,1,2,3 };
/*
void shadowMatrix(GLfloat* shadowMat, GLfloat* lightPos, GLfloat* plane) {
    float dot = plane[0]*lightPos[0] + plane[1]*lightPos[1] +
                plane[2]*lightPos[2] + plane[3]*lightPos[3];

    for (int i = 0; i < 16; i++) shadowMat[i] = 0;

    shadowMat[0]  = dot - lightPos[0]*plane[0];
    shadowMat[4]  = -lightPos[0]*plane[1];
    shadowMat[8]  = -lightPos[0]*plane[2];
    shadowMat[12] = -lightPos[0]*plane[3];

    shadowMat[1]  = -lightPos[1]*plane[0];
    shadowMat[5]  = dot - lightPos[1]*plane[1];
    shadowMat[9]  = -lightPos[1]*plane[2];
    shadowMat[13] = -lightPos[1]*plane[3];

    shadowMat[2]  = -lightPos[2]*plane[0];
    shadowMat[6]  = -lightPos[2]*plane[1];
    shadowMat[10] = dot - lightPos[2]*plane[2];
    shadowMat[14] = -lightPos[2]*plane[3];

    shadowMat[3]  = -lightPos[3]*plane[0];
    shadowMat[7]  = -lightPos[3]*plane[1];
    shadowMat[11] = -lightPos[3]*plane[2];
    shadowMat[15] = dot - lightPos[3]*plane[3];
}
*/
double lastX = 0, lastY = 0;
float rotX = 0, rotY = 0;
bool firstMouse = true;

void cursor_pos(GLFWwindow*, double x, double y) {
    if (firstMouse) { lastX = x; lastY = y; firstMouse = false; return; }
    rotY += (x - lastX) * 0.3f;
    rotX += (y - lastY) * 0.3f;
    lastX = x; lastY = y;
}

int main() {
    glfwInit();
    GLFWwindow* w = glfwCreateWindow(1500, 800, "Cube", glfwGetPrimaryMonitor(), 0);
    glfwMakeContextCurrent(w); glewInit();

    glfwSetCursorPosCallback(w, cursor_pos);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45, 800.0/600.0, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);

    GLfloat lightPos[] = { 4.0f, 4.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat lightDiffuse[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);

   // GLfloat planeEq[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
    //GLfloat shadowMat[16];
   // shadowMatrix(shadowMat, lightPos, planeEq);

    while (!glfwWindowShouldClose(w)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0, 0, -10);
        glRotatef(rotX, 1, 0, 0);
        glRotatef(rotY, 0, 1, 0);

        // Light position updated each frame
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        // Draw plane
        glVertexPointer(3, GL_FLOAT, 6*sizeof(float), vplane);
        glNormalPointer(GL_FLOAT, 6*sizeof(float), vplane + 3);
        glColor3f(0.3f, 0.3f, 0.3f);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, qplane);

        // Draw cube
        glVertexPointer(3, GL_FLOAT, 6*sizeof(float), vcube);
        glNormalPointer(GL_FLOAT, 6*sizeof(float), vcube + 3);
        glColor3f(0.1f, 0.8f, 0.2f);
        glDrawElements(GL_QUADS, sizeof(qcube), GL_UNSIGNED_BYTE, qcube);

        // Draw projected shadow
        glDisable(GL_LIGHTING);
        glPushMatrix();
        //glMultMatrixf(shadowMat);
        glColor3f(0.0f, 0.0f, 0.0f); // solid shadow
        glDrawElements(GL_QUADS, sizeof(qcube), GL_UNSIGNED_BYTE, qcube);
        glPopMatrix();
        glEnable(GL_LIGHTING);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwDestroyWindow(w); glfwTerminate();
}

