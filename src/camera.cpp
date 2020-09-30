/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */

#include "camera.h"

glm::vec3 Camera::update(float dt, GLFWwindow *window){
    double tmpx, tmpy;
    glfwGetCursorPos(window, &tmpx, &tmpy);
    glm::vec2 mousediff(tmpx-mousex, tmpy-mousey);
    mousex = tmpx;
    mousey = tmpy;
    
    cameraX += 0.2f*mousediff.x;
    cameraY -= 0.2f*mousediff.y;

    rotation = glm::mat4(1.0f);

    glm::vec3 up1 = glm::vec3(0.0f, 0.0f, 1.0f);

    rotation = glm::rotate(rotation, cameraX, up1);

    glm::mat3 rotation3(rotation);

    glm::vec3 up = glm::transpose(rotation3)*glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::transpose(rotation3)*glm::vec3(1.0f, 0.0f, 0.0f);
    
    rotation = glm::rotate(rotation, -cameraY, right);
    glm::mat3 rotation33(rotation);

    glm::vec3 forward = glm::transpose(rotation33)*glm::vec3(0.0f, 0.0f,-1.0f);
  
    float speed = 2.0f;

    if(glfwGetKey(window, GLFW_KEY_SPACE)) {
        speed = 10.0f;
    }

    if(glfwGetKey(window, 'W')) {
        position += speed*dt*forward; 
    }  
    if(glfwGetKey(window, 'S')) {
        position -= speed*dt*forward;
    }
    if(glfwGetKey(window, 'D')) {
        position += speed*dt*right; 
    }  
    if(glfwGetKey(window, 'A')) {
        position -= speed*dt*right;
    }
    return position;
}

glm::mat4 Camera::viewProjection(float width, float height){
    glm::mat4 Projection = glm::perspective(70.0f, float(width) / height, 0.001f, 1000.f);
    glm::mat4 View = rotation*glm::translate(glm::mat4(1.0f), -position);//glm::lookAt(position, position+forward, up);//
    return Projection*View;
}
