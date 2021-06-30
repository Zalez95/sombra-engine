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
#include <se/app/ParticleSystemComponent.h>
#include <se/app/Scene.h>
#include <se/app/io/ImageReader.h>
#include <se/app/io/SceneImporter.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/graphics/RenderableShader.h>
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


	template <typename T>
	class RepositoryPanel::TypeNode : public RepositoryPanel::ITypeNode
	{
	protected:	// Attributes
		static constexpr std::size_t kMaxNameSize = 128;
	private:
		RepositoryPanel& mPanel;
		std::string mSelectedName;
		bool mShowCreate;

	public:		// Functions
		TypeNode(RepositoryPanel& panel) : mPanel(panel), mShowCreate(false) {};
		virtual ~TypeNode() = default;
		virtual void draw(Repository& repository) override
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Elements")) {
				if (ImGui::SmallButton(("Add" + getIdPrefix() + "AddResource").c_str())) {
					mShowCreate = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(("Clone" + getIdPrefix() + "CloneResource").c_str())) {
					auto selected = repository.findByName<T>(mSelectedName.c_str());
					if (selected) {
						Repository::ResourceRef<T> cloned = repository.clone<T>(selected);
						if (cloned) {
							cloned.setFakeUser();

							setRepoName<T>(cloned.getResource(), mSelectedName.c_str(), repository);
							mSelectedName = cloned.getResource().getName();
						}
					}
				}

				std::size_t i = 0;
				if (ImGui::BeginTable((getIdPrefix() + "Elements").c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
					repository.iterate<T>([&](Repository::ResourceRef<T> value) {
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (value.getResource().getName() == mSelectedName);
						if (ImGui::Selectable(value.getResource().getName(), isSelected)) {
							mSelectedName = value.getResource().getName();
						}

						ImGui::TableSetColumnIndex(1);
						std::string buttonLabel = (value.hasFakeUser()? "F" : "") + std::to_string(value.getUserCount() - 1)
							+ getIdPrefix() + "FakeUser" + std::to_string(i++);
						if (ImGui::Button(buttonLabel.c_str())) {
							value.setFakeUser(!value.hasFakeUser());
						}
					});

					ImGui::EndTable();
				}
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Selected element")) {
				auto selected = repository.findByName<T>(mSelectedName.c_str());
				if (selected) {
					std::array<char, kMaxNameSize> nameBuffer = {};
					std::strcpy(nameBuffer.data(), selected.getResource().getName());
					std::string name = "Name" + getIdPrefix() + "TypeNode::name";
					if (ImGui::InputText(name.c_str(), nameBuffer.data(), nameBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
						setRepoName<T>(selected.getResource(), name.data(), repository);
						mSelectedName = selected.getResource().getName();
					}

					std::array<char, 4 * kMaxNameSize> pathBuffer = {};
					std::strcpy(pathBuffer.data(), selected.getResource().getPath());
					std::string path = "Path" + getIdPrefix() + "TypeNode::path";
					if (ImGui::InputText(path.c_str(), pathBuffer.data(), pathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
						selected.getResource().setPath(pathBuffer.data());
					}

					if (selected.getResource().isLinked()) {
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					}
					draw(repository, selected);
					if (selected.getResource().isLinked()) {
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}
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
		std::string getIdPrefix() const { return "##RepositoryPanel" + std::to_string(mPanel.mPanelId) + "::"; };
		virtual void draw(Repository& /*repository*/, Repository::ResourceRef<T> /*resource*/) {};
		/** @return	true if the create menu must be shown, false otherwise */
		virtual bool create(Repository& /*repository*/) { return false; };
	};


	template <typename T>
	class RepositoryPanel::ImportTypeNode : public RepositoryPanel::TypeNode<T>
	{
	private:	// Attributes
		std::string mPath;
		FileWindow mFileWindow;

	public:		// Functions
		ImportTypeNode(RepositoryPanel& panel) :
			TypeNode<T>(panel), mFileWindow((TypeNode<T>::getIdPrefix() + "ImportFile").c_str()) {};
		virtual ~ImportTypeNode() = default;
	protected:
		virtual bool create(Repository& repository) override
		{
			bool ret = false;
			Alert importErrorPopUp((TypeNode<T>::getIdPrefix() + "ErrorAlert").c_str(), "Error", "Failed to import, see logs for more details", "Close");

			std::string label = (mPath.empty()? "Open File..." : ("Selected: " + mPath)) + TypeNode<T>::getIdPrefix() + "OpenFile";
			if (ImGui::Button(label.c_str())) {
				mFileWindow.show();
			}
			mFileWindow.execute(mPath);

			bool validOptions = options(repository);

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + TypeNode<T>::getIdPrefix() + "CancelCreate").c_str())) {
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


	template <typename T>
	class RepositoryPanel::SceneImporterTypeNode : public RepositoryPanel::ImportTypeNode<T>
	{
	private:	// Attributes
		SceneImporter::FileType mFileType;

	public:		// Functions
		SceneImporterTypeNode(RepositoryPanel& panel) :
			ImportTypeNode<T>(panel), mFileType(SceneImporter::FileType::GLTF) {};
	protected:
		virtual bool options(Repository&) override
		{
			static const char* fileTypeTags[] = { "GLTF" };
			int currentType = static_cast<int>(mFileType);
			std::string name = "Type" + TypeNode<T>::getIdPrefix() + "SceneImporterTypeNode::type";
			if (addDropdown(name.c_str(), fileTypeTags, IM_ARRAYSIZE(fileTypeTags), currentType)) {
				mFileType = static_cast<SceneImporter::FileType>(currentType);
			}

			return true;
		};

		virtual bool load(Repository& repository, const char* path) override
		{
			DefaultShaderBuilder shaderBuilder(TypeNode<T>::getEditor(), repository);
			auto SceneImporter = SceneImporter::createSceneImporter(mFileType, shaderBuilder);
			auto result = SceneImporter->load(path, *TypeNode<T>::getEditor().getScene());
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
		virtual void draw(Repository&, Repository::ResourceRef<Skin> skin) override
		{
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
		virtual void draw(Repository&, Repository::ResourceRef<LightSource> source) override
		{
			static const char* lightTypeTags[] = { "Directional", "Point", "Spot" };
			int currentType = static_cast<int>(source->type);
			std::string name = "Type" + getIdPrefix() + "LightSourceNode::type";
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

			std::string name = "Name" + getIdPrefix() + "LightSourceNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<LightSource>(mNameBuffer.data());

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();

			if (confirmButton(validName)) {
				auto source = repository.insert(std::make_shared<LightSource>(LightSource::Type::Directional), mNameBuffer.data());
				source.setFakeUser();
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
		virtual void draw(Repository&, Repository::ResourceRef<SkeletonAnimator> animator) override
		{
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
		virtual bool options(Repository& repository) override
		{
			std::string name = "Name" + getIdPrefix() + "AudioBufferNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<Buffer>(mNameBuffer.data());
			return validName;
		};

		virtual bool load(Repository& repository, const char* path) override
		{
			AudioFile<float> audioFile;
			if (!audioFile.load(path)) {
				SOMBRA_ERROR_LOG << "Error reading the audio file " << path;
				return false;
			}

			auto bufferSPtr = std::make_shared<Buffer>(
				audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
				FormatId::MonoFloat, audioFile.getSampleRate()
			);

			auto buffer = repository.insert(std::move(bufferSPtr), mNameBuffer.data());
			buffer.getResource().setPath(path);
			buffer.setFakeUser();

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
		virtual void draw(Repository& repository, Repository::ResourceRef<Force> force) override
		{
			auto gravity = std::dynamic_pointer_cast<Gravity>(force.get());
			auto punctual = std::dynamic_pointer_cast<PunctualForce>(force.get());
			auto directional = std::dynamic_pointer_cast<DirectionalForce>(force.get());

			static const char* forceTypeTags[] = { "Gravity", "Punctual", "Directional" };
			int currentType = gravity? 0 : punctual? 1 : 2;
			std::string name = "Type" + getIdPrefix() + "ForceNode::type";
			addDropdown(name.c_str(), forceTypeTags, IM_ARRAYSIZE(forceTypeTags), currentType);

			if (currentType == 0) {
				if (!gravity) {
					force.setFakeUser(false);

					gravity = std::make_shared<Gravity>();
					auto force2 = repository.insert<Force>(gravity);
					force2.setFakeUser(true);
					force2.getResource().setName(force.getResource().getName());
					force2.getResource().setPath(force.getResource().getPath());
					if (force.getResource().isLinked()) {
						force2.getResource().setLinkedFile(force.getResource().getLinkedFile());
					}
				}
				drawGravity(*gravity);
			}
			else if (currentType == 1) {
				if (!punctual) {
					force.setFakeUser(false);

					punctual = std::make_shared<PunctualForce>();
					auto force2 = repository.insert<Force>(punctual);
					force2.setFakeUser(true);
					force2.getResource().setName(force.getResource().getName());
					force2.getResource().setPath(force.getResource().getPath());
					if (force.getResource().isLinked()) {
						force2.getResource().setLinkedFile(force.getResource().getLinkedFile());
					}
				}
				drawPunctualForce(*punctual);
			}
			else if (currentType == 2) {
				if (!directional) {
					force.setFakeUser(false);

					directional = std::make_shared<DirectionalForce>();
					auto force2 = repository.insert<Force>(directional);
					force2.setFakeUser(true);
					force2.getResource().setName(force.getResource().getName());
					force2.getResource().setPath(force.getResource().getPath());
					if (force.getResource().isLinked()) {
						force2.getResource().setLinkedFile(force.getResource().getLinkedFile());
					}
				}
				drawDirectionalForce(*directional);
			}
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIdPrefix() + "ForceNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<Force>(mNameBuffer.data());

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validName)) {
				auto force = repository.insert<Force>(std::make_shared<Gravity>(), mNameBuffer.data());
				force.setFakeUser();
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
		ProgramNode(RepositoryPanel& panel) :
			TypeNode(panel), mNameBuffer{}, mPath(nullptr),
			mFileWindow((getIdPrefix() + "ProgramFile").c_str()) {};
		virtual const char* getName() const override { return "Program"; };
	protected:
		virtual bool create(Repository& repository) override
		{
			bool ret = false;
			Alert importErrorPopUp((getIdPrefix() + "ErrorAlert").c_str(), "Error", "Failed to import, see logs for more details", "Close");

			std::string name = "Name" + getIdPrefix() + "ProgramNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<LightSource>(mNameBuffer.data());

			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted("Some shaders are optional");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			std::string vLabel = (mPathVertex.empty()? "Open Vertex Shader..." : "Selected: " + mPathVertex) + getIdPrefix() + "OpenVertex";
			if (ImGui::Button(vLabel.c_str())) {
				mPath = &mPathVertex;
				mFileWindow.show();
			}

			std::string gLabel = (mPathGeometry.empty()? "Open Geometry Shader..." : "Selected: " + mPathGeometry) + getIdPrefix() + "OpenGeometry";
			if (ImGui::Button(gLabel.c_str())) {
				mPath = &mPathGeometry;
				mFileWindow.show();
			}

			std::string fLabel = (mPathFragment.empty()? "Open Fragment Shader..." : "Selected: " + mPathFragment) + getIdPrefix() + "OpenFragment";
			if (ImGui::Button(fLabel.c_str())) {
				mPath = &mPathFragment;
				mFileWindow.show();
			}

			if (mPath) {
				mFileWindow.execute(*mPath);
			}

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validName)) {
				std::shared_ptr<Program> programSPtr;
				auto result = ShaderLoader::createProgram(
					mPathVertex.empty()? nullptr : mPathVertex.c_str(),
					mPathGeometry.empty()? nullptr : mPathGeometry.c_str(),
					mPathFragment.empty()? nullptr : mPathFragment.c_str(),
					programSPtr
				);
				if (result) {
					auto program = repository.insert<Program>(std::move(programSPtr), mNameBuffer.data());
					program.setFakeUser();
					program.getResource().setPath((mPathVertex + "|" + mPathGeometry + "|" + mPathFragment).c_str());
					mNameBuffer.fill(0);
					ret = true;
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


	class RepositoryPanel::RenderableShaderStepNode : public RepositoryPanel::TypeNode<RenderableShaderStep>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer = {};
		int mRendererSelected = -1;
		int mBindableTypeSelected = -1;
		int mSubTypeSelected = -1;
		std::array<char, kMaxNameSize> mUniformName = {};
		std::string mNameSelected;

	public:		// Functions
		RenderableShaderStepNode(RepositoryPanel& panel) : TypeNode(panel) {};
		virtual const char* getName() const override { return "RenderableShaderStep"; };
	protected:
		virtual void draw(Repository& repository, Repository::ResourceRef<RenderableShaderStep> step) override
		{
			ImGui::Text("Renderer: %s", step->getPass()->getRenderer().getName().c_str());

			if (ImGui::TreeNode("Add Bindable")) {
				addBindable(repository, *step);
				ImGui::TreePop();
			}

			showBindables(repository, *step);
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIdPrefix() + "RenderableShaderStepNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<RenderableShaderStep>(mNameBuffer.data());

			std::vector<Renderer*> renderers;
			std::vector<const char*> rendererNames;
			getEditor().getExternalTools().graphicsEngine->getRenderGraph().processNodes([&](RenderNode* node) {
				if (Renderer* renderer = dynamic_cast<Renderer*>(node)) {
					renderers.push_back(renderer);
					rendererNames.push_back(renderer->getName().c_str());
				}
			});

			std::string name1 = "Renderer" + getIdPrefix() + "RenderableShaderStepNode::Renderer";
			addDropdown(name1.c_str(), rendererNames.data(), rendererNames.size(), mRendererSelected);
			bool isRendererSelected = (mRendererSelected >= 0) && (mRendererSelected < static_cast<int>(renderers.size()));

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validName && isRendererSelected)) {
				auto step = repository.insert<RenderableShaderStep>(std::make_shared<RenderableShaderStep>(*renderers[mRendererSelected]), mNameBuffer.data());
				step.setFakeUser();
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	private:
		void addBindable(Repository& repository, RenderableShaderStep& step)
		{
			const char* bindableTypeTags[] = { "UniformVariableValue", "UniformVariableValueVector", "Texture", "Program", "SetOperation", "SetDepthMask" };
			const char* uniformTypeTags[] = { "int", "unsigned int", "float", "vec2", "ivec2", "vec3", "ivec3", "vec4", "ivec4", "mat3", "mat4", "mat3x4" };
			const char* operationTypeTags[] = { "Culling", "DepthTest", "ScissorTest" };

			std::string name = "Bindable Type" + getIdPrefix() + "RenderableShaderStepNode::BindableType";
			if (addDropdown(name.c_str(), bindableTypeTags, IM_ARRAYSIZE(bindableTypeTags), mBindableTypeSelected)) {
				mSubTypeSelected = -1;
				mUniformName = {};
				mNameSelected = "";
			}

			switch (mBindableTypeSelected) {
				case 0:
				case 1: {
					std::string name1 = "Name" + getIdPrefix() + "RenderableShaderStepNode::UniformName";
					ImGui::InputText(name1.c_str(), mUniformName.data(), mUniformName.size());
					std::string name2 = "Type" + getIdPrefix() + "RenderableShaderStepNode::UniformType";
					addDropdown(name2.c_str(), uniformTypeTags, IM_ARRAYSIZE(uniformTypeTags), mSubTypeSelected);
				} break;
				case 2: {
					std::string name1 = "Texture" + getIdPrefix() + "RenderableShaderStepNode::Texture";
					auto texture = repository.findByName<Texture>(mNameSelected.c_str());
					if (addRepoDropdownShowSelected(name1.c_str(), repository, texture)) {
						mNameSelected = texture.getResource().getName();
					}
				} break;
				case 3: {
					std::string name1 = "Program" + getIdPrefix() + "RenderableShaderStepNode::Program";
					auto program = repository.findByName<Program>(mNameSelected.c_str());
					if (addRepoDropdownShowSelected(name1.c_str(), repository, program)) {
						mNameSelected = program.getResource().getName();
					}
				} break;
				case 4: {
					std::string name1 = "Operation" + getIdPrefix() + "RenderableShaderStepNode::Operation";
					addDropdown(name1.c_str(), operationTypeTags, IM_ARRAYSIZE(operationTypeTags), mSubTypeSelected);
				} break;
				default:
					break;
			}

			std::string name1 = "Add" + getIdPrefix() + "RenderableShaderStepNode::Add";
			if (ImGui::Button(name1.c_str())) {
				switch (mBindableTypeSelected) {
					case 0: {
						std::shared_ptr<Program> program;
						step.processPrograms([&](const auto& tmp) { program = tmp.get(); });
						switch (mSubTypeSelected) {
							case 0:		step.addBindable( std::make_shared<UniformVariableValue<int>>(mUniformName.data(), program) );				break;
							case 1:		step.addBindable( std::make_shared<UniformVariableValue<unsigned int>>(mUniformName.data(), program) );		break;
							case 2:		step.addBindable( std::make_shared<UniformVariableValue<float>>(mUniformName.data(), program) );			break;
							case 3:		step.addBindable( std::make_shared<UniformVariableValue<glm::vec2>>(mUniformName.data(), program) );		break;
							case 4:		step.addBindable( std::make_shared<UniformVariableValue<glm::ivec2>>(mUniformName.data(), program) );		break;
							case 5:		step.addBindable( std::make_shared<UniformVariableValue<glm::vec3>>(mUniformName.data(), program) );		break;
							case 6:		step.addBindable( std::make_shared<UniformVariableValue<glm::ivec3>>(mUniformName.data(), program) );		break;
							case 7:		step.addBindable( std::make_shared<UniformVariableValue<glm::vec4>>(mUniformName.data(), program) );		break;
							case 8:		step.addBindable( std::make_shared<UniformVariableValue<glm::ivec4>>(mUniformName.data(), program) );		break;
							case 9:		step.addBindable( std::make_shared<UniformVariableValue<glm::mat3>>(mUniformName.data(), program) );		break;
							case 10:	step.addBindable( std::make_shared<UniformVariableValue<glm::mat4>>(mUniformName.data(), program) );		break;
							default:	step.addBindable( std::make_shared<UniformVariableValue<glm::mat3x4>>(mUniformName.data(), program) );		break;
						}
					} break;
					case 1: {
						std::shared_ptr<Program> program;
						step.processPrograms([&](const auto& tmp) { program = tmp.get(); });
						switch (mSubTypeSelected) {
							case 0:		step.addBindable( std::make_shared<UniformVariableValueVector<int>>(mUniformName.data(), program) );			break;
							case 1:		step.addBindable( std::make_shared<UniformVariableValueVector<unsigned int>>(mUniformName.data(), program) );	break;
							case 2:		step.addBindable( std::make_shared<UniformVariableValueVector<float>>(mUniformName.data(), program) );			break;
							case 3:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::vec2>>(mUniformName.data(), program) );		break;
							case 4:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec2>>(mUniformName.data(), program) );		break;
							case 5:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::vec3>>(mUniformName.data(), program) );		break;
							case 6:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec3>>(mUniformName.data(), program) );		break;
							case 7:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::vec4>>(mUniformName.data(), program) );		break;
							case 8:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::ivec4>>(mUniformName.data(), program) );		break;
							case 9:		step.addBindable( std::make_shared<UniformVariableValueVector<glm::mat3>>(mUniformName.data(), program) );		break;
							case 10:	step.addBindable( std::make_shared<UniformVariableValueVector<glm::mat4>>(mUniformName.data(), program) );		break;
							default:	step.addBindable( std::make_shared<UniformVariableValueVector<glm::mat3x4>>(mUniformName.data(), program) );	break;
						}
					} break;
					case 2: {
						if (auto texture = repository.findByName<Texture>(mNameSelected.c_str())) {
							step.addResource(texture);
						}
					} break;
					case 3: {
						if (auto program = repository.findByName<Program>(mNameSelected.c_str())) {
							step.addResource(program);
						}
					} break;
					case 4: {
						switch (mSubTypeSelected) {
							case 0:		step.addBindable(std::make_shared<SetOperation>(Operation::Culling));		break;
							case 1:		step.addBindable(std::make_shared<SetOperation>(Operation::DepthTest));		break;
							default:	step.addBindable(std::make_shared<SetOperation>(Operation::ScissorTest));	break;
						}
					} break;
					default: {
						step.addBindable(std::make_shared<SetDepthMask>());
					} break;
				}
			}
		};

		void showBindables(Repository& repository, RenderableShaderStep& step)
		{
			std::size_t numBindables = 0;
			step.processPrograms([&](const auto& program) {
				std::size_t bindableIndex = numBindables++;

				std::string name = "x" + getIdPrefix() + "RenderableShaderStepNode::remove" + std::to_string(bindableIndex);
				if (ImGui::Button(name.c_str())) {
					step.removeResource(program);
				}
				ImGui::SameLine();

				std::string treeId = "RenderableShaderStepNode::bindable" + std::to_string(bindableIndex);
				if (!ImGui::TreeNode(treeId.c_str(), "Program")) { return; }
				auto program2 = program;
				std::string name1 = getIdPrefix() + "RenderableShaderStepNode::ChangeProgram" + std::to_string(bindableIndex);
				if (addRepoDropdownShowSelected(name1.c_str(), repository, program2)) {
					step.removeResource(program);
					step.addResource(program2);
				}
				ImGui::TreePop();
			});
			step.processTextures([&](const auto& texture) {
				std::size_t bindableIndex = numBindables++;

				std::string name = "x" + getIdPrefix() + "RenderableShaderStepNode::remove" + std::to_string(bindableIndex);
				if (ImGui::Button(name.c_str())) {
					step.removeResource(texture);
				}
				ImGui::SameLine();

				std::string treeId = "RenderableShaderStepNode::bindable" + std::to_string(bindableIndex);
				if (!ImGui::TreeNode(treeId.c_str(), "Texture (Unit %d)", texture->getTextureUnit())) { return; }
				auto texture2 = texture;
				std::string name1 = getIdPrefix() + "RenderableShaderStepNode::ChangeTexture" + std::to_string(bindableIndex);
				if (addRepoDropdownShowSelected(name1.c_str(), repository, texture2)) {
					step.removeResource(texture);
					step.addResource(texture2);
				}
				ImGui::TreePop();
			});
			step.processBindables([&](std::shared_ptr<Bindable> bindable) {
				if (std::dynamic_pointer_cast<Program>(bindable) || std::dynamic_pointer_cast<Texture>(bindable)) {
					return;
				}

				std::size_t bindableIndex = numBindables++;

				std::string name = "x" + getIdPrefix() + "RenderableShaderStepNode::remove" + std::to_string(bindableIndex);
				if (ImGui::Button(name.c_str())) {
					step.removeBindable(bindable);
				}
				ImGui::SameLine();

				std::string treeId = "RenderableShaderStepNode::bindable" + std::to_string(bindableIndex);
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
				else if (auto setOperation = std::dynamic_pointer_cast<SetOperation>(bindable)) {
					const char* operation = (setOperation->getOperation() == Operation::Culling)? "Culling" :
											(setOperation->getOperation() == Operation::DepthTest)? "DepthTest" :
											(setOperation->getOperation() == Operation::ScissorTest)? "ScissorTest" :
											"Blending";
					if (!ImGui::TreeNode(treeId.c_str(), "SetOperation %s", operation)) { return; }
					bool enable = setOperation->getEnable();
					if (ImGui::Checkbox("Enable", &enable)) {
						setOperation->setEnable(enable);
					}
					ImGui::TreePop();
				}
				else if (auto setDepthMask = std::dynamic_pointer_cast<SetDepthMask>(bindable)) {
					if (!ImGui::TreeNode(treeId.c_str(), "SetDepthMask")) { return; }
					bool enable = setDepthMask->getEnable();
					if (ImGui::Checkbox("Enable", &enable)) {
						setDepthMask->setEnable(enable);
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
		virtual void draw(Repository& repository, Repository::ResourceRef<RenderableShader> shader) override
		{
			Repository::ResourceRef<RenderableShaderStep> step;
			std::string name = getIdPrefix() + "RenderableShaderNode::Add";
			if (addRepoDropdownButton(name.c_str(), "Add Shader", repository, step)) {
				shader->addStep(std::move(step));
			}

			std::size_t passIndex = 0;
			shader->processSteps([&](const auto& step1) {
				std::string name1 = "x" + getIdPrefix() + "RenderableShaderNode::Remove" + std::to_string(passIndex++);
				if (ImGui::Button(name1.c_str())) {
					shader->removeStep(step1);
				}
				ImGui::SameLine();

				Repository::ResourceRef<RenderableShaderStep> step2 = step1;
				std::string name2 = getIdPrefix() + "RenderableShaderNode::Change" + std::to_string(passIndex);
				if (addRepoDropdownShowSelected(name2.c_str(), repository, step2)) {
					shader->removeStep(step1);
					shader->addStep(step2);
				}
			});
		};

		virtual bool create(Repository& repository) override
		{
			bool ret = false;

			std::string name = "Name" + getIdPrefix() + "TextureNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<RenderableShader>(mNameBuffer.data());

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validName)) {
				auto emitter = repository.insert(std::make_shared<RenderableShader>(getEditor().getEventManager()), mNameBuffer.data());
				emitter.setFakeUser();
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
		virtual void draw(Repository&, Repository::ResourceRef<Texture> texture) override
		{
			ImGui::Image(static_cast<void*>(texture.get().get()), ImVec2{ 200.0f, 200.0f });

			int textureUnit = texture->getTextureUnit();
			if (ImGui::DragInt("Texture Unit", &textureUnit, 1, 0, 16)) {
				texture->setTextureUnit(textureUnit);
			}

			TextureFilter min, mag;
			texture->getFiltering(&min, &mag);
			int iMin = static_cast<int>(min);
			int iMag = static_cast<int>(mag);

			static const char* filterTypeTags[] = { "Nearest", "Linear", "Nearest MipMap Nearest", "Linear MipMap Nearest", "Nearest MipMap Linear", "Linear MipMap Linear" };
			std::string name = getIdPrefix() + "TextureNode::MinFilter";
			if (addDropdown(name.c_str(), filterTypeTags, IM_ARRAYSIZE(filterTypeTags), iMin)) {
				min = static_cast<TextureFilter>(iMin);
				texture->setFiltering(min, mag);
			}
			std::string name1 = getIdPrefix() + "TextureNode::MagFilter";
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
			std::string name2 = getIdPrefix() + "TextureNode::WrapS";
			if (addDropdown(name2.c_str(), wrapTypeTags, IM_ARRAYSIZE(wrapTypeTags), iWrapS)) {
				wrapS = static_cast<TextureWrap>(iWrapS);
				set = true;
			}
			if (texture->getTarget() != TextureTarget::Texture1D) {
				std::string name3 = getIdPrefix() + "TextureNode::WrapT";
				if (addDropdown(name3.c_str(), wrapTypeTags, IM_ARRAYSIZE(wrapTypeTags), iWrapT)) {
					wrapT = static_cast<TextureWrap>(iWrapT);
					set = true;
				}

				if (texture->getTarget() != TextureTarget::Texture2D) {
					std::string name4 = getIdPrefix() + "TextureNode::WrapR";
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
			std::string name = "Name" + getIdPrefix() + "TextureNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<Texture>(mNameBuffer.data());

			ImGui::Checkbox(("Is HDR" + getIdPrefix() + "TextureNode::isHDR").c_str(), &mIsHDR);

			static const char* colorTypeTags[] = { "Red", "RG", "RGB", "RGBA" };
			int currentType = static_cast<int>(mColorType);
			std::string name1 = "Type" + getIdPrefix() + "TextureNode::type";
			if (addDropdown(name1.c_str(), colorTypeTags, IM_ARRAYSIZE(colorTypeTags), currentType)) {
				mColorType = static_cast<ColorFormat>(currentType);
			}

			return validName;
		};

		virtual bool load(Repository& repository, const char* path) override
		{
			auto textureSPtr = std::make_shared<Texture>(TextureTarget::Texture2D);
			textureSPtr->setTextureUnit(0);

			if (mIsHDR) {
				Image<float> image;
				auto result = ImageReader::readHDR(path, image);
				if (!result) {
					SOMBRA_ERROR_LOG << result.description();
					return false;
				}

				textureSPtr->setImage(
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

				textureSPtr->setImage(
					image.pixels.get(), TypeId::UnsignedByte, mColorType, mColorType,
					image.width, image.height
				);
			}

			auto texture = repository.insert(textureSPtr, mNameBuffer.data());
			texture.getResource().setPath(path);
			texture.setFakeUser();

			return true;
		};
	};


	class RepositoryPanel::MeshNode : public RepositoryPanel::SceneImporterTypeNode<Mesh>
	{
	public:		// Functions
		MeshNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "Mesh"; };
	protected:
		virtual void draw(Repository&, Repository::ResourceRef<Mesh> mesh) override
		{
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
		virtual void draw(Repository&, Repository::ResourceRef<ParticleEmitter> emitter) override
		{
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

			std::string name = "Name" + getIdPrefix() + "ParticleEmitterNode::name";
			ImGui::InputText(name.c_str(), mNameBuffer.data(), mNameBuffer.size());
			bool validName = !repository.findByName<ParticleEmitter>(mNameBuffer.data());

			ImGui::Separator();
			if (ImGui::Button(("Cancel" + getIdPrefix() + "CancelCreate").c_str())) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validName)) {
				auto emitter = repository.insert(std::make_shared<ParticleEmitter>(), mNameBuffer.data());
				emitter.setFakeUser();
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
		mTypes.emplace_back(new RenderableShaderStepNode(*this));
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
			if (ImGui::BeginTable(
				("RepositoryTable" + std::to_string(mPanelId)).c_str(), 2,
				ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)
			) {
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
