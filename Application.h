#pragma once

#include <memory>

struct ApplicationData;

class Application {
public:
    Application(int windowWidth, int windowHeight);
    Application(const Application&);
    Application(Application&&) noexcept;
    Application& operator=(const Application&);
    Application& operator=(Application&&) noexcept;
    ~Application();

    void Startup();
    void Run();
    void Shutdown();

private:
    std::unique_ptr<ApplicationData> data;
};
