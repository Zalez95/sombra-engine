#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <AudioFile.h>
#include <se/animation/CompositeAnimator.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Gravity.h>
#include <se/graphics/core/Program.h>
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

				mPanel.mEditor.getScene()->repository.iterate<std::string, T>([&](const std::string& key, std::shared_ptr<T>) {
					if (ImGui::Selectable(key.c_str(), key == mSelected)) {
						mSelected = key;
					}
				});
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::CollapsingHeader("Selected element")) {
				if (!mSelected.empty()) {
					std::array<char, kMaxNameSize> nameBuffer = {};
					std::copy(mSelected.begin(), mSelected.end(), nameBuffer.data());
					if (ImGui::InputText("Name##SelectedName", nameBuffer.data(), nameBuffer.size())) {
						auto element = mPanel.mEditor.getScene()->repository.find<std::string, T>(mSelected);
						mPanel.mEditor.getScene()->repository.remove<std::string, T>(mSelected);

						mSelected = nameBuffer.data();
						mPanel.mEditor.getScene()->repository.add<std::string, T>(mSelected, std::move(element));
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
		virtual void draw(const std::string& /*key*/) {};
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
			if (ImGui::BeginCombo("Type:##SceneImporter", types[currentType])) {
				for (std::size_t i = 0; i < IM_ARRAYSIZE(types); ++i) {
					bool isSelected = (i == currentType);
					if (ImGui::Selectable(types[i], isSelected)) {
						currentType = i;
						mFileType = static_cast<SceneImporter::FileType>(currentType);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
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
		virtual void draw(const std::string& key) override
		{
			auto skin = mPanel.mEditor.getScene()->repository.find<std::string, Skin>(key);
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
		virtual void draw(const std::string& key) override
		{
			auto source = mPanel.mEditor.getScene()->repository.find<std::string, LightSource>(key);

			static const char* types[] = { "Directional", "Point", "Spot" };
			std::size_t currentType = static_cast<std::size_t>(source->type);
			if (ImGui::BeginCombo("Type:##LightSource", types[currentType])) {
				for (std::size_t i = 0; i < IM_ARRAYSIZE(types); ++i) {
					bool isSelected = (i == currentType);
					if (ImGui::Selectable(types[i], isSelected)) {
						currentType = i;
						source->type = static_cast<LightSource::Type>(currentType);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
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
			bool validKey = !mPanel.mEditor.getScene()->repository.has<std::string, LightSource>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();

			if (confirmButton(validKey)) {
				mPanel.mEditor.getScene()->repository.add(std::string(mNameBuffer.data()), std::make_shared<LightSource>(LightSource::Type::Directional));
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};
	};


	class RepositoryPanel::RenderableShaderNode : public RepositoryPanel::SceneImporterTypeNode<RenderableShader>
	{
	public:		// Functions
		RenderableShaderNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "RenderableShader"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			auto renderable = mPanel.mEditor.getScene()->repository.find<std::string, RenderableShader>(key);
			renderable->getTechnique()->processPasses([&](const auto& pass) {
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Pass:");
				ImGui::SameLine();
				bool found = false;
				mPanel.mEditor.getScene()->repository.iterate<std::string, Pass>([&](const auto& key, const auto& pass2) {
					if (pass == pass2) {
						ImGui::Text("%s", key.c_str());
						found = true;
					}
				});
				if (!found) {
					ImGui::Text("[Not found]");
				}
			});
		};
	};


	class RepositoryPanel::CompositeAnimatorNode : public RepositoryPanel::SceneImporterTypeNode<CompositeAnimator>
	{
	public:		// Functions
		CompositeAnimatorNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "CompositeAnimator"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			auto animator = mPanel.mEditor.getScene()->repository.find<std::string, CompositeAnimator>(key);
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
		virtual bool options() override
		{
			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<std::string, LightSource>(mNameBuffer.data());
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

			if (!mPanel.mEditor.getScene()->repository.add(std::string(mNameBuffer.data()), buffer)) {
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
		virtual void draw(const std::string& key) override
		{
			auto force = mPanel.mEditor.getScene()->repository.find<std::string, Force>(key);
			auto gravity = std::dynamic_pointer_cast<Gravity>(force);

			const char* types[] = { "Gravity" };
			std::size_t currentType = gravity? 0 : 0;
			if (ImGui::BeginCombo("Type##ForceNode", types[currentType])) {
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
				if (!gravity) {
					gravity = std::make_shared<Gravity>();
					mPanel.mEditor.getScene()->repository.remove<std::string, Force>(key);
					mPanel.mEditor.getScene()->repository.add<std::string, Force>(key, gravity);
				}
				drawGravity(*gravity);
			}
		};

		virtual bool create() override
		{
			bool ret = false;

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<std::string, Force>(mNameBuffer.data());

			ImGui::Separator();
			if (cancelButton()) {
				mNameBuffer.fill(0);
				ret = true;
			}
			ImGui::SameLine();
			if (confirmButton(validKey)) {
				mPanel.mEditor.getScene()->repository.add<std::string, Force>(mNameBuffer.data(), std::make_shared<Gravity>());
				mNameBuffer.fill(0);
				ret = true;
			}

			return ret;
		};

		void drawGravity(Gravity& gravity)
		{
			glm::vec3 value = gravity.getValue();
			if (ImGui::DragFloat3("Gravity", glm::value_ptr(value), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f)) {
				gravity.setValue(value);
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
		virtual bool create() override
		{
			bool ret = false;
			Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validOptions = !mPanel.mEditor.getScene()->repository.has<std::string, LightSource>(mNameBuffer.data());

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
				std::shared_ptr<Program> program = ShaderLoader::createProgram(
					mPathVertex.empty()? nullptr : mPathVertex.c_str(),
					mPathGeometry.empty()? nullptr : mPathGeometry.c_str(),
					mPathFragment.empty()? nullptr : mPathFragment.c_str()
				);

				if (program && mPanel.mEditor.getScene()->repository.add(std::string(mNameBuffer.data()), program)) {
					mNameBuffer.fill(0);
					ret = true;
				}
				else {
					importErrorPopUp.show();
				}
			}

			importErrorPopUp.execute();
			return ret;
		};
	};


	/*class RepositoryPanel::PassNode : public RepositoryPanel::TypeNode<Pass>
	{
	public:		// Functions
		virtual const char* getName() const override { return "Pass"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			std::size_t bindableCount = 0;
			element->processBindables([&](const auto&) { ++bindableCount; });
			ImGui::Text("Number of bindables: %u", bindableCount);
		};

		virtual void create(std::string&) override
		{
			mShowCreate = false;
		};
	};*/


	/*class RepositoryPanel::TechniqueNode : public RepositoryPanel::TypeNode<Technique>
	{
	public:		// Functions
		virtual const char* getName() const override { return "Technique"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			std::size_t passCount = 0;
			element->processPasses([&](const auto&) { ++passCount; });
			ImGui::Text("Number of passes: %u", passCount);
			// TODO:
		};

		virtual void create(std::string&) override
		{
			mShowCreate = false;
		};
	};*/


	class RepositoryPanel::TextureNode : public RepositoryPanel::ImportTypeNode<Texture>
	{
	private:	// Attributes
		std::array<char, kMaxNameSize> mNameBuffer;
		ColorFormat mColorType = ColorFormat::RGB;

	public:		// Functions
		TextureNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "Texture"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			auto texture = mPanel.mEditor.getScene()->repository.find<std::string, Texture>(key);
			ImGui::Image(static_cast<void*>(texture.get()), ImVec2{ 200.0f, 200.0f });
		};

		virtual bool options() override
		{
			ImGui::InputText("Name##CreateName", mNameBuffer.data(), mNameBuffer.size());
			bool validKey = !mPanel.mEditor.getScene()->repository.has<std::string, LightSource>(mNameBuffer.data());

			static const char* colorType[] = { "Red", "RG", "RGB", "RGBA" };
			std::size_t currentType = static_cast<std::size_t>(mColorType);
			if (ImGui::BeginCombo("Type:##Texture", colorType[currentType])) {
				for (std::size_t i = 0; i < IM_ARRAYSIZE(colorType); ++i) {
					bool isSelected = (i == currentType);
					if (ImGui::Selectable(colorType[i], isSelected)) {
						currentType = i;
						mColorType = static_cast<ColorFormat>(currentType);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			return validKey;
		};

		virtual bool load(const char* path) override
		{
			Image<unsigned char> image;
			auto result = ImageReader::read(path, image);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return false;
			}

			auto texture = std::make_shared<Texture>(TextureTarget::Texture2D);
			texture->setImage(
				image.pixels.get(), TypeId::UnsignedByte, mColorType, mColorType,
				image.width, image.height
			);
			if (!mPanel.mEditor.getScene()->repository.add(std::string(mNameBuffer.data()), texture)) {
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
		virtual void draw(const std::string& key) override
		{
			auto mesh = mPanel.mEditor.getScene()->repository.find<std::string, Mesh>(key);
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
		mTypes.emplace_back(new RenderableShaderNode(*this));
		mTypes.emplace_back(new CompositeAnimatorNode(*this));
		mTypes.emplace_back(new AudioBufferNode(*this));
		mTypes.emplace_back(new ForceNode(*this));
		mTypes.emplace_back(new ProgramNode(*this));
		/*mTypes.emplace_back(new PassNode(*this));
		mTypes.emplace_back(new TechniqueNode(*this));*/
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
