#include "Application.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <chrono>
#include <list>
#include <stdexcept>

struct Entity {
    float x = 0;
    float y = 0;
    float dx = 0;
    float dy = 0;
    float width = 0;
    float height = 0;
    SDL_Texture* texture = nullptr;
    unsigned char red = 0;
    unsigned char green = 0;
    unsigned char blue = 0;
    unsigned char alpha = 0;
};

struct Player : Entity {
    int health = 0;
    int reload = 0;

    Player() {
        width = 100;
        height = 50;
        red = 0xFF;
        green = 0xFF;
        blue = 0xFF;
        alpha = 0xFF;
    }
};

struct Bullet : Entity {
    Bullet() {
        width = 10;
        height = 10;
        red = 0xFF;
        alpha = 0xFF;
    }
};

struct ApplicationData {
    SDL_Window* window = nullptr;
    int windowWidth;
    int windowHeight;
    bool windowResized = false;
    bool windowMinimized = false;
    bool windowClosed = false;
    SDL_Renderer* renderer = nullptr;
    Player player;
    std::list<Bullet> bullets;
    bool down_ = false;
    bool up_ = false;
    bool left_ = false;
    bool right_ = false;
    bool fire_ = false;

    ApplicationData(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight) {}
};

static void ProcessInput(ApplicationData* data);
static void Update(ApplicationData* data);
static void Render(ApplicationData* data);

Application::Application(int windowWidth, int windowHeight) : data(std::make_unique<ApplicationData>(windowWidth, windowHeight)) {
    data->windowWidth = windowWidth;
    data->windowHeight = windowHeight;
}

Application::Application(const Application& other) : data(std::make_unique<ApplicationData>(*other.data)) {}

Application::Application(Application&& other) noexcept = default;

Application& Application::operator=(const Application & other) {
    *data = *other.data;
    return *this;
}

Application& Application::operator=(Application&&) noexcept = default;

Application::~Application() = default;

void Application::Startup() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    data->window = SDL_CreateWindow("Simple Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, data->windowWidth, data->windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (data->window == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GetWindowSize(data->window, &data->windowWidth, &data->windowHeight);

    data->renderer = SDL_CreateRenderer(data->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (data->renderer == NULL) {
        throw std::runtime_error(SDL_GetError());
    }

    data->player.texture = IMG_LoadTexture(data->renderer, "images/player.png");
    int width;
    int height;
    SDL_QueryTexture(data->player.texture, NULL, NULL, &width, &height);
    data->player.width = 128;
    data->player.height = 128;
    data->player.x = 100;
    data->player.y = (data->windowHeight - data->player.height) / 2;
}

void Application::Run() {
    //std::chrono::high_resolution_clock::time_point current_time;
    //std::chrono::high_resolution_clock::time_point previous_time;
    //long long accumulated_time = 0;

    //current_time = std::chrono::high_resolution_clock::now();

    while (!data->windowClosed) {
        //previous_time = current_time;
        //current_time = std::chrono::high_resolution_clock::now();
        //accumulated_time += std::chrono::duration_cast<std::chrono::microseconds>(current_time - previous_time).count();

        ProcessInput(data.get());

        Update(data.get());

        if (!data->windowMinimized) {
            Render(data.get());
        }
    }
}

void Application::Shutdown() {
    SDL_DestroyWindow(data->window);
    SDL_Quit();
}

static void ProcessKey(ApplicationData * data, SDL_KeyboardEvent * event, bool keyDown) {
    if (event->repeat == 0) {
        switch (event->keysym.scancode) {
        case SDL_SCANCODE_ESCAPE:
            if (keyDown) {
                data->windowClosed = true;
            }
            break;
        case SDL_SCANCODE_UP:
            data->up_ = keyDown;
            break;
        case SDL_SCANCODE_DOWN:
            data->down_ = keyDown;
            break;
        case SDL_SCANCODE_LEFT:
            data->left_ = keyDown;
            break;
        case SDL_SCANCODE_RIGHT:
            data->right_ = keyDown;
            break;
        case SDL_SCANCODE_LCTRL:
            data->fire_ = keyDown;
            break;
        }
    }
}

static void ProcessInput(ApplicationData * data) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            data->windowClosed = true;
            break;
        case SDL_KEYDOWN:
            ProcessKey(data, &event.key, true);
            break;
        case SDL_KEYUP:
            ProcessKey(data, &event.key, false);
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                data->windowResized = true;
                SDL_GetWindowSize(data->window, &data->windowWidth, &data->windowHeight);
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                data->windowMinimized = true;
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
                data->windowMinimized = false;
                break;
            }
            break;
        }
    }
}

static void Update(ApplicationData * data) {
    data->player.dx = 0;
    data->player.dy = 0;

    if (data->left_) {
        data->player.dx += -4;
    }

    if (data->right_) {
        data->player.dx += 4;
    }

    if (data->up_) {
        data->player.dy += -5;
    }

    if (data->down_) {
        data->player.dy += 5;
    }

    data->player.x += data->player.dx;
    data->player.y += data->player.dy;

    if (data->player.x + data->player.width > data->windowWidth) {
        data->player.x = data->windowWidth - data->player.width;
    } else if (data->player.x < 0) {
        data->player.x = 0;
    }

    if (data->player.y + data->player.height > data->windowHeight) {
        data->player.y = data->windowHeight - data->player.height;
    } else if (data->player.y < 0) {
        data->player.y = 0;
    }

    if (data->player.reload > 0) {
        data->player.reload--;
    }

    if (data->fire_ && data->player.reload == 0) {
        Bullet bullet;
        bullet.x = data->player.x + data->player.width - bullet.width;
        bullet.y = data->player.y + (data->player.height / 2) - (bullet.height / 2);
        bullet.dx = bullet.width;
        bullet.dy = 0;
        data->bullets.push_back(bullet);
        data->player.reload = 8;
    }

    for (auto bullet = data->bullets.begin(); bullet != data->bullets.end(); ) {
        bullet->x += bullet->dx;
        bullet->y += bullet->dy;

        if (bullet->x > data->windowWidth) {
            bullet = data->bullets.erase(bullet);
        } else {
            bullet++;
        }
    }
}

static void RenderEntity(SDL_Renderer * renderer, Entity & entity) {
    SDL_Rect dest = {static_cast<int>(entity.x), static_cast<int>(entity.y), static_cast<int>(entity.width), static_cast<int>(entity.height)};

    if (entity.texture) {
        SDL_RenderCopy(renderer, entity.texture, NULL, &dest);
    } else {
        SDL_SetRenderDrawColor(renderer, entity.red, entity.green, entity.blue, entity.alpha);
        SDL_RenderFillRect(renderer, &dest);
    }
}

static void Render(ApplicationData * data) {
    SDL_SetRenderDrawColor(data->renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(data->renderer);

    RenderEntity(data->renderer, data->player);

    for (auto bullet : data->bullets) {
        RenderEntity(data->renderer, bullet);
    }

    SDL_RenderPresent(data->renderer);
}
