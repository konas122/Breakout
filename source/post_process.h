#ifndef POST_PROCESS_H
#define POST_PROCESS_H

#include <glm/glm.hpp>

#include "shader.h"
#include "texture.h"
#include "sprite_renderer.h"


class PostProcessor {
public:
    Shader PostProcessor_Shader;
    Texture2D Texture;

    unsigned int Width, Height;
    bool confuse, chaos, shake;

    PostProcessor(Shader shader, unsigned int width, unsigned int height);
    void BeginRender();
    void EndRender();
    void Render(float time);

private:
    unsigned int MSFBO, FBO;    // MSFBO = Multisampled FBO. FBO is regular, used for blitting MS color-buffer to texture
    unsigned int RBO;           // RBO is used for multisampled color buffer
    unsigned int VAO;

    void initRenderData();
};

#endif
