#include <utility/threading.h>
#include <nap/core.h>

namespace nap
{

    class ControlThread
    {
    public:
        ControlThread(Core& core) : mCore(&core)
        {
            setControlRate(60.f);
        }

        ~ControlThread()
        {
            stop();
        }

        void setControlRate(float rate)
        {
            mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(rate)));
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
                mCore->update(mUpdateFunction);

                // Only sleep when there is at least 1 millisecond that needs to be compensated for
                // The actual outcome of the sleep call can vary greatly from system to system
                // And is more accurate with lower framerate limitations
                delay_time = frame_time - timer.getMicros();
                if(std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
                    std::this_thread::sleep_for(delay_time);
            }
        }

        MicroSeconds mWaitTime;
        Core* mCore = nullptr;
        std::function<void(double)> mUpdateFunction = [](double){};
        std::atomic<bool> mRunning = { false };
        TaskQueue mTaskQueue;
        std::unique_ptr<std::thread> mThread = nullptr;
    };

}