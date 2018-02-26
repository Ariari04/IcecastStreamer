#pragma once

class IProgressManager
{
public:
	virtual void reportProgress(float fProgress) = 0;
	virtual ~IProgressManager() {};

	void reportProgress(size_t nBytesReaded, size_t nTotalBytes)
	{
		reportProgress(float(nBytesReaded)/float(nTotalBytes));
	}
};
