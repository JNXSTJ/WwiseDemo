//***************************************************************************************
// SkinnedMeshApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <iostream>
#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/Camera.h"
#include "Common/FrameResource.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include "Common/SkinnedData.h"
#include "Common/LoadM3d.h"
#include "SkinnedMeshApp.h"
#include <tracy/Tracy.hpp>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#include <DirectXMath.h>
#include <iostream>
#include <vector>
#include <pybind11/embed.h>
#include <shared_mutex>
#include <time.h>
#include "audio.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX::PackedVector;
using namespace DirectX;


namespace py = pybind11;

// 游戏对象基类
class GameObject {
public:
    virtual void update(float dt) = 0;
    virtual void on_key_event(int key) = 0;
    virtual ~GameObject() = default;

    float x = 0;
    float y = 0;
};

// 修改后（正确）
PYBIND11_EMBEDDED_MODULE(gameengine, m) {
    py::class_<GameObject>(m, "GameObject")
        .def_readwrite("x", &GameObject::x)  // 使用def_readwrite暴露公共成员变量
        .def_readwrite("y", &GameObject::y);
}

// Python脚本管理的游戏对象
class ScriptedObject : public GameObject {
public:
    ScriptedObject(py::object instance)
        : instance(instance) {
    }

    void update(float dt) override {
        try {
            instance.attr("update")(dt);
        }
        catch (py::error_already_set& e) {
            std::cerr << "Python error in update: " << e.what() << "\n";
        }
    }

    void on_key_event(int key) override {
        try {
            instance.attr("on_key_event")(key);
        }
        catch (py::error_already_set& e) {
            std::cerr << "Python error in on_key_event: " << e.what() << "\n";
        }
    }

private:
    py::object instance;
};

class GameEngine {
public:
    GameEngine() {
        // 初始化Python解释器
        py::initialize_interpreter();

        // 添加当前目录到Python路径
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("insert")(0, ".");
    }

    ~GameEngine() {
        py::finalize_interpreter();
    }

    void load_script(const std::string& path) {
        try {
            auto module = py::module_::import(path.c_str());
            auto create_func = module.attr("create_object");

            // 创建Python对象并包装为C++对象
            auto py_obj = create_func();
            objects.emplace_back(std::make_unique<ScriptedObject>(py_obj));
        }
        catch (py::error_already_set& e) {
            std::cerr << "Failed to load script: " << e.what() << "\n";
        }
    }

    void run() {
        float dt = 0.016f; // 假设60帧
        while (running) {
            process_input();
            update(dt);
            render();
            ::_Thrd_sleep_for(1000);
        }
    }

private:
    void process_input() {
        // 模拟按键事件（这里假设检测到按键1）
        int pressed_key = 1;
        for (auto& obj : objects) {
            obj->on_key_event(pressed_key);
        }
    }

    void update(float dt) {
        for (auto& obj : objects) {
            obj->update(dt);
        }
    }

    void render() {
        std::cout << "Rendering...\n";
        for (auto& obj : objects) {
            std::cout << "Object at (" << obj->x << ", " << obj->y << ")\n";
        }
        std::cout << std::endl;
    }

    std::vector<std::unique_ptr<GameObject>> objects;
    bool running = true;
};

void PrintWorkdir() {
    // 定义一个缓冲区来存储当前工作路径
    wchar_t buffer[MAX_PATH];

    // 获取当前工作路径
    DWORD dwRet = GetCurrentDirectory(MAX_PATH, buffer);

    // 检查是否成功获取路径
    if (dwRet == 0) {
        // 获取失败，显示错误消息
        MessageBox(NULL, L"Failed to get current directory.", L"Error", MB_OK | MB_ICONERROR);
        return; // 退出程序
    }


    // 显示当前工作路径
    MessageBox(NULL, buffer, L"Current Directory", MB_OK | MB_ICONINFORMATION);

    // ... 其他代码 ...

    return; // 正常退出程序
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//myengine::assimp::Model ElysiaBody("C:/Users/taojian/Desktop/wwise_demo/asset/rain_restaurant.obj");
    //PrintWorkdir();


	myengine::audio::Audio& instance = myengine::audio::Audio::Instance();
    instance.LoadBnk("Init.bnk");
	instance.LoadBnk("Reflect.bnk");

    std::ofstream out("output.txt");
    auto old_buf = std::cout.rdbuf(out.rdbuf()); // 保存并重定向
    std::cout << "test" << std::endl;

    //GameEngine engine;

    //engine.load_script("demo_script"); // 加载demo_script.py
    //engine.run();

    try
    {
        SkinnedMeshApp theApp(hInstance);
        if (!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}




