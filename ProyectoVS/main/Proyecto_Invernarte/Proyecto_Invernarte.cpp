/*
*
* Proyecto Base Galería Estática (Basado en 09-Animation)
*
*/

#include <iostream>
#include <stdlib.h>
#include <vector> // Asegúrate de tener esta (para las luces)
#include <string> // Y esta (para los helpers de luces)
#include <sstream> // Y esta (para los helpers de luces)

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Clases de carga (Headers de tu proyecto)
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

// Funciones
bool Start();
bool Update();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Globales
GLFWwindow* window;

// Tamaño de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición inicial en XYZ)
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables de tiempo
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Shaders
Shader* staticLightShader; // Renombrado de mLightsShader para más claridad
Shader* cubemapShader;

// Modelos
Model* gallery; // Tu modelo de galería estática

// Cubemap (fondo)
CubeMap* mainCubeMap;

// Luces
std::vector<Light> gLights;

// Materiales (puedes definir más si tu galería usa varios)
Material material01;

// --- Funciones de ayuda para luces (Copiadas de tu práctica) ---
void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();
	shader->setInt(uniformName.c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();
	shader->setFloat(uniformName.c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();
	shader->setVec4(uniformName.c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();
	shader->setVec3(uniformName.c_str(), value);
}
// --- Fin de funciones de ayuda ---


// Entrada a función principal
int main()
{
	if (!Start())
		return -1;

	/* Loop principal de renderizado */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;
}

bool Start() {
	// Inicialización de GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Galeria", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Capturar el cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Carga de GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);
	// Activar transparencias
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compilación de shaders (¡Necesitaré estos archivos!)
	staticLightShader = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");

	// Carga del modelo de la galería
	// Asegúrate que la ruta sea correcta dentro de tu carpeta 'bin'
	gallery = new Model("modelos/galeria.fbx"); // <-- CAMBIA "galeria.fbx" POR EL NOMBRE DE TU ARCHIVO

	// Carga del Cubemap (fondo)
	vector<std::string> faces
	{
		"textures/cubemap/01/posx.png",
		"textures/cubemap/01/negx.png",
		"textures/cubemap/01/posy.png",
		"textures/cubemap/01/negy.png",
		"textures/cubemap/01/posz.png",
		"textures/cubemap/01/negz.png"
		// Asegúrate que estas texturas existan en 'bin/textures/...'
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	// Configuración de luces (puedes ajustar esto como necesites)
	Light light01;
	light01.Position = glm::vec3(5.0f, 2.0f, 5.0f);
	light01.Color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	gLights.push_back(light01);

	Light light02;
	light02.Position = glm::vec3(-5.0f, 2.0f, 5.0f);
	light02.Color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	gLights.push_back(light02);

	// Configuración de material (puedes ignorar esto si tu modelo .fbx ya trae materiales)
	material01.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	material01.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	material01.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	material01.transparency = 1.0f;

	return true;
}


bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Procesar entrada
	processInput(window);

	// Limpiar buffers
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Matrices de Vista y Proyección (solo cámara en 1ra persona)
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// 1. Dibujar el Cubemap (fondo)
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}

	// 2. Dibujar la Galería (Modelo Estático)
	{
		staticLightShader->use();

		staticLightShader->setMat4("projection", projection);
		staticLightShader->setMat4("view", view);

		// Transformaciones del modelo (Galería)
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Posición en el mundo
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// Escala
		staticLightShader->setMat4("model", model);

		// Configuración de luces
		staticLightShader->setInt("numLights", (int)gLights.size());
		for (size_t i = 0; i < gLights.size(); ++i) {
			SetLightUniformVec3(staticLightShader, "Position", i, gLights[i].Position);
			SetLightUniformVec4(staticLightShader, "Color", i, gLights[i].Color);
			// ... (puedes añadir más propiedades de luz si tu shader las usa)
		}

		staticLightShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales (Opcional, si el modelo no las tiene)
		// staticLightShader->setVec4("MaterialAmbientColor", material01.ambient);
		// staticLightShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		// staticLightShader->setVec4("MaterialSpecularColor", material01.specular);
		// staticLightShader->setFloat("transparency", material01.transparency);

		gallery->Draw(*staticLightShader); //¡Dibujamos la galería!
	}

	glUseProgram(0);

	// Swap buffers y poll events
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Controles de cámara
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// Modos de dibujado
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Relleno
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Puntos
}

// Callback: Actualizar viewport si la ventana cambia de tamaño
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Callback: Movimiento del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // Invertido

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// Callback: Scroll del mouse (Zoom)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}