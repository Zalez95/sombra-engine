#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <AudioFile.h>
#include <se/animation/SkeletonAnimator.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/forces/PunctualForce.h>
#include <se/physics/forces/DirectionalForce.h>
#include <se/graphics/core/Program.h>
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include <se/graphics/Pass.h>
#include <se/graphics/Technique.h>
#include <se/graphics/core/Texture.h>
#include <se/graphics/3D/Mesh.h>
#include <se/app/SkinComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/RenderableShader.h>
#include <se/app/Scene.h>
#include <se/app/io/ImageReader.h>
#include <se/app/io/SceneImporter.h>
#include <se/app/io/ShaderLoader.h>
#include "Editor.h"
#include "ImGuiUtils.h"
#include "RepositoryPanel.h"
#include "DefaultShaderBuilder.h"

using namespace se::app;
using namespace se::graphics;
using namespace se::animation;
using namespace se::audio;
using namespace se::physics;

namespace editor {

	bool cancelButton()
	{
		return ImGui::Button("Cancel");
	}


	bool confirmButton(bool enabled = true)
	{
		if (!enabled) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		bool ret = ImGui::Button("Confirm");
		if (!enabled) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		return ret;
	}


	class RepositoryPanel::ITypeNode
	{
	public:		// Functions
		virtual ~ITypeNode() = default;
		virtual const char* getName() const = 0;
		virtual void draw() = 0;
	};


	template <typename T>
	class RepositoryPanel::TypeNode : public RepositoryPanel::ITypeNode
	{
	protected:	// Attributes
		static constexpr std::size_t kMaxNameSize = 128;
		RepositoryPanel& mPanel;
	private:
		std::string mSelected;
		bool mShowCreate;

	public:		// Functions
		TypeNode(RepositoryPanel& panel) : mPanel(panel), mShowCreate(false) {};
		virtual ~TypeNode() = default;
		virtual void draw() override
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Elements")) {
				if (ImGui::SmallButton("Add")) {
					mShowCreate = true;
				}

				mPanel.mEditor.getScene()->repository.iterate<Scene::Key, T>([&](const std::string& key, std::shared_ptr<T>) {
					if (ImGui::Selectable(key.c_str(), key == mSelected)) {
						mSelected = key;
					}
				});
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Selected element")) {
				if (!mPanel.mEditor.getScene()->repository.find<Scene::Key, T>(mSelected)) {
					mSelected = "";
				}

				if (!mSelected.empty()) {
					std::array<char, kMaxNameSize> nameBuffer = {};
					std::copy(mSelected.begin(), mSelected.end(), nameBuffer.data());
					if (ImGui::InputText("Name##SelectedName", nameBuffer.data(), nameBuffer.size())) {
						auto element = mPanel.mEditor.getScene()->repository.find<Scene::Key, T>(mSelected);
						mPanel.mEditor.getScene()->repository.remove<Scene::Key, T>(mSelected);

						mSelected = nameBuffer.data();
						mPanel.mEditor.getScene()->repository.add<Scene::Key, T>(mSelected, std::move(element));
					}

					draw(mSelected);
				}
			}

