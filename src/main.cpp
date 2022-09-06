#include <windows.h>
#include <GLFW/glfw3.h>

// change this to int main() to allow the console
int main(int argc, char** argv)
{
    GLFWwindow* window;
    int windowSizeW = 640, windowSizeH = 480;
    // initialize the library
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    int count, windowWidth, windowHeight, monitorX, monitorY;

    // I am assuming that main monitor is in the 0 position
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    // width: 75% of the screen
    windowWidth = static_cast<int>(videoMode->width / 1.5);
    // aspect ratio 16 to 9
    windowHeight = static_cast<int>(videoMode->height / 16 * 9);

    glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(windowSizeW, windowSizeH, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowPos(window, monitorX + (videoMode->width - windowWidth) / 2, monitorY + (videoMode->height - windowHeight) / 2);

    // make the window's context current
    glfwMakeContextCurrent(window);

    // reset the window hints to default
    glfwDefaultWindowHints();

    // show the window
    glfwShowWindow(window);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // render
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_QUADS);
        glColor3f(0, 0, 1);
        glVertex3f(-0.5, -0.5, -1);
        glColor3f(0, 1, 0);
        glVertex3f(0.5, -0.5, -1);
        glColor3f(1, 0, 1);
        glVertex3f(0.5, 0.5, -1);
        glColor3f(1, 1, 0);
        glVertex3f(-0.5, 0.5, -1);
        glEnd();

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
