#include <SDL2/SDL.h>
#include <iostream>
#include "messages.pb.h"
#include <zmq.hpp>
#include <thread>

#include "telemetry_receiver.hpp"

struct RenderState {
    float x = 0.0f;        // meters
    float y = 0.0f;        // meters
    float heading = 0.0f;  // radians
    float speed = 0.0f;    // m/s (optional, debug only)
};

constexpr float METERS_TO_PIXELS = 50.0f;

SDL_FPoint worldToScreen(float x, float y) {
    return {
        400.0f + x * METERS_TO_PIXELS,
        300.0f - y * METERS_TO_PIXELS
    };
}

SDL_Texture* createCarTexture(SDL_Renderer* renderer) {
    constexpr int TEX_W = 60;
    constexpr int TEX_H = 30;

    SDL_Texture* tex = SDL_CreateTexture(
        renderer, // renderer image will go on
        SDL_PIXELFORMAT_RGBA8888, // Format how pixels are stored
        SDL_TEXTUREACCESS_TARGET, // Used for drawing
        TEX_W, // width of pixels
        TEX_H // height of pixels
    );

    // How pixels are blended with those already on screen
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    // Draws texture on backbuffer
    SDL_SetRenderTarget(renderer, tex);
    // Sets color of car
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);

    // Draws from backbuffer
    SDL_RenderClear(renderer);

    // Draw front indicator
    // Set the color
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // Draw a line from the middle to the front. Y-axis stays in the middle
    SDL_RenderDrawLine(renderer, TEX_W / 2, TEX_H / 2, TEX_W, TEX_H / 2);
    // Displays line
    SDL_SetRenderTarget(renderer, nullptr);
    return tex;
}

void renderCar(SDL_Renderer* renderer, SDL_Texture* carTexture, const RenderState& car)
{
  // Sets size of car
    constexpr float CAR_LENGTH_M = 0.90f;
    constexpr float CAR_WIDTH_M  = 0.45f;

    float w = CAR_LENGTH_M * METERS_TO_PIXELS;
    float h = CAR_WIDTH_M  * METERS_TO_PIXELS;

    // Sets position of car on screen. Initial is 0,0 which is center
    SDL_FPoint center = worldToScreen(car.x, car.y);

    // x is x coordinate top left of rectangle
    // y is y coordinate top left of rectangle
    // w is width of rectangle
    // h is height of rectangle
    SDL_FRect dst {
        center.x - w / 2.0f,
        center.y - h / 2.0f,
        w,
        h
    };

    // Rotation point of object. Defaults to center of object with this equation
    SDL_FPoint rotation_center { w / 2.0f, h / 2.0f };
    float angle_deg = -car.heading * 180.0f / M_PI;

    SDL_RenderCopyExF(
        renderer,
        carTexture, // source image data
        nullptr, // part of texture to cocpy. NULL for whole image
        &dst, // destination no screen
        angle_deg, // Degrees to rotate the image
        &rotation_center, // rotating point of image
        SDL_FLIP_NONE // Whether to flip the image. IE SDL_FLIP_HORIZONTAL
    );
}

int main() {
  std::atomic<bool> running{true};
  TelemetryState telemetry;
  std::mutex telemetry_mutex;

// Not using the thread for now
/*
  std::thread rx_thread(
      telemetryReceiverThread,
      std::ref(telemetry),
      std::ref(telemetry_mutex),
      std::ref(running)
  );
  */

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Visualizer Sim",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* carTexture = createCarTexture(renderer);

    SDL_Event e;

    zmq::context_t context(1);
    zmq::socket_t sub(context, zmq::socket_type::sub);

    sub.connect("tcp://localhost:5556");
    sub.set(zmq::sockopt::subscribe, "");

    std::cout << "[CTRL] Connected. Sending commands..." << std::endl;

    RenderState car;

    while (running.load()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        zmq::message_t msg;
        if (sub.recv(msg, zmq::recv_flags::dontwait)) {
            car::Telemetry telem;
            if (telem.ParseFromArray(msg.data(), msg.size())) {
                car.x = telem.x();
                car.y = telem.y();
                car.heading = telem.heading();
                car.speed = telem.measured_speed();
            }
        }

        // Sets background color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // Displays background color
        SDL_RenderClear(renderer);

        // Renders car using received telemetry data
        renderCar(renderer, carTexture, car);

        // Updates screen with latest
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
      }

    // rx_thread.join();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
