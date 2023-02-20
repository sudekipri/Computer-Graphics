#ifndef RENDERER_CLASS_H
#define RENDERER_CLASS_H



#include<glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "Animator.h"

#include "VAO.h"
#include "VBO.h"
#include "Variables.cpp"

#include <ft2build.h>
#include FT_FREETYPE_H


// settings
extern const unsigned int SCR_WIDTH = 1280;
extern const unsigned int SCR_HEIGHT = 920;


extern Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
extern Camera cameraSpy(glm::vec3(0.0f, 0.0f, -30.0f));
extern float lastX = SCR_WIDTH / 2.0f;
extern float lastY = SCR_HEIGHT / 2.0f;
extern bool firstMouse = true;

// timing
extern float deltaTime = 0.0f;	// time between current frame and last frame
extern float lastFrame = 0.0f;

//health and points
extern int health = 3; // number of remaining lives
extern int points = 0;


class Renderer
{
public:
	Renderer();
	unsigned int loadTexture(char const* path);
	void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
	void renderCharacter(Model ourModel, Shader characterShader, unsigned int texture, Animator animator);
	void renderEnvironment(Shader lightingShader, unsigned int rockMap);
	void renderHUD(Shader textShader, Shader hearthShader, unsigned int texture);
	void renderHUDEntity(Entity& hearth, Shader textShader, Shader hearthShader, unsigned int texture);
	void renderEntity(Entity& ourEntity, Shader characterShader, unsigned int texture, const Frustum camFrustum, int select);
	void renderSpyViewEntity(Entity& ourEntity, Shader characterShader, unsigned int texture, const Frustum camFrustum, int select);
	void setupFreeType(Shader textShader);
	void setupVAOVBO();
	void deleteVAOVBO();

	struct Character {
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2   Size;      // Size of glyph
		glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
		unsigned int Advance;   // Horizontal offset to advance to next glyph
	};

	std::map<GLchar, Character> Characters;
	unsigned int UIVAO, UIVBO;

	VAO health_VAO, env_VAO, char_VAO, lightEnvironmentVAO;
	VBO health_VBO, env_VBO, char_VBO;










};





#endif