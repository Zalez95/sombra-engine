#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "utils/Logger.h"
#include "utils/FileReader.h"

#include "window/WindowSystem.h"

#include "graphics/GraphicsSystem.h"
#include "graphics/Texture.h"
#include "graphics/2D/Renderable2D.h"
#include "graphics/3D/Mesh.h"
#include "graphics/3D/Material.h"
#include "graphics/3D/Camera.h"
#include "graphics/3D/Lights.h"
#include "graphics/3D/Renderable3D.h"
#include "graphics/GraphicsSystem.h"

#include "loaders/MeshLoader.h"


#define WIDTH	1280
#define HEIGHT	720
#define FOV		45.0f
#define Z_NEAR	1.0f
#define	Z_FAR	500.0f


int main(int argc, char** argv)
{
	// Window
	window::WindowSystem* windowSystem;
	if (!(windowSystem = new window::WindowSystem("< FAZE >", WIDTH, HEIGHT))) {
		Logger::writeLog(LogType::ERROR, "Error initializing the window system");
		return -1;
	}
	windowSystem->setMousePosition(WIDTH / (float)2, HEIGHT / (float)2);
	windowSystem->printGLInfo();

	// Graphics
	graphics::GraphicsSystem* graphicsSystem;
	if (!(graphicsSystem = new graphics::GraphicsSystem())) {
		Logger::writeLog(LogType::ERROR, "Error initializing the graphics system");
		return -1;
	}

	/*********************************************************************
	 * GRAPHICS DATA
	 *********************************************************************/
	// Graphics Primitives
	graphics::MeshLoader meshLoader;
	std::vector<GLfloat> positions = {
		-0.5f,	-0.5f,	-0.5f,
		-0.5f,	-0.5f,	0.5f,
		-0.5f,	0.5f,	-0.5f,
		-0.5f,	0.5f,	0.5f,
		0.5f,	-0.5f,	-0.5f,
		0.5f,	-0.5f,	0.5f,
		0.5f,	0.5f,	-0.5f,
		0.5f,	0.5f,	0.5f
	};
	std::vector<GLushort> indices = {
		0, 1, 2,
		1, 3, 2,
		0, 2, 4,
		2, 6, 4,
		4, 6, 5,
		5, 6, 7,
		1, 5, 3,
		3, 5, 7,
		0, 4, 1,
		1, 4, 5,
		2, 3, 6,
		3, 7, 6
	};
	std::vector<GLfloat> normals = meshLoader.calculateNormals(positions, indices);
	std::shared_ptr<graphics::Mesh> mesh1 = std::move(meshLoader.createMesh("Cubo", positions, normals, std::vector<GLfloat>(16), indices));

	auto material1 = std::make_shared<graphics::Material>(
		"material1",
		graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
		graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
		graphics::RGBColor{ 0.1f, 1.0f, 0.1f },
		0.1f
	);

	auto material2 = std::make_shared<graphics::Material>(
		"material2",
		graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
		graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
		graphics::RGBColor{ 0.1f, 0.1f, 1.0f },
		0.1f
	);

	auto material3 = std::make_shared<graphics::Material>(
		"material3",
		graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
		graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
		graphics::RGBColor{ 1.0f, 0.1f, 0.1f },
		0.1f
	);

	auto material4 = std::make_shared<graphics::Material>(
		"material4",
		graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
		graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
		graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
		0.1f
	);

	auto texture1 = std::make_shared<graphics::Texture>("res/images/test.png", GL_TEXTURE_2D);

	graphics::BaseLight baseLight1(4.0f, 24.0f);
	graphics::Attenuation attenuation1{ 0.5f, 0.25f, 0.2f };

	// Cameras
	graphics::Camera camera1(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

	// Lights
	std::vector<const graphics::PointLight*> pointLights;
	graphics::PointLight pointLight1(baseLight1, attenuation1, glm::vec3(2, 1, 5));
	graphics::PointLight pointLight2(baseLight1, attenuation1, glm::vec3(-3, 1, 5));
	pointLights.push_back(&pointLight1);
	pointLights.push_back(&pointLight2);

	// Renderable2Ds
	std::vector<const graphics::Renderable2D*> renderable2Ds;
	graphics::Renderable2D renderable2D1(glm::vec2(0.8f, 0.75f), glm::vec2(0.125f, 0.2f), texture1);
	renderable2Ds.push_back(&renderable2D1);

	// Renderable3Ds
	std::vector<const graphics::Renderable3D*> renderable3Ds;
	for (unsigned int i = 0; i < 500; ++i) {
		auto renderable3D1 = new graphics::Renderable3D(mesh1, material1, nullptr, false);
		renderable3D1->setModelMatrix(glm::translate(glm::vec3(
			100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
			100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
			100 * (static_cast<float>(rand()) / RAND_MAX) - 50)
		));
		renderable3Ds.push_back(renderable3D1);
	}

	graphics::Renderable3D* renderable3D_centro = new graphics::Renderable3D(mesh1, material4, nullptr, false);
	renderable3D_centro->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, -10)));
	renderable3Ds.push_back(renderable3D_centro);

	graphics::Renderable3D* renderable3D_derecha = new graphics::Renderable3D(mesh1, material3, nullptr, false);
	renderable3D_derecha->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(2, 0, -10)));
	renderable3Ds.push_back(renderable3D_derecha);

	graphics::Renderable3D* renderable3D_arriba = new graphics::Renderable3D(mesh1, material1, nullptr, false);
	renderable3D_arriba->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 2, -10)));
	renderable3Ds.push_back(renderable3D_arriba);

	graphics::Renderable3D* renderable3D_frente = new graphics::Renderable3D(mesh1, material2, nullptr, false);
	renderable3D_frente->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, -8)));
	renderable3Ds.push_back(renderable3D_frente);


	/*********************************************************************
	 * MAIN LOOP
	 *********************************************************************/
	bool end = false;

	float lastTime = windowSystem->getTime(), elapsed = lastTime;
	int fps = 0;
	while ( !end ) {
		// Calculate delta (elapsed time)
		float curTime = windowSystem->getTime();
		float delta = curTime - lastTime;
		lastTime = curTime;

		// Update the FPSs
		fps++;
		if (lastTime - elapsed >= 1.0f) {
			elapsed = lastTime;
			std::cout << "FPS: " << fps << '\r' << std::flush;
			fps = 0;
		}

		windowSystem->update();
		window::InputData inputData = windowSystem->getInputData();
		if (inputData.mKeys[GLFW_KEY_ESCAPE] || windowSystem->isClosed()) { end = true; }
		graphicsSystem->render(&camera1, renderable3Ds, renderable2Ds, pointLights);
		windowSystem->swapBuffers();
	}

	for (const graphics::Renderable3D* r3d : renderable3Ds) {
		delete r3d;
	}

	delete graphicsSystem;
	delete windowSystem;

	return 0;
}
