#include <numeric>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Functions/BlendFunction.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/AstcCompressor.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"
#include "Render/SH/SHCoeffs.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IRayTracer.h"
#include "Shape/Editor/Bake/TracerEnvironment.h"
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/IrradianceGridResource.h"

#if !defined(__RPI__)
#	include <OpenImageDenoise/oidn.h>
#endif

namespace traktor
{
	namespace shape
	{
		namespace
		{

Ref< drawing::Image > denoise(const GBuffer& gbuffer, drawing::Image* lightmap)
{
#if !defined(__RPI__)
	int32_t width = lightmap->getWidth();
	int32_t height = lightmap->getHeight();

	lightmap->convert(drawing::PixelFormat::getRGBAF32());

	drawing::Image albedo(
		drawing::PixelFormat::getRGBAF32(),
		width,
		height
	);
	albedo.clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	drawing::Image normals(
		drawing::PixelFormat::getRGBAF32(),
		width,
		height
	);
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			const auto elm = gbuffer.get(x, y);
			normals.setPixelUnsafe(x, y, Color4f(elm.normal));
		}
	}

	Ref< drawing::Image > output = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	output->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
	oidnCommitDevice(device);

	OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filter
	oidnSetSharedFilterImage(filter, "color",  lightmap->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetSharedFilterImage(filter, "albedo", albedo.getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "normal", normals.getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "output", output->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetFilter1b(filter, "hdr", true); // image is HDR
	oidnCommitFilter(filter);

	oidnExecuteFilter(filter);	

	// Check for errors
	const char* errorMessage;
	if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
		log::error << mbstows(errorMessage) << Endl;

	// Cleanup
	oidnReleaseFilter(filter);
	oidnReleaseDevice(device);	
	return output;
#else
	return lightmap;
#endif
}

void encodeRGBM(drawing::Image* image)
{
	const Scalar c_rangeDenom = 1.0_simd / 8.0_simd;
	const Scalar c_epsilon= 1e-6_simd;

	Color4f cl;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, cl);
			cl *= c_rangeDenom;

			float a = clamp< float >( max( max( cl.getRed(), cl.getGreen() ), cl.getBlue() ), 0.0f, 1.0f );
			if (a > FUZZY_EPSILON)
			{
				a = std::ceil(a * 255.0f) / 255.0f;
				cl /= Scalar(a);
				cl.setAlpha(Scalar(a));
			}
			else
				cl.set(0.0f, 0.0f, 0.0f, 0.0f);

			image->setPixelUnsafe(x, y, cl);
		}
	}
}

bool writeTexture(
	db::Database* outputDatabase,
	const std::wstring& compressionMethod,
	const Guid& lightmapId,
	const std::wstring& name,
	const drawing::Image* lightmap
)
{
	render::TextureFormat textureFormat = render::TfInvalid;
	Ref< drawing::Image > lightmapFormat = lightmap->clone();
	Ref< render::ICompressor > compressor;
	bool needAlpha;

	// Convert image to match texture format.
	if (compareIgnoreCase(compressionMethod, L"DXTn") == 0)
	{
		encodeRGBM(lightmapFormat);
		lightmapFormat->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		//textureFormat = render::TfDXT5;
		//compressor = new render::DxtnCompressor();
		textureFormat = render::TfR8G8B8A8;
		compressor = new render::UnCompressor();
		needAlpha = true;
	}
	else if (compareIgnoreCase(compressionMethod, L"ASTC") == 0)
	{
		encodeRGBM(lightmapFormat);
		lightmapFormat->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		textureFormat = render::TfASTC4x4;
		compressor = new render::AstcCompressor();
		needAlpha = true;
	}
	else
	{
		encodeRGBM(lightmapFormat);
		lightmapFormat->convert(drawing::PixelFormat::getABGRF16().endianSwapped());
		textureFormat = render::TfR8G8B8A8;
		compressor = new render::UnCompressor();
		needAlpha = true;
	}

	Ref< db::Instance > outputInstance = outputDatabase->createInstance(
		L"Generated/" + lightmapId.format(),
		db::CifReplaceExisting,
		&lightmapId
	);
	if (!outputInstance)
		return false;

	Ref< render::TextureResource > outputResource = new render::TextureResource();
	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		outputInstance->revert();
		return false;
	}

	Writer writer(stream);

	// Write texture resource header.
	writer << uint32_t(12);
	writer << int32_t(lightmapFormat->getWidth());
	writer << int32_t(lightmapFormat->getHeight());
	writer << int32_t(1);
	writer << int32_t(1);
	writer << int32_t(textureFormat);
	writer << bool(false);
	writer << uint8_t(render::Tt2D);
	writer << bool(false);
	writer << bool(false);

	// Write texture data.
	RefArray< drawing::Image > mipImages(1);
	mipImages[0] = lightmapFormat;
	compressor->compress(writer, mipImages, textureFormat, needAlpha, 3);

	stream->close();

	if (!outputInstance->commit())
		return false;
	
	return true;
}

