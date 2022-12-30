#include "glad/glad.h"
#include "glad.c"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "Image.h"
#define RJSON_IMPLEMENTATION
#include "rj_obj.h"
#include <Windows.h>
#include "json_parser.h"
#include "GLFW/glfw3.h"
#include "shader.eg"
#include "glm/common.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef unsigned int UI;
UI Program;
struct VecV
{
    float x = 0;
    float y = 0;
    float z = 0;
};

struct planet
{
    UI VAO;
    UI VBO;
    UI Texture;
    obj Model;
};

planet CreateModel(const char *ModelFilePath, const char *TextureFilePath)
{
    planet Result = {};

    HANDLE File = CreateFileA(ModelFilePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    LARGE_INTEGER Size = {};
    GetFileSizeEx(File, &Size);

    char *Buffer = (char *)malloc(Size.QuadPart);
    ReadFile(File, Buffer, Size.QuadPart, 0, 0);

    Result.Model = ParseObj((u8 *)Buffer, Size.QuadPart);

    glGenVertexArrays(1, &Result.VAO);
    glBindVertexArray(Result.VAO);

    glGenBuffers(1, &Result.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (Result.Model.NormalCount * 3 + Result.Model.VertexCount * 3 + Result.Model.TexCount * 2), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * (Result.Model.VertexCount * 3), Result.Model.Vertex);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * (Result.Model.VertexCount * 3), sizeof(float) * (Result.Model.TexCount * 2), Result.Model.TexCoords);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * ((Result.Model.TexCount * 2) + (Result.Model.VertexCount * 3)), sizeof(float) * (Result.Model.NormalCount * 3), Result.Model.Normals);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(float) * Result.Model.VertexCount * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(float) * ((Result.Model.VertexCount * 3) + (Result.Model.TexCount * 2))));

    glGenTextures(1, &Result.Texture);
    glBindTexture(GL_TEXTURE_2D, Result.Texture);
    int x, y, c;
    unsigned char *data = stbi_load(TextureFilePath, &x, &y, &c, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    return Result;
};

void CreateSphere(float OC, float x, float y, float z, planet a)

{
    glm::mat4 Model(1.0f);
    Model = glm::translate(Model, glm::vec3(x, y, z));
    Model = glm::rotate(Model, (float)glfwGetTime(), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(Program, "Model"), 1, 0, glm::value_ptr(Model));
    glUniform1f(glGetUniformLocation(Program, "ObjInt"), OC);
        glUniform3f(glGetUniformLocation(Program,"ObjColor"),0.1,0.1,0.1);
    glBindVertexArray(a.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, a.VBO);
    glBindTexture(GL_TEXTURE_2D, a.Texture);
    glDrawArrays(GL_TRIANGLES, 0, a.Model.TexCount);
}

planet OrbitData(float center1, float center2, float center3, float x, float y, float z)
{
    planet Rajat = {};
    float CA = 0;
    VecV Circle;
    Circle.x = 0;
    Circle.y = 0;
    Circle.z = 0;
    int CI = 0;
    float *CircData = (float *)malloc(sizeof(float) * 3600 * 3);
    while (CA < 360)
    {
        Circle.x = center1 - x * sinf((CA * 3.14) / 180);
        CircData[CI++] = Circle.x;
        Circle.y = center2 - y * sinf((CA * 3.14) / 180);
        CircData[CI++] = Circle.y;
        Circle.z = center3 - z * cosf((CA * 3.14) / 180);
        CircData[CI++] = Circle.z;
        CA += 0.1f;
    }

    glGenVertexArrays(1, &Rajat.VAO);
    glBindVertexArray(Rajat.VAO);

    glGenBuffers(1, &Rajat.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Rajat.VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3600 * 3, CircData, GL_STATIC_DRAW);
    free(CircData);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(float) * 3, 0);
    glBindVertexArray(Rajat.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Rajat.VBO);

   
    return Rajat;
};

