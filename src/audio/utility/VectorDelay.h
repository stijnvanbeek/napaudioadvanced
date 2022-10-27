#pragma once

#include <audio/utility/audiotypes.h>
#include <audio/utility/audiofunctions.h>
#include <audio/utility/vectorextension.h>
#include <mathutils.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Utility class representing a single delay that can be written and read from.
		 * Supports interpolation between samples while reading.
		 */
		template <typename real>
		class NAPAPI VectorDelay
		{
		public:
			/**
			 * The buffer size has to be a power of 2
			 */
			VectorDelay(unsigned int bufferSize)
			{
				mBuffer.resize(bufferSize, real(0.f));
				mWriteIndex = 0;
			}
			
			~VectorDelay() = default;
			
			/**
			 * Write a sample to the delay line at the current write position
			 */
			void write(const real& sample)
			{
				mBuffer[mWriteIndex++] = sample;
				mWriteIndex = wrap(mWriteIndex, mBuffer.size());
			}
			
			/**
			 * Read a sample from the delay line at @time samples behind the write position.
			 * Non interpolating.
			 */
			const real& read(unsigned int time)
			{
				// Update the read index
				int readIndex = mWriteIndex - time - 1;
				while (readIndex < 0) readIndex += mBuffer.size();

				unsigned int integerIndex = wrap(readIndex, mBuffer.size());

				return mBuffer[integerIndex];
			}
			
			/**
			 * Same as @read() but with interpolation between samples
			 */
			real readInterpolating(float sampleTime)
			{
				// Update the read index
				assert(sampleTime < mBuffer.size());
				SampleValue readIndex = mWriteIndex - sampleTime - 1;
				while (readIndex < 0) readIndex += mBuffer.size();

				unsigned int floorReadIndex = (unsigned int) readIndex;
				unsigned int integerIndex = wrap(floorReadIndex, mBuffer.size());
				unsigned int nextIntegerIndex = wrap(integerIndex + 1, mBuffer.size());

				SampleValue frac = readIndex - floorReadIndex;

				return math::lerp<real>(mBuffer[integerIndex], mBuffer[nextIntegerIndex], frac);
			}
			
			/**
			 * Clear the delay line by flushing its buffer.
			 */
			void clear()
			{
				real zero = real(0.f);
				for (unsigned int i = 0; i < mBuffer.size(); i++)
					mBuffer[i] = zero;
			}
			
			/**
			 * @return: return the maximum delay. (equalling the size of the buffer)
			 */
			unsigned int getMaxDelay() { return mBuffer.size(); }
			
			/**
			 * Operator to read from the delay line without interpolation at @index before the write position
			 */
			inline const real& operator[](unsigned int index) { return read(index); }
		
		private:
			std::vector<real> mBuffer;
			unsigned int mWriteIndex = 0;
		};
		
	}
}
