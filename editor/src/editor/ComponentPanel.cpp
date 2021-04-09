#include <sstream>
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/io/MeshLoader.h>
#include <se/app/Scene.h>
#include <se/app/TagComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/TerrainComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/app/AnimationComponent.h>
#include <se/app/graphics/TextureUtils.h>
#include <se/physics/RigidBody.h>
#include <se/collision/BoundingBox.h>
#include <se/collision/BoundingSphere.h>
#include <se/collision/Capsule.h>
#include <se/collision/TriangleCollider.h>
#include <se/collision/TerrainCollider.h>
#include <se/collision/CompositeCollider.h>
#include <se/animation/SkeletonAnimator.h>
#include <se/utils/StringUtils.h>
#include "ComponentPanel.h"
#include "Editor.h"
#include "ImGuiUtils.h"

using namespace se::app;
using namespace se::graphics;
using namespace se::animation;
using namespace se::physics;
using namespace se::collision;

namespace editor {

	class ComponentPanel::IComponentNode
	{
	public:		// Functions
		virtual ~IComponentNode() = default;
		virtual const char* getName() const = 0;
		virtual void create(Entity entity) = 0;
		virtual bool check(Entity entity) = 0;
		virtual void draw(Entity entity) = 0;
		virtual void remove(Entity entity) = 0;
	};


	template <typename T>
	class ComponentPanel::ComponentNode : public ComponentPanel::IComponentNode
	{
	protected:	// Attributes
		Editor& mEditor;

	public:		// Functions
		ComponentNode(Editor& editor) : IComponentNode(), mEditor(editor) {};
		virtual ~ComponentNode() = default;
		virtual bool check(Entity entity) override
		{
			return mEditor.getEntityDatabase().hasComponents<T>(entity);
		};
		virtual void remove(Entity entity) override
		{
			mEditor.getEntityDatabase().removeComponent<T>(entity);
		};
	};


	class ComponentPanel::TagComponentNode : public ComponentPanel::ComponentNode<TagComponent>
	{
	public:		// Functions
		TagComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Tag"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<TagComponent>(entity, ""); };
		virtual void draw(Entity entity) override
		{
			auto [tag] = mEditor.getEntityDatabase().getComponents<TagComponent>(entity);

			char nameBuffer[TagComponent::kMaxLength] = {};
			std::copy(tag->getName(), tag->getName() + tag->getLength(), nameBuffer);
			if (ImGui::InputText("Name##TagName", nameBuffer, TagComponent::kMaxLength)) {
				tag->setName(nameBuffer);
			}
		};
	};


	class ComponentPanel::TransformsComponentNode : public ComponentPanel::ComponentNode<TransformsComponent>
	{
	private:	// Orientation type
		int mOrientationType;

