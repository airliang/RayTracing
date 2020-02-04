#pragma once
#include <mutex>
#include <atomic>

namespace AIR
{
	class AtomicFloat
	{
	public:
		// AtomicFloat Public Methods
		explicit AtomicFloat(Float v = 0) 
		{ 
			bits = FloatToBits(v); 
		}
		operator Float() const 
		{ 
			return BitsToFloat(bits); 
		}
		Float operator=(Float v) 
		{
			bits = FloatToBits(v);
			return v;
		}
		void Add(Float v) 
		{
			uint32_t oldBits = bits, newBits;
			do 
			{
				newBits = FloatToBits(BitsToFloat(oldBits) + v);
			} while (!bits.compare_exchange_weak(oldBits, newBits));
		}

	private:
		std::atomic<uint32_t> bits;

	};
}
