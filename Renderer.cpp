
#include "Renderer.h"





Renderer::Renderer()
{


}
// render line of text
// -------------------
void Renderer::RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(UIVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, UIVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::renderCharacter(Model ourModel, Shader characterShader, unsigned int texture, Animator animator) {


    //activate shader
    characterShader.use();

    // bind rock map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    characterShader.setMat4("projection", projection);
    characterShader.setMat4("view", view);

    // render the loaded model
    auto transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        characterShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, camera.Position + glm::vec3(0.0f, -1.0f, -4.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
    float angle = 90.0f;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 15.0f, 0.0f));
    characterShader.setMat4("model", model);
    ourModel.Draw(characterShader);


}

void Renderer::renderEnvironment(Shader lightingShader, unsigned int rockMap) {



    // render
    // ------


    //activate shader
    lightingShader.use();

    // create transformations
    // pass projection matrix to shader (note that in this case it could change every frame)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    lightingShader.setMat4("projection", projection);
    lightingShader.setVec3("light.position", camera.Position + glm::vec3(0.0f, -1.0f, -4.0f));
    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("view", view);

    // bind rock map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockMap);

    // render boxes
    env_VAO.Bind();



    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 1.0f, -1.0f));
    float angle = 0.0f;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    lightingShader.setMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 36);



}


void Renderer::renderHUD(Shader textShader, Shader hearthShader, unsigned int texture) {


    

    

    glDisable(GL_DEPTH_TEST);


    // render
    // ------


    //activate shader
    hearthShader.use();



    // bind rock map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    health_VAO.Bind();
    health_VBO.Bind();

    glDisable(GL_BLEND);
    glEnable(GL_BLEND);


    // render hearths



    hearthShader.setFloat("sampler", 0);
    hearthShader.setVec4("color", glm::vec4(1.0, 1.0, 1.0, 1.0));

    glm::mat4 model = glm::mat4(1.0);
    //buyukluk ayari
    model = glm::scale(model, glm::vec3(60.0, 40.0, 1));
    hearthShader.setMat4("matrices.modelMatrix", model);

    for (int i = 0; i < health; i++) {

        glm::mat4 _orthoMatrix = glm::ortho(-300.0f - (i * 90.0f), (float)SCR_WIDTH, -30.0f, (float)SCR_HEIGHT);
        hearthShader.setMat4("matrices.projectionMatrix", _orthoMatrix);
        //draw the heart



        glDrawArrays(GL_TRIANGLES, 0, 6);


    }

    glEnable(GL_DEPTH_TEST);




    RenderText(textShader, "Points : " + std::to_string(points), 25.0f, 80.0f, 1.0f, glm::vec3(0.1, 0.1f, 0.9f));
    RenderText(textShader, "Health : ", 25.0f, 15.0f, 1.0f, glm::vec3(0.1, 0.1f, 0.9f));

}



void Renderer::renderHUDEntity(Entity& hearth, Shader textShader, Shader hearthShader, unsigned int texture) {


    glDisable(GL_DEPTH_TEST);


    // render
    // ------


    //activate shader
    hearthShader.use();



    // bind rock map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    health_VAO.Bind();
    health_VBO.Bind();



    // render hearths



    hearthShader.setFloat("sampler", 0);
    hearthShader.setVec4("color", glm::vec4(1.0, 1.0, 1.0, 1.0));

    glm::mat4 model = glm::mat4(1.0);
    //buyukluk ayari
    model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down



    glDisable(GL_BLEND);
    glEnable(GL_BLEND);

    for (int i = 0; i < health; i++) {

        float angle = 0.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 15.0f  , 0.0f));
        hearthShader.setMat4("matrices.modelMatrix", model);

        glm::mat4 _orthoMatrix = glm::ortho(-300.0f - (i * 90.0f), (float)SCR_WIDTH, -30.0f, (float)SCR_HEIGHT);
        hearthShader.setMat4("matrices.projectionMatrix", _orthoMatrix);
        //draw the heart

        hearth.pModel->Draw(hearthShader);


    }

    glEnable(GL_DEPTH_TEST);




    RenderText(textShader, "Points : " + std::to_string(points), 25.0f, 80.0f, 1.0f, glm::vec3(0.1, 0.1f, 0.9f));
    RenderText(textShader, "Health : ", 25.0f, 15.0f, 1.0f, glm::vec3(0.1, 0.1f, 0.9f));

}