	public:		// Functions
		TransformsComponentNode(Editor& editor) : ComponentNode(editor), mOrientationType(0) {};
		virtual const char* getName() const override
		{ return "Transforms"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<TransformsComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [transforms] = mEditor.getEntityDatabase().getComponents<TransformsComponent>(entity);

			bool updated = false;
			updated |= ImGui::DragFloat3("Position", glm::value_ptr(transforms->position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat3("Velocity", glm::value_ptr(transforms->velocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= drawOrientation("Orientation##TransformsOrientation", transforms->orientation, mOrientationType);
			updated |= ImGui::DragFloat3("Scale", glm::value_ptr(transforms->scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

			if (updated) {
				transforms->updated.reset();
			}
		};
	};


	class ComponentPanel::AnimationComponentNode : public ComponentPanel::ComponentNode<AnimationComponent>
	{
	private:	// Functions
		std::array<char, NodeData::kMaxLength> mName = {};

	public:		// Functions
		AnimationComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Animation"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<AnimationComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [animation] = mEditor.getEntityDatabase().getComponents<AnimationComponent>(entity);
			auto node = animation->getRootNode();
			if (node) {
				ImGui::Text("%s (0x%p)", node->getData().name.data(), static_cast<void*>(node));
			}
			else {
				ImGui::Text("No node setted");
			}

			if (ImGui::TreeNode("Animators:")) {
				std::shared_ptr<SkeletonAnimator> sAnimator;
				if (addRepoDropdownButton("##AnimationComponent::AddSkeletonAnimator", "Add Animator", mEditor.getScene()->repository, sAnimator)) {
					animation->addAnimator(sAnimator);
				}

				std::size_t sAnimatorIndex = 0;
				animation->processSAnimators([&, animation = animation](std::shared_ptr<SkeletonAnimator> sAnimator2) {
					std::string buttonName = "x##AnimationComponent::RemoveSAnimator" + std::to_string(sAnimatorIndex++);
					if (ImGui::Button(buttonName.c_str())) {
						animation->removeAnimator(sAnimator2);
					}
					else {
						ImGui::SameLine();

						auto oldSAnimator2 = sAnimator2;
						std::string dropDownName = "##AnimationComponent::SelectSAnimator" + std::to_string(sAnimatorIndex++);
						if (addRepoDropdownShowSelected(dropDownName.c_str(), mEditor.getScene()->repository, sAnimator2)) {
							animation->removeAnimator(oldSAnimator2);
							animation->addAnimator(sAnimator2);
						}
					}
				});
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Change node:")) {
				ImGui::InputText("Name##AnimationComponent::NodeChangeName", mName.data(), mName.size());
				if (ImGui::Button("Change##AnimationComponent::ChangeNode")) {
					void* nodePtr = nullptr;
					std::istringstream(mName.data()) >> nodePtr;

					AnimationNode& root = mEditor.getScene()->rootNode;
					auto it = std::find_if(root.begin(), root.end(), [&](const AnimationNode& node) {
						return static_cast<const void*>(&node) == nodePtr;
					});
					if (it != root.end()) {
						animation->setRootNode(&(*it));
					}
				}
				ImGui::TreePop();
			}
		};
	};


	class ComponentPanel::CameraComponentNode : public ComponentPanel::ComponentNode<CameraComponent>
	{
	public:		// Functions
		CameraComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Camera"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<CameraComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [camera] = mEditor.getEntityDatabase().getComponents<CameraComponent>(entity);

			bool updated = false;
			bool ortho = camera->hasOrthographicProjection();
			if (ImGui::RadioButton("Orthographic", ortho)) { updated = !ortho; ortho = true; }
			ImGui::SameLine();
			if (ImGui::RadioButton("Perspective", !ortho)) { updated = ortho; ortho = false; }

			if (ortho) {
				float left = 0.0f, right = 1280.0f, bottom = 0.0f, top = 720.0f, zNear = 0.1f, zFar = 10000.0f;
				camera->getOrthographicParams(left, right, bottom, top, zNear, zFar);

				updated |= ImGui::DragFloat("Left", &left, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("Right", &right, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("Bottom", &bottom, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("Top", &top, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("zNear", &zNear, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("zFar", &zFar, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

				if (updated) {
					camera->setOrthographicProjection(left, right, bottom, top, zNear, zFar);
				}
			}
			else {
				float fovy = glm::pi<float>() / 3.0f, aspectRatio = 1280.0f / 720.0f, zNear = 0.1f, zFar = 10000.0f;
				camera->getPerspectiveParams(fovy, aspectRatio, zNear, zFar);

				float fovyDegrees = glm::degrees(fovy);
				updated |= ImGui::DragFloat("fovy", &fovyDegrees, 0.05f, 0.0f, 360, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("Aspect Ratio", &aspectRatio, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("zNear", &zNear, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat("zFar", &zFar, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

				if (updated) {
					camera->setPerspectiveProjection(glm::radians(fovyDegrees), aspectRatio, zNear, zFar);
				}
			}
		};
	};


	class ComponentPanel::LightComponentNode : public ComponentPanel::ComponentNode<LightComponent>
	{
	public:		// Functions
		LightComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Light"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<LightComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [light] = mEditor.getEntityDatabase().getComponents<LightComponent>(entity);

			ImGui::Text("Source:");
			ImGui::SameLine();
			addRepoDropdownShowSelected("##LightComponent", mEditor.getScene()->repository, light->source);
		};
	};


	class ComponentPanel::LightProbeComponentNode : public ComponentPanel::ComponentNode<LightProbe>
	{
	private:	// Attributes
		std::shared_ptr<Texture> mEnvironmentTexture;
		bool mIsCubeMap = false;
		int mCubeMapSize = 512;
		int mIrradianceMapSize = 32;
		int mPrefilterMapSize = 128;

	public:		// Functions
		LightProbeComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Light Probe"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<LightProbe>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [lightProbe] = mEditor.getEntityDatabase().getComponents<LightProbe>(entity);

			addRepoDropdownShowSelected("Irradiance map##IrradianceMap", mEditor.getScene()->repository, lightProbe->irradianceMap);
			addRepoDropdownShowSelected("Prefilter map##PrefilterMap", mEditor.getScene()->repository, lightProbe->prefilterMap);

			if (ImGui::TreeNode("Create from texture")) {
				addRepoDropdownShowSelected("Environment Map##EnvironmentMap", mEditor.getScene()->repository, mEnvironmentTexture);
				ImGui::Checkbox("Is cube map", &mIsCubeMap);
				if (!mIsCubeMap) {
					ImGui::DragInt("New cube map resolution", &mCubeMapSize, 0.01f, 0, INT_MAX);
				}

				ImGui::DragInt("Irradiance map resolution", &mIrradianceMapSize, 0.01f, 0, INT_MAX);
				ImGui::DragInt("Prefilter map resolution", &mPrefilterMapSize, 0.01f, 0, INT_MAX);

				if (ImGui::Button("Build probe##BuildProbe")) {
					auto cubeMap = mEnvironmentTexture;
					if (!mIsCubeMap) {
						cubeMap = TextureUtils::equirectangularToCubeMap(mEnvironmentTexture, mCubeMapSize);
						for (std::size_t i = 0; !mEditor.getScene()->repository.add("cubeMap" + std::to_string(i), cubeMap); ++i);
					}

					lightProbe->irradianceMap = TextureUtils::convoluteCubeMap(cubeMap, mIrradianceMapSize);
					lightProbe->prefilterMap = TextureUtils::prefilterCubeMap(cubeMap, mPrefilterMapSize);

					for (std::size_t i = 0; !mEditor.getScene()->repository.add("irradianceMap" + std::to_string(i), lightProbe->irradianceMap); ++i);
					for (std::size_t i = 0; !mEditor.getScene()->repository.add("prefilterMap" + std::to_string(i), lightProbe->prefilterMap); ++i);
				}
				ImGui::TreePop();
			}
		};
	};


	class ComponentPanel::MeshComponentNode : public ComponentPanel::ComponentNode<MeshComponent>
	{
	private:	// Attributes
		bool mHasSkinning = false;

	public:		// Functions
		MeshComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Mesh"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<MeshComponent>(entity, mEditor.getEventManager(), entity); };
		virtual void draw(Entity entity) override
		{
			auto [mesh] = mEditor.getEntityDatabase().getComponents<MeshComponent>(entity);

			bool canAddRMesh = !mesh->full();
			if (!canAddRMesh) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			std::shared_ptr<Mesh> gMesh;
			if (addRepoDropdownButton("##MeshComponent::AddGMesh", "Add RenderableMesh", mEditor.getScene()->repository, gMesh)) {
				mesh->add(mHasSkinning, gMesh);
			}
			ImGui::SameLine();
			ImGui::Checkbox("Has Skinning", &mHasSkinning);
			if (!canAddRMesh) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				std::string buttonName = "x##MeshComponent::RemoveGMesh" + std::to_string(i);
				if (ImGui::Button(buttonName.c_str())) {
					mesh->remove(i);
				}
				else {
					ImGui::SameLine();
					std::string treeNodeName = "RenderableMesh #" + std::to_string(i);
					if (ImGui::TreeNode(treeNodeName.c_str())) {
						ImGui::BulletText("Has Skin: %s", mesh->hasSkinning(i)? "yes" : "no");

						ImGui::BulletText("Mesh:");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);

						gMesh = mesh->get(i).getMesh();
						std::string dropdownName = "##MeshComponent::SelectGMesh" + std::to_string(i);
						if (addRepoDropdownShowSelected(dropdownName.c_str(), mEditor.getScene()->repository, gMesh)) {
							mesh->get(i).setMesh(gMesh);
						}

						if (ImGui::TreeNode("Shaders:")) {
							std::shared_ptr<se::app::RenderableShader> shader;
							std::string buttonName2 = "##MeshComponent::AddShaderMesh" + std::to_string(i);
							if (addRepoDropdownButton(buttonName2.c_str(), "Add Shader", mEditor.getScene()->repository, shader)) {
								mesh->addRenderableShader(i, shader);
							}

							std::size_t shaderIndex = 0;
							mesh->processRenderableShaders(i, [&](const auto& shader1) {
								std::string buttonName = "x##MeshComponent::RemoveShader" + std::to_string(shaderIndex++) + "Mesh" + std::to_string(i);
								if (ImGui::Button(buttonName.c_str())) {
									mesh->removeRenderableShader(i, shader1);
								}
								else {
									ImGui::SameLine();
									std::shared_ptr<se::app::RenderableShader> shader2 = shader1;
									std::string dropdownName2 = "##MeshComponent::SelectShader" + std::to_string(shaderIndex++) + "Mesh" + std::to_string(i);
									if (addRepoDropdownShowSelected(dropdownName2.c_str(), mEditor.getScene()->repository, shader2)) {
										mesh->removeRenderableShader(i, shader1);
										mesh->addRenderableShader(i, shader2);
									}
								}
							});
							ImGui::TreePop();
						}
						ImGui::TreePop();
					}
				}
			});
		};
	};


	class ComponentPanel::TerrainComponentNode : public ComponentPanel::ComponentNode<TerrainComponent>
	{
	public:		// Functions
		TerrainComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Terrain"; };
		virtual void create(Entity entity) override
		{
			const float size = 500.0f, maxHeight = 10.0f;
			const std::vector<float> lodDistances{ 2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 75.0f, 40.0f, 20.0f, 10.0f, 0.0f };
			mEditor.getEntityDatabase().emplaceComponent<TerrainComponent>(entity, mEditor.getEventManager(), entity, size, maxHeight, lodDistances);
		};
		virtual void draw(Entity entity) override
		{
			auto [terrain] = mEditor.getEntityDatabase().getComponents<TerrainComponent>(entity);

			// TODO: lod and size

			if (ImGui::TreeNode("Shaders:")) {
				std::shared_ptr<se::app::RenderableShader> shader;
				if (addRepoDropdownButton("##TerrainComponent::Shader", "Add Shader", mEditor.getScene()->repository, shader)) {
					terrain->addRenderableShader(shader);
				}

				terrain->processRenderableShaders([&](const auto& shader1) {
					ImGui::Text("Shader:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
					std::shared_ptr<se::app::RenderableShader> shader2 = shader1;
					if (addRepoDropdownShowSelected("##TerrainComponent::Shader2", mEditor.getScene()->repository, shader2)) {
						terrain->removeRenderableShader(shader1);
						terrain->addRenderableShader(shader2);
					}
				});
				ImGui::TreePop();
			}
		};
	};


	class ComponentPanel::RigidBodyComponentNode : public ComponentPanel::ComponentNode<RigidBody>
	{
	public:		// Functions
		RigidBodyComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Rigid Body"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<RigidBody>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [rigidBody] = mEditor.getEntityDatabase().getComponents<RigidBody>(entity);
			auto& rbConfig = rigidBody->getConfig();
			auto& rbData = rigidBody->getData();

			bool infiniteMass = (rbConfig.invertedMass == 0);

			static const char* massTypes[] = { "infinite", "custom" };
			std::size_t currentType = infiniteMass? 0 : 1;
			if (addDropdown("Mass##mass", massTypes, IM_ARRAYSIZE(massTypes), currentType)) {
				infiniteMass = (currentType == 0);
			}

			if (infiniteMass) {
				rbConfig.invertedMass = 0.0f;
				rbConfig.invertedInertiaTensor = glm::mat3(0.0f);
			}
			else {
				float mass = 1.0f;
				glm::mat3 inertiaTensor = glm::mat3(1.0f);
				if (rbConfig.invertedMass > 0.0f) {
					mass = 1.0f / rbConfig.invertedMass;
					inertiaTensor = glm::inverse(rbConfig.invertedInertiaTensor);
				}
				else {
					rbConfig.invertedMass = mass;
					rbConfig.invertedInertiaTensor = inertiaTensor;
				}

				if (ImGui::DragFloat("Mass", &mass, 0.005f, FLT_MIN, FLT_MAX, "%.3f", 1.0f)) {
					rbConfig.invertedMass = 1.0f / mass;
				}

				if (drawMat3ImGui("Inertia Tensor", inertiaTensor)) {
					rbConfig.invertedInertiaTensor = glm::inverse(inertiaTensor);
				}
			}

			ImGui::DragFloat("Linear drag", &rbConfig.linearDrag, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Angular drag", &rbConfig.angularDrag, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Friction coefficient", &rbConfig.frictionCoefficient, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Sleep motion", &rbConfig.sleepMotion, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat3("Linear Velocity", glm::value_ptr(rbData.linearVelocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			ImGui::DragFloat3("Angular Velocity", glm::value_ptr(rbData.angularVelocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

			std::shared_ptr<Force> force;
			if (addRepoDropdownButton("##RigidBodyComponent::AddForce", "Add Force", mEditor.getScene()->repository, force)) {
				rigidBody->addForce(force);
			}
			std::size_t i = 0;
			rigidBody->processForces([&, rigidBody = rigidBody](std::shared_ptr<Force> force2) {
				std::string buttonName = "x##RigidBodyComponent::RemoveForce" + std::to_string(i++);
				if (ImGui::Button(buttonName.c_str())) {
					rigidBody->removeForce(force2);
				}
				else {
					ImGui::SameLine();

					auto oldForce2 = force2;
					std::string dropdownName = "##RigidBodyComponent::SelectForce" + std::to_string(i);
					if (addRepoDropdownShowSelected(dropdownName.c_str(), mEditor.getScene()->repository, force2)) {
						rigidBody->removeForce(oldForce2);
						rigidBody->addForce(force2);
					}
				}
			});
		};
	};


	class ComponentPanel::ColliderComponentNode : public ComponentPanel::ComponentNode<Collider>
	{
	private:	// Attributes
		std::shared_ptr<Texture> mHeightTexture;
		int mSize1 = 128;
		int mSize2 = 128;
		std::shared_ptr<Mesh> mMesh;

	public:		// Functions
		ColliderComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Collider"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<BoundingBox>()); };
		virtual void draw(Entity entity) override
		{
			auto [collider] = mEditor.getEntityDatabase().getComponents<Collider>(entity);

			auto bBox		= dynamic_cast<BoundingBox*>(collider);
			auto bSphere	= dynamic_cast<BoundingSphere*>(collider);
			auto capsule	= dynamic_cast<Capsule*>(collider);
			auto triangle	= dynamic_cast<TriangleCollider*>(collider);
			auto terrain	= dynamic_cast<TerrainCollider*>(collider);
			auto cPoly		= dynamic_cast<ConvexPolyhedron*>(collider);
			auto composite	= dynamic_cast<CompositeCollider*>(collider);

			static const char* types[] = { "Bounding Box", "Bounding Sphere", "Capsule", "Triangle", "Terrain", "Convex Polyhedron", "Composite" };
			std::size_t currentType = bBox? 0 : bSphere? 1 : capsule? 2 : triangle? 3 : terrain? 4 : cPoly? 5 : 6;
			addDropdown("Type##ColliderType", types, IM_ARRAYSIZE(types), currentType);

			switch (currentType) {
				case 0:
					if (!bBox) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<BoundingBox>());
					}
					break;
				case 1:
					if (!bSphere) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<BoundingSphere>());
					}
					break;
				case 2:
					if (!capsule) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<Capsule>());
					}
					break;
				case 3:
					if (!triangle) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<TriangleCollider>());
					}
					break;
				case 4:
					if (!terrain) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<TerrainCollider>());
					}
					break;
				case 5:
					if (!cPoly || bBox) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<ConvexPolyhedron>());
					}
					break;
				case 6:
					if (!composite) {
						mEditor.getEntityDatabase().removeComponent<Collider>(entity);
						collider = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<CompositeCollider>());
					}
					break;
			}
			drawCollider(*collider);
		};
	protected:
		void drawCollider(Collider& collider)
		{
			if (auto bBox = dynamic_cast<BoundingBox*>(&collider)) {
				drawBBox(*bBox);
			}
			else if (auto bSphere = dynamic_cast<BoundingSphere*>(&collider)) {
				drawBSphere(*bSphere);
			}
			else if (auto capsule = dynamic_cast<Capsule*>(&collider)) {
				drawCapsule(*capsule);
			}
			else if (auto triangle = dynamic_cast<TriangleCollider*>(&collider)) {
				drawTriangle(*triangle);
			}
			else if (auto terrain = dynamic_cast<TerrainCollider*>(&collider)) {
				drawTerrain(*terrain);
			}
			else if (auto cPoly = dynamic_cast<ConvexPolyhedron*>(&collider)) {
				drawCPoly(*cPoly);
			}
			else if (auto composite = dynamic_cast<CompositeCollider*>(&collider)) {
				drawComposite(*composite);
			}
		}
		void drawBBox(BoundingBox& bBox)
		{
			glm::vec3 lengths = bBox.getLengths();
			if (ImGui::DragFloat3("Lengths", glm::value_ptr(lengths), 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f)) {
				bBox.setLengths(lengths);
			}
		}
		void drawBSphere(BoundingSphere& bSphere)
		{
			float radius = bSphere.getRadius();
			if (ImGui::DragFloat("Radius", &radius, 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f)) {
				bSphere.setRadius(radius);
			}
		}
		void drawCapsule(Capsule& capsule)
		{
			float radius = capsule.getRadius();
			if (ImGui::DragFloat("Radius", &radius, 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f)) {
				capsule.setRadius(radius);
			}

			float height = capsule.getHeight();
			if (ImGui::DragFloat("Height", &height, 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f)) {
				capsule.setHeight(height);
			}
		}
		void drawTriangle(TriangleCollider& triangle)
		{
			bool setVertices = false;
			auto vertices = triangle.getLocalVertices();

			setVertices |= ImGui::DragFloat3("v0", glm::value_ptr(vertices[0]), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			setVertices |= ImGui::DragFloat3("v1", glm::value_ptr(vertices[1]), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			setVertices |= ImGui::DragFloat3("v2", glm::value_ptr(vertices[2]), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

			if (setVertices) {
				triangle.setLocalVertices(vertices);
			}
		}
		void drawTerrain(TerrainCollider& terrain)
		{
			if (ImGui::TreeNode("Create from texture")) {
				addRepoDropdownShowSelected("Height Map##HeightMap", mEditor.getScene()->repository, mHeightTexture);
				ImGui::DragInt("Size X", &mSize1, 0.01f, 0, INT_MAX);
				ImGui::DragInt("Size Z", &mSize2, 0.01f, 0, INT_MAX);

				if (ImGui::Button("Build terrain##BuildTerrain")) {
					auto image = TextureUtils::textureToImage<unsigned char>(
						*mHeightTexture, TypeId::UnsignedByte, ColorFormat::Red, mSize1, mSize2
					);
					auto heights = se::app::MeshLoader::calculateHeights(image.pixels.get(), image.width, image.height);
					terrain.setHeights(heights.data(), image.width, image.height);
				}
				ImGui::TreePop();
			}
		}
		void drawCPoly(ConvexPolyhedron& cPoly)
		{
			ImGui::Text("Number of vertices: %u", cPoly.getLocalMesh().vertices.size());
			ImGui::Text("Number of edges: %u", cPoly.getLocalMesh().edges.size());
			ImGui::Text("Number of faces: %u", cPoly.getLocalMesh().faces.size());

			if (ImGui::TreeNode("Create from mesh")) {
				addRepoDropdownShowSelected("Mesh##MeshToCPoly", mEditor.getScene()->repository, mMesh);

				if (ImGui::Button("Build Convex Polyhedron##BuildCPoly")) {
					auto [heMesh, loaded] = MeshLoader::createHalfEdgeMesh(MeshLoader::createRawMesh(*mMesh));
					if (loaded) {
						cPoly.setLocalMesh(heMesh);
					}
					else {
						SOMBRA_ERROR_LOG << "Failed to Load the HalfEdgeMesh";
					}
				}
				ImGui::TreePop();
			}
		}
		void drawComposite(CompositeCollider& composite)
		{
			if (!ImGui::TreeNode("Parts")) { return; }

			if (ImGui::SmallButton("Add")) {
				ImGui::OpenPopup("add_composite_part");
			}
			if (ImGui::BeginPopup("add_composite_part")) {
				if (ImGui::MenuItem("Add BoundingBox")) {
					composite.addPart(std::make_unique<BoundingBox>());
				}
				if (ImGui::MenuItem("Add BoundingSphere")) {
					composite.addPart(std::make_unique<BoundingSphere>());
				}
				if (ImGui::MenuItem("Add Capsule")) {
					composite.addPart(std::make_unique<Capsule>());
				}
				if (ImGui::MenuItem("Add TriangleCollider")) {
					composite.addPart(std::make_unique<TriangleCollider>());
				}
				if (ImGui::MenuItem("Add TerrainCollider")) {
					composite.addPart(std::make_unique<TerrainCollider>());
				}
				if (ImGui::MenuItem("Add ConvexPolyhedron")) {
					composite.addPart(std::make_unique<ConvexPolyhedron>());
				}
				if (ImGui::MenuItem("Add CompositeCollider")) {
					composite.addPart(std::make_unique<CompositeCollider>());
				}
				ImGui::EndPopup();
			}

			std::size_t i = 0;
			Collider* colliderToRemove = nullptr;

			composite.processParts([&](Collider& collider) {
				bool partOpened = ImGui::TreeNode(("Part " + std::to_string(i++)).c_str());

				se::utils::ArrayStreambuf<char, 128> aStreambuf;
				std::ostream(&aStreambuf) << "CompositeColliderPart_" << static_cast<void*>(&collider);
				if (ImGui::BeginPopupContextItem(aStreambuf.data())) {
					if (ImGui::MenuItem("Remove")) {
						colliderToRemove = &collider;
					}
					ImGui::EndPopup();
				}

				if (partOpened) {
					drawCollider(collider);
					ImGui::TreePop();
				}
			});

			composite.removePart(colliderToRemove);

			ImGui::TreePop();
		}
	};


	ComponentPanel::ComponentPanel(Editor& editor) : mEditor(editor)
	{
		mNodes.emplace_back(new TagComponentNode(mEditor));
		mNodes.emplace_back(new TransformsComponentNode(mEditor));
		mNodes.emplace_back(new AnimationComponentNode(mEditor));
		mNodes.emplace_back(new CameraComponentNode(mEditor));
		mNodes.emplace_back(new LightComponentNode(mEditor));
		mNodes.emplace_back(new LightProbeComponentNode(mEditor));
		mNodes.emplace_back(new MeshComponentNode(mEditor));
		mNodes.emplace_back(new TerrainComponentNode(mEditor));
		mNodes.emplace_back(new RigidBodyComponentNode(mEditor));
		mNodes.emplace_back(new ColliderComponentNode(mEditor));
	}


	ComponentPanel::~ComponentPanel()
	{
		for (IComponentNode* node : mNodes) {
			delete node;
		}
	}


	void ComponentPanel::render()
	{
		if (ImGui::Begin("Component Panel")) {
			Entity selectedEntity = mEditor.getActiveEntity();
			if (selectedEntity != kNullEntity) {
				ImGui::Text("Entity #%u selected", selectedEntity);
				ImGui::SameLine();
				if (ImGui::SmallButton("Add")) {
					ImGui::OpenPopup("add_component");
				}
				if (ImGui::BeginPopup("add_component")) {
					for (IComponentNode* node : mNodes) {
						if (!node->check(selectedEntity) && ImGui::Selectable(node->getName(), false)) {
							node->create(selectedEntity);
						}
					}
					ImGui::EndPopup();
				}

				for (IComponentNode* node : mNodes) {
					if (node->check(selectedEntity)) {
						bool open = ImGui::CollapsingHeader(node->getName());
						bool draw = true;
						if (ImGui::BeginPopupContextItem()) {
							if (ImGui::MenuItem("Remove")) {
								node->remove(selectedEntity);
								draw = false;
							}
							ImGui::EndPopup();
						}
						if (open) {
							if (draw) {
								node->draw(selectedEntity);
							}
						}
					}
				}
			}
			else {
				ImGui::Text("No Entity selected");
			}
		}
		ImGui::End();
	}

}
