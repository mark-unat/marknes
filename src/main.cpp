#include "GL/glut.h"
#include "GL/gl.h"

#include "Nes.hpp"

Nes nes;
GLuint texture = 0;

void renderFrame()
{
    nes.renderFrame();

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

void help()
{
    fprintf(stdout, "Usage:   marknes rom_file\n");
    fprintf(stdout, "Example: marknes roms/supermario.nes\n");
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        help();
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Mark NES Emulator\n");

    auto nesRomFile = std::string{argv[1]};
    nes.load(nesRomFile);
    nes.reset();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(nes.getWidth(), nes.getHeight());
    glutInitWindowPosition(0, 0);
    glutCreateWindow(nes.getName());
    glutDisplayFunc(&renderFrame);
    glutIdleFunc(&renderFrame);
    glutMainLoop();

    return EXIT_SUCCESS;
}
