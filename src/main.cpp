#include "GL/glut.h"
#include "GL/gl.h"

#include "Nes.hpp"

Nes nes;
GLuint texture = 0;

void renderFrame()
{
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (!texture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nes.getWidth(), nes.getHeight(), 0,
                     GL_RGB, GL_UNSIGNED_BYTE, nes.getFrameBuffer());
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nes.getWidth(), nes.getHeight(),
                        GL_RGB, GL_UNSIGNED_BYTE, nes.getFrameBuffer());
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0 ,0);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 1);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1, 1);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glTexCoord2f(1, 0);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glEnd();
    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    fprintf(stdout, "Mark NES Emulator\n");

    nes.load("supermario.nes");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(nes.getWidth(), nes.getHeight());
    glutInitWindowPosition(0, 0);
    glutCreateWindow(nes.getName());
    glutDisplayFunc(&renderFrame);
    glutIdleFunc(&renderFrame);
    glutMainLoop();

    return 0;
}
