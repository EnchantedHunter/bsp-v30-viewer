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

#include "bsp.h"
#include "utils.h"
#include "debug.h"
#include "camera.h"
#include "container.h"


bool check_shader_compile_status(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    return true;
}

bool check_program_link_status(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;   
    }
    return true;
}

int main(int argv, char** argc) {

    int width = 640;
    int height = 480;
    
    if(glfwInit() == GL_FALSE) {
        std::cerr << "failed to init GLFW" << std::endl;
        return 1;
    }

    // select opengl version
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 
    // create a window
    GLFWwindow *window;
    if((window = glfwCreateWindow(width, height, "bsp", 0, 0)) == 0) {
        std::cerr << "failed to open window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);

    if(glxwInit()) {
        std::cerr << "failed to init GL3W" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }    
    
    // shader source code
    std::string vertex_source =
        "#version 330\n"
        "uniform mat4 ViewProjection;\n" // the projection matrix uniform
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec2 vertexUV;\n"
        "layout(location = 2) in vec2 lightMapUV;\n"
        "out vec4 fcolor;\n"
        "out vec2 UV;\n"
        "out vec2 lmUV;\n"
        "void main() {\n"
        "   lmUV = lightMapUV;\n"
        "   UV = vertexUV;\n"
        "   gl_Position = ViewProjection*vposition;\n"
        "}\n";
        
    std::string fragment_source =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "in vec2 UV;\n"
        "in vec2 lmUV;\n"
        "uniform sampler2D textureSampler;\n"
        "uniform sampler2D lightmapSampler;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "vec4 texColor = texture(textureSampler, UV);"
        "if(texColor.a < 0.1)"
        "    discard;"
        "vec4 lightMap = texture(lightmapSampler, lmUV) + 0.1;\n"
        "FragColor = lightMap*texColor;\n"
        "}\n";
   
   
    // program and shader handles
    GLuint shader_program, vertex_shader, fragment_shader;
    
    // we need these to properly pass the strings
    const char *source;
    int length;

    // create and compiler vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = vertex_source.c_str();
    length = vertex_source.size();
    glShaderSource(vertex_shader, 1, &source, &length); 
    glCompileShader(vertex_shader);
    if(!check_shader_compile_status(vertex_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
 
    // create and compiler fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment_source.c_str();
    length = fragment_source.size();
    glShaderSource(fragment_shader, 1, &source, &length);   
    glCompileShader(fragment_shader);
    if(!check_shader_compile_status(fragment_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // create program
    shader_program = glCreateProgram();
    
    // attach shaders
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    
    // link the program and check for errors
    glLinkProgram(shader_program);
    check_program_link_status(shader_program);
    
    // obtain location of projection uniform
    GLint ViewProjection_location = glGetUniformLocation(shader_program, "ViewProjection");
    GLint ViewPosition_location = glGetUniformLocation(shader_program, "ViewPosition");

    GLint DiffuseTex = glGetUniformLocation(shader_program, "textureSampler");
    GLint LightMapTex = glGetUniformLocation(shader_program, "lightmapSampler");

    // vao and vbo handle
    GLuint vao, vbo, ibo;
 
    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // generate and bind the vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
            
    unsigned char* bspFile = readBspFile(argc[1]);

    if(bspFile == NULL){
#ifdef DEBUG_LEVEL_1
        printf("File not exist: %s\n",argc[1]);
#endif 
        return 1;
    }else{
#ifdef DEBUG_LEVEL_1
        printf("Successfully load bsp file: %s\n",argc[1]);
#endif 
    }

    BSPHEADER* header = (BSPHEADER*)bspFile;
    if(header->nVersion == 30){
#ifdef DEBUG_LEVEL_1
        printf("Version: %u\n", header->nVersion);
#endif  
    }else{
#ifdef DEBUG_LEVEL_1
        printf("Wrong map format: %u\n", header->nVersion);
#endif  
        return 1;
    }

    //============ LOAD TEXTURES ==================
   
    uint32_t texturesCount;
    TEXTURE* texturesRaw = loadTextures(bspFile, &texturesCount);

    GLuint* textureArray = (GLuint*)malloc(sizeof(GLuint)*texturesCount);
    glGenTextures(texturesCount, textureArray);

    for(size_t i = 0 ; i < texturesCount ; i++){

        TEXTURE* texture = (texturesRaw + i );

        glBindTexture(GL_TEXTURE_2D, textureArray[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->iWidth, texture->iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    
    //============= LOAD MESH & UV ============

    uint32_t ind_count;
    uint32_t verts_count;
    uint32_t textures_count;
    unsigned char* ves;
    unsigned char* ind;
    VECTOR* materials;

    uint32_t lmAtlCount;
    TEXTURE* lmAtlRaw;

    loadVertexesIndexes(bspFile, &ves, &verts_count, &materials, &textures_count, &ind, &ind_count, texturesRaw, texturesCount, &lmAtlRaw, &lmAtlCount);
    
    //============ LOAD LIGHT MAPS ==================

    GLuint* lmAtlArray = (GLuint*)malloc(sizeof(GLuint)*lmAtlCount);
    glGenTextures(lmAtlCount, lmAtlArray);

    for(size_t i = 0 ; i < lmAtlCount ; i++){

        TEXTURE* tex = (lmAtlRaw + i);

        glBindTexture(GL_TEXTURE_2D, lmAtlArray[i]);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->iWidth, tex->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    }

    //============= FILL VAO & VBO ============

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* verts_count, (GLfloat*)ves, GL_STATIC_DRAW);
                    
    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));
 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (char*)0 + 5*sizeof(GLfloat));

    // generate and bind the index buffer object
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // fill with data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* ind_count, (GLuint*)ind, GL_STATIC_DRAW);
    
    // we are drawing 3d objects so we want depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);  

    //camera 
    Camera* camera = new Camera();

    float t = glfwGetTime();

    // mouse position
    double mousex, mousey;

    uint32_t idx = 0;

    while(!glfwWindowShouldClose(window)) {
        
        // listen events
        glfwPollEvents();

        // disable mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
        // calculate timestep
        float new_t = glfwGetTime();
        float dt = new_t - t;
        t = new_t;        
        
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // calculate ViewProjection matrix
        glm::vec3 position = camera->update(dt, window);
        glm::mat4 ViewProjection = camera->viewProjection((float)width,  (float)height);

        // clear first
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // use the shader program
        glUseProgram(shader_program);
        glUniform1i(DiffuseTex, 0);
        glUniform1i(LightMapTex,  1);
        glUniformMatrix4fv(ViewProjection_location, 1, GL_FALSE, glm::value_ptr(ViewProjection));
        glUniform3fv(ViewPosition_location, 1, glm::value_ptr(position));
        
        // bind the vao
        glBindVertexArray(vao);

        for(size_t i = 0 ; i < texturesCount ; i ++){

            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, *(textureArray + i));

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, *(lmAtlArray + i));

            for(size_t j = 0 ; j < materials[i].size; j+=2)
            {

                uint32_t offset = *((uint32_t*)(materials[i].data) + j + 1);
                uint32_t size = *(((uint32_t*)materials[i].data) + j);

                glDrawElements(GL_TRIANGLES, size , GL_UNSIGNED_INT, (char*)0 + offset*sizeof(GLuint));
            }
        }

        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }
        
        // finally swap buffers
        glfwSwapBuffers(window);   
    }
    
    // delete the created objects
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    
    glDetachShader(shader_program, vertex_shader);	
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
