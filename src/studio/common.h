#pragma once

#include <cstdint>

namespace r5
{
	namespace v121
	{
		struct studiohdr_t;
	}

	namespace v8
	{
		struct mstudioseqdesc_t;
	}
}

extern bool g_isRigPhase;

enum class StudioLogDomain : uint8_t
{
	MDL,
	VG
};

extern StudioLogDomain g_logDomain;

class StudioLogDomainScope
{
public:
	explicit StudioLogDomainScope(StudioLogDomain newDomain);
	~StudioLogDomainScope();

private:
	StudioLogDomain m_prev;
};

extern void ConvertSurfaceProperties(const char* const pOldBVHData, char* const pNewBVHData);

extern void ConvertCollisionData_V120(const char* const pOldBVHData,
	size_t vgDataAbsoluteOffset,
	size_t bvhAbsoluteOffset);
extern void ConvertCollisionData_V120_HeadersOnly(const char* const pOldBVHData, char* const newData);

extern void CopyAnimRefData(const char* const pOldAnimRefData, char* const pNewAnimRefData, const int numlocalseq);

int GetSequenceBlendCount(const r5::v8::mstudioseqdesc_t* seqDesc);
