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
#include <se/app/ParticleSystemComponent.h>
#include <se/app/Scene.h>
#include <se/app/io/ImageReader.h>
#include <se/app/io/SceneImporter.h>
#include <se/app/io/ShaderLoader.h>
#include "Editor.h"
#include "ImGuiUtils.h"
#include "RepositoryPanel.h"
#include "DefaultShaderBuilder.h"

using namespace se::app;
using namespace se::utils;
using namespace se::graphics;
using namespace se::animation;
using namespace se::audio;
using namespace se::physics;

namespace editor {

	static bool cancelButton()
	{
		return ImGui::Button("Cancel");
	}


	static bool confirmButton(bool enabled = true)
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
		virtual void draw(Repository& repository) = 0;
	};


	template <typename T, bool hasPath>
	class RepositoryPanel::TypeNode : public RepositoryPanel::ITypeNode
	{
	protected:	// Attributes
		static constexpr std::size_t kMaxNameSize = 128;
	private:
		RepositoryPanel& mPanel;
		Scene::Key mSelected;
		bool mShowCreate;

	public:		// Functions
		TypeNode(RepositoryPanel& panel) : mPanel(panel), mShowCreate(false) {};
		virtual ~TypeNode() = default;
		virtual void draw(Repository& repository) override
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Elements")) {
				if (ImGui::SmallButton("Add")) {
					mShowCreate = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove")) {
					repository.remove<Scene::Key, T>(mSelected);
					if (hasPath) {
						repository.remove<Scene::Key, ResourcePath<T>>(mSelected);
					}
					mSelected = "";
				}

				repository.iterate<Scene::Key, T>([&](const Scene::Key& key, std::shared_ptr<T>) {
					if (ImGui::Selectable(key.c_str(), key == mSelected)) {
						mSelected = key;
					}
				});
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Selected element")) {
				if (!repository.find<Scene::Key, T>(mSelected)) {
					mSelected = "";
				}

				if (!mSelected.empty()) {
					std::array<char, kMaxNameSize> nameBuffer = {};
					std::copy(mSelected.begin(), mSelected.end(), nameBuffer.data());
					std::string name = "Name" + getIGPrefix() + "::TypeNode::name";
					if (ImGui::InputText(name.c_str(), nameBuffer.data(), nameBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
						Scene::Key oldName = mSelected;
						mSelected = nameBuffer.data();

						auto element = repository.find<Scene::Key, T>(oldName);
						repository.remove<Scene::Key, T>(oldName);
						repository.add<Scene::Key, T>(mSelected, std::move(element));

						if (hasPath) {
							auto elementPath = repository.find<Scene::Key, ResourcePath<T>>(oldName);
							repository.remove<Scene::Key, ResourcePath<T>>(oldName);
							repository.add<Scene::Key, ResourcePath<T>>(mSelected, std::move(elementPath));
						}
					}

					draw(repository, mSelected);
				}
			}

			if (mShowCreate) {
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::CollapsingHeader("Create")) {
					mShowCreate = !create(repository);
				}
			}
		};
	protected:
		Editor& getEditor() const { return mPanel.mEditor; };
		std::string getIGPrefix() const { return "##ComponentPanel" + std::to_string(mPanel.mPanelId); };
		virtual void draw(Repository& /*repository*/, const Scene::Key& /*key*/) {};
		/** @return	true if the create menu must be shown, false otherwise */
		virtual bool create(Repository& /*repository*/) { return false; };
	};


	template <typename T, bool hasPath>
	class RepositoryPanel::ImportTypeNode : public RepositoryPanel::TypeNode<T, hasPath>
	{
	private:	// Attributes
		std::string mPath;
		FileWindow mFileWindow;

	public:		// Functions
		ImportTypeNode(RepositoryPanel& panel) : TypeNode<T, hasPath>(panel) {};
		virtual ~ImportTypeNode() = default;
	protected:
		virtual bool create(Repository& repository) override
		{
			bool ret = false;
			Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

			std::string label = mPath.empty()? "Open File..." : ("Selected: " + mPath);
			if (ImGui::Button(label.c_str())) {
				mFileWindow.show();
			}
			mFileWindow.execute(mPath);

			bool validOptions = options(repository);

			ImGui::Separator();
			if (cancelButton()) {
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validOptions && !mPath.empty())) {
				if (load(repository, mPath.c_str())) {
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
		virtual bool options(Repository& /*repository*/) { return true; };
		/** @return	true on success, false otherwise */
		virtual bool load(Repository& /*repository*/, const char* /*path*/) { return true; };
	};


	template <typename T, bool hasPath>
	class RepositoryPanel::SceneImporterTypeNode : public RepositoryPanel::ImportTypeNode<T, hasPath>
	{
	private:	// Attributes
		SceneImporter::FileType mFileType;

	public:		// Functions
		SceneImporterTypeNode(RepositoryPanel& panel) :
			ImportTypeNode<T, hasPath>(panel), mFileType(SceneImporter::FileType::GLTF) {};
	protected:
		virtual bool options(Repository&) override
		{
			static const char* fileTypeTags[] = { "GLTF" };
			int currentType = static_cast<int>(mFileType);
			std::string name = "Type" + getIGPrefix() + "::SceneImporterTypeNode::type";
			if (addDropdown(name.c_str(), fileTypeTags, IM_ARRAYSIZE(fileTypeTags), currentType)) {
				mFileType = static_cast<SceneImporter::FileType>(currentType);
			}

			return true;
		};

		virtual bool load(Repository& repository, const char* path) override
		{
			DefaultShaderBuilder shaderBuilder(getEditor(), repository);
			auto SceneImporter = SceneImporter::createSceneImporter(mFileType, shaderBuilder);
			auto result = SceneImporter->load(path, *getEditor().getScene());
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto skin = repository.find<Scene::Key, Skin>(key);
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto source = repository.find<Scene::Key, LightSource>(key);

			static const char* lightTypeTags[] = { "Directional", "Point", "Spot" };
			int currentType = static_cast<int>(source->type);
			std::string name = "Type" + getIGPrefix() + "::LightSourceNode::type";
			if (addDropdown(name.c_str(), lightTypeTags, IM_ARRAYSIZE(lightTypeTags), currentType)) {
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

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIGPrefix() + "::LightSourceNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, LightSource>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();

			if (confirmButton(validKey)) {
				repository.add(Scene::Key(mNameBuffer.data()), std::make_shared<LightSource>(LightSource::Type::Directional));
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto animator = repository.find<Scene::Key, SkeletonAnimator>(key);
			ImGui::Text("Loop time: %.3f seconds", animator->getLoopTime());
		};
	};


	class RepositoryPanel::AudioBufferNode : public RepositoryPanel::ImportTypeNode<Buffer, true>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		AudioBufferNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "AudioBuffer"; };
	protected:
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto path = repository.find<Scene::Key, ResourcePath<Buffer>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}
		}

		virtual bool options(Repository& repository) override
		{
			std::string name = "Name" + getIGPrefix() + "::AudioBufferNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, LightSource>(mNameBuffer.data());
			return validKey;
		};

		virtual bool load(Repository& repository, const char* path) override
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

			if (!repository.add(Scene::Key(mNameBuffer.data()), buffer)
				|| !repository.emplace<Scene::Key, ResourcePath<Buffer>>(mNameBuffer.data(), path)
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto force = repository.find<Scene::Key, Force>(key);
			auto gravity = std::dynamic_pointer_cast<Gravity>(force);
			auto punctual = std::dynamic_pointer_cast<PunctualForce>(force);
			auto directional = std::dynamic_pointer_cast<DirectionalForce>(force);

			static const char* forceTypeTags[] = { "Gravity", "Punctual", "Directional" };
			int currentType = gravity? 0 : punctual? 1 : 2;
			std::string name = "Type" + getIGPrefix() + "::ForceNode::type";
			addDropdown(name.c_str(), forceTypeTags, IM_ARRAYSIZE(forceTypeTags), currentType);

			if (currentType == 0) {
				if (!gravity) {
					gravity = std::make_shared<Gravity>();
					repository.remove<Scene::Key, Force>(key);
					repository.add<Scene::Key, Force>(key, gravity);
				}
				drawGravity(*gravity);
			}
			else if (currentType == 1) {
				if (!punctual) {
					punctual = std::make_shared<PunctualForce>();
					repository.remove<Scene::Key, Force>(key);
					repository.add<Scene::Key, Force>(key, punctual);
				}
				drawPunctualForce(*punctual);
			}
			else if (currentType == 2) {
				if (!directional) {
					directional = std::make_shared<DirectionalForce>();
					repository.remove<Scene::Key, Force>(key);
					repository.add<Scene::Key, Force>(key, directional);
				}
				drawDirectionalForce(*directional);
			}
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIGPrefix() + "::ForceNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, Force>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				repository.add<Scene::Key, Force>(mNameBuffer.data(), std::make_shared<Gravity>());
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


	class RepositoryPanel::ProgramNode : public RepositoryPanel::TypeNode<Program, true>
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto path = repository.find<Scene::Key, ResourcePath<Program>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}
		}

		virtual bool create(Repository& repository) override
		{
			bool ret = false;
			Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

			std::string name = "Name" + getIGPrefix() + "::ProgramNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validOptions = !repository.has<Scene::Key, LightSource>(mNameBuffer.data());

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
					if (repository.add<Scene::Key, Program>(mNameBuffer.data(), std::move(program))
						&& repository.emplace<Scene::Key, ResourcePath<Program>>(mNameBuffer.data(), resourcePath)
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
		std::array<char, kMaxNameSize> mNameBuffer = {};
		int mRendererSelected = -1;
		int mBindableTypeSelected = -1;
		int mSubTypeSelected = -1;
		std::array<char, kMaxNameSize> mUniformName = {};
		Scene::Key mKeyTypeSelected;

	public:		// Functions
		PassNode(RepositoryPanel& panel) : TypeNode(panel) {};
		virtual const char* getName() const override { return "Pass"; };
	protected:
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto pass = repository.find<Scene::Key, Pass>(key);

			ImGui::Text("Renderer: %s", pass->getRenderer().getName().c_str());

			if (ImGui::TreeNode("Add Bindable")) {
				addBindable(repository, *pass);
				ImGui::TreePop();
			}

			showBindables(repository, *pass);
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIGPrefix() + "::PassNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, Pass>(mNameBuffer.data());

			std::vector<Renderer*> renderers;
			std::vector<const char*> rendererNames;
			getEditor().getExternalTools().graphicsEngine->getRenderGraph().processNodes([&](RenderNode* node) {
				if (Renderer* renderer = dynamic_cast<Renderer*>(node)) {
					renderers.push_back(renderer);
					rendererNames.push_back(renderer->getName().c_str());
				}
			});

			std::string name1 = "Renderer" + getIGPrefix() + "::PassNode::Renderer";
			addDropdown(name1.c_str(), rendererNames.data(), rendererNames.size(), mRendererSelected);
			bool isRendererSelected = (mRendererSelected >= 0) && (mRendererSelected < static_cast<int>(renderers.size()));

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey && isRendererSelected)) {
				repository.emplace<Scene::Key, Pass>(mNameBuffer.data(), *renderers[mRendererSelected]);
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	private:
		void addBindable(Repository& repository, Pass& pass)
		{
			const char* bindableTypeTags[] = { "UniformVariableValue", "UniformVariableValueVector", "Texture", "Program", "SetOperation", "SetDepthMask" };
			const char* uniformTypeTags[] = { "int", "unsigned int", "float", "vec2", "ivec2", "vec3", "ivec3", "vec4", "ivec4", "mat3", "mat4", "mat3x4" };
			const char* operationTypeTags[] = { "Culling", "DepthTest", "ScissorTest" };

			std::string name = "Bindable Type" + getIGPrefix() + "::PassNode::BindableType";
			if (addDropdown(name.c_str(), bindableTypeTags, IM_ARRAYSIZE(bindableTypeTags), mBindableTypeSelected)) {
				mSubTypeSelected = -1;
				mUniformName = {};
				mKeyTypeSelected = "";
			}

			switch (mBindableTypeSelected) {
				case 0:
				case 1: {
					std::string name1 = "Name" + getIGPrefix() + "::PassNode::UniformName";
					ImGui::InputText(name1.c_str(), mUniformName.data(), mUniformName.size());
					std::string name2 = "Type" + getIGPrefix() + "::PassNode::UniformType";
					addDropdown(name2.c_str(), uniformTypeTags, IM_ARRAYSIZE(uniformTypeTags), mSubTypeSelected);
				} break;
				case 2: {
					std::string name1 = "Texture" + getIGPrefix() + "::PassNode::Texture";
					addRepoDropdownShowSelected<Scene::Key, Texture>(name1.c_str(), repository, mKeyTypeSelected);
				} break;
				case 3: {
					std::string name1 = "Program" + getIGPrefix() + "::PassNode::Program";
					addRepoDropdownShowSelected<Scene::Key, Program>(name1.c_str(), repository, mKeyTypeSelected);
				} break;
				case 4: {
					std::string name1 = "Operation" + getIGPrefix() + "::PassNode::Operation";
					addDropdown(name1.c_str(), operationTypeTags, IM_ARRAYSIZE(operationTypeTags), mSubTypeSelected);
				} break;
				default:
					break;
			}

			std::string name1 = "Add" + getIGPrefix() + "::PassNode::Add";
			if (ImGui::Button(name1.c_str())) {
				switch (mBindableTypeSelected) {
					case 0: {
						std::shared_ptr<Program> program;
						pass.processBindables([&](const std::shared_ptr<Bindable>& bindable) { if (auto tmp = std::dynamic_pointer_cast<Program>(bindable)) {
							program = tmp;
						}});
						switch (mSubTypeSelected) {
							case 0:		pass.addBindable( std::make_shared<UniformVariableValue<int>>(mUniformName.data(), program) );				break;
							case 1:		pass.addBindable( std::make_shared<UniformVariableValue<unsigned int>>(mUniformName.data(), program) );		break;
							case 2:		pass.addBindable( std::make_shared<UniformVariableValue<float>>(mUniformName.data(), program) );			break;
							case 3:		pass.addBindable( std::make_shared<UniformVariableValue<glm::vec2>>(mUniformName.data(), program) );		break;
							case 4:		pass.addBindable( std::make_shared<UniformVariableValue<glm::ivec2>>(mUniformName.data(), program) );		break;
							case 5:		pass.addBindable( std::make_shared<UniformVariableValue<glm::vec3>>(mUniformName.data(), program) );		break;
							case 6:		pass.addBindable( std::make_shared<UniformVariableValue<glm::ivec3>>(mUniformName.data(), program) );		break;
							case 7:		pass.addBindable( std::make_shared<UniformVariableValue<glm::vec4>>(mUniformName.data(), program) );		break;
							case 8:		pass.addBindable( std::make_shared<UniformVariableValue<glm::ivec4>>(mUniformName.data(), program) );		break;
							case 9:		pass.addBindable( std::make_shared<UniformVariableValue<glm::mat3>>(mUniformName.data(), program) );		break;
							case 10:	pass.addBindable( std::make_shared<UniformVariableValue<glm::mat4>>(mUniformName.data(), program) );		break;
							default:	pass.addBindable( std::make_shared<UniformVariableValue<glm::mat3x4>>(mUniformName.data(), program) );		break;
						}
					} break;
					case 1: {
						std::shared_ptr<Program> program;
						pass.processBindables([&](const std::shared_ptr<Bindable>& bindable) { if (auto tmp = std::dynamic_pointer_cast<Program>(bindable)) {
							program = tmp;
						}});
						switch (mSubTypeSelected) {
							case 0:		pass.addBindable( std::make_shared<UniformVariableValueVector<int>>(mUniformName.data(), program) );			break;
							case 1:		pass.addBindable( std::make_shared<UniformVariableValueVector<unsigned int>>(mUniformName.data(), program) );	break;
							case 2:		pass.addBindable( std::make_shared<UniformVariableValueVector<float>>(mUniformName.data(), program) );			break;
							case 3:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::vec2>>(mUniformName.data(), program) );		break;
							case 4:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec2>>(mUniformName.data(), program) );		break;
							case 5:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::vec3>>(mUniformName.data(), program) );		break;
							case 6:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec3>>(mUniformName.data(), program) );		break;
							case 7:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::vec4>>(mUniformName.data(), program) );		break;
							case 8:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec4>>(mUniformName.data(), program) );		break;
							case 9:		pass.addBindable( std::make_shared<UniformVariableValueVector<glm::mat3>>(mUniformName.data(), program) );		break;
							case 10:	pass.addBindable( std::make_shared<UniformVariableValueVector<glm::mat4>>(mUniformName.data(), program) );		break;
							default:	pass.addBindable( std::make_shared<UniformVariableValueVector<glm::mat3x4>>(mUniformName.data(), program) );	break;
						}
					} break;
					case 2: {
						if (auto texture = repository.find<Scene::Key, Texture>(mKeyTypeSelected)) {
							pass.addBindable(texture);
						}
					} break;
					case 3: {
						if (auto program = repository.find<Scene::Key, Program>(mKeyTypeSelected)) {
							pass.addBindable(program);
						}
					} break;
					case 4: {
						switch (mSubTypeSelected) {
							case 0:		pass.addBindable(std::make_shared<SetOperation>(Operation::Culling));		break;
							case 1:		pass.addBindable(std::make_shared<SetOperation>(Operation::DepthTest));		break;
							default:	pass.addBindable(std::make_shared<SetOperation>(Operation::ScissorTest));	break;
						}
					} break;
					default: {
						pass.addBindable(std::make_shared<SetDepthMask>());
					} break;
				}
			}
		};

		void showBindables(Repository& repository, Pass& pass)
		{
			std::size_t numBindables = 0;
			pass.processBindables([&](std::shared_ptr<Bindable> bindable) {
				std::size_t bindableIndex = numBindables++;

				std::string name = "x" + getIGPrefix() + "::PassNode::remove" + std::to_string(bindableIndex);
				if (ImGui::Button(name.c_str())) {
					pass.removeBindable(bindable);
				}
				ImGui::SameLine();

				std::string treeId = "PassNode::bindable" + std::to_string(bindableIndex);
				if (auto uniform1 = std::dynamic_pointer_cast<UniformVariableValue<int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<int>, %d)", uniform1->getName().c_str(), uniform1->found())) { return; }
					if (int value = uniform1->getValue(); ImGui::DragInt("Value", &value, 1, -INT_MAX, INT_MAX)) {
						uniform1->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform2 = std::dynamic_pointer_cast<UniformVariableValue<unsigned int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<unsigned int>, %d)", uniform2->getName().c_str(), uniform2->found())) { return; }
					if (unsigned int value = uniform2->getValue(); ImGui::DragInt("Value", reinterpret_cast<int*>(&value), 1, 0, INT_MAX)) {
						uniform2->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform3 = std::dynamic_pointer_cast<UniformVariableValue<float>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<float>, %d)", uniform3->getName().c_str(), uniform3->found())) { return; }
					if (float value = uniform3->getValue(); ImGui::DragFloat("Value", &value, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform3->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform4 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<vec2>, %d)", uniform4->getName().c_str(), uniform4->found())) { return; }
					if (glm::vec2 value = uniform4->getValue(); ImGui::DragFloat2("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform4->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform5 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec2>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<ivec2>, %d)", uniform5->getName().c_str(), uniform5->found())) { return; }
					if (glm::ivec2 value = uniform5->getValue(); ImGui::DragInt2("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform5->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform6 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<vec3>, %d)", uniform6->getName().c_str(), uniform6->found())) { return; }
					if (glm::vec3 value = uniform6->getValue(); ImGui::DragFloat3("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform6->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform7 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<ivec3>, %d)", uniform7->getName().c_str(), uniform7->found())) { return; }
					if (glm::ivec3 value = uniform7->getValue(); ImGui::DragInt3("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform7->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform8 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<vec4>, %d)", uniform8->getName().c_str(), uniform8->found())) { return; }
					if (glm::vec4 value = uniform8->getValue(); ImGui::DragFloat4("Value", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
						uniform8->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform9 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<ivec4>, %d)", uniform9->getName().c_str(), uniform9->found())) { return; }
					if (glm::ivec4 value = uniform9->getValue(); ImGui::DragInt4("Value", glm::value_ptr(value), 0.005f, -INT_MAX, INT_MAX)) {
						uniform9->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform10 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<mat3>, %d)", uniform10->getName().c_str(), uniform10->found())) { return; }
					glm::mat3 value = uniform10->getValue();
					if (drawMat3ImGui("value", value)) {
						uniform10->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform11 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<mat4>, %d)", uniform11->getName().c_str(), uniform11->found())) { return; }
					glm::mat4 value = uniform11->getValue();
					if (drawMat4ImGui("value", value)) {
						uniform11->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform12 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3x4>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (Uniform<mat3x4>, %d)", uniform12->getName().c_str(), uniform12->found())) { return; }
					glm::mat3x4 value = uniform12->getValue();
					if (drawMat3x4ImGui("value", value)) {
						uniform12->setValue(value);
					}
					ImGui::TreePop();
				}
				else if (auto uniform13 = std::dynamic_pointer_cast<UniformVariableValueVector<int>>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<int>, %d)", uniform13->getName().c_str(), uniform13->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<unsigned int>, %d)", uniform14->getName().c_str(), uniform14->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<float>, %d)", uniform15->getName().c_str(), uniform15->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<vec2>, %d)", uniform16->getName().c_str(), uniform16->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<ivec2>, %d)", uniform17->getName().c_str(), uniform17->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<vec3>, %d)", uniform18->getName().c_str(), uniform18->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<ivec3>, %d)", uniform19->getName().c_str(), uniform19->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<vec4>, %d)", uniform20->getName().c_str(), uniform20->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<ivec4>, %d)", uniform21->getName().c_str(), uniform21->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<mat3>, %d)", uniform22->getName().c_str(), uniform22->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<mat4>, %d)", uniform23->getName().c_str(), uniform23->found())) { return; }
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
					if (!ImGui::TreeNode(treeId.c_str(), "%s (UniformV<mat3x4>, %d)", uniform24->getName().c_str(), uniform24->found())) { return; }
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
					std::string name1 = getIGPrefix() + "::PassNode::ChangeTexture" + std::to_string(bindableIndex);
					if (addRepoDropdownShowSelectedValue<Scene::Key, Texture>(name1.c_str(), repository, texture2)) {
						pass.removeBindable(texture);
						pass.addBindable(texture2);
					}
					ImGui::TreePop();
				}
				else if (auto program = std::dynamic_pointer_cast<Program>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "Program")) { return; }
					std::shared_ptr<Program> program2 = program;
					std::string name1 = getIGPrefix() + "::PassNode::ChangeProgram" + std::to_string(bindableIndex);
					if (addRepoDropdownShowSelectedValue<Scene::Key, Program>(name1.c_str(), repository, program2)) {
						pass.removeBindable(program);
						pass.addBindable(program2);
					}
					ImGui::TreePop();
				}
				else if (auto setOperation = std::dynamic_pointer_cast<SetOperation>(bindable)) {
					const char* operation = (setOperation->getOperation() == Operation::Culling)? "Culling" :
											(setOperation->getOperation() == Operation::DepthTest)? "DepthTest" :
											(setOperation->getOperation() == Operation::ScissorTest)? "ScissorTest" :
											"Blending";
					if (!ImGui::TreeNode(treeId.c_str(), "SetOperation %s", operation)) { return; }
					bool isEnabled = setOperation->isEnabled();
					if (ImGui::Checkbox("Enabled", &isEnabled)) {
						setOperation->setEnabled(isEnabled);
					}
					ImGui::TreePop();
				}
				else if (auto setDepthMask = std::dynamic_pointer_cast<SetDepthMask>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "SetDepthMask")) { return; }
					bool isEnabled = setDepthMask->isEnabled();
					if (ImGui::Checkbox("Enabled", &isEnabled)) {
						setDepthMask->setEnabled(isEnabled);
					}
					ImGui::TreePop();
				}
			});
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto shader = repository.find<Scene::Key, RenderableShader>(key);

			std::shared_ptr<Pass> pass;
			std::string name = getIGPrefix() + "::RenderableShaderNode::Add";
			if (addRepoDropdownButtonValue<Scene::Key, Pass>(name.c_str(), "Add Shader", repository, pass)) {
				shader->addPass(std::move(pass));
			}

			std::size_t passIndex = 0;
			shader->getTechnique()->processPasses([&](const std::shared_ptr<Pass>& pass1) {
				std::string name1 = "x" + getIGPrefix() + "::RenderableShaderNode::Remove" + std::to_string(passIndex++);
				if (ImGui::Button(name1.c_str())) {
					shader->removePass(pass1);
				}
				ImGui::SameLine();

				std::shared_ptr<Pass> pass2 = pass1;
				std::string name2 = getIGPrefix() + "::RenderableShaderNode::Change" + std::to_string(passIndex);
				if (addRepoDropdownShowSelectedValue<Scene::Key, Pass>(name2.c_str(), repository, pass2)) {
					shader->removePass(pass1);
					shader->addPass(pass2);
				}
			});
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIGPrefix() + "::TextureNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, RenderableShader>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				repository.emplace<Scene::Key, RenderableShader>(mNameBuffer.data(), getEditor().getEventManager());
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	class RepositoryPanel::TextureNode : public RepositoryPanel::ImportTypeNode<Texture, true>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;
		ColorFormat mColorType = ColorFormat::RGB;
		bool mIsHDR = false;

	public:		// Functions
		TextureNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "Texture"; };
	protected:
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto path = repository.find<Scene::Key, ResourcePath<Texture>>(key);
			if (path) {
				ImGui::Text("Path: %s", path->path.c_str());
			}
			else {
				ImGui::Text("No path");
			}

			auto texture = repository.find<Scene::Key, Texture>(key);
			ImGui::Image(static_cast<void*>(texture.get()), ImVec2{ 200.0f, 200.0f });

			int textureUnit = texture->getTextureUnit();
			if (ImGui::DragInt("Texture Unit", &textureUnit, 1, 0, 16)) {
				texture->setTextureUnit(textureUnit);
			}

			TextureFilter min, mag;
			texture->getFiltering(&min, &mag);
			int iMin = static_cast<int>(min);
			int iMag = static_cast<int>(mag);

			static const char* filterTypeTags[] = { "Nearest", "Linear", "Nearest MipMap Nearest", "Linear MipMap Nearest", "Nearest MipMap Linear", "Linear MipMap Linear" };
			std::string name = getIGPrefix() + "::TextureNode::MinFilter";
			if (addDropdown(name.c_str(), filterTypeTags, IM_ARRAYSIZE(filterTypeTags), iMin)) {
				min = static_cast<TextureFilter>(iMin);
				texture->setFiltering(min, mag);
			}
			std::string name1 = getIGPrefix() + "::TextureNode::MagFilter";
			if (addDropdown(name1.c_str(), filterTypeTags, IM_ARRAYSIZE(filterTypeTags), iMag)) {
				mag = static_cast<TextureFilter>(iMag);
				texture->setFiltering(min, mag);
			}

			TextureWrap wrapS, wrapT, wrapR;
			texture->getWrapping(&wrapS, &wrapT, &wrapR);
			int iWrapS = static_cast<int>(wrapS);
			int iWrapT = static_cast<int>(wrapT);
			int iWrapR = static_cast<int>(wrapR);

			static const char* wrapTypeTags[] = { "Repeat", "Mirrored Repeat", "Clamp to Edge", "Clamp to Border" };
			bool set = false;
			std::string name2 = getIGPrefix() + "::TextureNode::WrapS";
			if (addDropdown(name2.c_str(), wrapTypeTags, IM_ARRAYSIZE(wrapTypeTags), iWrapS)) {
				wrapS = static_cast<TextureWrap>(iWrapS);
				set = true;
			}
			if (texture->getTarget() != TextureTarget::Texture1D) {
				std::string name3 = getIGPrefix() + "::TextureNode::WrapT";
				if (addDropdown(name3.c_str(), wrapTypeTags, IM_ARRAYSIZE(wrapTypeTags), iWrapT)) {
					wrapT = static_cast<TextureWrap>(iWrapT);
					set = true;
				}

				if (texture->getTarget() != TextureTarget::Texture2D) {
					std::string name4 = getIGPrefix() + "::TextureNode::WrapR";
					if (addDropdown(name4.c_str(), wrapTypeTags, IM_ARRAYSIZE(wrapTypeTags), iWrapR)) {
						wrapR = static_cast<TextureWrap>(iWrapR);
						set = true;
					}
				}
			}

			if (set) {
				texture->setWrapping(wrapS, wrapT, wrapR);
			}
		};

		virtual bool options(Repository& repository) override
		{
			std::string name = "Name" + getIGPrefix() + "::TextureNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, LightSource>(mNameBuffer.data());

			ImGui::Checkbox("Is HDR", &mIsHDR);

			static const char* colorTypeTags[] = { "Red", "RG", "RGB", "RGBA" };
			int currentType = static_cast<int>(mColorType);
			std::string name1 = "Type" + getIGPrefix() + "::TextureNode::type";
			if (addDropdown(name1.c_str(), colorTypeTags, IM_ARRAYSIZE(colorTypeTags), currentType)) {
				mColorType = static_cast<ColorFormat>(currentType);
			}

			return validKey;
		};

		virtual bool load(Repository& repository, const char* path) override
		{
			auto texture = std::make_shared<Texture>(TextureTarget::Texture2D);
			texture->setTextureUnit(0);

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

			if (!repository.add(Scene::Key(mNameBuffer.data()), texture)
				|| !repository.emplace<Scene::Key, ResourcePath<Texture>>(mNameBuffer.data(), path)
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
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto mesh = repository.find<Scene::Key, Mesh>(key);
			auto [min, max] = mesh->getBounds();

			ImGui::Text("Bounds:");
			ImGui::BulletText("Minimum [%.3f, %.3f, %.3f]", min.x, min.y, min.z);
			ImGui::BulletText("Maximum [%.3f, %.3f, %.3f]", max.x, max.y, max.z);
		};
	};


	class RepositoryPanel::ParticleEmitterNode : public RepositoryPanel::TypeNode<ParticleEmitter>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;

	public:		// Functions
		ParticleEmitterNode(RepositoryPanel& panel) : TypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "ParticleEmitter"; };
	protected:
		virtual void draw(Repository& repository, const Scene::Key& key) override
		{
			auto emitter = repository.find<Scene::Key, ParticleEmitter>(key);

			ImGui::DragInt("Maximum particles", reinterpret_cast<int*>(&emitter->maxParticles), 1, 0, INT_MAX);
			ImGui::DragFloat("Particle simulation duration", &emitter->duration, 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f);
			ImGui::Checkbox("Loop simulation", &emitter->loop);
			ImGui::DragFloat("Initial velocity", &emitter->initialVelocity, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			ImGui::DragFloat("Initial position randomness", &emitter->initialPositionRandomFactor, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Initial velocity randomness", &emitter->initialVelocityRandomFactor, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Initial rotation randomness", &emitter->initialRotationRandomFactor, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Particle scale", &emitter->scale, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Initial scale randomness", &emitter->initialScaleRandomFactor, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Life length", &emitter->lifeLength, 0.005f, 0.0f, FLT_MAX, "%.3f", 1.0f);
			ImGui::DragFloat("Life length randomness", &emitter->lifeLengthRandomFactor, 0.005f, 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::DragFloat("Gravity", &emitter->gravity, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIGPrefix() + "::ParticleEmitterNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !repository.has<Scene::Key, ParticleEmitter>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				repository.emplace<Scene::Key, ParticleEmitter>(mNameBuffer.data());
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	RepositoryPanel::RepositoryPanel(Editor& editor) : IEditorPanel(editor)
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
		mTypes.emplace_back(new ParticleEmitterNode(*this));
	}


	RepositoryPanel::~RepositoryPanel()
	{
		for (ITypeNode* type : mTypes) {
			delete type;
		}
	}


	bool RepositoryPanel::render()
	{
		bool open = true;
		if (ImGui::Begin(("Scene Repository##SceneRepository" + std::to_string(mPanelId)).c_str(), &open)) {
			if (ImGui::BeginTable("RepositoryTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				for (std::size_t i = 0; i < mTypes.size(); ++i) {
					if (ImGui::Selectable(mTypes[i]->getName(), (static_cast<int>(i) == mTypeSelected))) {
						mTypeSelected = static_cast<int>(i);
					}
				}

				ImGui::TableSetColumnIndex(1);
				auto scene = mEditor.getScene();
				if (scene && (mTypeSelected >= 0)) {
					mTypes[mTypeSelected]->draw(scene->repository);
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
		return open;
	}

}
