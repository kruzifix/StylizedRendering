#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <dc/Shader.hpp>
#include <dc/ObjLoader.hpp>
#include <dc/Mesh.hpp>
#include <dc/Texture.hpp>
#include <dc/FrameBuffer.hpp>

const unsigned int dpi_scale = 1;
const unsigned int width = 1280 * dpi_scale;
const unsigned int height = 720 * dpi_scale;
const unsigned int fboDownscale = 1;

struct OrbitCamera
{
    glm::vec3 target;
    float azimuth;
    float elevation;
    float distance;
    bool moving;

    int lastX;
    int lastY;

    glm::mat4 getViewMatrix() const
    {
        glm::vec3 eyePos{ sin(elevation) * cos(azimuth), cos(elevation), sin(elevation) * sin(azimuth) };
        eyePos *= distance;
        return glm::lookAt(target + eyePos, target, { 0.0f, 1.0f, 0.0f });
    }
};

OrbitCamera* camera = nullptr;

static void mouse_button_callback(GLFWwindow* window, int button, int state, int)
{
    if (camera)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            camera->moving = state == GLFW_PRESS;
        }
    }
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y)
{
    if (camera)
    {
        int curX = static_cast<int>(x);
        int curY = static_cast<int>(y);

        if (camera->moving)
        {
            int deltaX = camera->lastX - curX;
            int deltaY = camera->lastY - curY;

            camera->azimuth -= deltaX * 0.01f;
            camera->elevation += deltaY * 0.01f;
            camera->elevation = glm::clamp(camera->elevation, 0.001f, glm::pi<float>());
        }

        camera->lastX = curX;
        camera->lastY = curY;
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (camera)
    {
        camera->distance -= yoffset;
        camera->distance = glm::clamp(camera->distance, 1.0f, 100.0f);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Mesh Rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    camera = new OrbitCamera{ { 0.0f, 0.5f, 0.0f }, 0.0f, 0.5f, 4.0f, false, 0, 0 };

    dc::Shader shader({ { dc::ShaderStage::Vertex, "vertex.glsl" },{ dc::ShaderStage::Fragment, "fragment.glsl" } });
    dc::Shader quadShader({ { dc::ShaderStage::Vertex, "quad.glsl" },{ dc::ShaderStage::Fragment, "sobel.glsl" } });

    dc::ObjLoader loader("../models/basic_model.obj");
    auto mesh = loader.exportMesh();

    GLuint quadVAO;
    glGenVertexArrays(1, std::addressof(quadVAO));

    unsigned int fboWidth = width / fboDownscale;
    unsigned int fboHeight = height / fboDownscale;

    dc::FrameBuffer fbo(fboWidth, fboHeight, {
        { dc::FBAttachmentType::AttachColor, dc::TextureFormat::RGB8 },
        { dc::FBAttachmentType::AttachColor, dc::TextureFormat::RGB8 },
        { dc::FBAttachmentType::AttachDepth, dc::TextureFormat::Depth24 }
    });

    glm::mat4 projection = glm::perspectiveFov<float>(glm::radians(60.0f), width, height, 0.1f, 100.0f);
    //glm::mat4 projection = glm::ortho<float>(-10, 10, -8, 8, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    int lastSpace = GLFW_RELEASE;
    int lastS = GLFW_RELEASE;

    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        int space = glfwGetKey(window, GLFW_KEY_SPACE);
        int skey = glfwGetKey(window, GLFW_KEY_S);
        if (space == GLFW_PRESS && lastSpace == GLFW_RELEASE)
        {
            // reload mesh!
            // TODO: find out why this doesn't work with asset forge! it doesn't want to write to the obj file while app is running
            dc::ObjLoader newLoader("../models/basic_model.obj");
            mesh = loader.exportMesh();
        }
        if (skey == GLFW_PRESS && lastS == GLFW_RELEASE)
        {
            // reload shader!
            shader.reload();
            quadShader.reload();
        }
        lastSpace = space;
        lastS = skey;

        fbo.bind();
        glViewport(0, 0, fbo.width(), fbo.height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        shader.use();
        shader.setMat4("view", camera->getViewMatrix());
        shader.setMat4("projection", projection);

        for (int z = -1; z < 2; ++z)
        {
            for (int x = -1; x < 2; ++x)
            {
                glm::mat4 model = glm::rotate(glm::translate(glm::mat4(1.0f), { 15.0f * x, 0, 15.0f * z }), (x + z)*2.0f, { 0, 1, 0 });
                shader.setMat4("model", model);
                mesh->draw(shader);
            }
        }

        glUseProgram(0);
        fbo.unbind();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        quadShader.use();
        glActiveTexture(GL_TEXTURE0);
        fbo.texture(0)->bind();
        quadShader.setInt("colorTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        fbo.texture(1)->bind();
        quadShader.setInt("normalTexture", 1);

        glActiveTexture(GL_TEXTURE2);
        fbo.texture(2)->bind();
        quadShader.setInt("depthTexture", 2);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, std::addressof(quadVAO));

    delete camera;

    glfwTerminate();
    return 0;
}