			if (mShowCreate) {
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::CollapsingHeader("Create")) {
					mShowCreate = !create();
				}
			}
		};
	protected:
		virtual void draw(const Scene::Key& /*key*/) {};
		/** @return	true if the create menu must be shown, false otherwise */
		virtual bool create() { return false; };
	};


	template <typename T>
	class RepositoryPanel::ImportTypeNode : public RepositoryPanel::TypeNode<T>
	{
	private:	// Attributes
		std::string mPath;
		FileWindow mFileWindow;

	public:		// Functions
		ImportTypeNode(RepositoryPanel& panel) : TypeNode<T>(panel) {};
		virtual ~ImportTypeNode() = default;
	protected:
		virtual bool create() override
		{
			bool ret = false;
			Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

			std::string label = mPath.empty()? "Open File..." : ("Selected: " + mPath);
			if (ImGui::Button(label.c_str())) {
				mFileWindow.show();
			}
			mFileWindow.execute(mPath);

			bool validOptions = options();

			ImGui::Separator();
			if (cancelButton()) {
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validOptions && !mPath.empty())) {
				if (load(mPath.c_str())) {
					ret = true;
				}
				else {
					importErrorPopUp.show();
				}
			}
			importErrorPopUp.execute();

			return ret;
		};

		/** @return	true if the options are valid, false otherwise */
		virtual bool options() { return true; };
		/** @return	true on success, false otherwise */
		virtual bool load(const char* /*path*/) { return true; };
	};


	template <typename T>
	class RepositoryPanel::SceneImporterTypeNode : public RepositoryPanel::ImportTypeNode<T>
	{
	protected:	// Attributes
		using TypeNode<T>::mPanel;
	private:
		SceneImporter::FileType mFileType;

	public:		// Functions
		SceneImporterTypeNode(RepositoryPanel& panel) :
			ImportTypeNode<T>(panel), mFileType(SceneImporter::FileType::GLTF) {};
	protected:
		virtual bool options() override
		{
			static const char* types[] = { "GLTF" };
			std::size_t currentType = static_cast<std::size_t>(mFileType);
			if (addDropdown("Type:##SceneImporter", types, IM_ARRAYSIZE(types), currentType)) {
				mFileType = static_cast<SceneImporter::FileType>(currentType);
			}

			return true;
		};

		virtual bool load(const char* path) override
		{
			DefaultShaderBuilder shaderBuilder(mPanel.mEditor, mPanel.mEditor.getScene()->repository);
			auto SceneImporter = SceneImporter::createSceneImporter(mFileType, shaderBuilder);
			auto result = SceneImporter->load(path, *mPanel.mEditor.getScene());
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
			}
			return result;
		};
	};


	class RepositoryPanel::SkinNode : public RepositoryPanel::SceneImporterTypeNode<Skin>
	{
	public:		// Functions
		SkinNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "Skin"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto skin = mPanel.mEditor.getScene()->repository.find<Scene::Key, Skin>(key);
			ImGui::Text("Inverse bind matrices: %lu", skin->inverseBindMatrices.size());
		};
	};


	class RepositoryPanel::LightSourceNode : public RepositoryPanel::TypeNode<LightSource>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		LightSourceNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "LightSource"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto source = mPanel.mEditor.getScene()->repository.find<Scene::Key, LightSource>(key);

			static const char* types[] = { "Directional", "Point", "Spot" };
			std::size_t currentType = static_cast<std::size_t>(source->type);
			if (addDropdown("Type:##LightSource", types, IM_ARRAYSIZE(types), currentType)) {
				source->type = static_cast<LightSource::Type>(currentType);
			}
			ImGui::ColorPicker3("Color", glm::value_ptr(source->color));
			ImGui::DragFloat("Intensity", &source->intensity, 0.005f, 0, FLT_MAX, "%.3f", 1.0f);
			if (source->type != LightSource::Type::Point) {
				ImGui::DragFloat("Range", &source->range, 0.005f, 0, FLT_MAX, "%.3f", 1.0f);
			}
			if (source->type == LightSource::Type::Spot) {
				ImGui::DragFloat("Inner cone range", &source->innerConeAngle, 0.005f, 0, glm::pi<float>(), "%.3f", 1.0f);
				ImGui::DragFloat("Outer cone range", &source->outerConeAngle, 0.005f, 0, glm::pi<float>(), "%.3f", 1.0f);
			}
		};

		virtual bool create() override
		{
			bool ret = false;

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, LightSource>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();

			if (confirmButton(validKey)) {
				mPanel.mEditor.getScene()->repository.add(Scene::Key(mNameBuffer.data()), std::make_shared<LightSource>(LightSource::Type::Directional));
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	class RepositoryPanel::SkeletonAnimatorNode : public RepositoryPanel::SceneImporterTypeNode<SkeletonAnimator>
	{
	public:		// Functions
		SkeletonAnimatorNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "SkeletonAnimator"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto animator = mPanel.mEditor.getScene()->repository.find<Scene::Key, SkeletonAnimator>(key);
			ImGui::Text("Loop time: %.3f seconds", animator->getLoopTime());
		};
	};


	class RepositoryPanel::AudioBufferNode : public RepositoryPanel::ImportTypeNode<Buffer>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		AudioBufferNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "AudioBuffer"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto path = mPanel.mEditor.getScene()->repository.find<Scene::Key, ResourcePath<Buffer>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}
		}

		virtual bool options() override
		{
			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, LightSource>(mNameBuffer.data());
			return validKey;
		};

		virtual bool load(const char* path) override
		{
			AudioFile<float> audioFile;
			if (!audioFile.load(path)) {
				SOMBRA_ERROR_LOG << "Error reading the audio file " << path;
				return false;
			}

			auto buffer = std::make_shared<Buffer>(
				audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
				FormatId::MonoFloat, audioFile.getSampleRate()
			);

			if (!mPanel.mEditor.getScene()->repository.add(Scene::Key(mNameBuffer.data()), buffer)
				|| !mPanel.mEditor.getScene()->repository.emplace<Scene::Key, ResourcePath<Buffer>>(mNameBuffer.data(), path)
			) {
				return false;
			}

			return true;
		};
	};


	class RepositoryPanel::ForceNode : public RepositoryPanel::TypeNode<Force>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		ForceNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "Force"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto force = mPanel.mEditor.getScene()->repository.find<Scene::Key, Force>(key);
			auto gravity = std::dynamic_pointer_cast<Gravity>(force);
			auto punctual = std::dynamic_pointer_cast<PunctualForce>(force);
			auto directional = std::dynamic_pointer_cast<DirectionalForce>(force);

			static const char* types[] = { "Gravity", "Punctual", "Directional" };
			std::size_t currentType = gravity? 0 : punctual? 1 : 2;
			addDropdown("Type##ForceNode", types, IM_ARRAYSIZE(types), currentType);

			if (currentType == 0) {
				if (!gravity) {
					gravity = std::make_shared<Gravity>();
					mPanel.mEditor.getScene()->repository.remove<Scene::Key, Force>(key);
					mPanel.mEditor.getScene()->repository.add<Scene::Key, Force>(key, gravity);
				}
				drawGravity(*gravity);
			}
			else if (currentType == 1) {
				if (!punctual) {
					punctual = std::make_shared<PunctualForce>();
					mPanel.mEditor.getScene()->repository.remove<Scene::Key, Force>(key);
					mPanel.mEditor.getScene()->repository.add<Scene::Key, Force>(key, punctual);
				}
				drawPunctualForce(*punctual);
			}
			else if (currentType == 2) {
				if (!directional) {
					directional = std::make_shared<DirectionalForce>();
					mPanel.mEditor.getScene()->repository.remove<Scene::Key, Force>(key);
					mPanel.mEditor.getScene()->repository.add<Scene::Key, Force>(key, directional);
				}
				drawDirectionalForce(*directional);
			}
		};

		virtual bool create() override
		{
			bool ret = false;

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, Force>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				mPanel.mEditor.getScene()->repository.add<Scene::Key, Force>(mNameBuffer.data(), std::make_shared<Gravity>());
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};

		void drawGravity(Gravity& gravity)
		{
			float value = gravity.getValue();
			if (ImGui::DragFloat("Gravity", &value, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
				gravity.setValue(value);
			}
		};

		void drawPunctualForce(PunctualForce& force)
		{
			glm::vec3 value = force.getValue();
			if (ImGui::DragFloat3("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
				force.setValue(value);
			}

			glm::vec3 point = force.getPoint();
			if (ImGui::DragFloat3("Point", glm::value_ptr(point), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
				force.setPoint(point);
			}
		};

		void drawDirectionalForce(DirectionalForce& force)
		{
			glm::vec3 value = force.getValue();
			if (ImGui::DragFloat3("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
				force.setValue(value);
			}
		};
	};


	class RepositoryPanel::ProgramNode : public RepositoryPanel::TypeNode<Program>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;
		std::string mPathVertex, mPathGeometry, mPathFragment;
		std::string* mPath;
		FileWindow mFileWindow;

	public:		// Functions
		ProgramNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{}, mPath(nullptr) {};
		virtual const char* getName() const override { return "Program"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto path = mPanel.mEditor.getScene()->repository.find<Scene::Key, ResourcePath<Program>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}
		}

		virtual bool create() override
		{
			bool ret = false;
			Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validOptions = !mPanel.mEditor.getScene()->repository.has<Scene::Key, LightSource>(mNameBuffer.data());

			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted("Some shaders are optional");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			std::string vLabel = mPathVertex.empty()? "Open Vertex Shader..." : "Selected: " + mPathVertex;
			if (ImGui::Button(vLabel.c_str())) {
				mPath = &mPathVertex;
				mFileWindow.show();
			}

			std::string gLabel = mPathGeometry.empty()? "Open Geometry Shader..." : "Selected: " + mPathGeometry;
			if (ImGui::Button(gLabel.c_str())) {
				mPath = &mPathGeometry;
				mFileWindow.show();
			}

			std::string fLabel = mPathFragment.empty()? "Open Fragment Shader..." : "Selected: " + mPathFragment;
			if (ImGui::Button(fLabel.c_str())) {
				mPath = &mPathFragment;
				mFileWindow.show();
			}

			if (mPath) {
				mFileWindow.execute(*mPath);
			}

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validOptions)) {
				std::shared_ptr<Program> program;
				auto result = ShaderLoader::createProgram(
					mPathVertex.empty()? nullptr : mPathVertex.c_str(),
					mPathGeometry.empty()? nullptr : mPathGeometry.c_str(),
					mPathFragment.empty()? nullptr : mPathFragment.c_str(),
					program
				);
				if (result) {
					std::string resourcePath = mPathVertex + "," + mPathGeometry + "," + mPathFragment;
					if (mPanel.mEditor.getScene()->repository.add<Scene::Key, Program>(mNameBuffer.data(), std::move(program))
						&& mPanel.mEditor.getScene()->repository.emplace<Scene::Key, ResourcePath<Program>>(mNameBuffer.data(), resourcePath)
					) {
						mNameBuffer.fill(0);
						ret = true;
					}
					else {
						SOMBRA_ERROR_LOG << "Failed to add the program";
						importErrorPopUp.show();
					}
				}
				else {
					SOMBRA_ERROR_LOG << result.description();
					importErrorPopUp.show();
				}
			}

			importErrorPopUp.execute();
			return ret;
		};
	};


	class RepositoryPanel::PassNode : public RepositoryPanel::TypeNode<Pass>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer, mRendererNameBuffer;

	public:		// Functions
		PassNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{}, mRendererNameBuffer{} {};
		virtual const char* getName() const override { return "Pass"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto pass = mPanel.mEditor.getScene()->repository.find<Scene::Key, Pass>(key);

			std::size_t numBindables = 0;
			pass->processBindables([&](std::shared_ptr<Bindable> bindable) {
				std::size_t bindableIndex = numBindables++;

				std::string buttonName = "x##PassNode::RemoveBindable" + std::to_string(bindableIndex);
				if (ImGui::Button(buttonName.c_str())) {
					pass->removeBindable(bindable);
				}
				ImGui::SameLine();

				std::string treeId = "PassNode::bindable" + std::to_string(bindableIndex);
				if (auto uniform1 = std::dynamic_pointer_cast<UniformVariableValue<int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<int> %s", uniform1->getName().c_str())) { return; }
					if (int value = uniform1->getValue(); ImGui::DragInt("Value", &value, 1, -INT_MAX, INT_MAX)) {
						uniform1->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform2 = std::dynamic_pointer_cast<UniformVariableValue<unsigned int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<unsigned int> %s", uniform2->getName().c_str())) { return; }
					if (unsigned int value = uniform2->getValue(); ImGui::DragInt("Value", reinterpret_cast<int*>(&value), 1, 0, INT_MAX)) {
						uniform2->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform3 = std::dynamic_pointer_cast<UniformVariableValue<float>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<float> %s", uniform3->getName().c_str())) { return; }
					if (float value = uniform3->getValue(); ImGui::DragFloat("Value", &value, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform3->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform4 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<vec2> %s", uniform4->getName().c_str())) { return; }
					if (glm::vec2 value = uniform4->getValue(); ImGui::DragFloat2("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform4->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform5 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<ivec2> %s", uniform5->getName().c_str())) { return; }
					if (glm::ivec2 value = uniform5->getValue(); ImGui::DragInt2("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform5->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform6 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<vec3> %s", uniform6->getName().c_str())) { return; }
					if (glm::vec3 value = uniform6->getValue(); ImGui::DragFloat3("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform6->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform7 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<ivec3> %s", uniform7->getName().c_str())) { return; }
					if (glm::ivec3 value = uniform7->getValue(); ImGui::DragInt3("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform7->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform8 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<vec4> %s", uniform8->getName().c_str())) { return; }
					if (glm::vec4 value = uniform8->getValue(); ImGui::DragFloat4("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform8->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform9 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<ivec4> %s", uniform9->getName().c_str())) { return; }
					if (glm::ivec4 value = uniform9->getValue(); ImGui::DragInt4("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform9->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform10 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<mat3> %s", uniform10->getName().c_str())) { return; }
					glm::mat3 value = uniform10->getValue();
					if (drawMat3ImGui("value", value)) {
						uniform10->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform11 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<mat4> %s", uniform11->getName().c_str())) { return; }
					glm::mat4 value = uniform11->getValue();
					if (drawMat4ImGui("value", value)) {
						uniform11->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform12 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3x4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Uniform<mat3x4> %s", uniform12->getName().c_str())) { return; }
					glm::mat3x4 value = uniform12->getValue();
					if (drawMat3x4ImGui("value", value)) {
						uniform12->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform13 = std::dynamic_pointer_cast<UniformVariableValueVector<int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<int> %s", uniform13->getName().c_str())) { return; }
					const int* valuePtr;	std::size_t valueSize;
					uniform13->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<int> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragInt("Value", &v, 1, -INT_MAX, INT_MAX);
					}
					if (update) {
						uniform13->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform14 = std::dynamic_pointer_cast<UniformVariableValueVector<unsigned int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<unsigned int> %s", uniform14->getName().c_str())) { return; }
					const unsigned int* valuePtr;	std::size_t valueSize;
					uniform14->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<unsigned int> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragInt("Value", reinterpret_cast<int*>(&v), 1, 0, INT_MAX);
					}
					if (update) {
						uniform14->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform15 = std::dynamic_pointer_cast<UniformVariableValueVector<float>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<float> %s", uniform15->getName().c_str())) { return; }
					const float* valuePtr;	std::size_t valueSize;
					uniform15->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<float> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragFloat("Value", &v, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					}
					if (update) {
						uniform15->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform16 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<vec2> %s", uniform16->getName().c_str())) { return; }
					const glm::vec2* valuePtr;
					std::size_t valueSize;
					uniform16->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::vec2> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragFloat2("Value", glm::value_ptr(v), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					}
					if (update) {
						uniform16->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform17 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<ivec2> %s", uniform17->getName().c_str())) { return; }
					const glm::ivec2* valuePtr;
					std::size_t valueSize;
					uniform17->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::ivec2> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragInt2("Value", glm::value_ptr(v), 0.005f, -INT_MAX, INT_MAX);
					}
					if (update) {
						uniform17->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform18 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<vec3> %s", uniform18->getName().c_str())) { return; }
					const glm::vec3* valuePtr;
					std::size_t valueSize;
					uniform18->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::vec3> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragFloat3("Value", glm::value_ptr(v), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					}
					if (update) {
						uniform18->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform19 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<ivec3> %s", uniform19->getName().c_str())) { return; }
					const glm::ivec3* valuePtr;
					std::size_t valueSize;
					uniform19->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::ivec3> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragInt3("Value", glm::value_ptr(v), 0.005f, -INT_MAX, INT_MAX);
					}
					if (update) {
						uniform19->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform20 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<vec4> %s", uniform20->getName().c_str())) { return; }
					const glm::vec4* valuePtr;
					std::size_t valueSize;
					uniform20->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::vec4> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragFloat4("Value", glm::value_ptr(v), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					}
					if (update) {
						uniform20->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform21 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<ivec4> %s", uniform21->getName().c_str())) { return; }
					const glm::ivec4* valuePtr;
					std::size_t valueSize;
					uniform21->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::ivec4> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= ImGui::DragInt4("Value", glm::value_ptr(v), 0.005f, -INT_MAX, INT_MAX);
					}
					if (update) {
						uniform21->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform22 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<mat3> %s", uniform22->getName().c_str())) { return; }
					const glm::mat3* valuePtr;
					std::size_t valueSize;
					uniform22->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::mat3> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= drawMat3ImGui("value", v);
					}
					if (update) {
						uniform22->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform23 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<mat4> %s", uniform23->getName().c_str())) { return; }
					const glm::mat4* valuePtr;
					std::size_t valueSize;
					uniform23->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::mat4> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= drawMat4ImGui("value", v);
					}
					if (update) {
						uniform23->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto uniform24 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat3x4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "UniformV<mat3x4> %s", uniform24->getName().c_str())) { return; }
					const glm::mat3x4* valuePtr;
					std::size_t valueSize;
					uniform24->getValue(valuePtr, valueSize);

					bool update = false;
					std::vector<glm::mat3x4> value(valuePtr, valuePtr + valueSize);
					for (auto& v : value) {
						update |= drawMat3x4ImGui("value", v);
					}
					if (update) {
						uniform24->setValue(value.data(), value.size());
					}
					ImGui::TreePop();
				}
				else if (auto texture = std::dynamic_pointer_cast<Texture>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Texture (Unit %d)", texture->getTextureUnit())) { return; }
					std::shared_ptr<Texture> texture2 = texture;
					std::string dropdownName = "##PassNode::SelectTexture" + std::to_string(bindableIndex);
					if (addRepoDropdownShowSelected(dropdownName.c_str(), mPanel.mEditor.getScene()->repository, texture2)) {
						pass->removeBindable(texture);
						pass->addBindable(texture2);
					}
					ImGui::TreePop();
				}
				else if (auto program = std::dynamic_pointer_cast<Program>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Program")) { return; }
					std::shared_ptr<Program> program2 = program;
					std::string dropdownName = "##PassNode::SelectProgram" + std::to_string(bindableIndex);
					if (addRepoDropdownShowSelected(dropdownName.c_str(), mPanel.mEditor.getScene()->repository, program2)) {
						pass->removeBindable(program);
						pass->addBindable(program2);
					}
					ImGui::TreePop();
				}
				else if (auto setOperation = std::dynamic_pointer_cast<SetOperation>(bindable)) {
					const char* operation = (setOperation->getOperation() == Operation::Culling)? "Culling" :
											(setOperation->getOperation() == Operation::DepthTest)? "DepthTest" :
											(setOperation->getOperation() == Operation::ScissorTest)? "ScissorTest" :
											"Blending";
					if (!ImGui::TreeNode(treeId.c_str(), "SetOperation %s", operation)) { return; }
					ImGui::Text("%s", setOperation->enableOperation()? "Active" : "Inactive");
					ImGui::TreePop();
				}
				else if (auto setDepthMask = std::dynamic_pointer_cast<SetDepthMask>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "SetDepthMask")) { return; }
					ImGui::Text("%s", setDepthMask->isActive()? "Active" : "Inactive");
					ImGui::TreePop();
				}
			});
		};

		virtual bool create() override
		{
			bool ret = false;

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, Pass>(mNameBuffer.data());

			ImGui::InputText("Renderer##CreateRendererName", mRendererNameBuffer.data(), mRendererNameBuffer.size());
			auto node = mPanel.mEditor.getExternalTools().graphicsEngine->getRenderGraph().getNode(mRendererNameBuffer.data());
			Renderer* renderer = dynamic_cast<Renderer*>(node);

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey && renderer)) {
				mPanel.mEditor.getScene()->repository.emplace<Scene::Key, Pass>(mNameBuffer.data(), *renderer);
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	class RepositoryPanel::RenderableShaderNode : public RepositoryPanel::TypeNode<RenderableShader>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		RenderableShaderNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "RenderableShader"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto shader = mPanel.mEditor.getScene()->repository.find<Scene::Key, RenderableShader>(key);

			std::shared_ptr<Pass> pass;
			std::string buttonName = "##RenderableShaderNode::AddPass";
			if (addRepoDropdownButton(buttonName.c_str(), "Add Shader", mPanel.mEditor.getScene()->repository, pass)) {
				shader->addPass(std::move(pass));
			}

			std::size_t passIndex = 0;
			shader->getTechnique()->processPasses([&](const std::shared_ptr<Pass>& pass1) {
				std::string buttonName = "x##RenderableShaderNode::RemovePass" + std::to_string(passIndex++);
				if (ImGui::Button(buttonName.c_str())) {
					shader->removePass(pass1);
				}
				ImGui::SameLine();

				std::shared_ptr<Pass> pass2 = pass1;
				std::string dropdownName2 = "##RenderableShaderNode::SelectPass" + std::to_string(passIndex++);
				if (addRepoDropdownShowSelected(dropdownName2.c_str(), mPanel.mEditor.getScene()->repository, pass2)) {
					shader->removePass(pass1);
					shader->addPass(pass2);
				}
			});
		};

		virtual bool create() override
		{
			bool ret = false;

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, RenderableShader>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				mPanel.mEditor.getScene()->repository.emplace<Scene::Key, RenderableShader>(mNameBuffer.data(), mPanel.mEditor.getEventManager());
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	class RepositoryPanel::TextureNode : public RepositoryPanel::ImportTypeNode<Texture>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;
		ColorFormat mColorType = ColorFormat::RGB;
		bool mIsHDR = false;

	public:		// Functions
		TextureNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "Texture"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto path = mPanel.mEditor.getScene()->repository.find<Scene::Key, ResourcePath<Texture>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}

			auto texture = mPanel.mEditor.getScene()->repository.find<Scene::Key, Texture>(key);
			ImGui::Image(static_cast<void*>(texture.get()), ImVec2{ 200.0f, 200.0f });

			int textureUnit = texture->getTextureUnit();
			if (ImGui::DragInt("Texture Unit", &textureUnit, 1, 0, 255)) {
				texture->setTextureUnit(textureUnit);
			}

			TextureFilter min, mag;
			texture->getFiltering(&min, &mag);
			std::size_t iMin = static_cast<std::size_t>(min);
			std::size_t iMag = static_cast<std::size_t>(mag);

			static const char* filterType[] = { "Nearest", "Linear", "Nearest MipMap Nearest", "Linear MipMap Nearest", "Nearest MipMap Linear", "Linear MipMap Linear" };
			if (addDropdown("Minification##TextureMinFilter", filterType, IM_ARRAYSIZE(filterType), iMin)) {
				min = static_cast<TextureFilter>(iMin);
				texture->setFiltering(min, mag);
			}
			if (addDropdown("Magnification##TextureMagFilter", filterType, IM_ARRAYSIZE(filterType), iMag)) {
				mag = static_cast<TextureFilter>(iMag);
				texture->setFiltering(min, mag);
			}

			TextureWrap wrapS, wrapT, wrapR;
			texture->getWrapping(&wrapS, &wrapT, &wrapR);
			std::size_t iWrapS = static_cast<std::size_t>(wrapS);
			std::size_t iWrapT = static_cast<std::size_t>(wrapT);
			std::size_t iWrapR = static_cast<std::size_t>(wrapR);

			static const char* wrapType[] = { "Repeat", "Mirrored Repeat", "Clamp to Edge", "Clamp to Border" };
			bool set = false;
			if (addDropdown("Wrap S##TextureWrapS", wrapType, IM_ARRAYSIZE(wrapType), iWrapS)) {
				wrapS = static_cast<TextureWrap>(iWrapS);
				set = true;
			}
			if (texture->getTarget() != TextureTarget::Texture1D) {
				if (addDropdown("Wrap T##TextureWrapT", wrapType, IM_ARRAYSIZE(wrapType), iWrapT)) {
					wrapT = static_cast<TextureWrap>(iWrapT);
					set = true;
				}

				if (texture->getTarget() != TextureTarget::Texture2D) {
					if (addDropdown("Wrap R##TextureWrapR", wrapType, IM_ARRAYSIZE(wrapType), iWrapR)) {
						wrapR = static_cast<TextureWrap>(iWrapR);
						set = true;
					}
				}
			}

			if (set) {
				texture->setWrapping(wrapS, wrapT, wrapR);
			}
		};

		virtual bool options() override
		{
			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<Scene::Key, LightSource>(mNameBuffer.data());

			ImGui::Checkbox("Is HDR", &mIsHDR);

			static const char* colorType[] = { "Red", "RG", "RGB", "RGBA" };
			std::size_t currentType = static_cast<std::size_t>(mColorType);
			if (addDropdown("Type:##Texture", colorType, IM_ARRAYSIZE(colorType), currentType)) {
				mColorType = static_cast<ColorFormat>(currentType);
			}

			return validKey;
		};

		virtual bool load(const char* path) override
		{
			auto texture = std::make_shared<Texture>(TextureTarget::Texture2D);

			if (mIsHDR) {
				Image<float> image;
				auto result = ImageReader::readHDR(path, image);
				if (!result) {
					SOMBRA_ERROR_LOG << result.description();
					return false;
				}

				texture->setImage(
					image.pixels.get(), TypeId::Float, mColorType, mColorType,
					image.width, image.height
				);
			}
			else {
				Image<unsigned char> image;
				auto result = ImageReader::read(path, image);
				if (!result) {
					SOMBRA_ERROR_LOG << result.description();
					return false;
				}

				texture->setImage(
					image.pixels.get(), TypeId::UnsignedByte, mColorType, mColorType,
					image.width, image.height
				);
			}

			if (!mPanel.mEditor.getScene()->repository.add(Scene::Key(mNameBuffer.data()), texture)
				|| !mPanel.mEditor.getScene()->repository.emplace<Scene::Key, ResourcePath<Texture>>(mNameBuffer.data(), path)
			) {
				return false;
			}

			return true;
		};
	};


	class RepositoryPanel::MeshNode : public RepositoryPanel::SceneImporterTypeNode<Mesh>
	{
	public:		// Functions
		MeshNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "Mesh"; };
	protected:
		virtual void draw(const Scene::Key& key) override
		{
			auto mesh = mPanel.mEditor.getScene()->repository.find<Scene::Key, Mesh>(key);
			auto [min, max] = mesh->getBounds();

			ImGui::Text("Bounds:");
			ImGui::BulletText("Minimum [%.3f, %.3f, %.3f]", min.x, min.y, min.z);
			ImGui::BulletText("Maximum [%.3f, %.3f, %.3f]", max.x, max.y, max.z);
		};
	};


	RepositoryPanel::RepositoryPanel(Editor& editor) : mEditor(editor), mTypeSelected(-1)
	{
		mTypes.emplace_back(new SkinNode(*this));
		mTypes.emplace_back(new LightSourceNode(*this));
		mTypes.emplace_back(new SkeletonAnimatorNode(*this));
		mTypes.emplace_back(new AudioBufferNode(*this));
		mTypes.emplace_back(new ForceNode(*this));
		mTypes.emplace_back(new ProgramNode(*this));
		mTypes.emplace_back(new PassNode(*this));
		mTypes.emplace_back(new RenderableShaderNode(*this));
		mTypes.emplace_back(new TextureNode(*this));
		mTypes.emplace_back(new MeshNode(*this));
	}


	RepositoryPanel::~RepositoryPanel()
	{
		for (ITypeNode* type : mTypes) {
			delete type;
		}
	}


	void RepositoryPanel::render()
	{
		if (!ImGui::Begin("Scene Repository")) {
			ImGui::End();
			return;
		}

		const char* selectedTypeLabel = (mTypeSelected >= 0)? mTypes[mTypeSelected]->getName() : nullptr;
		if (ImGui::BeginCombo("Type:##RepositoryPanel", selectedTypeLabel)) {
			for (std::size_t i = 0; i < mTypes.size(); ++i) {
				bool isSelected = (static_cast<int>(i) == mTypeSelected);
				if (ImGui::Selectable(mTypes[i]->getName(), isSelected)) {
					mTypeSelected = static_cast<int>(i);
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Separator();

		auto scene = mEditor.getScene();
		if (scene && (mTypeSelected >= 0)) {
			mTypes[mTypeSelected]->draw();
		}

		ImGui::End();
	}

}
