#include <utility/threading.h>
#include <nap/core.h>

namespace nap
{

    class ControlThread
    {
    public:
        ControlThread(std::function<void(double)> updateFunction) : mUpdateFunction(updateFunction)
        {
            setControlRate(60.f);
        }

        void setControlRate(float rate)
        {
            mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(rate)));
        }

        void addCore(Core& core)
        {
            auto corePtr = &core;
            enqueue([&, corePtr]()
            {
                mCores.emplace(corePtr);
            });
        }

        void removeCore(Core& core)
        {
            auto corePtr = &core;
            enqueue([&, corePtr]()
            {
                mCores.erase(corePtr);
            });
        }

        void start()
        {
            if (!mRunning)
                mThread = std::make_unique<std::thread>([&](){ loop(); });
        }

        void stop()
        {
            mRunning = false;
            mThread->join();
            mThread = nullptr;
        }

        bool isRunning() const { return mRunning; }
        void enqueue(TaskQueue::Task task) { mTaskQueue.enqueue(task); }

    private:
        void loop()
        {
            HighResolutionTimer timer;
            MicroSeconds frame_time;
            MicroSeconds delay_time;
            mRunning =  true;

            while (isRunning())
            {
                mTaskQueue.process();

                // Get time point for next frame
                frame_time = timer.getMicros() + mWaitTime;

                // update
                for (auto core : mCores)
                    core->update(mUpdateFunction);

                // Only sleep when there is at least 1 millisecond that needs to be compensated for
                // The actual outcome of the sleep call can vary greatly from system to system
                // And is more accurate with lower framerate limitations
                delay_time = frame_time - timer.getMicros();
                if(std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
                    std::this_thread::sleep_for(delay_time);
            }
        }

        MicroSeconds mWaitTime;
        std::set<Core*> mCores;
        std::function<void(double)> mUpdateFunction = nullptr;
        std::atomic<bool> mRunning = { false };
        TaskQueue mTaskQueue;
        std::unique_ptr<std::thread> mThread = nullptr;
    };

}