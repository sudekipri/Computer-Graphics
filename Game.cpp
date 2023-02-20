#include "Game.h"


//Game State
GameState Game::gameState = GameState::GAME_MENU;
GLFWwindow* window;




/// <summary>
/// this function inits everything before the game can be started
/// </summary>
void Game::init()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CS405 Final Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return ;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return ;
    }
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    //Camera movement speed
    camera.MovementSpeed = 20.f;

    // build and compile our shader zprogram
    // ------------------------------------

    Renderer renderer;

    Shader lightingShader("LightingShader.vert", "LightingShader.frag");
    Shader characterShader("characterShader.vert", "characterShader.frag");
    Shader textShader("textShader.vert", "textShader.frag");
    Shader hearthShader("hearthShader.vert", "hearthShader.frag");
    Shader modelShader("modelShader.vert", "modelShader.frag");

    Model ourModel("bird/bird.obj");
    Model corridorModel("untitled.obj");
    Model hearthModel("hearth2.obj");

    Animation fly("bird/fly.dae", &ourModel);
    Animator animator(&fly);

    Entity birdEntity(ourModel);
    Entity corridorEntity(corridorModel);
    Entity hearthEntity(hearthModel);

    
    birdEntity.locAndScale( glm::vec3(0.0f, -1.0f, -4.0f), 0.1f);
    birdEntity.transform.setLocalRotation({ 0.0f, 90.f, 0.0f });




    corridorEntity.locAndScale(glm::vec3(0.0f, -20.0f, 0.0f), 0.075f);
    corridorEntity.transform.setLocalRotation({0.0f, 90.0f, 0.0f});

    //birdEntity.addChild(corridorEntity);

    renderer.setupFreeType(textShader);


    //lighting shader settings
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    // be sure to activate shader when setting uniforms/drawing objects
    lightingShader.use();
    lightingShader.setVec3("light.position", camera.Position + glm::vec3(0.0f, -1.0f, -4.0f));
    lightingShader.setVec3("light.direction", camera.Front + glm::vec3(0.0f, -1.0f, -4.0f));
    lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(35.0f)));
    lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(75.0f)));
    lightingShader.setVec3("viewPos", camera.Position);

    // light properties
    lightingShader.setVec3("light.ambient", 25.0f, 25.0f, 25.0f);
    lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("light.constant", 1.0f);
    lightingShader.setFloat("light.linear", 0.09f);
    lightingShader.setFloat("light.quadratic", 0.032f);

    // material properties
    lightingShader.setFloat("material.shininess", 32.0f);

    // load and create a texture 
// -------------------------

    unsigned int wallMap = renderer.loadTexture("wall.jpg");
    unsigned int rockMap = renderer.loadTexture("_Carrera_Marble_1.jpg");
    unsigned int birdTexture = renderer.loadTexture("bird/txtr01.jpg");
    unsigned int hearthTexture = renderer.loadTexture("hearth.jpg");


    renderer.setupVAOVBO();

    bool flag = false;
    while (!glfwWindowShouldClose(window))
    {


        // render loop
        // -----------


            // per-frame time logic
            // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
         // -----

        birdEntity.locAndScale(camera.Position + glm::vec3(0.0f, -1.0f, -4.0f), 0.1f);
        birdEntity.updateSelfAndChild();
        
        int input = 0;
        input = processInput(window);
  
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            
            flag = !flag;

        }


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        //Environment

        //renderer.renderEnvironment(lightingShader, rockMap);

        //Character Model
        //renderer.renderCharacter(ourModel, modelShader, birdTexture, animator);


        
        const Frustum camFrustum = createFrustumFromCamera(camera,(float)SCR_WIDTH / (float)SCR_HEIGHT, glm::radians(camera.Zoom), 0.1f, 10.0f);

        if (flag) {
            renderer.renderSpyViewEntity(corridorEntity, lightingShader, rockMap, camFrustum, 4);
            renderer.renderSpyViewEntity(birdEntity, modelShader, birdTexture, camFrustum, 1);
        }
        else {
            renderer.renderEntity(corridorEntity, lightingShader, rockMap, camFrustum, 4);
            renderer.renderEntity(birdEntity, modelShader, birdTexture, camFrustum, 1);
        }


        //Render HUD
        renderer.renderHUDEntity(hearthEntity, textShader, hearthShader, hearthTexture);

        int colCheck = 0;
        if (CheckCollision(birdEntity, corridorEntity)) {

            points++;
        }
        else {
            colCheck++;
            if (input == 1) {
                camera.ProcessKeyboard(DOWNWARD, deltaTime * 7);
                input = 0;


            }
            if (input == 2) {
                camera.ProcessKeyboard(RIGHT, deltaTime *7);
                input = 0;


            }
            if (input == 3) {
                camera.ProcessKeyboard(UPWARD, deltaTime*7);
                input = 0;

            }
            if (input == 4) {
                camera.ProcessKeyboard(LEFT, deltaTime*7);
                input = 0;


            }
            
            
            health--;

         
            
        }






        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    renderer.deleteVAOVBO();


	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();

}

void Game::gameLoop(Renderer renderer, Shader lightingShader, Shader modelShader, Shader textShader, Shader hearthShader, Model ourModel, Animator animator, unsigned int wallMap, unsigned int rockMap, unsigned int birdTexture)
{


    while (!glfwWindowShouldClose(window))
    {


        // render loop
        // -----------


            // per-frame time logic
            // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
         // -----
        processInput(window);
      

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        //Environment

        renderer.renderEnvironment(lightingShader, rockMap);


        //Character Model
        renderer.renderCharacter(ourModel, modelShader, birdTexture, animator);



        //Render HUD
        renderer.renderHUD(textShader, hearthShader, wallMap);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}
int Game::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(UPWARD, deltaTime);
        return 1;

    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {

        camera.ProcessKeyboard(LEFT, deltaTime);
        return 2;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        //camera.ProcessKeyboard(BACKWARD, deltaTime);
        camera.ProcessKeyboard(DOWNWARD, deltaTime);
        return 3;

    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);

        return 4;
        
    }
    return 0;


}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void Game::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
    cameraSpy.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Game::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}