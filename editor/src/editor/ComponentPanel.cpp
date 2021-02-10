#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/Scene.h>
#include <se/app/EntityDatabase.h>
#include <se/app/TagComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/LightProbe.h>
#include <se/app/MeshComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/animation/AnimationNode.h>
#include <se/physics/RigidBody.h>
#include <se/collision/BoundingBox.h>
#include <se/collision/BoundingSphere.h>
#include <se/collision/Capsule.h>
#include <se/collision/TriangleCollider.h>
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
	public:		// Functions
		TransformsComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Transforms"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<TransformsComponent>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [transforms] = mEditor.getEntityDatabase().getComponents<TransformsComponent>(entity);
			transforms->updated.reset( static_cast<int>(TransformsComponent::Update::Input) );

			bool updated = false;
			updated |= ImGui::DragFloat3("Position", glm::value_ptr(transforms->position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat3("Velocity", glm::value_ptr(transforms->velocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat4("Orientation", glm::value_ptr(transforms->orientation), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat3("Scale", glm::value_ptr(transforms->scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

			if (updated) {
				transforms->updated.set( static_cast<int>(TransformsComponent::Update::Input) );
			}
		};
	};


	class ComponentPanel::AnimationNodeComponentNode : public ComponentPanel::ComponentNode<AnimationNode*>
	{
	public:		// Functions
		AnimationNodeComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Animation Node"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<AnimationNode*>(entity, new AnimationNode()); };
		virtual void draw(Entity entity) override
		{
			auto [animationNode] = mEditor.getEntityDatabase().getComponents<AnimationNode*>(entity);
			auto& animationData = (*animationNode)->getData();
			ImGui::InputText("Name##NodeName", animationData.name.data(), animationData.name.size());

			ImGui::Text("Local transforms:");
			bool updated = false;
			updated |= ImGui::DragFloat3("Position", glm::value_ptr(animationData.localTransforms.position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat4("Orientation", glm::value_ptr(animationData.localTransforms.orientation), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= ImGui::DragFloat3("Scale", glm::value_ptr(animationData.localTransforms.scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			animationData.animated = updated;
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
	public:		// Functions
		LightProbeComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Light Probe"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<LightProbe>(entity); };
		virtual void draw(Entity entity) override
		{
			auto [lightProbe] = mEditor.getEntityDatabase().getComponents<LightProbe>(entity);

			// TODO: update in apprenderer
			ImGui::Text("Irradiance Map:");
			ImGui::SameLine();
			addRepoDropdownShowSelected("##IrradianceMap", mEditor.getScene()->repository, lightProbe->irradianceMap);

			ImGui::Text("Prefilter Map:");
			ImGui::SameLine();
			addRepoDropdownShowSelected("##PrefilterMap", mEditor.getScene()->repository, lightProbe->prefilterMap);
		};
	};


	class ComponentPanel::MeshComponentNode : public ComponentPanel::ComponentNode<MeshComponent>
	{
	public:		// Functions
		MeshComponentNode(Editor& editor) : ComponentNode(editor) {};
		virtual const char* getName() const override
		{ return "Mesh"; };
		virtual void create(Entity entity) override
		{ mEditor.getEntityDatabase().emplaceComponent<MeshComponent>(entity, mEditor.getEventManager(), entity); };
		virtual void draw(Entity entity) override
		{
			auto [mesh] = mEditor.getEntityDatabase().getComponents<MeshComponent>(entity);

			bool canAddRMesh = (mesh->size() < MeshComponent::kMaxMeshes);
			if (!canAddRMesh) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			std::shared_ptr<Mesh> gMesh;
			if (addRepoDropdownButton("##MeshComponent::gMesh", "Add RenderableMesh", mEditor.getScene()->repository, gMesh)) {
				mesh->add(false, gMesh);
			}
			if (addRepoDropdownButton("##MeshComponent::gMeshSkinned", "Add Skinned RenderableMesh", mEditor.getScene()->repository, gMesh)) {
				mesh->add(true, gMesh);
			}
			if (!canAddRMesh) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			for (std::size_t i = 0; i < mesh->size(); ++i) {
				std::string rMeshName = "RenderableMesh #" + std::to_string(i);
				if (ImGui::TreeNode(rMeshName.c_str())) {
					ImGui::BulletText("Has Skin: %s", mesh->hasSkinning(i)? "yes" : "no");

					ImGui::BulletText("Mesh:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
					gMesh = mesh->get(i).getMesh();
					if (addRepoDropdownShowSelected("##MeshComponent::gMesh2", mEditor.getScene()->repository, gMesh)) {
						mesh->get(i).setMesh(gMesh);
					}

					if (ImGui::TreeNode("Shaders:")) {
						std::shared_ptr<se::app::RenderableShader> shader;
						if (addRepoDropdownButton("##MeshComponent::Shader", "Add Shader", mEditor.getScene()->repository, shader)) {
							mesh->addRenderableShader(i, shader);
						}

						mesh->processRenderableShaders(i, [&](const auto& shader1) {
							ImGui::Text("Shader:");
							ImGui::SameLine();
							ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
							std::shared_ptr<se::app::RenderableShader> shader2 = shader1;
							if (addRepoDropdownShowSelected("##MeshComponent::Shader2", mEditor.getScene()->repository, shader2)) {
								mesh->removeRenderableShader(i, shader1);
								mesh->addRenderableShader(i, shader2);
							}
						});
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
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

			bool infiniteMass = (rbConfig.invertedMass == 0);
			if (ImGui::BeginCombo("Mass##mass", infiniteMass? "infinite" : "custom")) {
				if (ImGui::Selectable("infinite", infiniteMass)) {
					infiniteMass = true;
				}
				if (ImGui::Selectable("custom", !infiniteMass)) {
					infiniteMass = false;
				}
				ImGui::EndCombo();
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
		};
	};


	class ComponentPanel::ColliderComponentNode : public ComponentPanel::ComponentNode<Collider>
	{
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

			static const char* types[] = { "Bounding Box", "Bounding Sphere", "Capsule", "Triangle" };
			std::size_t currentType = bBox? 0 : bSphere? 1 : capsule? 2 : 3;
			if (ImGui::BeginCombo("Type##ColliderType", types[currentType])) {
				for (std::size_t i = 0; i < IM_ARRAYSIZE(types); ++i) {
					bool isSelected = (i == currentType);
					if (ImGui::Selectable(types[i], isSelected)) {
						currentType = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (currentType == 0) {
				if (!bBox) {
					mEditor.getEntityDatabase().removeComponent<Collider>(entity);
					Collider* c = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<BoundingBox>());
					bBox = dynamic_cast<BoundingBox*>(c);
				}
				drawBBox(*bBox);
			}
			else if (currentType == 1) {
				if (!bSphere) {
					mEditor.getEntityDatabase().removeComponent<Collider>(entity);
					Collider* c = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<BoundingSphere>());
					bSphere = dynamic_cast<BoundingSphere*>(c);
				}
				drawBSphere(*bSphere);
			}
			else if (currentType == 2) {
				if (!capsule) {
					mEditor.getEntityDatabase().removeComponent<Collider>(entity);
					Collider* c = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<Capsule>());
					capsule = dynamic_cast<Capsule*>(c);
				}
				drawCapsule(*capsule);
			}
			else if (currentType == 3) {
				if (!triangle) {
					mEditor.getEntityDatabase().removeComponent<Collider>(entity);
					Collider* c = mEditor.getEntityDatabase().addComponent<Collider>(entity, std::make_unique<TriangleCollider>());
					triangle = dynamic_cast<TriangleCollider*>(c);
				}
				drawTriangle(*triangle);
			}
		};
	protected:
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
	};


	ComponentPanel::ComponentPanel(Editor& editor) : mEditor(editor)
	{
		mNodes.emplace_back(new TagComponentNode(mEditor));
		mNodes.emplace_back(new TransformsComponentNode(mEditor));
		mNodes.emplace_back(new AnimationNodeComponentNode(mEditor));
		mNodes.emplace_back(new CameraComponentNode(mEditor));
		mNodes.emplace_back(new LightComponentNode(mEditor));
		mNodes.emplace_back(new LightProbeComponentNode(mEditor));
		mNodes.emplace_back(new MeshComponentNode(mEditor));
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
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
				if (ImGui::BeginCombo("##AddComponent", "Add component")) {
					for (IComponentNode* node : mNodes) {
						if (!node->check(selectedEntity) && ImGui::Selectable(node->getName(), false)) {
							node->create(selectedEntity);
						}
					}
					ImGui::EndCombo();
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
