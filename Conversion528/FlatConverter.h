#pragma once

#include "Conversion.h"

namespace Conversion
{
	class FlatConverter : public ISoundConverter
	{
	public:
		virtual uint32 getInputDozeSize() const;
		virtual uint32 getOutputDozeSize() const;
		virtual ConversionFuncPointerType getConversionFunc(int nBitsPerSample);
	};
}
