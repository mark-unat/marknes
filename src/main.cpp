#include "GL/glut.h"
#include "GL/gl.h"

#include "Nes.hpp"

Nes nes;
GLuint texture = 0;

void mapKeysToController(uint8_t key, bool state)
{
    switch (key) {
    // Controller #1
    case 111:
    case 79:
        // 'o' keyboard mapped to 'A' controller button
        nes.setControllerKey(0, NesButton::A, state);
        break;
    case 112:
    case 80:
        // 'p' keyboard mapped to 'B' controller button
        nes.setControllerKey(0, NesButton::B, state);
        break;
    case 107:
    case 75:
        // 'k' keyboard mapped to 'Select' controller button
        nes.setControllerKey(0, NesButton::Select, state);
        break;
    case 108:
    case 76:
        // 'l' keyboard mapped to 'Start' controller button
        nes.setControllerKey(0, NesButton::Start, state);
        break;
    case 101:
        // 'Up' keyboard mapped to 'Up' controller button
        nes.setControllerKey(0, NesButton::Up, state);
        break;
    case 103:
        // 'Down' keyboard mapped to 'Down' controller button
        nes.setControllerKey(0, NesButton::Down, state);
        break;
    case 100:
        // 'Left' keyboard mapped to 'Left' controller button
        nes.setControllerKey(0, NesButton::Left, state);
        break;
    case 102:
        // 'Right' keyboard mapped to 'Right' controller button
        nes.setControllerKey(0, NesButton::Right, state);
        break;
    // Controller #2
    // nes.setControllerKey(1, NesButton::A, state);
    default:
        break;
    }
}

void readPressedKeys(unsigned char key, int x, int y)
{
    // Key pressed
    mapKeysToController(static_cast<uint8_t>(key), true);
}

void readReleasedKeys(unsigned char key, int x, int y)
{
    // Key released
    mapKeysToController(static_cast<uint8_t>(key), false);
}

void readPressedSpecialKeys(int key, int x, int y)
{
    // Key pressed
    mapKeysToController(static_cast<uint8_t>(key), true);
}

void readReleasedSpecialKeys(int key, int x, int y)
{
    // Key released
    mapKeysToController(static_cast<uint8_t>(key), false);
}

void renderFrame()
{
    nes.renderFrame();

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (!texture) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nes.getWidth(), nes.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                     nes.getFrameBuffer());
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nes.getWidth(), nes.getHeight(), GL_RGB, GL_UNSIGNED_BYTE,
                        nes.getFrameBuffer());
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
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

int main(int argc, char** argv)
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
    glutKeyboardFunc(&readPressedKeys);
    glutKeyboardUpFunc(&readReleasedKeys);
    glutSpecialFunc(&readPressedSpecialKeys);
    glutSpecialUpFunc(&readReleasedSpecialKeys);
    glutDisplayFunc(&renderFrame);
    glutIdleFunc(&renderFrame);
    glutMainLoop();

    return EXIT_SUCCESS;
}
