#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/joystick.h>

#include "GL/freeglut.h"
#include "GL/glut.h"
#include "GL/gl.h"

#ifdef SIDEBAR
#include "sidebar.h"
#endif

#include "Nes.hpp"

Nes nes;
GLuint texture = 0;
static int joystickFD0 = -1;
static int joystickFD1 = -1;

std::vector<uint8_t> displayBuffer;
static uint32_t displayWidth = 0;
static uint32_t displayHeight = 0;

void readJoystick(uint8_t id)
{
    struct js_event event;
    int fd = -1;
    if ((id == 0) && (joystickFD0 >= 0)) {
        fd = joystickFD0;
    } else if ((id == 1) && (joystickFD1 >= 0)) {
        fd = joystickFD1;
    } else {
        return;
    }

    ssize_t bytes = read(fd, &event, sizeof(event));
    if (bytes == sizeof(event)) {
        switch (event.type) {
        case JS_EVENT_BUTTON:
        {
            switch (event.number) {
            case 0:
                nes.setControllerKey(id, NesButton::B, event.value);
                break;
            case 1:
                nes.setControllerKey(id, NesButton::A, event.value);
                break;
            case 8:
                nes.setControllerKey(id, NesButton::Select, event.value);
                break;
            case 9:
                nes.setControllerKey(id, NesButton::Start, event.value);
                break;
            default:
                break;
            }
            break;
        }
        case JS_EVENT_AXIS:
        {
            switch (event.number) {
            case 0:
            {
                // Horizontal Axis
                if (event.value > 0) {
                    nes.setControllerKey(id, NesButton::Right, true);
                    nes.setControllerKey(id, NesButton::Left, false);
                } else if (event.value < 0) {
                    nes.setControllerKey(id, NesButton::Left, true);
                    nes.setControllerKey(id, NesButton::Right, false);
                } else {
                    nes.setControllerKey(id, NesButton::Right, false);
                    nes.setControllerKey(id, NesButton::Left, false);
                }
                break;
            }
            case 1:
            {
                // Vertical Axis
                if (event.value > 0) {
                    nes.setControllerKey(id, NesButton::Down, true);
                    nes.setControllerKey(id, NesButton::Up, false);
                } else if (event.value < 0) {
                    nes.setControllerKey(id, NesButton::Up, true);
                    nes.setControllerKey(id, NesButton::Down, false);
                } else {
                    nes.setControllerKey(id, NesButton::Up, false);
                    nes.setControllerKey(id, NesButton::Down, false);
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        default:
            /* Ignore init events. */
            break;
        }
    }
}

void joystick(unsigned int buttonmask, int x, int y, int z)
{
    // Controller #1
    readJoystick(0);
    // Controller #2
    readJoystick(1);
}

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

void initializeDisplay()
{
    displayHeight = nes.getHeight();
    displayWidth = nes.getWidth();

#ifdef SIDEBAR
    // If we have sidebar enabled, we add
    // two extra width's on both sides.
    displayWidth += 2 * sideBarWidth;
    displayBuffer.resize(displayWidth * displayHeight * 3);
#endif
}

uint8_t* getDisplayBuffer()
{
    uint8_t* nesFrameBuffer = nes.getFrameBuffer();

#ifdef SIDEBAR
    uint32_t pixelIndex = 0;
    for (uint32_t row = 0; row < nes.getHeight(); row++) {
        // Left sidebar
        for (uint32_t col = (row * sideBarWidth * 3); col < ((row + 1) * sideBarWidth * 3); col++) {
            displayBuffer[pixelIndex++] = sideBar[col];
        }

        // Main NES window
        for (uint32_t col = (row * nes.getWidth() * 3); col < ((row + 1) * nes.getWidth() * 3); col++) {
            displayBuffer[pixelIndex++] = nesFrameBuffer[col];
        }

        // Right sidebar
        for (uint32_t col = (row * sideBarWidth * 3); col < ((row + 1) * sideBarWidth * 3); col++) {
            displayBuffer[pixelIndex++] = sideBar[col];
        }
    }

    // Return this buffer to have the sidebar
    nesFrameBuffer = displayBuffer.data();
#endif

    return nesFrameBuffer;
}

void renderFrame()
{
    nes.renderFrame();

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    uint8_t* buffer = getDisplayBuffer();

    if (!texture) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, displayWidth, displayHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, displayWidth, displayHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer);
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

    // Get a hold of the joystick inputs
    joystickFD0 = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    joystickFD1 = open("/dev/input/js1", O_RDONLY | O_NONBLOCK);

    fprintf(stdout, "Mark NES Emulator\n");

    auto nesRomFile = std::string{argv[1]};
    nes.load(nesRomFile);
    nes.reset();

    initializeDisplay();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(displayWidth, displayHeight);
    glutInitWindowPosition(0, 0);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutCreateWindow(nes.getName());
    glutKeyboardFunc(&readPressedKeys);
    glutKeyboardUpFunc(&readReleasedKeys);
    glutSpecialFunc(&readPressedSpecialKeys);
    glutSpecialUpFunc(&readReleasedSpecialKeys);
    glutJoystickFunc(&joystick, 10);
    glutDisplayFunc(&renderFrame);
    glutIdleFunc(&renderFrame);
    glutMainLoop();

    if (joystickFD0 >= 0) {
        close(joystickFD0);
    }
    if (joystickFD1 >= 0) {
        close(joystickFD1);
    }

    return EXIT_SUCCESS;
}
