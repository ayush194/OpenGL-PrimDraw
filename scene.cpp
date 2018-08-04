//
//  scene.cpp
//  BasicOpenGL
//
//  Created by Ayush Kumar on 6/26/18.
//  Copyright © 2018 Ayush Kumar. All rights reserved.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <primitive.h>
#include <scene.h>
#include <camera.h>
#include <shader.h>
#include <material.h>
#include <texture.h>
#include <stb_image.h>
#include <iostream>

extern Camera CAMERA;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
glm::vec3 OBJECT_COLOR(1.0f, 0.5f, 0.31f);
float LASTFRAME = 0.0;
float DELTATIME = 0.0;
float CURSOR_POS_X = SCR_WIDTH / 2;
float CURSOR_POS_Y = SCR_HEIGHT / 2;
bool FIRST_MOUSE = true;
Camera CAMERA(glm::vec3(0.0, 0.0, 5.0));

void mouse_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow*, double, double);
void framebuffer_size_callback(GLFWwindow*, int, int);
void processInput(GLFWwindow*);
unsigned int loadTexture(char const*);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpengl", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, framebuffer_size_callback);
    
    //disable cursor outside of screen window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    //load function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    Scene myscene;
    //myscene.setSmooth(true);
    //myscene.createObject(ELLIPSOID, {1.0, 1.0, 1.0});
    int cube = myscene.createObject(CUBOID, {0.0, 0.0, 5.0});
    myscene.setUvmap(cube, FACE);
    int t1 = myscene.createTexture("resources/textures/crate_diffuse.png");
    int t2 = myscene.createTexture("resources/textures/crate_specular.png");
    int t3 = myscene.createTexture("resources/textures/crate_emission.jpg");
    myscene.setTexture(DIFFUSE, cube, t1);
    myscene.setTexture(SPECULAR, cube, t2);
    myscene.setTexture(EMISSION, cube, t3);
    uint plane = myscene.createObject(RECTANGULAR_PLANE, {0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {10.0, 10.0, 10.0}, 0, true, 1, 0);
    myscene.setLightAmbient({0.1f, 0.1f, 0.1f});
    myscene.setLightDiffuse({0.4f, 0.4f, 0.4f});
    myscene.setLightSpecular({0.4f, 0.4f, 0.4f});
    myscene.setLightConstants({1.0, 0.09, 0.032});
    //myscene.createLight(DIRECTEDLIGHT, plane, {5.0, 5.0, 5.0}, {0.0, 0.0, -1.0});
    myscene.createLight(DIRECTEDLIGHT, plane, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0});

    while (!glfwWindowShouldClose(window)) {
        //process input
        float currTime = glfwGetTime();
        DELTATIME = currTime - LASTFRAME;
        LASTFRAME = currTime;
        
        processInput(window);
        
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        myscene.setRenderLights(true);
        myscene.render();
         
        //accesorial actions
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //glDeleteTextures(1, &crateDiffuse);
    //glDeleteTextures(1, &crateSpecular);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        CAMERA.ProcessKeyboard(FORWARD, DELTATIME);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        CAMERA.ProcessKeyboard(BACKWARD, DELTATIME);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        CAMERA.ProcessKeyboard(LEFT, DELTATIME);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        CAMERA.ProcessKeyboard(RIGHT, DELTATIME);
    }
}
void mouse_callback(GLFWwindow* window, double posx, double posy) {
    if (FIRST_MOUSE) {
        CURSOR_POS_X = posx;
        CURSOR_POS_Y = posy;
        FIRST_MOUSE = false;
    }
    float posx_offset = posx - CURSOR_POS_X;
    float posy_offset = CURSOR_POS_Y - posy;
    CURSOR_POS_X = posx;
    CURSOR_POS_Y = posy;
    CAMERA.ProcessMouseMovement(posx_offset, posy_offset);
}
void scroll_callback(GLFWwindow* window, double offsetx, double offsety) {
    CAMERA.ProcessMouseScroll(offsety);
}

