#pragma once
#include "interaction.h"

namespace AIR
{
template <typename T>
class Texture 
{
public:
	// Texture Interface
	virtual T Evaluate(const Interaction &) const = 0;
	virtual ~Texture() {}
};
}
