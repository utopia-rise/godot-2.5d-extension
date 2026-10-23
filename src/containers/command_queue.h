#ifndef ISOMETRIC_MAPS_COMMAND_QUEUE_H
#define ISOMETRIC_MAPS_COMMAND_QUEUE_H

#include <condition_variable>
#include <core/memory.hpp>
#include <cstdint>
#include <mutex>
#include <templates/local_vector.hpp>
#include <tuple>
#include <utility>

namespace containers {
    // Multi-producer, single-consumer queue of method calls, the extension-side counterpart of the engine's
    // CommandQueueMT: producers push calls, the consumer thread runs them in order.
    class CommandQueue {
        struct Command {
            virtual ~Command() = default;
            virtual void call() = 0;
        };

        template<class T, class M, class... Args>
        struct MethodCommand : public Command {
            T* instance;
            M method;
            std::tuple<Args...> args;

            MethodCommand(T* p_instance, M p_method, Args... p_args) :
              instance(p_instance),
              method(p_method),
              args(std::move(p_args)...) {}

            void call() override {
                std::apply([this](auto&... p_args) { (instance->*method)(p_args...); }, args);
            }
        };

        std::mutex mutex;
        std::condition_variable pushed;
        std::condition_variable flushed;
        godot::LocalVector<Command*> pending;
        godot::LocalVector<Command*> running;
        uint64_t pushed_count = 0;
        uint64_t flushed_count = 0;
        bool consumer_running = false;

        uint64_t enqueue(Command* p_command) {
            uint64_t ticket;
            {
                std::lock_guard<std::mutex> lock(mutex);
                pending.push_back(p_command);
                ticket = ++pushed_count;
            }
            pushed.notify_one();
            return ticket;
        }

    public:
        template<class T, class M, class... Args>
        void push(T* p_instance, M p_method, Args... p_args) {
            if (!consumer_running) {
                (p_instance->*p_method)(p_args...);
                return;
            }
            enqueue(memnew((MethodCommand<T, M, Args...>) (p_instance, p_method, p_args...)));
        }

        // Blocks the calling thread until the consumer ran this call and every call pushed before it.
        template<class T, class M, class... Args>
        void push_and_sync(T* p_instance, M p_method, Args... p_args) {
            if (!consumer_running) {
                (p_instance->*p_method)(p_args...);
                return;
            }
            uint64_t ticket = enqueue(memnew((MethodCommand<T, M, Args...>) (p_instance, p_method, p_args...)));
            std::unique_lock<std::mutex> lock(mutex);
            flushed.wait(lock, [this, ticket] { return flushed_count >= ticket; });
        }

        // Consumer side: waits for at least one call, then runs every pending call.
        void wait_and_flush() {
            {
                std::unique_lock<std::mutex> lock(mutex);
                pushed.wait(lock, [this] { return !pending.is_empty(); });
                running = std::move(pending);
                pending = godot::LocalVector<Command*>();
            }

            for (Command* command : running) {
                command->call();
                godot::memdelete(command);
            }
            uint64_t ran = running.size();
            running.clear();

            {
                std::lock_guard<std::mutex> lock(mutex);
                flushed_count += ran;
            }
            flushed.notify_all();
        }

        // Calls run on the producer's thread until a consumer starts, and again once it has exited.
        void start_consumer() { consumer_running = true; }

        void stop_consumer() {
            consumer_running = false;
            for (Command* command : pending) {
                command->call();
                godot::memdelete(command);
            }
            pending.clear();
        }

        ~CommandQueue() {
            for (Command* command : pending) {
                godot::memdelete(command);
            }
        }
    };
}// namespace containers

#endif// ISOMETRIC_MAPS_COMMAND_QUEUE_H
