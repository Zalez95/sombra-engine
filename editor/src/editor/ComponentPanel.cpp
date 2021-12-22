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
#include <se/app/LightProbeComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/TerrainComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/app/AnimationComponent.h>
#include <se/app/ParticleSystemComponent.h>
#include <se/app/RigidBodyComponent.h>
#include <se/app/AudioSourceComponent.h>
#include <se/app/ScriptComponent.h>
#include <se/app/graphics/TextureUtils.h>
#include <se/physics/collision/BoundingBox.h>
#include <se/physics/collision/BoundingSphere.h>
#include <se/physics/collision/Capsule.h>
#include <se/physics/collision/TriangleCollider.h>
#include <se/physics/collision/TerrainCollider.h>
#include <se/physics/collision/TriangleMeshCollider.h>
#include <se/physics/collision/CompositeCollider.h>
#include <se/animation/SkeletonAnimator.h>
#include <se/utils/StringUtils.h>
#include "ComponentPanel.h"
#include "Editor.h"
#include "ImGuiUtils.h"

using namespace se::app;
using namespace se::graphics;
using namespace se::animation;
using namespace se::physics;

namespace editor {

	class ComponentPanel::IComponentNode
	{
	public:		// Functions
		virtual ~IComponentNode() = default;
		virtual const char* getName() const = 0;
		virtual void create(Entity entity) = 0;
		virtual bool active(Entity entity) = 0;
		virtual void enable(Entity entity) = 0;
		virtual bool enabled(Entity entity) = 0;
		virtual void disable(Entity entity) = 0;
		virtual void draw(Entity entity) = 0;
		virtual void remove(Entity entity) = 0;
	};


	template <typename T>
	class ComponentPanel::ComponentNode : public ComponentPanel::IComponentNode
	{
	private:	// Attributes
		ComponentPanel& mPanel;

	public:		// Functions
		ComponentNode(ComponentPanel& panel) : IComponentNode(), mPanel(panel) {};
		virtual ~ComponentNode() = default;
		virtual bool active(Entity entity) override
		{
			return mPanel.mEditor.getEntityDatabase().hasComponents<T>(entity);
		};
		virtual void enable(Entity entity) override
		{
			mPanel.mEditor.getEntityDatabase().enableComponent<T>(entity);
		};
		virtual bool enabled(Entity entity) override
		{
			return mPanel.mEditor.getEntityDatabase().hasComponentsEnabled<T>(entity);
		};
		virtual void disable(Entity entity) override
		{
			mPanel.mEditor.getEntityDatabase().disableComponent<T>(entity);
		};
		virtual void remove(Entity entity) override
		{
			mPanel.mEditor.getEntityDatabase().removeComponent<T>(entity);
		};
	protected:
		Editor& getEditor() const { return mPanel.mEditor; };
		std::string getIdPrefix() const { return "##ComponentPanel" + std::to_string(mPanel.mPanelId) + "::"; };
	};


	class ComponentPanel::TagComponentNode : public ComponentPanel::ComponentNode<TagComponent>
	{
	public:		// Functions
		TagComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Tag"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<TagComponent>(entity, ""); };
		virtual void draw(Entity entity) override
		{
			auto [tag] = getEditor().getEntityDatabase().getComponents<TagComponent>(entity);

			char nameBuffer[TagComponent::kMaxLength] = {};
			std::copy(tag->getName(), tag->getName() + tag->getLength(), nameBuffer);
			std::string name = "Name" + getIdPrefix() + "TagComponentNode::name";
			if (ImGui::InputText(name.c_str(), nameBuffer, TagComponent::kMaxLength)) {
				tag->setName(nameBuffer);
			}
		};
	};


	class ComponentPanel::TransformsComponentNode : public ComponentPanel::ComponentNode<TransformsComponent>
	{
	private:	// Orientation type
		int mOrientationType;