void line(const Vector2& from, const Vector2& to, const std::function< void(const Vector2, float) >& fn)
{
	Vector2 ad = (to - from);
	ad.x = std::abs(ad.x);
	ad.y = std::abs(ad.y);
	int32_t ln = (int32_t)(std::max(ad.x, ad.y) + 0.5f);
	for (int32_t i = 0; i <= ln; ++i)
	{
		float fraction = (float)i / ln;
		Vector2 position = lerp(from, to, fraction);
		fn(position, fraction);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerProcessor", TracerProcessor, Object)

TracerProcessor::TracerProcessor(const TypeInfo* rayTracerType, db::Database* outputDatabase, const std::wstring& compressionMethod, bool parallel)
:   m_rayTracerType(rayTracerType)
,   m_outputDatabase(outputDatabase)
,	m_compressionMethod(compressionMethod)
,	m_parallel(parallel)
,   m_thread(nullptr)
{
	T_FATAL_ASSERT(m_outputDatabase != nullptr);
	T_FATAL_ASSERT(m_rayTracerType != nullptr);

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &TracerProcessor::processorThread), L"Tracer");
	m_thread->start();
}

TracerProcessor::~TracerProcessor()
{
	// Ensure all tasks has finished until we stop thread.
	waitUntilIdle();

	// Stop task thread.
	if (m_thread != nullptr)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = nullptr;
	}
}

void TracerProcessor::enqueue(const TracerTask* task)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	cancel(task->getSceneId());

	// Add our task and issue processing thread.
	m_tasks.push_back(task);
	m_event.broadcast();
}

void TracerProcessor::cancel(const Guid& sceneId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	// Remove any pending task which reference the same scene.
	auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TracerTask* hs) {
		return hs->getSceneId() == sceneId;
	});
	if (it != m_tasks.end())
		m_tasks.erase(it);

	// Check if currently processing task is same scene.
	if (m_activeTask != nullptr && m_activeTask->getSceneId() == sceneId)
	{
		// Currently processing same scene, abort and restart.
		m_cancelled = true;
	}
}

void TracerProcessor::cancelAll()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_tasks.clear();
	m_cancelled = true;
}

void TracerProcessor::waitUntilIdle()
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (!m_tasks.empty() || m_activeTask != nullptr)
		thread->yield();
}

TracerProcessor::Status TracerProcessor::getStatus() const
{
	return m_status;
}

void TracerProcessor::processorThread()
{
	while (!m_thread->stopped())
	{
		if (!m_event.wait(100))
			continue;

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (!m_tasks.empty())
			{
				m_activeTask = m_tasks.front();
				m_tasks.pop_front();
			}
		}

		if (m_activeTask)
		{
			m_status.active = true;
			m_cancelled = false;
			process(m_activeTask);
			m_status.active = false;
			m_activeTask = nullptr;
		}
	}
	cancelAll();
}

