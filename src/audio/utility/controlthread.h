#pragma once

#include <utility/threading.h>
#include <nap/core.h>

namespace nap
{

	/**
	 * A control thread that can be used to perform control of audio outside the main thread.
	 * Can also be used as singleton. This is necessary when running multiple nap cores, because libpython only allows one python interpreter per process.
	 * The thread runs a loop on a specified speed.
	 */
	class ControlThread
	{
	public:
		/**
		 * Constructor
		 * @param rate initial control rate
		 */
		ControlThread(float rate = 60.f);
		~ControlThread();

		/**
		 * Set the control rate.
		 * @param rate in Hz
		 */
		void setControlRate(float rate);

		/**
		 * Starts the thread, if it is not running already.
		 */
		void start();

		/**
		 * Stops the thread, waits for the current cycle to finish before returning.
		 */
		void stop();

		/**
		 * @return wether the thread is currently running
		 */
		bool isRunning() const { return mRunning; }

		/**
		 * Connect a slot or function to a signal that is triggered every cycle of the loop.
		 * @param slot Can be anything that can be passed to Signal::connect()
		 */
		template <typename T>
		void connectPeriodicTask(T&& slot);

		/**
		 * Disconnect a slot or function to a signal that is triggered every cycle of the loop.
		 * @param slot Can be anything that can be passed to Signal::connect()
		 */
		template <typename T>
		void disconnectPeriodicTask(T&& slot);

		/**
		 * Enqueue a task to be executed once on the next cycle.
		 * @param task
		 * @param waitUntilDone if true, the method will wait for returning until the task is performed.
		 */
		void enqueue(TaskQueue::Task task, bool waitUntilDone = false);

		/**
		 * @return global instance of the control thread.
		 */
		static ControlThread& get();

	private:
		void loop();

		MicroSeconds mWaitTime;
		std::atomic<bool> mRunning = { false };
		TaskQueue mTaskQueue;
		std::unique_ptr<std::thread> mThread = nullptr;
		Signal<double> mUpdateSignal;
		std::condition_variable mCondition;
		std::mutex mMutex;
		bool mNotified = false;
	};


	template <typename T>
	void ControlThread::connectPeriodicTask(T&& slot)
	{
		auto slotPtr = &slot;
		enqueue([&, slotPtr](){ mUpdateSignal.connect(*slotPtr); });
	}


	template <typename T>
	void ControlThread::disconnectPeriodicTask(T&& slot)
	{
		auto slotPtr = &slot;
		enqueue([&, slotPtr](){ mUpdateSignal.disconnect(*slotPtr); }, true);
	}


}