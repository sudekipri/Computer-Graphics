#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include<iostream>

// Include GLAD
#include <glad/glad.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include "Renderer.h"




enum class GameState {
	GAME_LOADING,
	GAME_ACTIVE,
	GAME_GAME_OVER,
	GAME_PAUSED,
	GAME_MENU,
};


enum class MenuType {
	MENU_MAIN,
	MENU_OPTIONS,
	MENU_PAUSE,
};

enum class PlayerAction {
	None,
	moveLeft,
	moveRight,
	moveUp,
	moveDown,
	pause,
	menu,
	toggleFullscreen,
	enter,
};



static class Game
{
public:


	static void init();

	
	static void gameLoop(Renderer renderer, Shader lightingShader, Shader modelShader, Shader textShader, Shader hearthShader, Model ourModel, Animator animator, unsigned int wallMap, unsigned int rockMap, unsigned int birdTexture);

	static int processInput(GLFWwindow* window);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


	static void setGameState(GameState newGameState)
	{
		gameState = newGameState;
	}

	static GameState getGameState()
	{
		return gameState;
	}

	static GameState gameState;


};