#pragma once

#include "Core/Date/DateTime.h"

namespace traktor
{
	namespace editor
	{

/*! \brief Pipeline dependency flags.
 * \ingroup Editor
 */
enum PipelineDependencyFlags
{
	PdfBuild = 1,		//!< Build output from dependency.
	PdfUse = 2,			//!< Used by parent dependency; parent needs to be built if this dependency is modified.
	PdfResource = 4,	//!< Dependent as a external resource during runtime.
	PdfFailed = 8		//!< Failed dependency.
};

/*! \brief Pipeline build reasons.
 * \ingroup Editor
 */
enum PipelineBuildReason
{
	PbrNone = 0,				/*!< No reason, buildOutput shouldn't even be called. */
	PbrDependencyModified = 1,	/*!< Dependent resource has been modified. */
	PbrSourceModified = 2,		/*!< Source has been modified. */
	PbrAssetModified = 4,		/*!< Asset has been modified; only valid for Asset classes and are always combined with PbrSourceModified. */
	PbrSynthesized = 8,			/*!< Build synthesized. */
	PbrForced = 16,				/*!< Forced build issued, must build even if output is up-to-date. */
};

/*! \brief
 * \ingroup Editor
 */
struct PipelineDependencyHash
{
	uint32_t pipelineHash;
	uint32_t sourceAssetHash;
	uint32_t sourceDataHash;
	uint32_t filesHash;

	PipelineDependencyHash()
	:	pipelineHash(0)
	,	sourceAssetHash(0)
	,	sourceDataHash(0)
	,	filesHash(0)
	{
	}
};

/*! \brief
 * \ingroup Editor
 */
struct PipelineFileHash
{
	uint64_t size;
	DateTime lastWriteTime;
	uint32_t hash;

	PipelineFileHash()
	:	size(0)
	,	hash(0)
	{
	}
};

	}
}