	public:		// Functions
		TransformsComponentNode(ComponentPanel& panel) : ComponentNode(panel), mOrientationType(0) {};
		virtual const char* getName() const override
		{ return "Transforms"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<TransformsComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [transforms] = getEditor().getEntityDatabase().getComponents<TransformsComponent>(entity);

			bool updated = false;
			updated |= ImGui::DragFloat3("Position", glm::value_ptr(transforms->position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat3("Velocity", glm::value_ptr(transforms->velocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= drawOrientation("Orientation", transforms->orientation, mOrientationType);
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
		AnimationComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Animation"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<AnimationComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [animation] = getEditor().getEntityDatabase().getComponents<AnimationComponent>(entity);
			auto node = animation->getRootNode();
			if (node) {
				ImGui::Text("%s (0x%p)", node->getData().name.data(), static_cast<void*>(node));
			}
			else {
				ImGui::Text("No node setted");
			}

			if (ImGui::TreeNode("Animators:")) {
				Repository::ResourceRef<SkeletonAnimator> sAnimator;
				std::string name = getIdPrefix() + "AnimationComponentNode::AddAnimator";
				if (addRepoDropdownButton(name.c_str(), "Add", getEditor().getScene()->repository, sAnimator)) {
					animation->addAnimator(sAnimator);
				}

				std::size_t sAnimatorIndex = 0;
				animation->processSAnimators([&, animation = animation](auto sAnimator2) {
					std::string name1 = "x" + getIdPrefix() + "AnimationComponentNode::RemoveAnimator" + std::to_string(sAnimatorIndex++);
					if (ImGui::Button(name1.c_str())) {
						animation->removeAnimator(sAnimator2);
					}
					else {
						ImGui::SameLine();

						auto oldSAnimator2 = sAnimator2;
						std::string name2 = getIdPrefix() + "AnimationComponentNode::ChangeAnimator" + std::to_string(sAnimatorIndex);
						if (addRepoDropdownShowSelected(name2.c_str(), getEditor().getScene()->repository, sAnimator2)) {
							animation->removeAnimator(oldSAnimator2);
							animation->addAnimator(sAnimator2);
						}
					}
				});
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Change node:")) {
				std::string name = "Name" + getIdPrefix() + "AnimationComponentNode::name";
				ImGui::InputText(name.c_str(), mName.data(), mName.size());
				if (ImGui::Button(("Change" + getIdPrefix() + "AnimationComponentNode::ChangeNode").c_str())) {
					void* nodePtr = nullptr;
					std::istringstream(mName.data()) >> nodePtr;

					AnimationNode& root = getEditor().getScene()->rootNode;
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
		CameraComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Camera"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<CameraComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [camera] = getEditor().getEntityDatabase().getComponents<CameraComponent>(entity);

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
		LightComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Light"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<LightComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [light] = getEditor().getEntityDatabase().getComponents<LightComponent>(entity);

			ImGui::Text("Source:");
			ImGui::SameLine();

			auto source = light->getSource();
			std::string name = getIdPrefix() + "LightComponentNode::ChangeSource";
			if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, source)) {
				light->setSource(source);
			}
		};
	};


	class ComponentPanel::LightProbeComponentNode : public ComponentPanel::ComponentNode<LightProbeComponent>
	{
	private:	// Attributes
		std::string mEnvironmentTextureName;
		bool mIsCubeMap = false;
		int mIrradianceMapSize = 32;
		int mPrefilterMapSize = 128;

	public:		// Functions
		LightProbeComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Light Probe"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<LightProbeComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [lightProbe] = getEditor().getEntityDatabase().getComponents<LightProbeComponent>(entity);

			std::string name1 = "Irradiance map" + getIdPrefix() + "LightProbeComponentNode::ChangeIrradiance";
			addRepoDropdownShowSelected(name1.c_str(), getEditor().getScene()->repository, lightProbe->irradianceMap);
			std::string name2 = "Prefilter map" + getIdPrefix() + "LightProbeComponentNode::ChangePrefilter";
			addRepoDropdownShowSelected(name2.c_str(), getEditor().getScene()->repository, lightProbe->prefilterMap);

			if (ImGui::TreeNode("Create from texture")) {
				auto environmentTexture = getEditor().getScene()->repository.findByName<Texture>(mEnvironmentTextureName.c_str());

				std::string name3 = "Environment Map" + getIdPrefix() + "LightProbeComponentNode::ChangeEnvironment";
				if (addRepoDropdownShowSelected(name3.c_str(), getEditor().getScene()->repository, environmentTexture)) {
					mEnvironmentTextureName = environmentTexture.getResource().getName();
				}

				ImGui::DragInt("Irradiance map resolution", &mIrradianceMapSize, 0.01f, 0, INT_MAX);
				ImGui::DragInt("Prefilter map resolution", &mPrefilterMapSize, 0.01f, 0, INT_MAX);

				if (!environmentTexture) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				if (ImGui::Button(("Build probe" + getIdPrefix() + "LightProbeComponentNode::BuildProbe").c_str())) {
					auto cubeMapSPtr = environmentTexture.get();

					auto irradianceMapSPtr = TextureUtils::convoluteCubeMap(cubeMapSPtr, mIrradianceMapSize);
					lightProbe->irradianceMap = getEditor().getScene()->repository.insert(std::move(irradianceMapSPtr));
					setRepoName<Texture>(
						lightProbe->irradianceMap.getResource(),
						(mEnvironmentTextureName + "IrradianceMap").c_str(), getEditor().getScene()->repository
					);

					auto prefilterMapSPtr = TextureUtils::prefilterCubeMap(cubeMapSPtr, mPrefilterMapSize);
					lightProbe->prefilterMap = getEditor().getScene()->repository.insert(std::move(prefilterMapSPtr));
					setRepoName<Texture>(
						lightProbe->prefilterMap.getResource(),
						(mEnvironmentTextureName + "PrefilterMap").c_str(), getEditor().getScene()->repository
					);
				}
				if (!environmentTexture) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
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
		MeshComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Mesh"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<MeshComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [mesh] = getEditor().getEntityDatabase().getComponents<MeshComponent>(entity);

			bool canAddRMesh = !mesh->full();
			if (!canAddRMesh) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			Repository::ResourceRef<Mesh> gMesh;
			std::string name = getIdPrefix() + "MeshComponentNode::AddMesh";
			if (addRepoDropdownButton(name.c_str(), "Add RenderableMesh", getEditor().getScene()->repository, gMesh)) {
				mesh->add(mHasSkinning, gMesh);
			}
			ImGui::SameLine();
			ImGui::Checkbox(("Has Skinning" + getIdPrefix() + "MeshComponentNode::hasSkinning").c_str(), &mHasSkinning);
			if (!canAddRMesh) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				std::string name1 = "x" + getIdPrefix() + "MeshComponentNode::RemoveMesh" + std::to_string(i);
				if (ImGui::Button(name1.c_str())) {
					mesh->remove(i);
				}
				else {
					ImGui::SameLine();
					std::string treeNodeName = "RenderableMesh #" + std::to_string(i);
					if (ImGui::TreeNode(treeNodeName.c_str())) {
						ImGui::BulletText("Has Skin: %s", mesh->hasSkinning(i)? "yes" : "no");

						ImGui::BulletText("Bounds:");
						auto [min, max] = mesh->get(i).getBounds();
						ImGui::Indent(16.0f);
						ImGui::BulletText("Minimum [%.3f, %.3f, %.3f]", min.x, min.y, min.z);
						ImGui::BulletText("Maximum [%.3f, %.3f, %.3f]", max.x, max.y, max.z);
						ImGui::Unindent(16.0f);

						ImGui::BulletText("Mesh:");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);

						gMesh = mesh->getMesh(i);
						std::string name2 = getIdPrefix() + "MeshComponentNode::ChangeMesh" + std::to_string(i);
						if (addRepoDropdownShowSelected(name2.c_str(), getEditor().getScene()->repository, gMesh)) {
							mesh->setMesh(i, gMesh);
						}

						if (ImGui::TreeNode("Shaders:")) {
							Repository::ResourceRef<RenderableShader> shader;
							std::string name3 = getIdPrefix() + "MeshComponentNode::AddShaderMesh" + std::to_string(i);
							if (addRepoDropdownButton(name3.c_str(), "Add Shader", getEditor().getScene()->repository, shader)) {
								mesh->addRenderableShader(i, shader);
							}

							std::size_t shaderIndex = 0;
							mesh->processRenderableShaders(i, [&](const auto& shader1) {
								std::string name4 = "x" + getIdPrefix() + "MeshComponentNode::changeShader" + std::to_string(shaderIndex++) + "Mesh" + std::to_string(i);
								if (ImGui::Button(name4.c_str())) {
									mesh->removeRenderableShader(i, shader1);
								}
								else {
									ImGui::SameLine();
									Repository::ResourceRef<RenderableShader> shader2 = shader1;
									std::string name5 = getIdPrefix() + "MeshComponentNode::changeShader" + std::to_string(shaderIndex) + "Mesh" + std::to_string(i);
									if (addRepoDropdownShowSelected(name5.c_str(), getEditor().getScene()->repository, shader2)) {
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
		TerrainComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Terrain"; };
		virtual void create(Entity entity) override
		{
			const float size = 500.0f, maxHeight = 10.0f;
			const std::vector<float> lodDistances = { 2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 75.0f, 40.0f, 20.0f, 10.0f, 0.0f };
			getEditor().getEntityDatabase().emplaceComponent<TerrainComponent>(entity, size, maxHeight, lodDistances);
		};
		virtual void draw(Entity entity) override
		{
			auto [terrain] = getEditor().getEntityDatabase().getComponents<TerrainComponent>(entity);

			float xzSize = terrain->get().getSize();
			if (ImGui::DragFloat("XZ Size", &xzSize, 0.005f, 0, FLT_MAX, "%.3f", 1.0f)) {
				terrain->get().setSize(xzSize);
			}

			float maxHeight = terrain->get().getMaxHeight();
			if (ImGui::DragFloat("Maximum Height", &maxHeight, 0.005f, 0, FLT_MAX, "%.3f", 1.0f)) {
				terrain->get().setMaxHeight(maxHeight);
			}

			if (ImGui::TreeNode("LOD distances:")) {
				auto lods = terrain->get().getLodDistances();
				bool updated = false;

				if (ImGui::DragFloat("LOD 0", &lods.back(), 0.005f, 0, FLT_MAX, "%.3f", 1.0f)) {
					updated = true;
				}
				for (std::size_t lod = 1; lod < lods.size(); ++lod) {
					std::size_t i = lods.size() - lod - 1;

					std::string name = "x" + getIdPrefix() + "TerrainComponentNode::RemoveLOD" + std::to_string(lod);
					if (ImGui::Button(name.c_str())) {
						lods.erase(lods.begin() + i);
						updated = true;
					}
					else {
						ImGui::SameLine();
						if (ImGui::DragFloat(("LOD " + std::to_string(lod)).c_str(), &lods[i], 0.005f, 0, FLT_MAX, "%.3f", 1.0f)) {
							updated = true;
						}
					}
				}

				std::string name = "x" + getIdPrefix() + "TerrainComponentNode::AddLOD";
				if (ImGui::Button(name.c_str())) {
					lods.insert(lods.begin(), lods.front());
					updated = true;
				}

				if (updated) {
					terrain->get().setLodDistances(lods);
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Shaders:")) {
				Repository::ResourceRef<RenderableShader> shader;
				std::string name = getIdPrefix() + "TerrainComponentNode::AddShader";
				if (addRepoDropdownButton(name.c_str(), "Add Shader", getEditor().getScene()->repository, shader)) {
					terrain->addRenderableShader(shader);
				}

				std::size_t shaderIndex = 0;
				terrain->processRenderableShaders([&](const auto& shader1) {
					std::string name1 = "x" + getIdPrefix() + "TerrainComponentNode::RemoveShader" + std::to_string(shaderIndex++);
					if (ImGui::Button(name1.c_str())) {
						terrain->removeRenderableShader(shader1);
					}
					else {
						ImGui::SameLine();
						Repository::ResourceRef<RenderableShader> shader2 = shader1;
						std::string name2 = getIdPrefix() + "TerrainComponentNode::ChangeShader" + std::to_string(shaderIndex);
						if (addRepoDropdownShowSelected(name2.c_str(), getEditor().getScene()->repository, shader2)) {
							terrain->removeRenderableShader(shader1);
							terrain->addRenderableShader(shader2);
						}
					}
				});
				ImGui::TreePop();
			}
		};
	};


	class ComponentPanel::RigidBodyComponentNode : public ComponentPanel::ComponentNode<RigidBodyComponent>
	{
	private:	// Attributes
		std::string mHeightTextureName;
		int mSize1 = 128;
		int mSize2 = 128;
		std::string mMeshName;

	public:		// Functions
		RigidBodyComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Rigid Body"; };
		virtual void create(Entity entity) override
		{ getEditor().getEntityDatabase().emplaceComponent<RigidBodyComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [rigidBody] = getEditor().getEntityDatabase().getComponents<RigidBodyComponent>(entity);
			auto rbProperties = rigidBody->get().getProperties();
			auto rbState = rigidBody->get().getState();
			bool updatedProperties = false, updatedState = false;

			static const char* rbTypeTags[] = { "Static", "Dynamic" };
			int currentType = (rbProperties.type == RigidBodyProperties::Type::Static)? 0 : 1;
			std::string name = "Type" + getIdPrefix() + "RigidBodyComponentNode::Type";
			if (addDropdown(name.c_str(), rbTypeTags, IM_ARRAYSIZE(rbTypeTags), currentType)) {
				rbProperties.type = (currentType == 0)? RigidBodyProperties::Type::Static : RigidBodyProperties::Type::Dynamic;
				updatedProperties = true;
			}

			bool infiniteMass = (rbProperties.invertedMass == 0);
			updatedProperties |= ImGui::Checkbox(("Has Infinite Mass" + getIdPrefix() + "RigidBodyComponentNode::infiniteMass").c_str(), &infiniteMass);

			if (infiniteMass) {
				rbProperties.invertedMass = 0.0f;
				rbProperties.invertedInertiaTensor = glm::mat3(0.0f);
			}
			else {
				float mass = 1.0f;
				glm::mat3 inertiaTensor = glm::mat3(1.0f);
				if (rbProperties.invertedMass > 0.0f) {
					mass = 1.0f / rbProperties.invertedMass;
					inertiaTensor = glm::inverse(rbProperties.invertedInertiaTensor);
				}
				else {
					rbProperties.invertedMass = mass;
					rbProperties.invertedInertiaTensor = inertiaTensor;
				}

				if (ImGui::DragFloat("Mass", &mass, 0.005f, FLT_MIN, FLT_MAX, "%.3f", 1.0f)) {
					rbProperties.invertedMass = 1.0f / mass;
					updatedProperties = true;
				}

				if (drawMat3ImGui("Inertia Tensor", inertiaTensor)) {
					rbProperties.invertedInertiaTensor = glm::inverse(inertiaTensor);
					updatedProperties = true;
				}
			}

			updatedProperties |= ImGui::DragFloat("Linear drag", &rbProperties.linearDrag, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			updatedProperties |= ImGui::DragFloat("Angular drag", &rbProperties.angularDrag, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			updatedProperties |= ImGui::DragFloat("Friction coefficient", &rbProperties.frictionCoefficient, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			updatedProperties |= ImGui::DragFloat("Sleep motion", &rbProperties.sleepMotion, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
			if (updatedProperties) {
				rigidBody->get().setProperties(rbProperties);
			}

			updatedState |= ImGui::DragFloat3("Linear Velocity", glm::value_ptr(rbState.linearVelocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updatedState |= ImGui::DragFloat3("Angular Velocity", glm::value_ptr(rbState.angularVelocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			if (updatedState) {
				rigidBody->get().setState(rbState);
			}

			ImGui::Text("Status:");
			bool bSleeping = rigidBody->get().getStatus(RigidBody::Status::Sleeping);
			if (ImGui::Checkbox("Sleeping", &bSleeping)) {
				rigidBody->get().setStatus(RigidBody::Status::Sleeping, bSleeping);
			}
			bool bPropertiesChanged = rigidBody->get().getStatus(RigidBody::Status::PropertiesChanged);
			if (ImGui::Checkbox("PropertiesChanged", &bPropertiesChanged)) {
				rigidBody->get().setStatus(RigidBody::Status::PropertiesChanged, bPropertiesChanged);
			}
			bool bStateChanged = rigidBody->get().getStatus(RigidBody::Status::StateChanged);
			if (ImGui::Checkbox("StateChanged", &bStateChanged)) {
				rigidBody->get().setStatus(RigidBody::Status::StateChanged, bStateChanged);
			}
			bool bColliderChanged = rigidBody->get().getStatus(RigidBody::Status::ColliderChanged);
			if (ImGui::Checkbox("ColliderChanged", &bColliderChanged)) {
				rigidBody->get().setStatus(RigidBody::Status::ColliderChanged, bColliderChanged);
			}
			bool bForcesChanged = rigidBody->get().getStatus(RigidBody::Status::ForcesChanged);
			if (ImGui::Checkbox("ForcesChanged", &bForcesChanged)) {
				rigidBody->get().setStatus(RigidBody::Status::ForcesChanged, bForcesChanged);
			}

			if (ImGui::TreeNode("Collider")) {
				auto collider = rigidBody->get().getCollider();
				auto bBox		= dynamic_cast<BoundingBox*>(collider);
				auto bSphere	= dynamic_cast<BoundingSphere*>(collider);
				auto capsule	= dynamic_cast<Capsule*>(collider);
				auto triangle	= dynamic_cast<TriangleCollider*>(collider);
				auto terrain	= dynamic_cast<TerrainCollider*>(collider);
				auto triMesh	= dynamic_cast<TriangleMeshCollider*>(collider);
				auto cPoly		= dynamic_cast<ConvexPolyhedron*>(collider);
				auto composite	= dynamic_cast<CompositeCollider*>(collider);

				static const char* colliderTypeTags[] = {
					"Bounding Box", "Bounding Sphere", "Capsule", "Triangle",
					"Terrain", "Triangle Mesh", "Convex Polyhedron", "Composite", "None"
				};
				int colliderCurrentType = bBox? 0 : bSphere? 1 : capsule? 2 : triangle? 3
										: terrain? 4 : triMesh? 5 : cPoly? 6 : composite? 7 : 8;
				std::string name1 = "Collider Type" + getIdPrefix() + "RigidBodyComponentNode::ColliderType";
				if (addDropdown(name1.c_str(), colliderTypeTags, IM_ARRAYSIZE(colliderTypeTags), colliderCurrentType)) {
					switch (colliderCurrentType) {
						case 0:
							if (!bBox) {
								rigidBody->get().setCollider(std::make_unique<BoundingBox>());
							}
							break;
						case 1:
							if (!bSphere) {
								rigidBody->get().setCollider(std::make_unique<BoundingSphere>());
							}
							break;
						case 2:
							if (!capsule) {
								rigidBody->get().setCollider(std::make_unique<Capsule>());
							}
							break;
						case 3:
							if (!triangle) {
								rigidBody->get().setCollider(std::make_unique<TriangleCollider>());
							}
							break;
						case 4:
							if (!terrain) {
								rigidBody->get().setCollider(std::make_unique<TerrainCollider>());
							}
							break;
						case 5:
							if (!triMesh) {
								rigidBody->get().setCollider(std::make_unique<TriangleMeshCollider>());
							}
							break;
						case 6:
							if (!cPoly || bBox) {
								rigidBody->get().setCollider(std::make_unique<ConvexPolyhedron>());
							}
							break;
						case 7:
							if (!composite) {
								rigidBody->get().setCollider(std::make_unique<CompositeCollider>());
							}
							break;
						default:
							if (collider) {
								rigidBody->get().setCollider(nullptr);
							}
							break;
					}
					collider = rigidBody->get().getCollider();
				}

				if (collider) {
					auto [min, max] = collider->getAABB();
					ImGui::Text("AABB:");
					ImGui::BulletText("Minimum [%.3f, %.3f, %.3f]", min.x, min.y, min.z);
					ImGui::BulletText("Maximum [%.3f, %.3f, %.3f]", max.x, max.y, max.z);

					ImGui::Text("Layers:");
					auto layers = collider->getLayers();
					for (std::size_t i = 0; i < Collider::kMaxLayers; ++i) {
						bool value = layers[i];
						if (ImGui::Checkbox((getIdPrefix() + "ColliderLayer" + std::to_string(i)).c_str(), &value)) {
							collider->setLayer(i, value);
						}
						if ((i + 1) % (Collider::kMaxLayers >> 2) != 0) {
							ImGui::SameLine();
						}
					}

					drawCollider(*collider);
				}

				glm::mat4 colliderLocalTransforms = rigidBody->get().getColliderLocalTransforms();
				std::string name2 = "Collider local transforms" + getIdPrefix() + "RigidBodyComponentNode::ColliderLocalTransforms";
				if (drawMat4ImGui(name2.c_str(), colliderLocalTransforms)) {
					rigidBody->get().setColliderLocalTrasforms(colliderLocalTransforms);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Forces")) {
				Repository::ResourceRef<Force> force;
				std::string name3 = getIdPrefix() + "RigidBodyComponentNode::AddForce";
				if (addRepoDropdownButton(name3.c_str(), "Add Force", getEditor().getScene()->repository, force)) {
					rigidBody->addForce(force);
				}
				std::size_t i = 0;
				rigidBody->processForces([&, rigidBody = rigidBody](const auto& force1) {
					std::string name4 = "x" + getIdPrefix() + "RigidBodyComponentNode::RemoveForce" + std::to_string(i++);
					if (ImGui::Button(name4.c_str())) {
						rigidBody->removeForce(force1);
					}
					else {
						ImGui::SameLine();

						Repository::ResourceRef<Force> force2 = force1;
						std::string name5 = getIdPrefix() + "RigidBodyComponentNode::ChangeForce" + std::to_string(i);
						if (addRepoDropdownShowSelected(name5.c_str(), getEditor().getScene()->repository, force2)) {
							rigidBody->removeForce(force1);
							rigidBody->addForce(force2);
						}
					}
				});

				ImGui::TreePop();
			}
		};
	private:
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
			else if (auto triMesh = dynamic_cast<TriangleMeshCollider*>(&collider)) {
				drawTriMesh(*triMesh);
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
				auto heightTexture = getEditor().getScene()->repository.findByName<Texture>(mHeightTextureName.c_str());
				std::string name = "Height Map" + getIdPrefix() + "RigidBodyComponentNode::HeightMap";
				if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, heightTexture)) {
					mHeightTextureName = heightTexture.getResource().getName();
				}

				ImGui::DragInt("Size X", &mSize1, 0.01f, 0, INT_MAX);
				ImGui::DragInt("Size Z", &mSize2, 0.01f, 0, INT_MAX);

				std::string name1 = "Build terrain" + getIdPrefix() + "RigidBodyComponentNode::BuildTerrain";
				if (ImGui::Button(name1.c_str())) {
					auto image = TextureUtils::textureToImage<unsigned char>(
						*heightTexture, TypeId::UnsignedByte, ColorFormat::R, mSize1, mSize2
					);
					auto heights = MeshLoader::calculateHeights(image.pixels.get(), image.width, image.height);
					terrain.setHeights(heights.data(), image.width, image.height);
				}
				ImGui::TreePop();
			}
		}
		void drawTriMesh(TriangleMeshCollider& triMesh)
		{
			ImGui::Text("Number of vertices: %lu", triMesh.getNumVertices());
			ImGui::Text("Number of indices: %lu", triMesh.getNumIndices());

			if (ImGui::TreeNode("Create from graphics mesh")) {
				auto mesh = getEditor().getScene()->repository.findByName<Mesh>(mMeshName.c_str());
				std::string name = "Mesh" + getIdPrefix() + "RigidBodyComponentNode::Mesh";
				if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, mesh)) {
					mMeshName = mesh.getResource().getName();
				}

				std::string name1 = "Build Triangle Mesh" + getIdPrefix() + "RigidBodyComponentNode::BuildTriangleMesh";
				if (ImGui::Button(name1.c_str())) {
					auto rawMesh = MeshLoader::createRawMesh(*mesh);
					triMesh.setMesh(
						rawMesh.positions.data(), rawMesh.positions.size(),
						rawMesh.indices.data(), rawMesh.indices.size()
					);
				}
				ImGui::TreePop();
			}


		}
		void drawCPoly(ConvexPolyhedron& cPoly)
		{
			ImGui::Text("Number of vertices: %lu", cPoly.getLocalMesh().vertices.size());
			ImGui::Text("Number of edges: %lu", cPoly.getLocalMesh().edges.size());
			ImGui::Text("Number of faces: %lu", cPoly.getLocalMesh().faces.size());

			if (ImGui::TreeNode("Create from mesh")) {
				auto mesh = getEditor().getScene()->repository.findByName<Mesh>(mMeshName.c_str());
				std::string name = "Mesh" + getIdPrefix() + "RigidBodyComponentNode::Mesh";
				if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, mesh)) {
					mMeshName = mesh.getResource().getName();
				}

				std::string name1 = "Build Convex Polyhedron" + getIdPrefix() + "RigidBodyComponentNode::BuildConvexPolyhedron";
				if (ImGui::Button(name1.c_str())) {
					auto [heMesh, loaded] = MeshLoader::createHalfEdgeMesh(MeshLoader::createRawMesh(*mesh));
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

			if (ImGui::SmallButton(("Add" + getIdPrefix() + "AddComposite").c_str())) {
				ImGui::OpenPopup((getIdPrefix() + "AddComposite").c_str());
			}
			if (ImGui::BeginPopup((getIdPrefix() + "AddComposite").c_str())) {
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


	class ComponentPanel::ParticleSystemComponentNode : public ComponentPanel::ComponentNode<ParticleSystemComponent>
	{
	public:		// Functions
		ParticleSystemComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "ParticleSystem"; };
		virtual void create(Entity entity) override
		{
			getEditor().getEntityDatabase().emplaceComponent<ParticleSystemComponent>(entity);
		};
		virtual void draw(Entity entity) override
		{
			auto [particleSystem] = getEditor().getEntityDatabase().getComponents<ParticleSystemComponent>(entity);

			auto mesh = particleSystem->getMesh();
			std::string name = "Mesh" + getIdPrefix() + "ParticleSystemComponentNode::Mesh";
			if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, mesh)) {
				particleSystem->setMesh(mesh);
			}

			if (ImGui::TreeNode("Shaders:")) {
				if (!mesh) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				Repository::ResourceRef<RenderableShader> shader;
				std::string name1 = getIdPrefix() + "ParticleSystemComponentNode::AddShader";
				if (addRepoDropdownButton(name1.c_str(), "Add Shader", getEditor().getScene()->repository, shader)) {
					particleSystem->addRenderableShader(shader);
				}

				std::size_t shaderIndex = 0;
				particleSystem->processRenderableShaders([&](const auto& shader1) {
					std::string name2 = "x" + getIdPrefix() + "ParticleSystemComponentNode::RemoveShader" + std::to_string(shaderIndex++);
					if (ImGui::Button(name2.c_str())) {
						particleSystem->removeRenderableShader(shader1);
					}
					else {
						ImGui::SameLine();
						Repository::ResourceRef<RenderableShader> shader2 = shader1;
						std::string name3 = getIdPrefix() + "ParticleSystemComponentNode::ChangeShader" + std::to_string(shaderIndex);
						if (addRepoDropdownShowSelected(name3.c_str(), getEditor().getScene()->repository, shader2)) {
							particleSystem->removeRenderableShader(shader1);
							particleSystem->addRenderableShader(shader2);
						}
					}
				});

				if (!mesh) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
				ImGui::TreePop();
			}

			Repository::ResourceRef<ParticleEmitter> emitter = particleSystem->getEmitter();
			std::string name4 = "Emitter" + getIdPrefix() + "ParticleSystemComponentNode::Emitter";
			if (addRepoDropdownShowSelected(name4.c_str(), getEditor().getScene()->repository, emitter)) {
				particleSystem->setEmitter(emitter);
			}
		};
	};


	class ComponentPanel::AudioSourceComponentNode : public ComponentPanel::ComponentNode<AudioSourceComponent>
	{
	public:		// Functions
		AudioSourceComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "AudioSource"; };
		virtual void create(Entity entity) override
		{
			getEditor().getEntityDatabase().emplaceComponent<AudioSourceComponent>(entity);
		};
		virtual void draw(Entity entity) override
		{
			auto [audioSource] = getEditor().getEntityDatabase().getComponents<AudioSourceComponent>(entity);

			auto buffer = audioSource->getBuffer();
			std::string name = "Buffer" + getIdPrefix() + "AudioSourceComponentNode::Buffer";
			if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, buffer)) {
				audioSource->setBuffer(buffer);
			}
		};
	};


	class ComponentPanel::ScriptComponentNode : public ComponentPanel::ComponentNode<ScriptComponent>
	{
	public:		// Functions
		ScriptComponentNode(ComponentPanel& panel) : ComponentNode(panel) {};
		virtual const char* getName() const override
		{ return "Script"; };
		virtual void create(Entity entity) override
		{
			getEditor().getEntityDatabase().emplaceComponent<ScriptComponent>(entity);
		};
		virtual void draw(Entity entity) override
		{
			auto [scriptC] = getEditor().getEntityDatabase().getComponents<ScriptComponent>(entity);

			auto script = scriptC->getScript();
			std::string name = "Script" + getIdPrefix() + "ScriptComponentNode::Script";
			if (addRepoDropdownShowSelected(name.c_str(), getEditor().getScene()->repository, script)) {
				scriptC->setScript(script);
			}
		};
	};


	ComponentPanel::ComponentPanel(Editor& editor) : IEditorPanel(editor)
	{
		mNodes.emplace_back(new TagComponentNode(*this));
		mNodes.emplace_back(new TransformsComponentNode(*this));
		mNodes.emplace_back(new AnimationComponentNode(*this));
		mNodes.emplace_back(new CameraComponentNode(*this));
		mNodes.emplace_back(new LightComponentNode(*this));
		mNodes.emplace_back(new LightProbeComponentNode(*this));
		mNodes.emplace_back(new MeshComponentNode(*this));
		mNodes.emplace_back(new TerrainComponentNode(*this));
		mNodes.emplace_back(new RigidBodyComponentNode(*this));
		mNodes.emplace_back(new ParticleSystemComponentNode(*this));
		mNodes.emplace_back(new AudioSourceComponentNode(*this));
		mNodes.emplace_back(new ScriptComponentNode(*this));
	}


	ComponentPanel::~ComponentPanel()
	{
		for (IComponentNode* node : mNodes) {
			delete node;
		}
	}


	bool ComponentPanel::render()
	{
		bool open = true;
		if (ImGui::Begin(("Component Panel##ComponentPanel" + std::to_string(mPanelId)).c_str(), &open)) {
			Entity selectedEntity = mEditor.getActiveEntity();
			if (selectedEntity != kNullEntity) {
				ImGui::Text("Entity #%u selected", selectedEntity);
				ImGui::SameLine();
				if (ImGui::SmallButton(("Add##ComponentPanel" + std::to_string(mPanelId) + "::addComponent").c_str())) {
					ImGui::OpenPopup(("ComponentPanel" + std::to_string(mPanelId) + "::addComponent").c_str());
				}
				if (ImGui::BeginPopup(("ComponentPanel" + std::to_string(mPanelId) + "::addComponent").c_str())) {
					for (IComponentNode* node : mNodes) {
						if (!node->active(selectedEntity) && ImGui::MenuItem(node->getName())) {
							node->create(selectedEntity);
						}
					}
					ImGui::EndPopup();
				}

				std::size_t iComponent = 0;
				for (IComponentNode* node : mNodes) {
					if (node->active(selectedEntity)) {
						bool enabled = node->enabled(selectedEntity);
						std::string name1 = "##" + std::to_string(mPanelId) + "enableComponent" + std::to_string(iComponent++);
						if (ImGui::Checkbox(name1.c_str(), &enabled)) {
							if (enabled) {
								node->enable(selectedEntity);
							}
							else {
								node->disable(selectedEntity);
							}
						}

						ImGui::SameLine();
						bool draw = ImGui::CollapsingHeader(node->getName());
						if (ImGui::BeginPopupContextItem()) {
							if (ImGui::MenuItem("Remove")) {
								node->remove(selectedEntity);
								draw = false;
							}
							ImGui::EndPopup();
						}
						if (draw) {
							node->draw(selectedEntity);
						}
					}
				}
			}
			else {
				ImGui::Text("No Entity selected");
			}
		}
		ImGui::End();
		return open;
	}

}
