#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <AudioFile.h>
#include <se/animation/SkeletonAnimator.h>
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
		virtual void draw(const std::string& key) override
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
		virtual void draw(const std::string& key) override
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


	class RepositoryPanel::RenderableShaderNode : public RepositoryPanel::SceneImporterTypeNode<RenderableShader>
	{
	public:		// Functions
		RenderableShaderNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "RenderableShader"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			auto renderable = mPanel.mEditor.getScene()->repository.find<Scene::Key, RenderableShader>(key);
			renderable->getTechnique()->processPasses([&](const auto& pass) {
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Pass:");
				ImGui::SameLine();

				std::string key2;
				if (mPanel.mEditor.getScene()->repository.findKey<Scene::Key, Pass>(pass, key2)) {
					ImGui::Text("%s", key2.c_str());
				}
				else {
					ImGui::Text("[Not found]");
				}
			});
		};
	};


	class RepositoryPanel::SkeletonAnimatorNode : public RepositoryPanel::SceneImporterTypeNode<SkeletonAnimator>
	{
	public:		// Functions
		SkeletonAnimatorNode(RepositoryPanel& panel) : SceneImporterTypeNode(panel) {};
		virtual const char* getName() const override { return "SkeletonAnimator"; };
	protected:
		virtual void draw(const std::string& key) override
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
		virtual void draw(const std::string& key) override
		{
			auto force = mPanel.mEditor.getScene()->repository.find<Scene::Key, Force>(key);
			auto gravity = std::dynamic_pointer_cast<Gravity>(force);

			const char* types[] = { "Gravity" };
			std::size_t currentType = gravity? 0 : 0;
			addDropdown("Type##ForceNode", types, IM_ARRAYSIZE(types), currentType);

			if (currentType == 0) {
				if (!gravity) {
					gravity = std::make_shared<Gravity>();
					mPanel.mEditor.getScene()->repository.remove<Scene::Key, Force>(key);
					mPanel.mEditor.getScene()->repository.add<Scene::Key, Force>(key, gravity);
				}
				drawGravity(*gravity);
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
		bool mIsHDR = false;

	public:		// Functions
		TextureNode(RepositoryPanel& panel) : ImportTypeNode(panel), mNameBuffer{} {};
		virtual const char* getName() const override { return "Texture"; };
	protected:
		virtual void draw(const std::string& key) override
		{
			auto texture = mPanel.mEditor.getScene()->repository.find<Scene::Key, Texture>(key);
			ImGui::Image(static_cast<void*>(texture.get()), ImVec2{ 200.0f, 200.0f });

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
		virtual void draw(const std::string& key) override
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
		mTypes.emplace_back(new RenderableShaderNode(*this));
		mTypes.emplace_back(new SkeletonAnimatorNode(*this));
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
