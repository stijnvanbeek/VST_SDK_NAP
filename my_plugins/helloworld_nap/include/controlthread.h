#include <utility/threading.h>
#include <nap/core.h>

namespace nap
{

    class ControlThread : public WorkerThread
    {
    public:
        ControlThread(Core& core, std::function<void(double)> updateFunction) : WorkerThread(false), mCore(core), mUpdateFunction(updateFunction)
        {
            setControlRate(60.f);
        }

        void setControlRate(float rate)
        {
            mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(rate)));
        }

    private:
        void loop() override
        {
            HighResolutionTimer timer;
            MicroSeconds frame_time;
            MicroSeconds delay_time;
            while (isRunning())
            {
                // Get time point for next frame
                frame_time = timer.getMicros() + mWaitTime;

                // update
                mCore.update(mUpdateFunction);

                // Only sleep when there is at least 1 millisecond that needs to be compensated for
                // The actual outcome of the sleep call can vary greatly from system to system
                // And is more accurate with lower framerate limitations
                delay_time = frame_time - timer.getMicros();
                if(std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
                    std::this_thread::sleep_for(delay_time);
            }
        }

        MicroSeconds mWaitTime;
        Core& mCore;
        std::function<void(double)> mUpdateFunction = nullptr;
    };

}