void CreateOrbit(planet OrbitData, float x, float y, float z)
{

    glBindVertexArray(OrbitData.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, OrbitData.VBO);
    glm::mat4 CModel(1.0f);
    CModel = glm::translate(CModel, glm::vec3(x, y, z));
    glUniformMatrix4fv(glGetUniformLocation(Program, "Model"), 1, 0, glm::value_ptr(CModel));
    glUniform3f(glGetUniformLocation(Program,"ObjColor"),1,1,1);
    glDrawArrays(GL_POINTS, 0, 3600);
};

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1500, 700, "Solar System In My Hand", 0, 0);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    UI vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, 0);
    glCompileShader(vertexShader);

    char infolog[1000];
    glGetShaderInfoLog(vertexShader, 1000, 0, infolog);

    printf("%s", infolog);
    UI fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, 0);
    glCompileShader(fragmentShader);
    glGetShaderInfoLog(fragmentShader, 1000, 0, infolog);

    printf("%s", infolog);

    Program = glCreateProgram();
    glAttachShader(Program, vertexShader);
    glAttachShader(Program, fragmentShader);
    glLinkProgram(Program);

    glGetProgramInfoLog(Program, 1000, 0, infolog);
    printf("%s", infolog);
    glUseProgram(Program);

    glm::mat4 Proj(1.0f);
    Proj = glm::perspective((65.0f * 3.14f) / 180.0f, 1500.0f / 700.0f, 0.1f, 200.0f);

    glUniformMatrix4fv(glGetUniformLocation(Program, "Proj"), 1, 0, glm::value_ptr(Proj));

    VecV Eye;
    Eye.x = 0;
    Eye.y = 0;
    Eye.z = 0;

    VecV Up;
    Up.x = 0;
    Up.y = 1;
    Up.z = 0;

    VecV Center;
    Center.x = 0;
    Center.y = 0;
    Center.z = 0;

    planet sun = CreateModel("../dp/earth.obj", "../Texture/8k_sun.jpg");
    planet mercury = CreateModel("../dp/earth.obj", "../Texture/2k_mercury.jpg");
    planet venus = CreateModel("../dp/earth.obj", "../Texture/2k_venus_surface.jpg");
    planet earth = CreateModel("../dp/earth.obj", "../Texture/8k_earth_daymap.jpg");
    planet moon = CreateModel("../dp/earth.obj", "../Texture/2k_moon.jpg");
    planet mars = CreateModel("../dp/earth.obj", "../Texture/2k_mars.jpg");
    planet jupiter = CreateModel("../dp/earth.obj", "../Texture/2k_jupiter.jpg");
    planet saturn = CreateModel("../dp/earth.obj", "../Texture/2k_saturn.jpg");
    planet uranus = CreateModel("../dp/earth.obj", "../Texture/2k_uranus.jpg");
    planet neptune = CreateModel("../dp/earth.obj", "../Texture/2k_neptune.jpg");

    planet Orbit1 = OrbitData(0, 0, 0, 5, 2, 5);
    planet Orbit2 = OrbitData(0, 0, 0, 10, 2, 10);
    planet Orbit3 = OrbitData(0, 0, 0, 15, 2, 15);
    planet Orbit4 = OrbitData(0, 0, 0, 0, 0, 0);
    planet Orbit5 = OrbitData(0, 0, 0, 25, 2, 25);
    planet Orbit6 = OrbitData(0, 0, 0, 30, 2, 30);
    planet Orbit7 = OrbitData(0, 0, 0, 35, 2, 35);
    planet Orbit8 = OrbitData(0, 0, 0, 40, 2, 40);
    planet Orbit9 = OrbitData(0, 0, 0, 45, 2, 45);
    planet Earth = OrbitData(0, 0, 0, 2, 1, 2);

    float a = 0;
    float a1 = 0;
    float a2 = 0;
    float a3 = 0;
    float a4 = 0;
    float a5 = 0;
    float a6 = 0;
    float a7 = 0;
    float a8 = 0;

    float Earthx = 0;
    float Earthy = 0;
    float Earthz = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0, 0,0, 1);
        glEnable(GL_DEPTH_TEST);

        // camera
        {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                Eye.z -= 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                Eye.z += 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                Eye.x -= 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                Eye.x += 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            {
                Eye.y += 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            {
                Eye.y -= 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            {
                Center.x -= 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
            {
                Center.x += 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
            {
                Center.y -= 0.01;
            };
            if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
            {
                Center.y += 0.01;
            };

            glm::mat4 View(1.0f);
            View = glm::lookAt(glm::vec3(Eye.x, Eye.y, Eye.z), glm::vec3(Center.x, Center.y, Center.z), glm::vec3(Up.x, Up.y, Up.z));
            glUniformMatrix4fv(glGetUniformLocation(Program, "View"), 1, 0, glm::value_ptr(View));
        }
        //
        CreateSphere(10, 0, 0, 0, sun);
        if (a < 360)
        {
            float x = 5 * sinf((a * 3.14) / 180);
            float y = 2 * sinf((a * 3.14) / 180);
            float z = 5 * cosf((a * 3.14) / 180);
            CreateSphere(0.5, x, y, z, mercury);
            CreateOrbit(Orbit1, 0, 0, 0);
            a += 0.01f;
        }
        else
        {
            a = 0;
        };

        if (a1 < 360)
        {
            float x = 10 * sinf((a1 * 3.14) / 180);
            float y = 2 * sinf((a1 * 3.14) / 180);
            float z = 10 * cosf((a1 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, venus);
            a1 += 0.005f;
            CreateOrbit(Orbit2, 0, 0, 0);
        }
        else
        {
            a1 = 0;
        };

        if (a2 < 360)
        {
             Earthx = 15 * sinf((a2 * 3.14) / 180);
             Earthy = 2 * sinf((a2 * 3.14) / 180);
             Earthz = 15 * cosf((a2 * 3.14) / 180);
            CreateSphere(0.5, Earthx, Earthy, Earthz, earth);
            a2 += 0.0078f;
            CreateOrbit(Orbit3, 0, 0, 0);
            // CreateSphere(1, Earthx,Earthy,Earthz, moon);
            CreateOrbit(Earth, Earthx, Earthy, Earthz);
        }
        else
        {
            a2 = 0;
        };

        if (a3 < 360)
        {
            float Moonx = Earthx - 2 * sinf((a3 * 3.14) / 180);
            float Moony = Earthy - 1 * sinf((a3*3.14)/180);
            float Moonz = Earthz - 2 * cosf((a3 * 3.14) / 180);
            CreateSphere(0.5, Moonx, Moony, Moonz, moon);
            a3 += 0.07f;
            // CreateOrbit(Orbit4,0,0,0);
        }
        else
        {
            a3 = 0;
        };

        if (a4 < 360)
        {
            float x = 25 * sinf((a4 * 3.14) / 180);
            float y = 2 * sinf((a4 * 3.14) / 180);
            float z = 25 * cosf((a4 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, mars);
            a4 += 0.0090f;
            CreateOrbit(Orbit5, 0, 0, 0);
        }
        else
        {
            a4 = 0;
        };

        if (a5 < 360)
        {
            float x = 30 * sinf((a5 * 3.14) / 180);
            float y = 2 * sinf((a5 * 3.14) / 180);
            float z = 30 * cosf((a5 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, jupiter);
            a5 += 0.00678f;
            CreateOrbit(Orbit6, 0, 0, 0);
        }
        else
        {
            a5 = 0;
        };

        if (a6 < 360)
        {
            float x = 35 * sinf((a6 * 3.14) / 180);
            float y = 2 * sinf((a6 * 3.14) / 180);
            float z = 35 * cosf((a6 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, saturn);
            a6 += 0.0023f;
            CreateOrbit(Orbit7, 0, 0, 0);
        }
        else
        {
            a6 = 0;
        };

        if (a7 < 360)
        {
            float x = 40 * sinf((a7 * 3.14) / 180);
            float y = 2 * sinf((a7 * 3.14) / 180);
            float z = 40 * cosf((a7 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, uranus);
            CreateOrbit(Orbit8, 0, 0, 0);
            a7 += 0.00456f;
        }
        else
        {
            a7 = 0;
        };

        if (a8 < 360)
        {
            float x = 45 * sinf((a8 * 3.14) / 180);
            float y = 2 * sinf((a8 * 3.14) / 180);
            float z = 45 * cosf((a8 * 3.14) / 180);
            CreateSphere(0.5, x, y, z, neptune);
            CreateOrbit(Orbit9, 0, 0, 0);
            a8 += 0.0056f;
        }
        else
        {
            a8 = 0;
        };

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 07;
}