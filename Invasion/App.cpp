#include "pch.h"

#include "Render/Mesh.hpp"
#include "Render/Renderer.hpp"
#include "Render/ShaderManager.hpp"
#include "Render/TextureManager.hpp"
#include "Util/XXML/Parser.hpp"

using namespace winrt;

using namespace winrt::Windows;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;

using namespace Invasion::Render;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
	void Initialize(const CoreApplicationView&) { }

	void Load(const hstring&) { }

	void SetWindow(const CoreWindow& window)
	{
		Renderer::GetInstance().Initialize(window);

		ShaderManager::GetInstance().Register(Shader::Create("default", { "Shader/Default", "Invasion" }));
		TextureManager::GetInstance().Register(Texture::Create("debug", { "Texture/Debug.dds", "Invasion"}, 
		{
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			0,
			8,
			D3D11_COMPARISON_NEVER,
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			0.0f,
			D3D11_FLOAT32_MAX
		}));

		mesh = Mesh::Create(ShaderManager::GetInstance().Get("default"), TextureManager::GetInstance().Get("debug"),
		{
			Vertex{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
			Vertex{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
			Vertex{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
			Vertex{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
		},
		{
			2, 1, 0,
			0, 3, 2
		});

		mesh->Generate();
	}

	void Update()
	{

	}

	void Render()
	{
		Renderer::GetInstance().Clear({ 0.0f, 0.45f, 0.75f, 1.0f });

		mesh->Render();

		Renderer::GetInstance().Present();
	}

	void Resize(const Vector<int, 2>& window)
	{
		Renderer::GetInstance().Resize(window);
	}

	void Uninitialize()
	{
		mesh->Uninitialize();

		ShaderManager::GetInstance().Uninitialize();
		Renderer::GetInstance().Uninitialize();
	}

	void Run()
	{
		CoreWindow window = CoreWindow::GetForCurrentThread();

		window.ResizeCompleted([&](auto&&, auto&&) { Resize({ (int)window.Bounds().Width, (int)window.Bounds().Height }); });

		LoadConfiguration(window);

		window.Activate();

		CoreDispatcher dispatcher = window.Dispatcher();
		
		isRunning = true;

		while (isRunning)
		{
			Update();
			Render();

			dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		}
	}

	void LoadConfiguration(const CoreWindow& window)
	{
		Shared<XXML::Lexer> lexer = XXML::Lexer::Create(FileSystem::ReadFile(AssetPath{ "EngineSettings.xxml", "Invasion" }.GetFullPath()));

		MutableArray<XXML::Token> tokens;

		try
		{
			tokens = lexer->Tokenize();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Lexing error: " << e.what() << "\n";
		}

		Shared<XXML::Parser> parser = XXML::Parser::Create(tokens);

		Shared<XXML::Scope> rootScope;

		try
		{
			rootScope = parser->Parse();
		}
		catch (const std::exception& e)
		{
			OutputDebugStringA(std::format("Parsing error: {}\n", e.what()).c_str());
		}

		if (!rootScope->Exists("GameID"))
			throw std::runtime_error("Invalid EngineSettings.xxml!");

		if (rootScope->Get<NarrowString>("GameID") != "0x00003E91A376E7AB")
			throw std::runtime_error("Invalid EngineSettings.xxml!");

		if (!rootScope->Exists("Invasion_Default"))
			throw std::runtime_error("Namespace 'Invasion_Default' not found! : EngineSettings.xxml");

		if (!rootScope->Exists("Invasion_Default.Version"))
			throw std::runtime_error("Variable 'Version' not found in namespace 'Invasion_Default'! : EngineSettings.xxml");

		if (!rootScope->Exists("Invasion_Default.WindowProperties"))
			throw std::runtime_error("Object 'WindowProperties' not found in namespace 'Invasion_Default'! : EngineSettings.xxml");

		if (!rootScope->Exists("Invasion_Default.WindowProperties.Title"))
			throw std::runtime_error("Variable 'Title' not found in object 'WindowProperties' within namespace 'Invasion_Default'! : EngineSettings.xxml");

		if (!rootScope->Exists("Invasion_Default.WindowProperties.Dimensions"))
			throw std::runtime_error("Variable 'Dimensions' not found in object 'WindowProperties' within namespace 'Invasion_Default'! : EngineSettings.xxml");

		auto applicationView = winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
		applicationView.Title((rootScope->Get<NarrowString>("Invasion_Default.WindowProperties.Title") + "* " + rootScope->Get<NarrowString>("Invasion_Default.Version")).operator std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>());

		MutableArray<XXML::Value> dimensions = rootScope->Get<MutableArray<XXML::Value>>("Invasion_Default.WindowProperties.Dimensions");

		winrt::Windows::Foundation::Size preferredSize((float)*std::get_if<double>(&dimensions[0].data), (float)*std::get_if<double>(&dimensions[1].data));
		
		applicationView.SetPreferredMinSize(preferredSize);
		applicationView.TryResizeView(preferredSize);

		Resize({ (int)preferredSize.Width, (int)preferredSize.Height });
	}

	IFrameworkView CreateView()
	{
		return *this;
	}

private:

	Shared<Mesh> mesh;

	bool isRunning = false;

};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