void Renderer::setupVAOVBO() {


    char_VBO.setup(cube, sizeof(cube));

    char_VAO.Bind();

    char_VBO.char_EBO();

    health_VBO.setup(hearth, sizeof(hearth));

    health_VAO.Bind();

    health_VBO.hearth_EBO();


    env_VBO.setup(vertices, sizeof(vertices));

    env_VAO.Bind();

    env_VBO.env_EBO();

    lightEnvironmentVAO.Bind();

}

void Renderer::deleteVAOVBO() {


    char_VAO.Delete();
    char_VBO.Delete();


    env_VAO.Delete();
    env_VBO.Delete();

    health_VAO.Delete();
    health_VBO.Delete();
}





void Renderer::renderEntity(Entity& ourEntity, Shader characterShader, unsigned int texture, const Frustum camFrustum, int select) {


    //activate shader
    characterShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (select == 1) {
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        characterShader.setMat4("projection", projection);
        characterShader.setMat4("view", view);



        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ourEntity.transform.getGlobalPosition()); // translate it down so it's at the center of the scene
        model = glm::scale(model, ourEntity.transform.getLocalScale());	// it's a bit too big for our scene, so scale it down
        float angle = 90.0f;
        model = glm::rotate(model, glm::radians(ourEntity.transform.getLocalRotation().y), ourEntity.transform.getLocalRotation());
        characterShader.setMat4("model", model);

        if (ourEntity.boundingVolume->isOnFrustum(camFrustum, ourEntity.transform))
        {

            ourEntity.pModel->Draw(characterShader);
            
        }
    }
    else {

        // create transformations
        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        characterShader.setMat4("projection", projection);
        characterShader.setVec3("light.position", camera.Position + glm::vec3(0.0f, -1.0f, -4.0f));
        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        characterShader.setMat4("view", view);


        // render boxes
        env_VAO.Bind();



        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ourEntity.transform.getGlobalPosition());
        float angle = 0.0f;
        model = glm::scale(model, ourEntity.transform.getLocalScale());	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(ourEntity.transform.getLocalRotation().y), ourEntity.transform.getLocalRotation());
        characterShader.setMat4("model", model);

        if (ourEntity.boundingVolume->isOnFrustum(camFrustum, ourEntity.transform))
        {

            ourEntity.pModel->Draw(characterShader);
            
        }
    }
    ourEntity.updateSelfAndChild();
    

}

void Renderer::renderSpyViewEntity(Entity& ourEntity, Shader characterShader, unsigned int texture, const Frustum camFrustum, int select) {

    if (select == 1) {
        //activate shader
        characterShader.use();


        if (ourEntity.boundingVolume->isOnFrustum(camFrustum, ourEntity.transform))
        {
            //activate shader
            characterShader.use();

            // bind rock map
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glm::mat4 projection = glm::perspective(glm::radians(cameraSpy.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 1000.0f);
            glm::mat4 view = cameraSpy.GetViewMatrix();
            characterShader.setMat4("projection", projection);
            characterShader.setMat4("view", view);



            // render the loaded model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, camera.Position + glm::vec3(0.0f, -1.0f, -4.0f)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
            float angle = 90.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 15.0f, 0.0f));
            characterShader.setMat4("model", model);
            ourEntity.pModel->Draw(characterShader);
            ourEntity.updateSelfAndChild();


        }
    }
    else {
        if (ourEntity.boundingVolume->isOnFrustum(camFrustum, ourEntity.transform))
        {

            //activate shader
            characterShader.use();

            // create transformations
            // pass projection matrix to shader (note that in this case it could change every frame)
            glm::mat4 projection = glm::perspective(glm::radians(cameraSpy.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            characterShader.setMat4("projection", projection);
            characterShader.setVec3("light.position", camera.Position + glm::vec3(0.0f, -1.0f, -4.0f));
            // camera/view transformation
            glm::mat4 view = cameraSpy.GetViewMatrix();
            characterShader.setMat4("view", view);

            // bind rock map
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            // render boxes
            env_VAO.Bind();



            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -20.0f, -80.0f));
            float angle = 0.0f;
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));	// it's a bit too big for our scene, so scale it down
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 15.0f, 0.0f));
            characterShader.setMat4("model", model);

            ourEntity.pModel->Draw(characterShader);
            ourEntity.updateSelfAndChild();


        }
    }


}


unsigned int Renderer::loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


void Renderer::setupFreeType(Shader textShader) 
{
    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // find path to font
    std::string font_name = "Raleway-Black.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Renderer::Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));





    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &UIVAO);
    glGenBuffers(1, &UIVBO);
    glBindVertexArray(UIVAO);
    glBindBuffer(GL_ARRAY_BUFFER, UIVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
