/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/ext.hpp>

#include <iostream>
#include <string>
#include <vector>

class Camera{

private:

    glm::vec3 position = glm::vec3(1.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    
    float cameraX = 0.f;
    float cameraY = 0.f;
        
    float mousex = 0.f;
    float mousey = 0.f;

public:

    glm::vec3 update(float dt, GLFWwindow *window);
    glm::mat4 viewProjection(float width, float height);
};