bool TracerProcessor::process(const TracerTask* task)
{
	auto configuration = task->getConfiguration();
	T_FATAL_ASSERT(configuration != nullptr);

	// Update status.
	m_status.description = L"Preparing...";

   	// Create raytracer implementation.
	Ref< IRayTracer > rayTracer = checked_type_cast< IRayTracer* >(m_rayTracerType->createInstance());
	if (!rayTracer->create(configuration))
		return false;

	// Setup raytracer scene.
	for (auto tracerEnvironment : task->getTracerEnvironments())
		rayTracer->addEnvironment(tracerEnvironment->getEnvironment());
	for (auto tracerLight : task->getTracerLights())
		rayTracer->addLight(tracerLight->getLight());
	for (auto tracerModel : task->getTracerModels())
		rayTracer->addModel(tracerModel->getModel(), tracerModel->getTransform());

	rayTracer->commit();

	// Get output tasks and sort them by priority.
	auto tracerOutputs = task->getTracerOutputs();
	tracerOutputs.sort([](const TracerOutput* lh, const TracerOutput* rh) {
		return lh->getPriority() > rh->getPriority();
	});

	// Calculate total progress.
	m_status.total = std::accumulate(tracerOutputs.begin(), tracerOutputs.end(), (int32_t)0, [](int32_t acc, const TracerOutput* iter) {
		return acc + (iter->getLightmapSize() / 16) * (iter->getLightmapSize() / 16);
	});
	m_status.current = 0;

	// Trace each lightmap in task.
	for (uint32_t i = 0; !m_cancelled && i < tracerOutputs.size(); ++i)
	{
		auto tracerOutput = tracerOutputs[i];
		auto renderModel = tracerOutput->getModel();
		T_FATAL_ASSERT(renderModel != nullptr);

		const int32_t width = tracerOutput->getLightmapSize();
		const int32_t height = width;
		const uint32_t channel = renderModel->getTexCoordChannel(L"Lightmap");

		// Update status.
		m_status.description = tracerOutput->getName() + L"...";

		// Create GBuffer of mesh's geometry.
		GBuffer gbuffer;
		gbuffer.create(width, height, *renderModel, tracerOutput->getTransform(), channel);

		// Preprocess GBuffer.
		rayTracer->preprocess(&gbuffer);

		// Trace lightmap.
		Ref< drawing::Image > lightmap = new drawing::Image(
			drawing::PixelFormat::getRGBAF32(),
			width,
			height
		);
		lightmap->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		// Only use jobs when building targets.
		if (m_parallel)
		{
			RefArray< Job > jobs;
			for (int32_t ty = 0; !m_cancelled && ty < height; ty += 16)
			{
				Ref< Job > job = JobManager::getInstance().add(makeFunctor([&, ty](){
					for (int32_t tx = 0; tx < width; tx += 16)
					{
						int32_t region[] = { tx, ty, tx + 16, ty + 16 };
						rayTracer->traceLightmap(renderModel, &gbuffer, lightmap, region);
						++m_status.current;
					}
				}));
				jobs.push_back(job);
			}
			while (!jobs.empty())
			{
				jobs.back()->wait();
				jobs.pop_back();
			}
		}
		else
		{
			for (int32_t ty = 0; !m_cancelled && ty < height; ty += 16)
			{
				for (int32_t tx = 0; !m_cancelled && tx < width; tx += 16)
				{
					int32_t region[] = { tx, ty, tx + 16, ty + 16 };
					rayTracer->traceLightmap(renderModel, &gbuffer, lightmap, region);
					++m_status.current;
				}
			}
		}
		if (m_cancelled)
			break;

		// Blur lightmap to reduce noise from path tracing.
		if (configuration->getEnableDenoise())
			lightmap = denoise(gbuffer, lightmap);

		// Filter seams.
		if (configuration->getEnableSeamFilter())
		{
			model::ModelAdjacency adjacency(renderModel, model::ModelAdjacency::MdByPosition);
			for (uint32_t i = 0; i < adjacency.getEdgeCount(); ++i)
			{
				// Get shared edges of this polygon's edge.
				model::ModelAdjacency::share_vector_t shared;
				adjacency.getSharedEdges(i, shared);
				if (shared.size() != 1)
					continue;

				// Get attributes of this edge.
				const model::Polygon& polygonA = renderModel->getPolygon(adjacency.getPolygon(i));
				uint32_t Aivx0 = polygonA.getVertex(adjacency.getPolygonEdge(i));
				uint32_t Aivx1 = polygonA.getVertex((Aivx0 + 1) % polygonA.getVertexCount());

				// Get attributes of shared edge.
				const model::Polygon& polygonB = renderModel->getPolygon(adjacency.getPolygon(shared[0]));
				uint32_t Bivx0 = polygonB.getVertex(adjacency.getPolygonEdge(shared[0]));
				uint32_t Bivx1 = polygonB.getVertex((Bivx0 + 1) % polygonB.getVertexCount());

				model::Vertex Avx0 = renderModel->getVertex(Aivx0);
				model::Vertex Avx1 = renderModel->getVertex(Aivx1);
				model::Vertex Bvx0 = renderModel->getVertex(Bivx0);
				model::Vertex Bvx1 = renderModel->getVertex(Bivx1);

				// Swap indices if order is reversed.
				if (Bvx0.getPosition() == Avx1.getPosition())
				{
					std::swap(Bivx0, Bivx1);
					std::swap(Bvx0, Bvx1);
				}

				// Check for lightmap seam.
				if (
					Avx0.getPosition() == Bvx0.getPosition() &&
					Avx1.getPosition() == Bvx1.getPosition() &&
					Avx0.getNormal() == Bvx0.getNormal() &&
					Avx1.getNormal() == Bvx1.getNormal() &&
					(
						Avx0.getTexCoord(channel) != Bvx0.getTexCoord(channel) ||
						Avx1.getTexCoord(channel) != Bvx1.getTexCoord(channel)
					)
				)
				{
					Vector2 imageSize(lightmap->getWidth() - 1, lightmap->getHeight() - 1);

					Vector4 Ap0 = renderModel->getPosition(Avx0.getPosition());
					Vector4 Ap1 = renderModel->getPosition(Avx1.getPosition());
					Vector4 An0 = renderModel->getNormal(Avx0.getNormal());
					Vector4 An1 = renderModel->getNormal(Avx1.getNormal());
					Vector2 Auv0 = renderModel->getTexCoord(Avx0.getTexCoord(channel)) * imageSize;
					Vector2 Auv1 = renderModel->getTexCoord(Avx1.getTexCoord(channel)) * imageSize;

					Vector4 Bp0 = renderModel->getPosition(Bvx0.getPosition());
					Vector4 Bp1 = renderModel->getPosition(Bvx1.getPosition());
					Vector4 Bn0 = renderModel->getNormal(Bvx0.getNormal());
					Vector4 Bn1 = renderModel->getNormal(Bvx1.getNormal());
					Vector2 Buv0 = renderModel->getTexCoord(Bvx0.getTexCoord(channel)) * imageSize;
					Vector2 Buv1 = renderModel->getTexCoord(Bvx1.getTexCoord(channel)) * imageSize;

					float Auvln = (Auv1 - Auv0).length();
					float Buvln = (Buv1 - Buv0).length();

					if (Auvln >= Buvln)
					{
						line(Auv0, Auv1, [&](const Vector2& Auv, float fraction) {
							Vector2 Buv = lerp(Buv0, Buv1, fraction);

							int32_t Ax = (int32_t)(Auv.x);
							int32_t Ay = (int32_t)(Auv.y);
							int32_t Bx = (int32_t)(Buv.x);
							int32_t By = (int32_t)(Buv.y);

							Color4f Aclr, Bclr;
							if (lightmap->getPixel(Ax, Ay, Aclr) && lightmap->getPixel(Bx, By, Bclr))
							{
								lightmap->setPixel(Ax, Ay, Aclr * Scalar(0.75f) + Bclr * Scalar(0.25f));
								lightmap->setPixel(Bx, By, Aclr * Scalar(0.25f) + Bclr * Scalar(0.75f));
							}
						});
					}
					else
					{
						line(Buv0, Buv1, [&](const Vector2& Buv, float fraction) {
							Vector2 Auv = lerp(Auv0, Auv1, fraction);

							int32_t Ax = (int32_t)(Auv.x);
							int32_t Ay = (int32_t)(Auv.y);
							int32_t Bx = (int32_t)(Buv.x);
							int32_t By = (int32_t)(Buv.y);

							Color4f Aclr, Bclr;
							if (lightmap->getPixel(Ax, Ay, Aclr) && lightmap->getPixel(Bx, By, Bclr))
							{
								lightmap->setPixel(Ax, Ay, Aclr * Scalar(0.75f) + Bclr * Scalar(0.25f));
								lightmap->setPixel(Bx, By, Aclr * Scalar(0.25f) + Bclr * Scalar(0.75f));
							}
						});
					}
				}
			}
		}

		// Discard alpha.
		lightmap->clearAlpha(1.0f);

		// Create final output instance.
		bool result = writeTexture(
			m_outputDatabase,
			m_compressionMethod,
			tracerOutput->getLightmapId(),
			tracerOutput->getName(),
			lightmap
		);
		if (!result)
		{
			log::error << L"Trace failed; unable to create output lightmap texture for \"" << tracerOutput->getName() << L"\", error = " << result << L"." << Endl;
			return false;
		}
	}

	// Trace irradiance grids.
	auto tracerIrradiances = task->getTracerIrradiances();
	for (uint32_t i = 0; !m_cancelled && i < tracerIrradiances.size(); ++i)
	{
		auto tracerIrradiance = tracerIrradiances[i];
		Guid irradianceGridId = tracerIrradiance->getIrradianceGridId();

		// Create output instance.
		Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
		Ref< db::Instance > outputInstance = m_outputDatabase->createInstance(
			L"Generated/" + tracerIrradiance->getIrradianceGridId().format(),
			db::CifReplaceExisting,
			&irradianceGridId
		);
		if (!outputInstance)
		{
			log::error << L"Trace failed; unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(outputResource);

		// Create output data stream.
		Ref< IStream > stream = outputInstance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Trace failed; unable to create texture data stream." << Endl;
			outputInstance->revert();
			return false;
		}

		const Scalar gridDensity(configuration->getIrradianceGridDensity());

		// Determine bounding box from all trace models if noone is already provided.
		Aabb3 boundingBox = tracerIrradiance->getBoundingBox();
		if (boundingBox.empty())
		{
			for (auto tracerModel : task->getTracerModels())
				boundingBox.contain(tracerModel->getModel()->getBoundingBox().transform(tracerModel->getTransform()));
			boundingBox.expand(gridDensity);
		}

		Vector4 worldSize = boundingBox.getExtent() * Scalar(2.0f);

		int32_t gridX = std::max((int32_t)(worldSize.x() * gridDensity + 0.5f), 2);
		int32_t gridY = std::max((int32_t)(worldSize.y() * gridDensity + 0.5f), 2);
		int32_t gridZ = std::max((int32_t)(worldSize.z() * gridDensity + 0.5f), 2);

		Writer writer(stream);

		writer << uint32_t(2);

		writer << (uint32_t)gridX;	// width
		writer << (uint32_t)gridY;	// height
		writer << (uint32_t)gridZ;	// depth

		writer << boundingBox.mn.x();
		writer << boundingBox.mn.y();
		writer << boundingBox.mn.z();
		writer << boundingBox.mx.x();
		writer << boundingBox.mx.y();
		writer << boundingBox.mx.z();

		m_status.description = str(L"Irradiance grid (%d, %d, %d)", gridX, gridY, gridZ);

		uint32_t progress = 0;
		for (int32_t x = 0; !m_cancelled && x < gridX; ++x)
		{
			float fx = x / (float)(gridX - 1.0f);
			for (int32_t y = 0; !m_cancelled && y < gridY; ++y)
			{
				float fy = y / (float)(gridY - 1.0f);
				for (int32_t z = 0; !m_cancelled && z < gridZ; ++z)
				{
					float fz = z / (float)(gridZ - 1.0f);

					m_status.current = progress++;
					m_status.total = gridX * gridY * gridZ;

					Vector4 position = boundingBox.mn + (boundingBox.mx - boundingBox.mn) * Vector4(fx, fy, fz);

					Ref< render::SHCoeffs > sh = rayTracer->traceProbe(position.xyz1());
					if (!sh)
					{
						log::error << L"Trace failed; unable to trace irradiance probe." << Endl;
						return false;
					}
					T_FATAL_ASSERT(sh->get().size() == 9);

					for (int32_t i = 0; i < 9; ++i)
					{
						auto c = (*sh)[i];
						writer << c.x();
						writer << c.y();
						writer << c.z();
					}
				}
			}
		}

		stream->close();

		if (!m_cancelled)
		{
			if (!outputInstance->commit())
			{
				log::error << L"Trace failed; unable to commit output instance." << Endl;
				return false;
			}
		}
		else
			outputInstance->revert();
	}

	return true;
}

	}
}
