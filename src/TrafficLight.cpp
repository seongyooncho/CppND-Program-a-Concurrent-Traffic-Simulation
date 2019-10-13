#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this]() { return !_messages.empty(); });
    T msg = std::move(_messages.back());
    _messages.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _messages.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true)
    {
        if (_queue->receive() == TrafficLightPhase::green)
            return;
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
void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);
    int delay = distr(eng);
    auto last_updated = std::chrono::system_clock::now();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_updated).count() > delay)
        {
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            delay = distr(eng);
            last_updated = std::chrono::system_clock::now();
            _queue->send(std::move(_currentPhase));
        }
    }
}

