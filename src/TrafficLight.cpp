#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this](){ return !_queue.empty(); });
    T message = std::move(_queue.back());
     _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true) {
        if (_queue.receive() == TrafficLightPhase::green) { return; }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
[[noreturn]] void TrafficLight::cycleThroughPhases()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<unsigned> dist(4000, 6000);

    auto lastUpdate = std::chrono::system_clock::now();
    unsigned cycleLen = dist(gen);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto timeSinceLastUpdate = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastUpdate - lastUpdate ).count() >= cycleLen) {
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            lastUpdate = std::chrono::system_clock::now();
            _queue.send(std::move(_currentPhase));
            cycleLen = dist(gen);
        }
    }
}